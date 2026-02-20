<#
.SYNOPSIS
    MMORPG Project Unified Build Script

.DESCRIPTION
    Build all project modules in correct dependency order.
    Supports phased builds, full builds, and various configurations.

.PARAMETER Phase
    Build phase (1-5):
    - Phase 1: Base static libs (YHLibrary, HSEL, ZipArchive)
    - Phase 2: Base DLLs (BaseNetwork, DBThread)
    - Phase 3: Game core libs (Ability, Skill, BattleSystem, Quest, Suryun)
    - Phase 4: Server apps (Monitoring, Agent, Map, Distribute)
    - Phase 5: Client app (MHClient)

.PARAMETER All
    Build all phases

.PARAMETER Rebuild
    Full rebuild instead of incremental

.PARAMETER Configuration
    Build configuration (Debug, Release, Debug_CHINA, etc.)
    Default: Debug

.PARAMETER Platform
    Target platform (Win32, x64)
    Default: Win32

.PARAMETER Verify
    Run verification after build

.PARAMETER Parallel
    Parallel build (default enabled)

.PARAMETER Verbose
    Verbose output

.EXAMPLE
    .\build_all.ps1 -All
    Build all modules

.EXAMPLE
    .\build_all.ps1 -Phase 1 -Rebuild
    Rebuild base libraries

.EXAMPLE
    .\build_all.ps1 -All -Configuration Release -Verify
    Build Release and verify
#>

param(
    [ValidateSet(1, 2, 3, 4, 5)]
    [int]$Phase = 0,

    [switch]$All,

    [switch]$Rebuild,

    [ValidateSet("Debug", "Release", "Debug_CHINA", "Debug_JAPAN", "Debug_HK", "Release_CHINA")]
    [string]$Configuration = "Debug",

    [ValidateSet("Win32", "x64")]
    [string]$Platform = "Win32",

    [switch]$Verify,

    [switch]$Parallel = $true,

    [switch]$Verbose
)

# ===============================
# Configuration
# ===============================

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Color output functions
function Write-Success { Write-Host $args -ForegroundColor Green }
function Write-Info { Write-Host $args -ForegroundColor Cyan }
function Write-Warning { Write-Host $args -ForegroundColor Yellow }
function Write-Err { Write-Host $args -ForegroundColor Red }

# Build phases definition
$BuildPhases = @{
    1 = @{
        Name = "Base Static Libraries"
        Projects = @(
            "[Lib]YHLibrary\YHLibrary.sln",
            "[Lib]HSEL\HSEL.sln",
            "[Lib]ZipArchive\ZipArchive.sln"
        )
        Outputs = @(
            "Lib\x86\$Configuration\YHLibrary.lib",
            "Lib\x86\$Configuration\HSEL.lib",
            "Lib\x86\$Configuration\ZipArchive_Debug.lib"
        )
    }
    2 = @{
        Name = "Base DLLs"
        Projects = @(
            "[Lib]BaseNetwork\BaseNetwork.sln",
            "[Lib]DBThread\DBThread.sln"
        )
        Outputs = @(
            "Lib\x86\$Configuration\BaseNetwork.lib",
            "Lib\x86\$Configuration\DBThread.lib"
        )
    }
    3 = @{
        Name = "Game Core Libraries"
        Projects = @(
            "[CC]Ability\Ability.sln",
            "[CC]Skill\Skill.sln",
            "[CC]BattleSystem\BattleSystem.sln",
            "[CC]Quest\Quest.sln",
            "[CC]Suryun\Suryun.sln"
        )
        Outputs = @(
            "Lib\x86\$Configuration\Ability.lib",
            "Lib\x86\$Configuration\Skill.lib",
            "Lib\x86\$Configuration\BattleSystem.lib",
            "Lib\x86\$Configuration\Quest.lib",
            "Lib\x86\$Configuration\Suryun.lib"
        )
    }
    4 = @{
        Name = "Server Applications"
        Projects = @(
            "[Monitoring]Server\MonitoringServer.sln",
            "[Server]Agent\AgentServer.sln",
            "[Server]Map\MapServer.sln",
            "[Server]Distribute\DistributeServer.sln"
        )
        Outputs = @(
            "Server\MonitoringServer.exe",
            "Server\AgentServer.exe",
            "Server\MapServer.exe",
            "Server\DistributeServer.exe"
        )
        ConfigOverride = "Debug_CHINA"
    }
    5 = @{
        Name = "Client Application"
        Projects = @(
            "[Client]MH\MHClient.sln"
        )
        Outputs = @(
            "[Client]MH\Debug\MHClient.exe"
        )
        ConfigOverride = "Debug_CHINA"
    }
}

