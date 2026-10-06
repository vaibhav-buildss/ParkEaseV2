@echo off
cd /d "%~dp0backend"
echo Compiling ParkEase C backend...
gcc main.c -o main.exe
gcc api.c -o api.exe
if errorlevel 1 (
    echo.
    echo Compilation failed.
    pause
    exit /b 1
)
echo.
echo Backend compiled successfully.
echo main.exe and api.exe are ready.
pause
