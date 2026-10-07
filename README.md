# ParkEase — College Parking Management System

## Features

- Dashboard with live parking status
- Parking slot visualization (20 bike slots + 10 car slots)
- Park Vehicle with Student ID, Student Name, Phone Number, Vehicle Number and Vehicle Type
- Search Vehicle
- Parking History
- Revenue
- Printable Parking Bill after a vehicle is removed
- C backend connected through Node.js
- Data stored in binary `.dat` files

## Architecture

Browser
  ↓
HTML / CSS / JavaScript
  ↓
Node.js bridge (`frontend/server.js`)
  ↓
C API (`backend/api.c`)
  ↓
`data/vehicles_v2.dat` + `data/transactions_v2.dat`

## Local Setup on Windows

1. Open a terminal in the project folder.
2. Run `compile.bat`.
3. Run `start.bat`.
4. Open `http://localhost:3000`.

The website must be opened through Node.js. Do not open `index.html` directly.

## Parking Bill

When staff searches a parked vehicle and clicks **Remove Vehicle & Generate Bill**, the C backend calculates the parking fee and returns the completed transaction. ParkEase opens the Parking Bill page. The staff can click **Print Bill** and hand the printed bill to the customer.

## Render

The Node server reads the Render `PORT` environment variable and listens on `0.0.0.0`.

Build command:

`gcc backend/api.c -o backend/api && npm install`

Start command:

`node frontend/server.js`

The current file-based storage is suitable for a college demo, but a production system should use a persistent database.
