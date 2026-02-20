<#
.SYNOPSIS
    MMORPG项目系统验证脚本

.DESCRIPTION
    验证编译环境、编译输出、服务器状态等。
    用于编译后验证和日常健康检查。

.PARAMETER CheckEnv
    仅检查编译环境

.PARAMETER CheckLibs
    仅检查库文件输出

.PARAMETER CheckServers
    仅检查服务器状态

.PARAMETER ScanLogs
    扫描日志文件中的错误

.PARAMETER Verbose
    显示详细输出

.EXAMPLE
    .\verify_system.ps1
    运行完整验证

.EXAMPLE
    .\verify_system.ps1 -CheckServers
    仅检查服务器状态

.NOTES
    作者: 开发团队
    版本: 1.0
    最后更新: 2026-02-20
#>

param(
    [switch]$CheckEnv,
    [switch]$CheckLibs,
    [switch]$CheckServers,
    [switch]$ScanLogs,
    [switch]$Verbose
)

$ErrorActionPreference = "Continue"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# ===============================
# 颜色输出
# ===============================

function Write-OK { Write-Host "[OK] " -NoNewline -ForegroundColor Green; Write-Host $args }
function Write-FAIL { Write-Host "[FAIL] " -NoNewline -ForegroundColor Red; Write-Host $args }
function Write-WARN { Write-Host "[WARN] " -NoNewline -ForegroundColor Yellow; Write-Host $args }
function Write-INFO { Write-Host "[INFO] " -NoNewline -ForegroundColor Cyan; Write-Host $args }
function Write-HEADER { Write-Host "`n=== $args ===" -ForegroundColor White }

# ===============================
# 检查函数
# ===============================

function Test-BuildEnvironment {
    Write-HEADER "环境检查"

    $envOK = $true

    # Visual Studio 2022
    $vsPath = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe"
    if (Test-Path $vsPath) {
        Write-OK "Visual Studio 2022: $vsPath"
    } else {
        $vsPath = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.exe"
        if (Test-Path $vsPath) {
            Write-OK "Visual Studio 2022 Professional: $vsPath"
        } else {
            Write-WARN "Visual Studio 2022 未找到"
            $envOK = $false
        }
    }

    # DirectX SDK
    if ($env:DXSDK_DIR) {
        if (Test-Path $env:DXSDK_DIR) {
            Write-OK "DirectX SDK: $($env:DXSDK_DIR)"
        } else {
            Write-FAIL "DirectX SDK 路径无效: $($env:DXSDK_DIR)"
            $envOK = $false
        }
    } else {
        Write-WARN "DXSDK_DIR 环境变量未设置"
        $envOK = $false
    }

    # MSBuild
    $msbuild = Get-Command msbuild -ErrorAction SilentlyContinue
    if ($msbuild) {
        Write-OK "MSBuild: $($msbuild.Source)"
    } else {
        Write-WARN "MSBuild 未在PATH中找到"
    }

    # Windows SDK
    $winSDKPath = "${env:ProgramFiles(x86)}\Windows Kits\10"
    if (Test-Path $winSDKPath) {
        Write-OK "Windows 10 SDK: $winSDKPath"
    } else {
        Write-WARN "Windows 10 SDK 未找到"
    }

    return $envOK
}

function Test-LibraryOutputs {
    Write-HEADER "库文件检查"

    $libsOK = $true

    $requiredLibs = @(
        @{ Name = "YHLibrary"; Path = "Lib\x86\Debug\YHLibrary.lib" },
        @{ Name = "HSEL"; Path = "Lib\x86\Debug\HSEL.lib" },
        @{ Name = "ZipArchive"; Path = "Lib\x86\Debug\ZipArchive_Debug.lib" },
        @{ Name = "BaseNetwork"; Path = "Lib\x86\Debug\BaseNetwork.lib" },
        @{ Name = "DBThread"; Path = "Lib\x86\Debug\DBThread.lib" },
        @{ Name = "Ability"; Path = "Lib\x86\Debug\Ability.lib" },
        @{ Name = "Skill"; Path = "Lib\x86\Debug\Skill.lib" },
        @{ Name = "BattleSystem"; Path = "Lib\x86\Debug\BattleSystem.lib" },
        @{ Name = "Quest"; Path = "Lib\x86\Debug\Quest.lib" },
        @{ Name = "Suryun"; Path = "Lib\x86\Debug\Suryun.lib" }
    )

    $foundCount = 0

    foreach ($lib in $requiredLibs) {
        $fullPath = Join-Path $ScriptDir $lib.Path
        if (Test-Path $fullPath) {
            $fileInfo = Get-Item $fullPath
            $size = '{0:N0}' -f $fileInfo.Length
            Write-OK "$($lib.Name) ($size bytes)"
            $foundCount++
        } else {
            Write-FAIL "$($lib.Name) - 文件不存在: $($lib.Path)"
            $libsOK = $false
        }
    }

    Write-INFO "总计: $foundCount / $($requiredLibs.Count) 个库文件"

    return $libsOK
}