# ===============================
# Helper Functions
# ===============================

function Find-MSBuild {
    $msbuildPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
    )

    foreach ($path in $msbuildPaths) {
        if (Test-Path $path) {
            return $path
        }
    }

    # Try from PATH
    $msbuild = Get-Command msbuild -ErrorAction SilentlyContinue
    if ($msbuild) {
        return $msbuild.Source
    }

    throw "MSBuild not found. Please install Visual Studio 2019 or 2022"
}

function Test-Environment {
    Write-Info "Checking build environment..."

    # Check DirectX SDK
    if (-not $env:DXSDK_DIR) {
        Write-Warning "DXSDK_DIR environment variable not set"
        Write-Info "Searching for DirectX SDK..."

        $dxsdkPaths = @(
            "${env:ProgramFiles(x86)}\Microsoft DirectX SDK (June 2010)",
            "${env:ProgramFiles}\Microsoft DirectX SDK (June 2010)"
        )

        foreach ($path in $dxsdkPaths) {
            if (Test-Path $path) {
                $env:DXSDK_DIR = $path
                Write-Success "Found DirectX SDK: $path"
                break
            }
        }
    } else {
        Write-Success "DirectX SDK: $env:DXSDK_DIR"
    }

    # Check MSBuild
    $script:MSBuildPath = Find-MSBuild
    Write-Success "MSBuild: $script:MSBuildPath"

    return $true
}

