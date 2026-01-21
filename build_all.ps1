<#
.SYNOPSIS
    MMORPG项目完整编译脚本 (PowerShell版本)

.DESCRIPTION
    按依赖顺序编译所有模块，支持并行编译、详细日志和错误处理

.PARAMETER Configuration
    Debug 或 Release (默认: Release)

.PARAMETER Clean
    清理所有输出目录

.PARAMETER Parallel
    启用并行编译 (默认: $true)

.PARAMETER LogFile
    日志文件路径 (默认: build_log_<Configuration>.txt)

.PARAMETER StartFrom
    从指定批次开始编译 (1-6)

.EXAMPLE
    .\build_all.ps1
    Release配置完整编译

.EXAMPLE
    .\build_all.ps1 -Configuration Debug
    Debug配置完整编译

.EXAMPLE
    .\build_all.ps1 -Configuration Release -Clean
    清理后Release配置编译

.EXAMPLE
    .\build_all.ps1 -StartFrom 3
    从第3批开始编译 (跳过基础库)

.NOTES
    作者: 开发团队
    版本: 1.0
    日期: 2026-01-21
#>

[CmdletBinding()]
param(
    [Parameter(Position=0)]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [switch]$Clean,

    [switch]$Parallel = $true,

    [string]$LogFile,

    [ValidateRange(1, 6)]
    [int]$StartFrom = 1
)

$ErrorActionPreference = "Continue"
$ProjectRoot = $PSScriptRoot

if ([string]::IsNullOrEmpty($LogFile)) {
    $LogFile = Join-Path $ProjectRoot "build_log_$Configuration.txt"
}

#region 函数定义

function Write-Log {
    <#
    .SYNOPSIS
        写入日志文件和控制台
    #>
    param(
        [Parameter(Mandatory=$true)]
        [string]$Message,

        [ValidateSet("INFO", "SUCCESS", "ERROR", "WARNING")]
        [string]$Level = "INFO"
    )

    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"

    $logMessage | Out-File -FilePath $LogFile -Append -Encoding UTF8

    switch ($Level) {
        "ERROR"   { Write-Host $logMessage -ForegroundColor Red }
        "SUCCESS" { Write-Host $logMessage -ForegroundColor Green }
        "WARNING" { Write-Host $logMessage -ForegroundColor Yellow }
        default   { Write-Host $logMessage }
    }
}

function Invoke-BuildModule {
    <#
    .SYNOPSIS
        编译单个模块
    #>
    param(
        [Parameter(Mandatory=$true)]
        [string]$Name,

        [Parameter(Mandatory=$true)]
        [string]$ProjectFile,

        [string[]]$Dependencies = @()
    )

    Write-Log "编译: $Name" INFO

    if ($Dependencies.Count -gt 0) {
        $depStr = $Dependencies -join ", "
        Write-Log "  依赖: $depStr" INFO
    }

    if (-not (Test-Path $ProjectFile)) {
        Write-Log "跳过 $Name`: 项目文件不存在: $ProjectFile" WARNING
        return $false
    }

    # MSBuild参数
    $arguments = @(
        "`"$ProjectFile`""
        "/p:Configuration=$Configuration"
        "/t:Rebuild"
        "/v:n"
        "/nologo"
    )

    if ($Parallel) {
        $arguments += "/m"
    }

    # 执行编译
    $process = Start-Process -FilePath "msbuild" -ArgumentList $arguments -NoNewWindow -Wait -PassThru

    if ($process.ExitCode -eq 0) {
        Write-Log "成功: $Name" SUCCESS
        return $true
    } else {
        Write-Log "失败: $Name (退出码: $($process.ExitCode))" ERROR
        return $false
    }
}

function Clear-OutputDirectories {
    <#
    .SYNOPSIS
        清理输出目录
    #>
    $directories = @("Bin", "Lib", "DLL", "Obj")

    foreach ($dir in $directories) {
        $path = Join-Path $ProjectRoot $dir

        if (Test-Path $path) {
            Write-Log "删除: $path" INFO
            Remove-Item -Path $path -Recurse -Force
        }
    }

    Write-Log "清理完成" SUCCESS
}

function Test-MsbuildAvailable {
    <#
    .SYNOPSIS
        检查MSBuild是否可用
    #>
    try {
        $null = Get-Command msbuild -ErrorAction Stop
        return $true
    } catch {
        return $false
    }
}

function Get-VisualStudioPath {
    <#
    .SYNOPSIS
        获取Visual Studio安装路径
    #>
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

    if (-not (Test-Path $vswhere)) {
        return $null
    }

    $vsPath = & $vswhere -latest -property installationPath

    if ($vsPath) {
        return $vsPath
    }

    return $null
}

function Initialize-BuildEnvironment {
    <#
    .SYNOPSIS
        初始化编译环境
    #>
    if (-not (Test-MsbuildAvailable)) {
        Write-Log "未找到MSBuild，尝试设置Visual Studio环境..." WARNING

        $vsPath = Get-VisualStudioPath

        if ($vsPath) {
            $vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"

            if (Test-Path $vcvars) {
                Write-Log "找到Visual Studio: $vsPath" INFO
                Write-Log "请手动运行: cmd /c `"`"$vcvars`" x86 && build_all.ps1`"" WARNING
                return $false
            }
        }

        Write-Log "无法找到Visual Studio，请确保已安装" ERROR
        return $false
    }

    return $true
}

