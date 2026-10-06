#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BIKE_SLOTS 20
#define CAR_SLOTS 10
#define MAX_VEHICLES 30
#define MAX_TRANSACTIONS 1000

#define DATA_FILE "../data/vehicles.dat"
#define TRANSACTION_FILE "../data/transactions.dat"

struct Vehicle {
    int studentId;
    char vehicleNumber[20];
    char vehicleType[10];
    int slotNumber;
    int active;
    time_t entryTime;
};

struct Transaction {
    int studentId;
    char vehicleNumber[20];
    char vehicleType[10];
    int slotNumber;
    time_t entryTime;
    time_t exitTime;
    double parkingHours;
    int fee;
};

struct Vehicle vehicles[MAX_VEHICLES];
struct Transaction transactions[MAX_TRANSACTIONS];
int vehicleCount = 0;
int transactionCount = 0;

void loadData() {
    FILE *f = fopen(DATA_FILE, "rb");
    if (!f) return;
    fread(&vehicleCount, sizeof(int), 1, f);
    if (vehicleCount < 0 || vehicleCount > MAX_VEHICLES) vehicleCount = 0;
    fread(vehicles, sizeof(struct Vehicle), vehicleCount, f);
    fclose(f);
}

void saveData() {
    FILE *f = fopen(DATA_FILE, "wb");
    if (!f) return;
    fwrite(&vehicleCount, sizeof(int), 1, f);
    fwrite(vehicles, sizeof(struct Vehicle), vehicleCount, f);
    fclose(f);
}

void loadTransactions() {
    FILE *f = fopen(TRANSACTION_FILE, "rb");
    if (!f) return;
    fread(&transactionCount, sizeof(int), 1, f);
    if (transactionCount < 0 || transactionCount > MAX_TRANSACTIONS) transactionCount = 0;
    fread(transactions, sizeof(struct Transaction), transactionCount, f);
    fclose(f);
}

void saveTransactions() {
    FILE *f = fopen(TRANSACTION_FILE, "wb");
    if (!f) return;
    fwrite(&transactionCount, sizeof(int), 1, f);
    fwrite(transactions, sizeof(struct Transaction), transactionCount, f);
    fclose(f);
}

int feeFor(const char *type, double hours) {
    int h = (int)hours;
    if (hours > h) h++;
    if (h < 1) h = 1;
    if (strcmp(type, "Bike") == 0)
        return h <= 1 ? 10 : 10 + (h - 1) * 5;
    return h <= 1 ? 20 : 20 + (h - 1) * 10;
}

void jsonEscape(const char *s) {
    putchar('"');
    for (; *s; s++) {
        if (*s == '"' || *s == '\\') putchar('\\');
        if (*s == '\n') { fputs("\\n", stdout); continue; }
        if (*s == '\r') { fputs("\\r", stdout); continue; }
        putchar(*s);
    }
    putchar('"');
}

void outputError(const char *message) {
    printf("{\"ok\":false,\"error\":");
    jsonEscape(message);
    printf("}\n");
}

void outputOk() {
    printf("{\"ok\":true}\n");
}

void state() {
    int occupied = 0, bikes = 0, cars = 0, revenue = 0;

    for (int i = 0; i < vehicleCount; i++) {
        if (!vehicles[i].active) continue;
        occupied++;
        if (strcmp(vehicles[i].vehicleType, "Bike") == 0) bikes++;
        else if (strcmp(vehicles[i].vehicleType, "Car") == 0) cars++;
    }

    for (int i = 0; i < transactionCount; i++)
        revenue += transactions[i].fee;

    printf("{\"ok\":true,\"stats\":{\"totalSlots\":30,\"occupied\":%d,\"available\":%d,\"bikes\":%d,\"cars\":%d,\"transactions\":%d,\"revenue\":%d},\"vehicles\":[",
           occupied, 30 - occupied, bikes, cars, transactionCount, revenue);

    int first = 1;
    for (int i = 0; i < vehicleCount; i++) {
        if (!vehicles[i].active) continue;
        if (!first) putchar(',');
        first = 0;
        printf("{\"studentId\":%d,\"number\":", vehicles[i].studentId);
        jsonEscape(vehicles[i].vehicleNumber);
        printf(",\"type\":");
        jsonEscape(vehicles[i].vehicleType);
        printf(",\"slot\":");
        char slot[8];
        snprintf(slot, sizeof(slot), "%s%02d",
                 strcmp(vehicles[i].vehicleType, "Bike") == 0 ? "A" : "B",
                 vehicles[i].slotNumber);
        jsonEscape(slot);
        printf(",\"entryTime\":%lld}", (long long)vehicles[i].entryTime);
    }
    printf("],\"transactions\":[");

    first = 1;
    for (int i = 0; i < transactionCount; i++) {
        if (!first) putchar(',');
        first = 0;
        printf("{\"studentId\":%d,\"number\":", transactions[i].studentId);
        jsonEscape(transactions[i].vehicleNumber);
        printf(",\"type\":");
        jsonEscape(transactions[i].vehicleType);
        printf(",\"slot\":");
        char slot[8];
        snprintf(slot, sizeof(slot), "%s%02d",
                 strcmp(transactions[i].vehicleType, "Bike") == 0 ? "A" : "B",
                 transactions[i].slotNumber);
        jsonEscape(slot);
        printf(",\"entryTime\":%lld,\"exitTime\":%lld,\"hours\":%.4f,\"fee\":%d}",
               (long long)transactions[i].entryTime,
               (long long)transactions[i].exitTime,
               transactions[i].parkingHours,
               transactions[i].fee);
    }
    printf("]}\n");
}

