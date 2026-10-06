let appState = {
    stats: { totalSlots: 30, occupied: 0, available: 30, bikes: 0, cars: 0, transactions: 0, revenue: 0 },
    vehicles: [],
    transactions: []
};

async function api(action, params = {}) {
    const query = new URLSearchParams({ action, ...params });
    const response = await fetch(`/api?${query.toString()}`);
    const data = await response.json();

    if (!response.ok || data.ok === false) {
        throw new Error(data.error || "Request failed");
    }

    return data;
}

async function refresh() {
    try {
        const data = await api("state");
        appState = data;
        renderAll();
        setConnection(true);
    } catch (error) {
        setConnection(false);
        showToast(error.message, "error");
    }
}

function setConnection(online) {
    const dot = document.querySelector(".status-dot");
    const strong = document.querySelector(".sidebar-footer strong");
    if (!dot || !strong) return;

    dot.style.background = online ? "#28c98c" : "#e55353";
    dot.style.boxShadow = online ? "0 0 0 4px rgba(40,201,140,.12)" : "0 0 0 4px rgba(229,83,83,.12)";
    strong.textContent = online ? "C Backend Online" : "C Backend Offline";
}

function slotName(type, number) {
    return type === "Bike" ? `A${String(number).padStart(2, "0")}` : `B${String(number).padStart(2, "0")}`;
}

function formatDate(ts) {
    return new Date(Number(ts) * 1000).toLocaleDateString("en-IN", {
        day: "2-digit", month: "2-digit", year: "numeric"
    });
}

function formatTime(ts) {
    return new Date(Number(ts) * 1000).toLocaleTimeString("en-IN", {
        hour: "2-digit", minute: "2-digit"
    });
}

function rupees(n) {
    return `₹${Number(n || 0).toLocaleString("en-IN")}`;
}

function renderAll() {
    const s = appState.stats;
    document.getElementById("totalSlots").textContent = s.totalSlots;
    document.getElementById("availableSlots").textContent = s.available;
    document.getElementById("occupiedSlots").textContent = s.occupied;
    document.getElementById("totalRevenue").textContent = rupees(s.revenue);

    document.getElementById("bikeCount").textContent = s.bikes;
    document.getElementById("carCount").textContent = s.cars;
    document.getElementById("freeCount").textContent = s.available;

    const percent = Math.round((s.occupied / s.totalSlots) * 100);
    document.getElementById("occupancyPercent").textContent = `${percent}%`;
    const degrees = percent * 3.6;
    document.getElementById("occupancyRing").style.background =
        `conic-gradient(var(--accent) ${degrees}deg, #edf0f5 ${degrees}deg)`;

    document.getElementById("slotFreeSummary").textContent = s.available;

    renderActiveTable();
    renderSlots();
    renderHistory();
    renderRevenue();
}

function renderActiveTable() {
    const body = document.getElementById("activeTable");

    if (!appState.vehicles.length) {
        body.innerHTML = `<tr><td colspan="6" class="empty">No vehicles are currently parked.</td></tr>`;
        return;
    }

    body.innerHTML = appState.vehicles.map(v => `
        <tr>
            <td><strong>${escapeHtml(v.number)}</strong></td>
            <td><span class="badge ${v.type === "Bike" ? "blue" : "orange"}">${v.type}</span></td>
            <td>${escapeHtml(v.studentId)}</td>
            <td><strong>${v.slot}</strong></td>
            <td>${formatTime(v.entryTime)}</td>
            <td><span class="badge green">Parked</span></td>
        </tr>
    `).join("");
}

function renderSlots() {
    const occupied = new Set(appState.vehicles.map(v => v.slot));

    document.getElementById("bikeSlots").innerHTML = Array.from({ length: 20 }, (_, i) => {
        const name = slotName("Bike", i + 1);
        const busy = occupied.has(name);
        return `<div class="slot ${busy ? "occupied" : ""}">
            ${name}<small>${busy ? "Occupied" : "Available"}</small>
        </div>`;
    }).join("");

    document.getElementById("carSlots").innerHTML = Array.from({ length: 10 }, (_, i) => {
        const name = slotName("Car", i + 1);
        const busy = occupied.has(name);
        return `<div class="slot ${busy ? "occupied" : ""}">
            ${name}<small>${busy ? "Occupied" : "Available"}</small>
        </div>`;
    }).join("");
}

function renderHistory() {
    const body = document.getElementById("historyTable");
    document.getElementById("historyRevenue").textContent = rupees(appState.stats.revenue);

    if (!appState.transactions.length) {
        body.innerHTML = `<tr><td colspan="7" class="empty">No completed parking transactions yet.</td></tr>`;
        return;
    }

    body.innerHTML = [...appState.transactions].reverse().map((t, index) => `
        <tr>
            <td>${appState.transactions.length - index}</td>
            <td><strong>${escapeHtml(t.number)}</strong></td>
            <td><span class="badge ${t.type === "Bike" ? "blue" : "orange"}">${t.type}</span></td>
            <td>${t.slot}</td>
            <td>${Number(t.hours).toFixed(2)} hrs</td>
            <td><strong>${rupees(t.fee)}</strong></td>
            <td>${formatDate(t.exitTime)}</td>
        </tr>
    `).join("");
}