#endregion

#region 主程序

# 清理输出目录
if ($Clean) {
    Write-Log "========================================" INFO
    Write-Log "清理输出目录" INFO
    Write-Log "========================================" INFO

    Clear-OutputDirectories
    exit 0
}

# 初始化编译环境
Write-Log "========================================" INFO
Write-Log "MMORPG项目完整编译 [$Configuration]" INFO
Write-Log "开始时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" INFO
Write-Log "========================================" INFO
Write-Log "" INFO

if (-not (Initialize-BuildEnvironment)) {
    exit 1
}

Write-Log "MSBuild版本:" INFO
msbuild /version /nologo | ForEach-Object { Write-Log "  $_" INFO }
Write-Log "" INFO

# 定义编译批次
$batches = @{
    1 = @(
        @{ Name="YHLibrary"; Project="[Lib]YHLibrary\YHLibrary.vcproj"; Dependencies=@() }
        @{ Name="HSEL"; Project="[Lib]HSEL\HSEL.vcproj"; Dependencies=@() }
        @{ Name="ZipArchive"; Project="[Lib]ZipArchive\ZipArchive.vcproj"; Dependencies=@() }
    )
    2 = @(
        @{ Name="BaseNetwork"; Project="[Lib]BaseNetwork\BaseNetwork.vcproj"; Dependencies=@("YHLibrary") }
        @{ Name="DBThread"; Project="[Lib]DBThread\DBThread.vcproj"; Dependencies=@("YHLibrary") }
    )
    3 = @(
        @{ Name="SoundLib"; Project="SoundLib\SoundLib.vcproj"; Dependencies=@("YHLibrary") }
        @{ Name="4DyuchiGXGeometry"; Project="4DyuchiGXGeometry\4DyuchiGXGeometry.vcproj"; Dependencies=@("YHLibrary") }
        @{ Name="4DyuchiGXExecutive"; Project="4DYUCHIGXEXECUTIVE\4DyuchiGXExecutive.vcproj"; Dependencies=@("YHLibrary") }
        @{ Name="4DyuchiFileStorage"; Project="4DyuchiFileStorage\4DyuchiFileStorage.vcproj"; Dependencies=@("YHLibrary") }
        @{ Name="4DyuchiNET"; Project="4DyuchiNET_Latest\I4DyuchiNET.vcproj"; Dependencies=@("YHLibrary", "BaseNetwork") }
        @{ Name="4DyuchiGX_Render"; Project="4DYUCHIGX_RENDER\4DyuchiGX_Render.vcproj"; Dependencies=@("DirectX SDK") }
    )
    4 = @(
        @{ Name="MapServer"; Project="[Server]Map\MapServer.vcproj"; Dependencies=@("YHLibrary", "BaseNetwork") }
        @{ Name="AgentServer"; Project="[Server]Agent\AgentServer.vcproj"; Dependencies=@("YHLibrary") }
        @{ Name="DistributeServer"; Project="[Server]Distribute\DistributeServer.vcproj"; Dependencies=@("YHLibrary", "BaseNetwork") }
        @{ Name="MonitoringServer"; Project="[Monitoring]Server\MonitoringServer.vcproj"; Dependencies=@("YHLibrary") }
    )
    5 = @(
        @{ Name="MHClient"; Project="[Client]MH\MHClient.vcproj"; Dependencies=@("多项依赖") }
        @{ Name="MHAutoPatch"; Project="[Client]MHAutoPatch\MHAutoPatch.vcproj"; Dependencies=@("ZipArchive", "SoundLib") }
        @{ Name="Selupdate"; Project="[Client]Selupdate\Selupdate.vcproj"; Dependencies=@("ZipArchive", "SoundLib") }
    )
    6 = @(
        @{ Name="ServerTool"; Project="[Monitoring]Tool\ServerTool.vcproj"; Dependencies=@() }
        @{ Name="PackingTool"; Project="[Tool]PackingMan\PackingTool.vcproj"; Dependencies=@() }
        @{ Name="RegenTool"; Project="[Tool]Regen\RegenTool.vcproj"; Dependencies=@() }
        @{ Name="DS_RMTool"; Project="[Tool]DS_RMTool\DS_RMTool.vcproj"; Dependencies=@() }
        @{ Name="ModelView"; Project="ModelView\ModelView.vcproj"; Dependencies=@() }
        @{ Name="MapEditor"; Project="4DyuchiGXMapEditor\4DyuchiGXMapEditor.vcproj"; Dependencies=@() }
        @{ Name="FilePack"; Project="4DyuchiFilePack\4DyuchiFilePack.vcproj"; Dependencies=@() }
    )
}