function Invoke-BuildProject {
    param(
        [string]$ProjectPath,
        [string]$Config,
        [string]$Plat,
        [bool]$Reb,
        [bool]$Par
    )

    $fullPath = Join-Path $ScriptDir $ProjectPath

    if (-not (Test-Path $fullPath)) {
        Write-Err "Project file not found: $fullPath"
        return $false
    }

    $target = if ($Reb) { "Rebuild" } else { "Build" }
    $parallelFlag = if ($Par) { "/m" } else { "" }

    $arguments = @(
        $fullPath,
        "/p:Configuration=$Config",
        "/p:Platform=$Plat",
        "/t:$target",
        $parallelFlag,
        "/v:minimal",
        "/nologo"
    )

    if ($Verbose) {
        $arguments += "/v:detailed"
    }

    Write-Info "Building: $ProjectPath ($Config|$Plat)"

    $startTime = Get-Date

    try {
        $process = Start-Process -FilePath $script:MSBuildPath `
            -ArgumentList $arguments `
            -NoNewWindow `
            -Wait `
            -PassThru `
            -ErrorAction Stop

        $duration = (Get-Date) - $startTime
        $durationSec = [math]::Round($duration.TotalSeconds, 1)

        if ($process.ExitCode -eq 0) {
            Write-Success "  Done ($durationSec seconds)"
            return $true
        } else {
            Write-Err "  Failed (exit code: $($process.ExitCode))"
            return $false
        }
    } catch {
        Write-Err "  Exception: $_"
        return $false
    }
}

function Invoke-BuildPhase {
    param([int]$PhaseNum)

    $phase = $BuildPhases[$PhaseNum]
    if (-not $phase) {
        Write-Err "Invalid phase: $PhaseNum"
        return $false
    }

    Write-Info ""
    Write-Info "========================================"
    Write-Info "Phase $PhaseNum : $($phase.Name)"
    Write-Info "========================================"

    $config = if ($phase.ConfigOverride -and $Configuration -eq "Debug") {
        $phase.ConfigOverride
    } else {
        $Configuration
    }

    $success = $true

    foreach ($project in $phase.Projects) {
        $result = Invoke-BuildProject -ProjectPath $project `
            -Config $config `
            -Plat $Platform `
            -Reb $Rebuild `
            -Par $Parallel

        if (-not $result) {
            $success = $false
            break
        }
    }

    return $success
}

function Test-BuildOutputs {
    param([int]$PhaseNum)

    $phase = $BuildPhases[$PhaseNum]
    $allExist = $true

    Write-Info "Checking Phase $PhaseNum outputs..."

    foreach ($output in $phase.Outputs) {
        $fullPath = Join-Path $ScriptDir $output
        if (Test-Path $fullPath) {
            $fileInfo = Get-Item $fullPath
            $size = $fileInfo.Length
            Write-Success "  [OK] $output ($size bytes)"
        } else {
            Write-Err "  [MISSING] $output"
            $allExist = $false
        }
    }

    return $allExist
}

function Invoke-FullVerification {
    Write-Info ""
    Write-Info "========================================"
    Write-Info "Build Verification"
    Write-Info "========================================"

    $totalLibs = 0
    $totalExes = 0

    # Check library files
    $libPath = Join-Path $ScriptDir "Lib\x86\$Configuration"
    if (Test-Path $libPath) {
        $libs = Get-ChildItem -Path $libPath -Filter "*.lib"
        $totalLibs = $libs.Count
        Write-Success "Library files: $totalLibs"
    } else {
        Write-Warning "Library directory not found: $libPath"
    }

    # Check server executables
    $serverPath = Join-Path $ScriptDir "Server"
    if (Test-Path $serverPath) {
        $exes = Get-ChildItem -Path $serverPath -Filter "*Server.exe"
        $totalExes = $exes.Count
        Write-Success "Server executables: $totalExes"
    }

    return ($totalLibs -ge 8 -and $totalExes -ge 3)
}

# ===============================
# Main Logic
# ===============================

function Main {
    Write-Info ""
    Write-Info "========================================"
    Write-Info "  MMORPG Unified Build Script v1.0"
    Write-Info "========================================"
    Write-Info ""

    # Parameter validation
    if ($Phase -eq 0 -and -not $All) {
        Write-Err "Please specify -Phase <1-5> or -All"
        Write-Info ""
        Write-Info "Usage examples:"
        Write-Info "  .\build_all.ps1 -Phase 1     # Build base libs"
        Write-Info "  .\build_all.ps1 -Phase 4     # Build servers"
        Write-Info "  .\build_all.ps1 -All         # Build everything"
        Write-Info "  .\build_all.ps1 -All -Rebuild # Full rebuild"
        exit 1
    }

    # Environment check
    try {
        Test-Environment | Out-Null
    } catch {
        Write-Err "Environment check failed: $_"
        exit 1
    }

    $totalStartTime = Get-Date
    $allSuccess = $true

    # Determine phases to build
    $phasesToBuild = if ($All) { 1..5 } else { @($Phase) }

    foreach ($p in $phasesToBuild) {
        $success = Invoke-BuildPhase -PhaseNum $p

        if ($success) {
            Test-BuildOutputs -PhaseNum $p | Out-Null
        } else {
            $allSuccess = $false
            Write-Err "Phase $p build failed, stopping"
            break
        }
    }

    $totalDuration = (Get-Date) - $totalStartTime
    $totalMin = [math]::Round($totalDuration.TotalMinutes, 1)

    # Final verification
    if ($Verify -and $allSuccess) {
        Invoke-FullVerification | Out-Null
    }

    # Summary
    Write-Info ""
    Write-Info "========================================"
    Write-Info "Build Summary"
    Write-Info "========================================"
    Write-Info "Total time: $totalMin minutes"

    if ($allSuccess) {
        Write-Success "Status: SUCCESS"
        exit 0
    } else {
        Write-Err "Status: FAILED"
        exit 1
    }
}

# Execute main function
Main
