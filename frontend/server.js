const http = require("http");
const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");
const { URL } = require("url");

const ROOT = __dirname;
const BACKEND = path.join(ROOT, "..", "backend");
const API_EXE = path.join(
    BACKEND,
    process.platform === "win32" ? "api.exe" : "api"
);

const PORT = Number(process.env.PORT) || 3000;

const mime = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".svg": "image/svg+xml",
    ".ico": "image/x-icon"
};

function runApi(args) {
    return new Promise((resolve, reject) => {
        if (!fs.existsSync(API_EXE)) {
            reject(new Error(
                `${path.basename(API_EXE)} not found. Compile backend/api.c first.`
            ));
            return;
        }

        const child = spawn(API_EXE, args, {
            cwd: BACKEND,
            windowsHide: true
        });

        let stdout = "";
        let stderr = "";

        child.stdout.on("data", chunk => stdout += chunk.toString());
        child.stderr.on("data", chunk => stderr += chunk.toString());

        child.on("error", reject);

        child.on("close", code => {
            if (code !== 0) {
                reject(new Error(stderr || `C API exited with code ${code}`));
                return;
            }

            try {
                resolve(JSON.parse(stdout.trim()));
            } catch {
                reject(new Error("C API returned invalid JSON: " + stdout));
            }
        });
    });
}

function sendJson(res, status, data) {
    res.writeHead(status, {
        "Content-Type": "application/json; charset=utf-8",
        "Cache-Control": "no-store"
    });
    res.end(JSON.stringify(data));
}

async function handleApi(url, res) {
    try {
        const action = url.searchParams.get("action");

        if (action === "state") {
            return sendJson(res, 200, await runApi(["state"]));
        }

        if (action === "park") {
            const studentId = url.searchParams.get("studentId") || "";
            const number = (url.searchParams.get("number") || "").toUpperCase();
            const name = url.searchParams.get("name") || "";
            const phone = url.searchParams.get("phone") || "";
            const type = url.searchParams.get("type") || "";
            return sendJson(res, 200, await runApi(["park", studentId, name, phone, number, type]));
        }

        if (action === "remove") {
            const number = (url.searchParams.get("number") || "").toUpperCase();
            return sendJson(res, 200, await runApi(["remove", number]));
        }

        if (action === "search") {
            const number = (url.searchParams.get("number") || "").toUpperCase();
            return sendJson(res, 200, await runApi(["search", number]));
        }

        return sendJson(res, 400, { ok: false, error: "Unknown API action." });
    } catch (err) {
        return sendJson(res, 500, { ok: false, error: err.message });
    }
}

function serveFile(urlPath, res) {
    let requested = urlPath === "/" ? "/index.html" : urlPath;
    requested = decodeURIComponent(requested);

    const filePath = path.normalize(path.join(ROOT, requested));

    if (!filePath.startsWith(ROOT)) {
        res.writeHead(403);
        res.end("Forbidden");
        return;
    }

    fs.readFile(filePath, (err, data) => {
        if (err) {
            res.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
            res.end("Not found");
            return;
        }

        const ext = path.extname(filePath).toLowerCase();
        res.writeHead(200, { "Content-Type": mime[ext] || "application/octet-stream" });
        res.end(data);
    });
}

const server = http.createServer(async (req, res) => {
    const url = new URL(req.url, `http://localhost:${PORT}`);

    if (url.pathname === "/api") {
        await handleApi(url, res);
        return;
    }

    serveFile(url.pathname, res);
});

server.listen(PORT, "0.0.0.0", () => {
    console.log(`ParkEase running on port ${PORT}`);
    console.log("Frontend -> Node bridge -> C API -> .dat files");
});
