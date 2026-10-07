#include <stdio.h>
#include <string.h>
#include <time.h>

#define BIKE_SLOTS 20
#define CAR_SLOTS 10
#define MAX_VEHICLES 30
#define MAX_TRANSACTIONS 1000

#define DATA_FILE "../data/vehicles_v2.dat"
#define TRANSACTION_FILE "../data/transactions_v2.dat"

struct Vehicle {
    int studentId;
    char studentName[60];
    char phone[20];
    char vehicleNumber[20];
    char vehicleType[10];
    int slotNumber;
    int active;
    time_t entryTime;
};

struct ParkingSlot {
    int slotNumber;
    char vehicleType[10];
    int occupied;
};

struct Transaction {
    int studentId;
    char studentName[60];
    char phone[20];
    char vehicleNumber[20];
    char vehicleType[10];
    int slotNumber;
    time_t entryTime;
    time_t exitTime;
    double parkingHours;
    int fee;
};

struct ParkingSlot bikeSlots[BIKE_SLOTS];
struct ParkingSlot carSlots[CAR_SLOTS];
struct Vehicle vehicles[MAX_VEHICLES];
struct Transaction transactions[MAX_TRANSACTIONS];
int transactionCount = 0;

void initializeSlots() {
    for (int i = 0; i < BIKE_SLOTS; i++) {
        bikeSlots[i].slotNumber = i + 1;
        strcpy(bikeSlots[i].vehicleType, "Bike");
        bikeSlots[i].occupied = 0;
    }
    for (int i = 0; i < CAR_SLOTS; i++) {
        carSlots[i].slotNumber = i + 1;
        strcpy(carSlots[i].vehicleType, "Car");
        carSlots[i].occupied = 0;
    }
}

void initializeVehicles(int *vehicleCount) {
    *vehicleCount = 0;
    for (int i = 0; i < MAX_VEHICLES; i++) vehicles[i].active = 0;
}

int calculateFee(char vehicleType[], double hours) {
    int roundedHours = (int)hours;
    if (hours > roundedHours) roundedHours++;
    if (roundedHours < 1) roundedHours = 1;

    if (strcmp(vehicleType, "Bike") == 0) {
        return roundedHours <= 1 ? 10 : 10 + (roundedHours - 1) * 5;
    }
    return roundedHours <= 1 ? 20 : 20 + (roundedHours - 1) * 10;
}

void saveData(struct Vehicle vehicles[], int vehicleCount) {
    FILE *file = fopen(DATA_FILE, "wb");
    if (!file) {
        printf("\nError opening vehicle data file!\n");
        return;
    }
    fwrite(&vehicleCount, sizeof(int), 1, file);
    fwrite(vehicles, sizeof(struct Vehicle), vehicleCount, file);
    fclose(file);
}

void loadData(struct Vehicle vehicles[], int *vehicleCount) {
    FILE *file = fopen(DATA_FILE, "rb");
    if (!file) {
        *vehicleCount = 0;
        return;
    }
    fread(vehicleCount, sizeof(int), 1, file);
    if (*vehicleCount > MAX_VEHICLES) *vehicleCount = MAX_VEHICLES;
    fread(vehicles, sizeof(struct Vehicle), *vehicleCount, file);
    fclose(file);
}

void saveTransactions() {
    FILE *file = fopen(TRANSACTION_FILE, "wb");
    if (!file) {
        printf("\nError opening transaction file!\n");
        return;
    }
    fwrite(&transactionCount, sizeof(int), 1, file);
    fwrite(transactions, sizeof(struct Transaction), transactionCount, file);
    fclose(file);
}

void loadTransactions() {
    FILE *file = fopen(TRANSACTION_FILE, "rb");
    if (!file) {
        transactionCount = 0;
        return;
    }
    fread(&transactionCount, sizeof(int), 1, file);
    if (transactionCount > MAX_TRANSACTIONS) transactionCount = MAX_TRANSACTIONS;
    fread(transactions, sizeof(struct Transaction), transactionCount, file);
    fclose(file);
}

