<#
.SYNOPSIS
    MHGame Database Restore Script

.DESCRIPTION
    Restores SQL Server databases from backup files.
    Creates databases and sets up users.

.PARAMETER ServerInstance
    SQL Server instance (default: .\SQLEXPRESS)

.PARAMETER DataDir
    Data directory for database files

.PARAMETER Force
    Overwrite existing databases

.EXAMPLE
    .\restore_database.ps1
    Restore databases using default settings

.EXAMPLE
    .\restore_database.ps1 -Force
    Force restore and overwrite existing databases
#>

param(
    [string]$ServerInstance = ".\SQLEXPRESS",
    [string]$DataDir = "",
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

function Write-Status { Write-Host "[*] $args" -ForegroundColor Cyan }
function Write-Success { Write-Host "[+] $args" -ForegroundColor Green }
function Write-Warning { Write-Host "[!] $args" -ForegroundColor Yellow }
function Write-Error { Write-Host "[-] $args" -ForegroundColor Red }

# Database backup files
$databases = @(
    @{ Name = "MHCMEMBER"; Backup = "数据库\MHCMEMBER.bak" },
    @{ Name = "MHGAME"; Backup = "数据库\MHGAME.bak" },
    @{ Name = "MHLOG"; Backup = "数据库\MHLOG.bak" }
)

function Test-SqlServer {
    Write-Status "Checking SQL Server connection..."

    try {
        $result = & sqlcmd -S $ServerInstance -Q "SELECT 1" -h -1 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Success "SQL Server connection OK"
            return $true
        }
    } catch {
        Write-Error "SQL Server connection failed: $_"
        return $false
    }

    Write-Error "SQL Server not accessible. Is SQL Server running?"
    return $false
}

function Restore-Database {
    param(
        [string]$Name,
        [string]$BackupFile
    )

    $fullBackupPath = Join-Path $ScriptDir $BackupFile

    if (-not (Test-Path $fullBackupPath)) {
        Write-Error "Backup file not found: $fullBackupPath"
        return $false
    }

    Write-Status "Restoring database: $Name"

    # Get file list from backup
    $fileListQuery = "RESTORE FILELISTONLY FROM DISK = '$fullBackupPath'"
    $fileList = & sqlcmd -S $ServerInstance -Q $fileListQuery -s "," -W -h -1 2>&1

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to read backup file list"
        return $false
    }

    # Parse file list
    $dataFile = ""
    $logFile = ""

    foreach ($line in $fileList) {
        if ($line -match "^([^,]+),([^,]+),([^,]+)") {
            $logicalName = $matches[1]
            $type = $matches[2]
            if ($type -eq "D") {
                $dataFile = $logicalName
            } elseif ($type -eq "L") {
                $logFile = $logicalName
            }
        }
    }

    # Build MOVE clauses
    $defaultDataDir = "C:\Program Files\Microsoft SQL Server\MSSQL15.SQLEXPRESS\MSSQL\DATA"
    if ($DataDir) {
        $defaultDataDir = $DataDir
    }

    $moveData = "MOVE '$dataFile' TO '$defaultDataDir\$Name.mdf'"
    $moveLog = "MOVE '$logFile' TO '$defaultDataDir\$Name_log.ldf'"

    # Check if database exists
    $checkQuery = "SELECT name FROM sys.databases WHERE name = '$Name'"
    $exists = & sqlcmd -S $ServerInstance -d master -Q $checkQuery -h -1 -W 2>&1

    if ($exists -match $Name) {
        if ($Force) {
            Write-Warning "Database $Name exists, setting offline..."
            & sqlcmd -S $ServerInstance -Q "ALTER DATABASE [$Name] SET OFFLINE WITH ROLLBACK IMMEDIATE" 2>&1 | Out-Null
        } else {
            Write-Warning "Database $Name already exists. Use -Force to overwrite."
            return $true
        }
    }

    # Restore database
    $restoreQuery = @"
RESTORE DATABASE [$Name]
FROM DISK = '$fullBackupPath'
WITH $moveData,
     $moveLog,
     REPLACE,
     STATS = 10
"@

    Write-Status "Executing restore..."
    $result = & sqlcmd -S $ServerInstance -Q $restoreQuery 2>&1

    if ($LASTEXITCODE -eq 0) {
        Write-Success "Database $Name restored successfully"
        return $true
    } else {
        Write-Error "Failed to restore database $Name"
        Write-Error $result
        return $false
    }
}

function New-DatabaseUser {
    Write-Status "Creating database user..."

    $createQuery = @"
USE master;
IF NOT EXISTS (SELECT name FROM sys.server_principals WHERE name = 'mhgame')
    CREATE LOGIN mhgame WITH PASSWORD = 'MHGame@2026!';

USE MHCMEMBER;
IF NOT EXISTS (SELECT name FROM sys.database_principals WHERE name = 'mhgame')
    CREATE USER mhgame FOR LOGIN mhgame;
ALTER ROLE db_owner ADD MEMBER mhgame;

USE MHGAME;
IF NOT EXISTS (SELECT name FROM sys.database_principals WHERE name = 'mhgame')
    CREATE USER mhgame FOR LOGIN mhgame;
ALTER ROLE db_owner ADD MEMBER mhgame;

USE MHLOG;
IF NOT EXISTS (SELECT name FROM sys.database_principals WHERE name = 'mhgame')
    CREATE USER mhgame FOR LOGIN mhgame;
ALTER ROLE db_owner ADD MEMBER mhgame;
"@

    $result = & sqlcmd -S $ServerInstance -Q $createQuery 2>&1

    if ($LASTEXITCODE -eq 0) {
        Write-Success "Database user created"
        return $true
    } else {
        Write-Warning "Could not create user (may already exist)"
        return $true
    }
}

# Main
Write-Host ""
Write-Host "========================================" -ForegroundColor White
Write-Host "  MHGame Database Restore Script" -ForegroundColor White
Write-Host "========================================" -ForegroundColor White
Write-Host ""

# Check SQL Server
if (-not (Test-SqlServer)) {
    Write-Error "Please ensure SQL Server is running and accessible"
    Write-Host ""
    Write-Host "To start SQL Server:" -ForegroundColor Yellow
    Write-Host "  net start MSSQL`$SQLEXPRESS"
    Write-Host ""
    exit 1
}

# Restore databases
$allSuccess = $true
foreach ($db in $databases) {
    $result = Restore-Database -Name $db.Name -BackupFile $db.Backup
    if (-not $result) {
        $allSuccess = $false
    }
}

# Create user
New-DatabaseUser | Out-Null

Write-Host ""
if ($allSuccess) {
    Write-Success "All databases restored successfully!"
    Write-Host ""
    Write-Host "Database credentials:" -ForegroundColor Yellow
    Write-Host "  Server: $ServerInstance"
    Write-Host "  User: mhgame"
    Write-Host "  Password: MHGame@2026!"
    Write-Host ""
    Write-Host "Update your server config files with these credentials." -ForegroundColor Yellow
} else {
    Write-Warning "Some databases may not have been restored correctly"
}

Write-Host ""