function Test-ServerExecutables {
    Write-HEADER "服务器可执行文件检查"

    $serversOK = $true

    $requiredServers = @(
        @{ Name = "MonitoringServer"; Path = "Server\MonitoringServer.exe" },
        @{ Name = "AgentServer"; Path = "Server\AgentServer.exe" },
        @{ Name = "MapServer"; Path = "Server\MapServer.exe" },
        @{ Name = "DistributeServer"; Path = "Server\DistributeServer.exe" }
    )

    $foundCount = 0

    foreach ($server in $requiredServers) {
        $fullPath = Join-Path $ScriptDir $server.Path
        if (Test-Path $fullPath) {
            $fileInfo = Get-Item $fullPath
            $size = '{0:N0}' -f $fileInfo.Length
            $modTime = $fileInfo.LastWriteTime.ToString("yyyy-MM-dd HH:mm")
            Write-OK "$($server.Name) ($size bytes, $modTime)"
            $foundCount++
        } else {
            if ($server.Name -eq "DistributeServer") {
                Write-WARN "$($server.Name) - 可选，未找到"
            } else {
                Write-FAIL "$($server.Name) - 文件不存在"
                $serversOK = $false
            }
        }
    }

    Write-INFO "总计: $foundCount 个服务器文件"

    return $serversOK
}

function Test-ServerProcesses {
    Write-HEADER "服务器进程检查"

    $processesOK = $true

    $serverProcesses = @(
        "MonitoringServer",
        "AgentServer",
        "MapServer",
        "DistributeServer"
    )

    foreach ($procName in $serverProcesses) {
        $processes = Get-Process -Name $procName -ErrorAction SilentlyContinue
        if ($processes) {
            $proc = $processes[0]
            $memMB = [math]::Round($proc.WorkingSet64 / 1MB, 1)
            Write-OK "$procName 正在运行 (PID: $($proc.Id), 内存: ${memMB}MB)"
        } else {
            if ($procName -eq "DistributeServer") {
                Write-WARN "$procName 未运行 (可选)"
            } else {
                Write-WARN "$procName 未运行"
                $processesOK = $false
            }
        }
    }

    return $processesOK
}

function Test-ServerPorts {
    Write-HEADER "端口监听检查"

    $portsOK = $true

    $requiredPorts = @(
        @{ Port = 20001; Name = "MAS (Monitoring Agent)" },
        @{ Port = 17001; Name = "Agent Server" },
        @{ Port = 18001; Name = "Map Server" },
        @{ Port = 16001; Name = "Distribute Server" }
    )

    # 获取所有监听端口
    $listeningPorts = netstat -an | Select-String "LISTENING" | ForEach-Object {
        if ($_ -match ":(\d+)\s") {
            [int]$matches[1]
        }
    }

    foreach ($portInfo in $requiredPorts) {
        if ($listeningPorts -contains $portInfo.Port) {
            Write-OK "端口 $($portInfo.Port) 正在监听 ($($portInfo.Name))"
        } else {
            if ($portInfo.Port -eq 16001) {
                Write-WARN "端口 $($portInfo.Port) 未监听 ($($portInfo.Name), 可选)"
            } else {
                Write-WARN "端口 $($portInfo.Port) 未监听 ($($portInfo.Name))"
                $portsOK = $false
            }
        }
    }

    return $portsOK
}