void rebuildSlotStatus(struct Vehicle vehicles[], int vehicleCount) {
    for (int i = 0; i < BIKE_SLOTS; i++) bikeSlots[i].occupied = 0;
    for (int i = 0; i < CAR_SLOTS; i++) carSlots[i].occupied = 0;

    for (int i = 0; i < vehicleCount; i++) {
        if (!vehicles[i].active) continue;

        if (strcmp(vehicles[i].vehicleType, "Bike") == 0 &&
            vehicles[i].slotNumber >= 1 && vehicles[i].slotNumber <= BIKE_SLOTS) {
            bikeSlots[vehicles[i].slotNumber - 1].occupied = 1;
        }

        if (strcmp(vehicles[i].vehicleType, "Car") == 0 &&
            vehicles[i].slotNumber >= 1 && vehicles[i].slotNumber <= CAR_SLOTS) {
            carSlots[vehicles[i].slotNumber - 1].occupied = 1;
        }
    }
}

void displaySlots() {
    printf("\n============================================\n");
    printf("           PARKEASE PARKING SLOTS\n");
    printf("============================================\n");

    printf("\nBIKE SLOTS\n--------------------------------------------\n");
    for (int i = 0; i < BIKE_SLOTS; i++)
        printf("A%02d : %s\n", bikeSlots[i].slotNumber,
               bikeSlots[i].occupied ? "OCCUPIED" : "AVAILABLE");

    printf("\nCAR SLOTS\n--------------------------------------------\n");
    for (int i = 0; i < CAR_SLOTS; i++)
        printf("B%02d : %s\n", carSlots[i].slotNumber,
               carSlots[i].occupied ? "OCCUPIED" : "AVAILABLE");

    printf("\n============================================\n");
}

void parkVehicle(struct Vehicle vehicles[], int *vehicleCount) {
    if (*vehicleCount >= MAX_VEHICLES) {
        printf("\nParking is full!\n");
        return;
    }

    int studentId, slotFound = 0, assignedSlot = -1;
    char studentName[60], phone[20];
    char vehicleNumber[20], vehicleType[10];

    printf("\n============================================\n");
    printf("              PARK VEHICLE\n");
    printf("============================================\n");

    printf("Enter Student ID: ");
    scanf("%d", &studentId);
    printf("Enter Student Name: ");
    scanf(" %59[^\n]", studentName);
    printf("Enter Phone Number: ");
    scanf("%19s", phone);
    printf("Enter Vehicle Number: ");
    scanf("%19s", vehicleNumber);
    printf("Enter Vehicle Type (Bike/Car): ");
    scanf("%9s", vehicleType);

    if (vehicleType[0] >= 'a' && vehicleType[0] <= 'z')
        vehicleType[0] = vehicleType[0] - 'a' + 'A';

    if (strcmp(vehicleType, "Bike") != 0 && strcmp(vehicleType, "Car") != 0) {
        printf("\nInvalid vehicle type!\n");
        return;
    }

    for (int i = 0; i < *vehicleCount; i++) {
        if (vehicles[i].active && strcmp(vehicles[i].vehicleNumber, vehicleNumber) == 0) {
            printf("\nThis vehicle is already parked!\n");
            return;
        }
    }

    if (strcmp(vehicleType, "Bike") == 0) {
        for (int i = 0; i < BIKE_SLOTS; i++) {
            if (!bikeSlots[i].occupied) {
                bikeSlots[i].occupied = 1;
                assignedSlot = bikeSlots[i].slotNumber;
                slotFound = 1;
                break;
            }
        }
    } else {
        for (int i = 0; i < CAR_SLOTS; i++) {
            if (!carSlots[i].occupied) {
                carSlots[i].occupied = 1;
                assignedSlot = carSlots[i].slotNumber;
                slotFound = 1;
                break;
            }
        }
    }

    if (!slotFound) {
        printf("\nNo available %s slot!\n", vehicleType);
        return;
    }

    vehicles[*vehicleCount].studentId = studentId;
    strcpy(vehicles[*vehicleCount].studentName, studentName);
    strcpy(vehicles[*vehicleCount].phone, phone);
    strcpy(vehicles[*vehicleCount].vehicleNumber, vehicleNumber);
    strcpy(vehicles[*vehicleCount].vehicleType, vehicleType);
    vehicles[*vehicleCount].slotNumber = assignedSlot;
    vehicles[*vehicleCount].active = 1;
    vehicles[*vehicleCount].entryTime = time(NULL);
    (*vehicleCount)++;

    saveData(vehicles, *vehicleCount);

    printf("\nVehicle parked successfully!\n");
    printf("Assigned Slot: %s%02d\n",
           strcmp(vehicleType, "Bike") == 0 ? "A" : "B", assignedSlot);
}

