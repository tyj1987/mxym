<#
.SYNOPSIS
    MHGame System Stop Script

.DESCRIPTION
    Stops all running game servers and optionally the client.

.PARAMETER StopClient
    Also stop the game client

.EXAMPLE
    .\stop_system.ps1
    Stop all servers

.EXAMPLE
    .\stop_system.ps1 -StopClient
    Stop servers and client
#>

param(
    [switch]$StopClient
)

$ErrorActionPreference = "Continue"

function Write-Status { Write-Host "[*] $args" -ForegroundColor Cyan }
function Write-Success { Write-Host "[+] $args" -ForegroundColor Green }
function Write-Warning { Write-Host "[!] $args" -ForegroundColor Yellow }

Write-Host ""
Write-Host "========================================" -ForegroundColor White
Write-Host "  MHGame System Stop Script" -ForegroundColor White
Write-Host "========================================" -ForegroundColor White
Write-Host ""

# Server processes to stop
$servers = @(
    "DistributeServer",
    "MapServer",
    "AgentServer",
    "MonitoringServer"
)

foreach ($server in $servers) {
    $process = Get-Process -Name $server -ErrorAction SilentlyContinue
    if ($process) {
        Write-Status "Stopping $server..."
        try {
            $process | Stop-Process -Force
            Write-Success "$server stopped"
        } catch {
            Write-Warning "Failed to stop $server : $_"
        }
    } else {
        Write-Warning "$server not running"
    }
}

if ($StopClient) {
    $clientProcesses = @("MHClient-Connect", "MHClient", "MHExecuter")
    foreach ($client in $clientProcesses) {
        $process = Get-Process -Name $client -ErrorAction SilentlyContinue
        if ($process) {
            Write-Status "Stopping $client..."
            $process | Stop-Process -Force
            Write-Success "$client stopped"
        }
    }
}

Write-Host ""
Write-Success "System stopped"
Write-Host ""
