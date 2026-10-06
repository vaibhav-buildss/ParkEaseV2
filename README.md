# ParkEase — Full Connected Project

## Architecture

Browser
  ↓
HTML/CSS/JavaScript
  ↓
Node.js bridge (server.js)
  ↓
C API (api.exe)
  ↓
data/vehicles.dat + data/transactions.dat

The original C terminal program is also included as `backend/main.c`.

## Folder structure

ParkEase/
├── backend/
│   ├── main.c
│   ├── api.c
│   └── (generated) main.exe / api.exe
├── frontend/
│   ├── index.html
│   ├── style.css
│   ├── app.js
│   └── server.js
├── data/
└── README.md

## Setup on Windows

1. Open a terminal in `ParkEase/backend`.

2. Compile the normal C program:
   gcc main.c -o main.exe

3. Compile the web API:
   gcc api.c -o api.exe

4. Open another terminal in `ParkEase/frontend`.

5. Start the bridge:
   node server.js

6. Open:
   http://localhost:3000

## What is connected?

The website does NOT use browser LocalStorage for parking data.

Park Vehicle -> Node bridge -> api.exe -> C -> vehicles.dat

Remove Vehicle -> Node bridge -> api.exe -> C -> transactions.dat

Dashboard/History/Revenue -> Node bridge -> C -> .dat files

## Important

Run `server.js`, not `index.html` directly, when testing the connected version.

The `data` folder must exist. The `.dat` files are created automatically after the C API saves data.

Node.js and GCC must be installed and available in PATH.


## Logo & favicon

- `frontend/logo.svg` is used as the ParkEase logo.
- The same SVG is registered as the browser favicon in `frontend/index.html`.

## Deploy on Render

This project includes `render.yaml`. The important part is that Render runs Linux, so it compiles
`backend/api.c` into `backend/api` instead of using the Windows `api.exe`.

1. Push the complete project to GitHub.
2. In Render, choose **New + -> Web Service** and connect the GitHub repository.
3. Render can use the included `render.yaml`, or set:
   - Build Command: `gcc backend/api.c -o backend/api && npm install`
   - Start Command: `node frontend/server.js`
4. Deploy and open the generated Render URL.

Note: Render's filesystem is not a permanent database. The `.dat` files can reset when the service
is redeployed/restarted. This is fine for a college demo, but a production version should use a
real database.
