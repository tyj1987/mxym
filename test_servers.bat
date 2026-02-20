@echo off
chcp 65001 >nul
echo ========================================
echo MHGame Server Test Script
echo ========================================
echo.

cd /d D:\mxym\Server

echo [1/5] Checking files...
if not exist "MonitoringServer.exe" (
    echo [ERROR] MonitoringServer.exe not found!
    pause
    exit /b 1
)
if not exist "AgentServer.exe" (
    echo [ERROR] AgentServer.exe not found!
    pause
    exit /b 1
)
if not exist "MapServer.exe" (
    echo [ERROR] MapServer.exe not found!
    pause
    exit /b 1
)
echo [OK] All server files found
echo.

echo [2/5] Checking DLLs...
if not exist "BaseNetwork.dll" (
    echo [ERROR] BaseNetwork.dll not found!
    pause
    exit /b 1
)
if not exist "DBThread.dll" (
    echo [ERROR] DBThread.dll not found!
    pause
    exit /b 1
)
echo [OK] Required DLLs found
echo.

echo [3/5] Checking configuration...
if not exist "masInfo.ini" (
    echo [ERROR] masInfo.ini not found!
    pause
    exit /b 1
)
if not exist "asInfo.ini" (
    echo [ERROR] asInfo.ini not found!
    pause
    exit /b 1
)
echo [OK] Configuration files found
echo.

echo [4/5] Checking database connection...
echo [INFO] Database: 127.0.0.1:1433
echo [INFO] Database: MHCMEMBER
echo [INFO] User: mhgame
echo [WARNING] Make sure SQL Server is running!
echo.

echo [5/5] Starting servers...
echo.
echo Starting MonitoringServer...
start "MonitoringServer" MonitoringServer.exe 1
timeout /t 3 /nobreak >nul

echo Starting AgentServer...
start "AgentServer" AgentServer.exe
timeout /t 2 /nobreak >nul

echo Starting MapServer...
start "MapServer" MapServer.exe
timeout /t 2 /nobreak >nul

echo.
echo ========================================
echo Servers started!
echo ========================================
echo.
echo Checking processes...
tasklist | findstr "Server"
echo.

echo Checking ports...
netstat -an | findstr "LISTENING" | findstr "17001 18001 20001"
echo.

echo ========================================
echo Press any key to stop servers...
pause >nul

echo Stopping servers...
taskkill /F /IM MonitoringServer.exe 2>nul
taskkill /F /IM AgentServer.exe 2>nul
taskkill /F /IM MapServer.exe 2>nul
taskkill /F /IM DistributeServer.exe 2>nul

echo.
echo Servers stopped.
pause
