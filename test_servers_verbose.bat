@echo off
chcp 65001 >nul
echo ========================================
echo MHGame Server Detailed Test
echo ========================================
echo.

cd /d D:\mxym\Server

:: Clear old logs
if exist "Log\*.txt" del "Log\*.txt"
if exist "MS_Debug.txt" del "MS_Debug.txt"
if exist "myeasylog.log" del "myeasylog.log"

echo Starting MonitoringServer...
start "MonitoringServer" MonitoringServer.exe 1

echo Waiting 5 seconds...
ping 127.0.0.1 -n 6 >nul

echo.
echo === Checking processes ===
tasklist | findstr "MonitoringServer"
echo.

echo === Checking ports ===
netstat -an | findstr "20001"
echo.

echo === Checking logs ===
if exist "MS_Debug.txt" (
    echo MS_Debug.txt content:
    type "MS_Debug.txt"
) else (
    echo MS_Debug.txt not created
)
echo.

if exist "Log\debug.txt" (
    echo Log\debug.txt content:
    type "Log\debug.txt"
) else (
    echo Log\debug.txt not created
)
echo.

echo === Now starting AgentServer ===
start "AgentServer" AgentServer.exe

echo Waiting 3 seconds...
ping 127.0.0.1 -n 4 >nul

echo.
echo === Checking processes ===
tasklist | findstr "Server"
echo.

echo === Checking ports ===
netstat -an | findstr "17001 20001"
echo.

echo.
echo ========================================
echo Test complete. Press any key to stop...
pause >nul

taskkill /F /IM MonitoringServer.exe 2>nul
taskkill /F /IM AgentServer.exe 2>nul
taskkill /F /IM MapServer.exe 2>nul

echo Servers stopped.