function renderRevenue() {
    const transactions = appState.transactions;
    const revenue = appState.stats.revenue;
    const count = transactions.length;
    const avg = count ? Math.round(revenue / count) : 0;
    const bikes = transactions.filter(t => t.type === "Bike").length;
    const cars = transactions.filter(t => t.type === "Car").length;

    document.getElementById("revenueBig").textContent = rupees(revenue);
    document.getElementById("transactionCount").textContent =
        `${count} completed transaction${count === 1 ? "" : "s"}`;
    document.getElementById("completedCount").textContent = count;
    document.getElementById("averageFee").textContent = rupees(avg);
    document.getElementById("bikeTransactions").textContent = bikes;
    document.getElementById("carTransactions").textContent = cars;

    const body = document.getElementById("revenueTable");

    if (!count) {
        body.innerHTML = `<tr><td colspan="5" class="empty">No revenue transactions yet.</td></tr>`;
        return;
    }

    body.innerHTML = [...transactions].reverse().slice(0, 8).map(t => `
        <tr>
            <td><strong>${escapeHtml(t.number)}</strong></td>
            <td>${t.type}</td>
            <td>${t.slot}</td>
            <td>${Number(t.hours).toFixed(2)}</td>
            <td><strong>${rupees(t.fee)}</strong></td>
        </tr>
    `).join("");
}

async function parkVehicle(event) {
    event.preventDefault();

    const studentId = document.getElementById("studentId").value.trim();
    const number = document.getElementById("vehicleNumber").value.trim().toUpperCase();
    const type = document.getElementById("vehicleType").value;

    if (!studentId || !number || !type) {
        showToast("Please fill all fields.", "error");
        return;
    }

    try {
        const result = await api("park", { studentId, number, type });
        await refresh();
        event.target.reset();
        showToast(`${number} assigned to slot ${result.slot}.`, "success");
        navigate("dashboard");
    } catch (error) {
        showToast(error.message, "error");
    }
}

async function removeVehicle(number) {
    try {
        const result = await api("remove", { number });
        await refresh();
        showToast(`${number} removed. Fee: ${rupees(result.fee)}.`, "success");
        navigate("dashboard");
    } catch (error) {
        showToast(error.message, "error");
    }
}

async function searchVehicle() {
    const number = document.getElementById("searchInput").value.trim().toUpperCase();
    const resultBox = document.getElementById("searchResult");

    if (!number) {
        resultBox.innerHTML = "";
        return;
    }

    try {
        const data = await api("search", { number });

        if (!data.found) {
            resultBox.innerHTML = `<div class="search-result-card">
                <h4>Vehicle not found</h4>
                <p style="color:var(--muted);font-size:12px">
                    No active vehicle matches <strong>${escapeHtml(number)}</strong>.
                </p>
            </div>`;
            return;
        }

        const v = data.vehicle;

        resultBox.innerHTML = `
            <div class="search-result-card">
                <h4>Vehicle Found <span class="badge green">Currently Parked</span></h4>
                <div class="result-grid">
                    <div class="result-item"><span>Vehicle Number</span><strong>${escapeHtml(v.number)}</strong></div>
                    <div class="result-item"><span>Vehicle Type</span><strong>${v.type}</strong></div>
                    <div class="result-item"><span>Student ID</span><strong>${escapeHtml(v.studentId)}</strong></div>
                    <div class="result-item"><span>Parking Slot</span><strong>${v.slot}</strong></div>
                    <div class="result-item"><span>Entry Date</span><strong>${formatDate(v.entryTime)}</strong></div>
                    <div class="result-item"><span>Entry Time</span><strong>${formatTime(v.entryTime)}</strong></div>
                </div>
                <button class="primary-btn" style="margin-top:16px"
                    onclick="removeVehicle('${escapeHtml(v.number)}')">
                    Remove Vehicle & Generate Bill
                </button>
            </div>`;
    } catch (error) {
        showToast(error.message, "error");
    }
}

function navigate(sectionId) {
    document.querySelectorAll(".section").forEach(s => s.classList.remove("active"));
    document.querySelectorAll(".nav-item").forEach(n => n.classList.remove("active"));

    document.getElementById(sectionId)?.classList.add("active");
    document.querySelector(`.nav-item[data-section="${sectionId}"]`)?.classList.add("active");

    const titles = {
        dashboard: "Dashboard",
        slots: "Parking Slots",
        park: "Park Vehicle",
        search: "Search Vehicle",
        history: "Parking History",
        revenue: "Revenue"
    };

    document.getElementById("pageTitle").textContent = titles[sectionId] || "Dashboard";
    document.querySelector(".sidebar").classList.remove("open");
    window.scrollTo({ top: 0, behavior: "smooth" });
}

function showToast(message, type = "success") {
    const toast = document.getElementById("toast");
    toast.textContent = message;
    toast.className = `toast show ${type}`;
    clearTimeout(window.toastTimer);
    window.toastTimer = setTimeout(() => toast.className = "toast", 2800);
}

function escapeHtml(value) {
    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

function updateClock() {
    document.getElementById("clock").textContent =
        new Date().toLocaleTimeString("en-IN", {
            hour: "2-digit", minute: "2-digit", second: "2-digit"
        });
}

document.querySelectorAll(".nav-item").forEach(button => {
    button.addEventListener("click", () => navigate(button.dataset.section));
});

document.querySelectorAll("[data-go]").forEach(button => {
    button.addEventListener("click", () => navigate(button.dataset.go));
});

document.getElementById("parkForm").addEventListener("submit", parkVehicle);
document.getElementById("searchBtn").addEventListener("click", searchVehicle);

document.getElementById("searchInput").addEventListener("keydown", event => {
    if (event.key === "Enter") searchVehicle();
});

document.getElementById("mobileMenu").addEventListener("click", () => {
    document.querySelector(".sidebar").classList.toggle("open");
});

updateClock();
setInterval(updateClock, 1000);
refresh();
setInterval(refresh, 5000);