void removeVehicle(struct Vehicle vehicles[], int *vehicleCount) {
    char vehicleNumber[20];

    printf("\nEnter Vehicle Number: ");
    scanf("%19s", vehicleNumber);

    for (int i = 0; i < *vehicleCount; i++) {
        if (vehicles[i].active && strcmp(vehicles[i].vehicleNumber, vehicleNumber) == 0) {
            time_t exitTime = time(NULL);
            double hours = difftime(exitTime, vehicles[i].entryTime) / 3600.0;
            int fee = calculateFee(vehicles[i].vehicleType, hours);

            if (transactionCount < MAX_TRANSACTIONS) {
                transactions[transactionCount].studentId = vehicles[i].studentId;
                strcpy(transactions[transactionCount].studentName, vehicles[i].studentName);
                strcpy(transactions[transactionCount].phone, vehicles[i].phone);
                strcpy(transactions[transactionCount].vehicleNumber, vehicles[i].vehicleNumber);
                strcpy(transactions[transactionCount].vehicleType, vehicles[i].vehicleType);
                transactions[transactionCount].slotNumber = vehicles[i].slotNumber;
                transactions[transactionCount].entryTime = vehicles[i].entryTime;
                transactions[transactionCount].exitTime = exitTime;
                transactions[transactionCount].parkingHours = hours;
                transactions[transactionCount].fee = fee;
                transactionCount++;
                saveTransactions();
            }

            if (strcmp(vehicles[i].vehicleType, "Bike") == 0)
                bikeSlots[vehicles[i].slotNumber - 1].occupied = 0;
            else
                carSlots[vehicles[i].slotNumber - 1].occupied = 0;

            vehicles[i].active = 0;
            saveData(vehicles, *vehicleCount);

            printf("\n========== PARKING BILL ==========\n");
            printf("Student : %s (ID %d)\n", vehicles[i].studentName, vehicles[i].studentId);
            printf("Phone   : %s\n", vehicles[i].phone);
            printf("Vehicle : %s\n", vehicles[i].vehicleNumber);
            printf("Type    : %s\n", vehicles[i].vehicleType);
            printf("Slot    : %s%02d\n",
                   strcmp(vehicles[i].vehicleType, "Bike") == 0 ? "A" : "B",
                   vehicles[i].slotNumber);
            printf("Hours   : %.2f\n", hours);
            printf("Fee     : Rs.%d\n", fee);
            printf("==================================\n");
            return;
        }
    }

    printf("\nVehicle not found!\n");
}

void searchVehicle(struct Vehicle vehicles[], int vehicleCount) {
    char vehicleNumber[20];
    printf("\nEnter Vehicle Number: ");
    scanf("%19s", vehicleNumber);

    for (int i = 0; i < vehicleCount; i++) {
        if (vehicles[i].active && strcmp(vehicles[i].vehicleNumber, vehicleNumber) == 0) {
            printf("\nVehicle Found!\n");
            printf("Student ID : %d\n", vehicles[i].studentId);
            printf("Name       : %s\n", vehicles[i].studentName);
            printf("Phone      : %s\n", vehicles[i].phone);
            printf("Vehicle    : %s\n", vehicles[i].vehicleNumber);
            printf("Type       : %s\n", vehicles[i].vehicleType);
            printf("Slot       : %s%02d\n",
                   strcmp(vehicles[i].vehicleType, "Bike") == 0 ? "A" : "B",
                   vehicles[i].slotNumber);
            return;
        }
    }
    printf("\nVehicle not found!\n");
}

