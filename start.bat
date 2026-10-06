@echo off
cd /d "%~dp0"
if not exist "backend\api.exe" (
    echo api.exe not found. Run compile.bat first.
    pause
    exit /b 1
)
cd frontend
echo Starting ParkEase...
node server.js
pause