$batchNames = @{
    1 = "基础静态库"
    2 = "基础功能DLL"
    3 = "中间服务层"
    4 = "服务器端"
    5 = "客户端"
    6 = "工具和辅助程序"
}

# 执行编译
$successCount = 0
$failedCount = 0
$skippedCount = 0
$failedModules = @()

for ($batch = $StartFrom; $batch -le 6; $batch++) {
    $modules = $batches[$batch]
    $batchName = $batchNames[$batch]

    Write-Log "========================================" INFO
    Write-Log "第${batch}批: $batchName" INFO
    Write-Log "========================================" INFO
    Write-Log "" INFO

    foreach ($module in $modules) {
        $projectPath = Join-Path $ProjectRoot $module.Project

        $result = Invoke-BuildModule -Name $module.Name -ProjectFile $projectPath -Dependencies $module.Dependencies

        if ($result) {
            $successCount++
        } else {
            if (Test-Path $projectPath) {
                $failedCount++
                $failedModules += $module.Name
            } else {
                $skippedCount++
            }
        }
    }

    Write-Log "" INFO
}

# 编译总结
Write-Log "========================================" INFO
Write-Log "编译完成!" INFO
Write-Log "结束时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" INFO
Write-Log "========================================" INFO
Write-Log "" INFO
Write-Log "统计信息:" INFO
Write-Log "  成功: $successCount 个模块" SUCCESS
Write-Log "  失败: $failedCount 个模块" $(if ($failedCount -gt 0) { "ERROR" } else { "SUCCESS" })
Write-Log "  跳过: $skippedCount 个模块" INFO

if ($failedCount -gt 0) {
    Write-Log "" INFO
    Write-Log "失败模块:" ERROR
    $failedModules | ForEach-Object { Write-Log "  - $_" ERROR }
}

Write-Log "" INFO
Write-Log "日志文件: $LogFile" INFO

# 返回适当的退出码
exit ($failedCount -gt 0 ? 1 : 0)

#endregion
