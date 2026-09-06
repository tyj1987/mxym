# MHGame Database Restore Script
# Usage: .\restore_database.ps1 -ServerInstance ".\SQLEXPRESS" -BakDir "C:\path\to\bak"
# Do not commit real passwords or .bak files.

param(
    [string]$ServerInstance = ".\SQLEXPRESS",
    [string]$BakDir = "",
    [string]$DbPassword = $env:MHGAME_DB_PASSWORD
)

function Write-Status { Write-Host "[*] $args" -ForegroundColor Cyan }
function Write-Success { Write-Host "[+] $args" -ForegroundColor Green }
function Write-Warn { Write-Host "[!] $args" -ForegroundColor Yellow }
function Write-Err { Write-Host "[-] $args" -ForegroundColor Red }

if (-not $DbPassword) {
    Write-Err "Set -DbPassword or env MHGAME_DB_PASSWORD (not committed)."
    exit 1
}

Write-Host "MHGame Database Restore Script" -ForegroundColor White

# Ensure SQL is up
try {
    sqlcmd -S $ServerInstance -E -Q "SELECT 1" | Out-Null
} catch {
    Write-Err "Cannot reach SQL Server at $ServerInstance"
    Write-Host "To start SQL Server: net start MSSQL`$SQLEXPRESS"
    exit 1
}

if (-not $BakDir -or -not (Test-Path $BakDir)) {
    Write-Err "Provide -BakDir pointing to local .bak files (not in git)."
    exit 1
}

$dbs = @(
    @{ Name = "MHCMEMBER"; File = "MHCMEMBER.bak" },
    @{ Name = "MHGAME"; File = "MHGAME.bak" },
    @{ Name = "MHLOG"; File = "MHLOG.bak" }
)

foreach ($db in $dbs) {
    $bak = Join-Path $BakDir $db.File
    if (-not (Test-Path $bak)) { Write-Warn "Missing $bak — skip"; continue }
    Write-Status "Restoring $($db.Name) from $bak"
    $q = @"
RESTORE DATABASE [$($db.Name)] FROM DISK = N'$bak' WITH REPLACE
"@
    sqlcmd -S $ServerInstance -E -Q $q
    if ($LASTEXITCODE -ne 0) { Write-Err "Restore failed: $($db.Name)"; exit 1 }
    Write-Success "Restored $($db.Name)"
}

Write-Status "Ensuring login mhgame exists"
$sql = @"
IF NOT EXISTS (SELECT 1 FROM sys.server_principals WHERE name = 'mhgame')
BEGIN
    CREATE LOGIN mhgame WITH PASSWORD = 'CHANGE_ME_DB_PASSWORD';
END
"@
# Prefer parameter; placeholder above is documentation-only — replace at runtime:
$sql = $sql.Replace('CHANGE_ME_DB_PASSWORD', $DbPassword.Replace("'","''"))
sqlcmd -S $ServerInstance -E -Q $sql

Write-Success "Restore finished."
Write-Host "Database login created (password not printed)." -ForegroundColor Yellow
Write-Host "Set DBPassword in Server/*.ini from your local secrets; do not commit real passwords." -ForegroundColor Yellow
