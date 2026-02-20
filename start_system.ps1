<#
.SYNOPSIS
    MHGame Complete System Startup Script

.DESCRIPTION
    This script handles the complete system startup including:
    - Environment checks
    - Database verification
    - Server startup in correct order
    - Client launch

.PARAMETER SkipServers
    Skip server startup (only launch client)

.PARAMETER SkipClient
    Skip client launch (only start servers)

.PARAMETER Verbose
    Show detailed output

.EXAMPLE
    .\start_system.ps1
    Start complete system

.EXAMPLE
    .\start_system.ps1 -SkipClient
    Start servers only
#>

param(
    [switch]$SkipServers,
    [switch]$SkipClient,
    [switch]$Verbose
)

$ErrorActionPreference = "Continue"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# ===============================
# Utility Functions
# ===============================

function Write-Status { Write-Host "[*] $args" -ForegroundColor Cyan }
function Write-Success { Write-Host "[+] $args" -ForegroundColor Green }
function Write-Warning { Write-Host "[!] $args" -ForegroundColor Yellow }
function Write-Error { Write-Host "[-] $args" -ForegroundColor Red }

function Wait-Seconds {
    param([int]$Seconds)
    Write-Status "Waiting $Seconds seconds..."
    Start-Sleep -Seconds $Seconds
}

function Test-Port {
    param([int]$Port)
    $listener = $null
    try {
        $listener = New-Object System.Net.Sockets.TcpListener([System.Net.IPAddress]::Loopback, $Port)
        $listener.Start()
        $listener.Stop()
        return $false  # Port is free
    } catch {
        return $true   # Port is in use
    }
}

function Start-ServerProcess {
    param(
        [string]$Name,
        [string]$ExePath,
        [string]$Args = "",
        [int]$WaitPort = 0
    )

    Write-Status "Starting $Name..."

    if (-not (Test-Path $ExePath)) {
        Write-Error "$Name executable not found: $ExePath"
        return $false
    }

    # Check if already running
    $process = Get-Process -Name $Name -ErrorAction SilentlyContinue
    if ($process) {
        Write-Warning "$Name is already running (PID: $($process.Id))"
        return $true
    }

    # Start the process
    try {
        $startInfo = New-Object System.Diagnostics.ProcessStartInfo
        $startInfo.FileName = $ExePath
        $startInfo.Arguments = $Args
        $startInfo.WorkingDirectory = Split-Path $ExePath
        $startInfo.UseShellExecute = $true
        $startInfo.WindowStyle = [System.Diagnostics.ProcessWindowStyle]::Normal

        $proc = [System.Diagnostics.Process]::Start($startInfo)
        Write-Success "$Name started (PID: $($proc.Id))"

        # Wait for port if specified
        if ($WaitPort -gt 0) {
            Write-Status "Waiting for $Name to listen on port $WaitPort..."
            $maxWait = 30
            $waited = 0
            while ($waited -lt $maxWait) {
                Start-Sleep -Seconds 1
                $waited++
                if (Test-Port $WaitPort) {
                    Write-Success "$Name is ready on port $WaitPort"
                    return $true
                }
            }
            Write-Warning "$Name may not be ready yet (timeout waiting for port $WaitPort)"
        }

        return $true
    } catch {
        Write-Error "Failed to start $Name : $_"
        return $false
    }
}

function Stop-ServerProcess {
    param([string]$Name)

    $process = Get-Process -Name $Name -ErrorAction SilentlyContinue
    if ($process) {
        Write-Status "Stopping $Name..."
        $process | Stop-Process -Force
        Write-Success "$Name stopped"
    }
}

# ===============================
# Environment Checks
# ===============================

function Test-Environment {
    Write-Status "Checking environment..."

    $issues = @()

    # Check Server directory
    $serverDir = Join-Path $ScriptDir "Server"
    if (-not (Test-Path $serverDir)) {
        $issues += "Server directory not found"
    }

    # Check server executables
    $servers = @("MonitoringServer.exe", "AgentServer.exe", "MapServer.exe")
    foreach ($server in $servers) {
        $path = Join-Path $serverDir $server
        if (-not (Test-Path $path)) {
            $issues += "Server executable not found: $server"
        }
    }

    # Check PlayDH directory
    $playDHDir = Join-Path $ScriptDir "PlayDH"
    if (-not (Test-Path $playDHDir)) {
        $issues += "PlayDH directory not found"
    }

    # Check client executable
    $clientExe = Join-Path $playDHDir "MHClient-Connect.exe"
    if (-not (Test-Path $clientExe)) {
        $issues += "Client executable not found: MHClient-Connect.exe"
    }

    # Check config files
    $masInfo = Join-Path $serverDir "masInfo.ini"
    if (-not (Test-Path $masInfo)) {
        $issues += "masInfo.ini not found"
    }

    if ($issues.Count -gt 0) {
        Write-Error "Environment check failed:"
        foreach ($issue in $issues) {
            Write-Error "  - $issue"
        }
        return $false
    }

    Write-Success "Environment check passed"
    return $true
}

