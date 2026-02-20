@echo off
chcp 65001 >nul
echo ========================================
echo MHGame Server Long Test
echo ========================================
echo.

cd /d D:\mxym\Server

:: Kill any existing servers
taskkill /F /IM MonitoringServer.exe 2>nul
taskkill /F /IM AgentServer.exe 2>nul
taskkill /F /IM MapServer.exe 2>nul

echo Starting MonitoringServer...
start "MonitoringServer" MonitoringServer.exe 1

echo Waiting 10 seconds for server to initialize...
ping 127.0.0.1 -n 11 >nul

echo.
echo === Process Check ===
tasklist | findstr "Server"
echo.

echo === Port Check ===
netstat -an | findstr "LISTENING" | findstr "20001"
echo.

echo === Logs ===
if exist "MS_Debug.txt" (
    type "MS_Debug.txt"
)
echo.

echo Starting AgentServer...
start "AgentServer" AgentServer.exe

echo Waiting 10 seconds...
ping 127.0.0.1 -n 11 >nul

echo.
echo === Process Check ===
tasklist | findstr "Server"
echo.

echo === Port Check ===
netstat -an | findstr "LISTENING" | findstr "17001 20001"
echo.

echo Starting MapServer...
start "MapServer" MapServer.exe

echo Waiting 10 seconds...
ping 127.0.0.1 -n 11 >nul

echo.
echo ========================================
echo Final Status
echo ========================================
echo.

echo === All Server Processes ===
tasklist | findstr "Server"
echo.

echo === All Listening Ports ===
netstat -an | findstr "LISTENING" | findstr "17001 18001 20001"
echo.

echo ========================================
echo Servers are running. Press any key to stop...
pause >nul

taskkill /F /IM MonitoringServer.exe 2>nul
taskkill /F /IM AgentServer.exe 2>nul
taskkill /F /IM MapServer.exe 2>nul

echo Servers stopped.
pause