void park(const char *studentStr, const char *numberRaw, const char *typeRaw) {
    if (vehicleCount >= MAX_VEHICLES) {
        outputError("Parking is full.");
        return;
    }

    int studentId = atoi(studentStr);
    char number[20], type[10];
    snprintf(number, sizeof(number), "%s", numberRaw);
    snprintf(type, sizeof(type), "%s", typeRaw);

    if (strcmp(type, "bike") == 0 || strcmp(type, "BIKE") == 0) strcpy(type, "Bike");
    if (strcmp(type, "car") == 0 || strcmp(type, "CAR") == 0) strcpy(type, "Car");

    if (strcmp(type, "Bike") != 0 && strcmp(type, "Car") != 0) {
        outputError("Vehicle type must be Bike or Car.");
        return;
    }

    for (int i = 0; i < vehicleCount; i++) {
        if (vehicles[i].active && strcmp(vehicles[i].vehicleNumber, number) == 0) {
            outputError("This vehicle is already parked.");
            return;
        }
    }

    int max = strcmp(type, "Bike") == 0 ? BIKE_SLOTS : CAR_SLOTS;
    int used[20] = {0};

    for (int i = 0; i < vehicleCount; i++) {
        if (vehicles[i].active && strcmp(vehicles[i].vehicleType, type) == 0 &&
            vehicles[i].slotNumber >= 1 && vehicles[i].slotNumber <= max)
            used[vehicles[i].slotNumber - 1] = 1;
    }

    int slot = -1;
    for (int i = 0; i < max; i++) {
        if (!used[i]) { slot = i + 1; break; }
    }

    if (slot == -1) {
        outputError("No available slot for this vehicle type.");
        return;
    }

    vehicles[vehicleCount].studentId = studentId;
    strcpy(vehicles[vehicleCount].vehicleNumber, number);
    strcpy(vehicles[vehicleCount].vehicleType, type);
    vehicles[vehicleCount].slotNumber = slot;
    vehicles[vehicleCount].active = 1;
    vehicles[vehicleCount].entryTime = time(NULL);
    vehicleCount++;

    saveData();

    char slotName[8];
    snprintf(slotName, sizeof(slotName), "%s%02d",
             strcmp(type, "Bike") == 0 ? "A" : "B", slot);

    printf("{\"ok\":true,\"message\":\"Vehicle parked successfully.\",\"slot\":");
    jsonEscape(slotName);
    printf("}\n");
}

void removeVehicle(const char *number) {
    for (int i = 0; i < vehicleCount; i++) {
        if (vehicles[i].active && strcmp(vehicles[i].vehicleNumber, number) == 0) {
            time_t exitTime = time(NULL);
            double hours = difftime(exitTime, vehicles[i].entryTime) / 3600.0;
            int fee = feeFor(vehicles[i].vehicleType, hours);

            if (transactionCount >= MAX_TRANSACTIONS) {
                outputError("Transaction history is full.");
                return;
            }

            transactions[transactionCount].studentId = vehicles[i].studentId;
            strcpy(transactions[transactionCount].vehicleNumber, vehicles[i].vehicleNumber);
            strcpy(transactions[transactionCount].vehicleType, vehicles[i].vehicleType);
            transactions[transactionCount].slotNumber = vehicles[i].slotNumber;
            transactions[transactionCount].entryTime = vehicles[i].entryTime;
            transactions[transactionCount].exitTime = exitTime;
            transactions[transactionCount].parkingHours = hours;
            transactions[transactionCount].fee = fee;
            transactionCount++;

            vehicles[i].active = 0;

            saveTransactions();
            saveData();

            printf("{\"ok\":true,\"message\":\"Vehicle removed successfully.\",\"fee\":%d,\"hours\":%.4f}\n", fee, hours);
            return;
        }
    }

    outputError("Vehicle not found.");
}

void search(const char *number) {
    for (int i = 0; i < vehicleCount; i++) {
        if (vehicles[i].active && strcmp(vehicles[i].vehicleNumber, number) == 0) {
            char slot[8];
            snprintf(slot, sizeof(slot), "%s%02d",
                     strcmp(vehicles[i].vehicleType, "Bike") == 0 ? "A" : "B",
                     vehicles[i].slotNumber);

            printf("{\"ok\":true,\"found\":true,\"vehicle\":{\"studentId\":%d,\"number\":",
                   vehicles[i].studentId);
            jsonEscape(vehicles[i].vehicleNumber);
            printf(",\"type\":");
            jsonEscape(vehicles[i].vehicleType);
            printf(",\"slot\":");
            jsonEscape(slot);
            printf(",\"entryTime\":%lld}}\n", (long long)vehicles[i].entryTime);
            return;
        }
    }
    printf("{\"ok\":true,\"found\":false}\n");
}

int main(int argc, char *argv[]) {
    loadData();
    loadTransactions();

    if (argc < 2) {
        outputError("No API action supplied.");
        return 1;
    }

    if (strcmp(argv[1], "state") == 0) state();
    else if (strcmp(argv[1], "park") == 0 && argc >= 5) park(argv[2], argv[3], argv[4]);
    else if (strcmp(argv[1], "remove") == 0 && argc >= 3) removeVehicle(argv[2]);
    else if (strcmp(argv[1], "search") == 0 && argc >= 3) search(argv[2]);
    else {
        outputError("Invalid API request.");
        return 1;
    }

    return 0;
}