function Test-Database {
    Write-Status "Checking database..."

    # Check SQLite database
    $sqliteDb = Join-Path $ScriptDir "Database\sqlite\mhgame.db"
    if (Test-Path $sqliteDb) {
        $size = (Get-Item $sqliteDb).Length
        Write-Success "SQLite database found ($size bytes)"
        return $true
    }

    Write-Warning "SQLite database not found"
    return $true  # Non-fatal, servers might use SQL Server
}

function Initialize-DevMode {
    Write-Status "Setting up development mode (no .pak files)..."

    # Create DeveloperWorking directory
    $devDir = Join-Path $ScriptDir "DeveloperWorking"
    $flagFile = Join-Path $devDir "_DONOTUSEENGINEPACK"

    if (-not (Test-Path $devDir)) {
        New-Item -ItemType Directory -Path $devDir -Force | Out-Null
    }

    if (-not (Test-Path $flagFile)) {
        New-Item -ItemType File -Path $flagFile -Force | Out-Null
        Write-Success "Created dev mode flag file"
    }

    # Create required data directories in PlayDH
    $playDHDir = Join-Path $ScriptDir "PlayDH"
    $dataDirs = @(
        "3DData\Character",
        "3DData\Monster",
        "3DData\Npc",
        "3DData\Pet",
        "3DData\Effect",
        "3DData\Map",
        "3DData\Titan",
        "Sound",
        "Resource",
        "BGM",
        "Ini"
    )

    foreach ($dir in $dataDirs) {
        $fullPath = Join-Path $playDHDir $dir
        if (-not (Test-Path $fullPath)) {
            New-Item -ItemType Directory -Path $fullPath -Force | Out-Null
            if ($Verbose) {
                Write-Status "Created directory: $dir"
            }
        }
    }

    Write-Success "Development mode configured"
}

# ===============================
# Server Management
# ===============================

function Start-AllServers {
    Write-Status "Starting all servers..."
    Write-Host ""

    $serverDir = Join-Path $ScriptDir "Server"

    # Stop any existing servers first
    Stop-ServerProcess "MonitoringServer"
    Stop-ServerProcess "AgentServer"
    Stop-ServerProcess "MapServer"
    Stop-ServerProcess "DistributeServer"

    Wait-Seconds 2

    # Start in correct order
    # 1. Monitoring Server (MAS mode)
    $result = Start-ServerProcess -Name "MonitoringServer" `
        -ExePath (Join-Path $serverDir "MonitoringServer.exe") `
        -Args "1" `
        -WaitPort 20001

    if (-not $result) {
        Write-Error "Failed to start MonitoringServer. Aborting."
        return $false
    }

    Wait-Seconds 3

    # 2. Agent Server
    $result = Start-ServerProcess -Name "AgentServer" `
        -ExePath (Join-Path $serverDir "AgentServer.exe") `
        -WaitPort 17001

    if (-not $result) {
        Write-Error "Failed to start AgentServer. Aborting."
        return $false
    }

    Wait-Seconds 2

    # 3. Map Server
    $result = Start-ServerProcess -Name "MapServer" `
        -ExePath (Join-Path $serverDir "MapServer.exe") `
        -WaitPort 18001

    if (-not $result) {
        Write-Error "Failed to start MapServer."
        # Non-fatal, continue anyway
    }

    Write-Host ""
    Write-Success "All servers started"
    return $true
}

function Start-GameClient {
    Write-Status "Starting game client..."

    $playDHDir = Join-Path $ScriptDir "PlayDH"
    $clientExe = Join-Path $playDHDir "MHClient-Connect.exe"

    if (-not (Test-Path $clientExe)) {
        Write-Error "Client executable not found: $clientExe"
        return $false
    }

    try {
        Push-Location $playDHDir
        $proc = Start-Process -FilePath $clientExe -PassThru
        Write-Success "Client started (PID: $($proc.Id))"
        Pop-Location
        return $true
    } catch {
        Write-Error "Failed to start client: $_"
        Pop-Location
        return $false
    }
}

# ===============================
# Main
# ===============================

function Main {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor White
    Write-Host "  MHGame System Startup Script v1.0" -ForegroundColor White
    Write-Host "========================================" -ForegroundColor White
    Write-Host ""

    # Environment check
    if (-not (Test-Environment)) {
        Write-Error "Please fix environment issues before starting"
        exit 1
    }

    # Database check
    Test-Database | Out-Null

    # Setup dev mode
    Initialize-DevMode

    Write-Host ""

    # Start servers
    if (-not $SkipServers) {
        if (-not (Start-AllServers)) {
            Write-Error "Server startup failed"
            exit 1
        }

        # Wait for servers to stabilize
        Wait-Seconds 5
    }

    # Start client
    if (-not $SkipClient) {
        Start-GameClient | Out-Null
    }

    Write-Host ""
    Write-Host "========================================" -ForegroundColor White
    Write-Host "  System startup complete!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor White
    Write-Host ""
    Write-Host "Servers:" -ForegroundColor Cyan
    Write-Host "  - Monitoring Server: Port 20001"
    Write-Host "  - Agent Server: Port 17001"
    Write-Host "  - Map Server: Port 18001"
    Write-Host ""
    Write-Host "To stop servers, run: .\stop_system.ps1" -ForegroundColor Yellow
    Write-Host ""

    return 0
}

# Execute
Main
exit $LASTEXITCODE