function Test-ServerLogs {
    Write-HEADER "日志错误扫描"

    $logsOK = $true

    $logPath = Join-Path $ScriptDir "Server\Log"
    if (-not (Test-Path $logPath)) {
        Write-WARN "日志目录不存在: $logPath"
        return $true
    }

    $errorPatterns = @("ERROR", "Exception", "Failed", "failed", "error")

    $logFiles = Get-ChildItem -Path $logPath -Filter "*.log" -ErrorAction SilentlyContinue
    $logFiles += Get-ChildItem -Path $logPath -Filter "*.txt" -ErrorAction SilentlyContinue

    if (-not $logFiles) {
        Write-INFO "未找到日志文件"
        return $true
    }

    $errorCount = 0

    foreach ($logFile in $logFiles | Select-Object -First 5) {
        $content = Get-Content $logFile.FullName -ErrorAction SilentlyContinue
        if ($content) {
            $recentLines = $content | Select-Object -Last 50
            foreach ($line in $recentLines) {
                foreach ($pattern in $errorPatterns) {
                    if ($line -match $pattern) {
                        $errorCount++
                        if ($Verbose) {
                            Write-WARN "$($logFile.Name): $line"
                        }
                        break
                    }
                }
            }
        }
    }

    if ($errorCount -eq 0) {
        Write-OK "未发现错误日志"
    } else {
        Write-WARN "发现 $errorCount 条可能的问题日志 (使用 -Verbose 查看详情)"
        $logsOK = $false
    }

    return $logsOK
}

function Test-ClientExecutable {
    Write-HEADER "客户端检查"

    $clientOK = $true

    $clientPath = Join-Path $ScriptDir "[Client]MH\Debug\MHClient.exe"
    if (Test-Path $clientPath) {
        $fileInfo = Get-Item $clientPath
        $size = '{0:N0}' -f $fileInfo.Length
        Write-OK "MHClient.exe ($size bytes)"
    } else {
        $clientPath = Join-Path $ScriptDir "[Client]MH\Release\MHClient.exe"
        if (Test-Path $clientPath) {
            $fileInfo = Get-Item $clientPath
            $size = '{0:N0}' -f $fileInfo.Length
            Write-OK "MHClient.exe (Release, $size bytes)"
        } else {
            Write-WARN "MHClient.exe 未找到"
            $clientOK = $false
        }
    }

    # 检查资源文件
    $pakFiles = Get-ChildItem -Path $ScriptDir -Filter "*.pak" -ErrorAction SilentlyContinue
    if ($pakFiles) {
        Write-OK "资源文件: $($pakFiles.Count) 个 .pak 文件"
    } else {
        Write-WARN "未找到资源文件 (.pak)"
    }

    return $clientOK
}

# ===============================
# 主逻辑
# ===============================

function Main {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor White
    Write-Host "  MMORPG系统验证工具 v1.0" -ForegroundColor White
    Write-Host "========================================" -ForegroundColor White
    Write-Host ""

    # 如果没有指定任何检查项，则执行所有检查
    $runAll = -not ($CheckEnv -or $CheckLibs -or $CheckServers -or $ScanLogs)

    $results = @{
        Env = $true
        Libs = $true
        Servers = $true
        Logs = $true
    }

    if ($runAll -or $CheckEnv) {
        $results.Env = Test-BuildEnvironment
    }

    if ($runAll -or $CheckLibs) {
        $results.Libs = Test-LibraryOutputs
    }

    if ($runAll -or $CheckLibs) {
        $results.Servers = Test-ServerExecutables
    }

    if ($runAll -or $CheckLibs) {
        $results.Client = Test-ClientExecutable
    }

    if ($runAll -or $CheckServers) {
        $results.Processes = Test-ServerProcesses
        $results.Ports = Test-ServerPorts
    }

    if ($runAll -or $ScanLogs) {
        $results.Logs = Test-ServerLogs
    }

    # 汇总
    Write-HEADER "验证结果汇总"

    $allPassed = $true
    foreach ($key in $results.Keys) {
        if (-not $results[$key]) {
            $allPassed = $false
            Write-FAIL "$key 检查未通过"
        }
    }

    if ($allPassed) {
        Write-Host ""
        Write-Host "所有检查通过!" -ForegroundColor Green
        return 0
    } else {
        Write-Host ""
        Write-Host "部分检查未通过，请查看上述详情" -ForegroundColor Yellow
        return 1
    }
}

# 执行
$result = Main
exit $result