void showStatistics(struct Vehicle vehicles[], int vehicleCount) {
    int occupied = 0, bikes = 0, cars = 0;

    for (int i = 0; i < vehicleCount; i++) {
        if (vehicles[i].active) {
            occupied++;
            if (strcmp(vehicles[i].vehicleType, "Bike") == 0) bikes++;
            else if (strcmp(vehicles[i].vehicleType, "Car") == 0) cars++;
        }
    }

    printf("\n========== PARKEASE DASHBOARD ==========\n");
    printf("Total Slots     : %d\n", BIKE_SLOTS + CAR_SLOTS);
    printf("Occupied Slots  : %d\n", occupied);
    printf("Available Slots : %d\n", BIKE_SLOTS + CAR_SLOTS - occupied);
    printf("Bikes Parked    : %d\n", bikes);
    printf("Cars Parked     : %d\n", cars);
    printf("Occupancy Rate  : %.2f%%\n",
           ((double)occupied / (BIKE_SLOTS + CAR_SLOTS)) * 100);
    printf("Completed Trips : %d\n", transactionCount);
    printf("========================================\n");
}

void showParkingHistory() {
    int revenue = 0;

    printf("\n========== PARKING HISTORY ==========\n");
    if (transactionCount == 0) {
        printf("No parking history available.\n");
        return;
    }

    for (int i = 0; i < transactionCount; i++) {
        printf("#%d | %s | %s | %s%02d | %.2f hrs | Rs.%d\n",
               i + 1,
               transactions[i].vehicleNumber,
               transactions[i].vehicleType,
               strcmp(transactions[i].vehicleType, "Bike") == 0 ? "A" : "B",
               transactions[i].slotNumber,
               transactions[i].parkingHours,
               transactions[i].fee);
        revenue += transactions[i].fee;
    }
    printf("Total Revenue: Rs.%d\n", revenue);
    printf("====================================\n");
}

void showRevenue() {
    int revenue = 0;
    for (int i = 0; i < transactionCount; i++)
        revenue += transactions[i].fee;

    printf("\n========== REVENUE ==========\n");
    printf("Completed Transactions : %d\n", transactionCount);
    printf("Total Revenue          : Rs.%d\n", revenue);
    printf("=============================\n");
}

int main() {
    int vehicleCount = 0, choice;

    initializeSlots();
    initializeVehicles(&vehicleCount);
    loadData(vehicles, &vehicleCount);
    loadTransactions();
    rebuildSlotStatus(vehicles, vehicleCount);

    do {
        printf("\n============================================\n");
        printf("                 PARKEASE\n");
        printf("      College Parking Management System\n");
        printf("============================================\n");
        printf("1. Park Vehicle\n");
        printf("2. Remove Vehicle\n");
        printf("3. View Parking Slots\n");
        printf("4. Search Vehicle\n");
        printf("5. Dashboard Statistics\n");
        printf("6. Parking History\n");
        printf("7. Revenue\n");
        printf("8. Exit\n");
        printf("--------------------------------------------\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: parkVehicle(vehicles, &vehicleCount); break;
            case 2: removeVehicle(vehicles, &vehicleCount); break;
            case 3: displaySlots(); break;
            case 4: searchVehicle(vehicles, vehicleCount); break;
            case 5: showStatistics(vehicles, vehicleCount); break;
            case 6: showParkingHistory(); break;
            case 7: showRevenue(); break;
            case 8: printf("\nExiting ParkEase...\n"); break;
            default: printf("\nInvalid choice!\n");
        }
    } while (choice != 8);

    return 0;
}
