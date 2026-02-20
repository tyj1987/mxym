# 回归测试流程 (Regression Test Guide)

> 代码修改后的标准化验证流程

---

## 概述

本文档定义了代码修改后必须执行的验证流程，确保修改不会引入新的问题。

### 测试级别

| 级别 | 名称 | 耗时 | 适用场景 |
|------|------|------|---------|
| Level 1 | 快速验证 | ~1分钟 | 任何代码修改后 (必须) |
| Level 2 | 依赖验证 | ~5分钟 | 修改了被依赖的库 |
| Level 3 | 功能验证 | ~15分钟 | 修改了服务器/客户端功能 |
| Level 4 | 完整回归 | ~30分钟+ | 重大修改、版本发布前 |

---

## Level 1: 快速验证

### 适用场景
- 任何源文件修改后
- 提交代码前

### 执行步骤

```batch
:: 1. 编译修改的模块
msbuild <修改的项目.sln> /p:Configuration=Debug /p:Platform=Win32 /m

:: 2. 检查编译结果
echo %ERRORLEVEL%
```

### 验证清单

- [ ] 编译无错误 (ERRORLEVEL = 0)
- [ ] 编译无新警告
- [ ] 输出文件时间戳已更新

### 示例

```batch
:: 修改了 MapServer 中的 Player.cpp
msbuild "[Server]Map\MapServer.sln" /p:Configuration=Debug_CHINA /p:Platform=Win32 /m

if %ERRORLEVEL% EQU 0 (
    echo [PASS] 编译成功
) else (
    echo [FAIL] 编译失败
    exit /b 1
)
```

---

## Level 2: 依赖验证

### 适用场景
- 修改了共享头文件 (`[CC]Header/`)
- 修改了基础库 (`[Lib]*/`)
- 修改了核心库 (`[CC]*/`)

### 依赖关系

```
[CC]Header/  ──┬──> [CC]Ability/
               ├──> [CC]Skill/
               ├──> [CC]BattleSystem/
               ├──> [CC]Quest/
               └──> [CC]Suryun/
                           │
                           v
               [Server]Map/  [Client]MH/
```

### 执行步骤

```batch
:: 1. 识别受影响的模块
:: 例如：修改了 [CC]Header/CommonGameDefine.h

:: 2. 重新编译所有游戏核心库
msbuild "[CC]Ability\Ability.sln" /t:Rebuild /p:Configuration=Debug /p:Platform=Win32
msbuild "[CC]Skill\Skill.sln" /t:Rebuild /p:Configuration=Debug /p:Platform=Win32
msbuild "[CC]BattleSystem\BattleSystem.sln" /t:Rebuild /p:Configuration=Debug /p:Platform=Win32
msbuild "[CC]Quest\Quest.sln" /t:Rebuild /p:Configuration=Debug /p:Platform=Win32
msbuild "[CC]Suryun\Suryun.sln" /t:Rebuild /p:Configuration=Debug /p:Platform=Win32

:: 3. 重新编译服务器
msbuild "[Server]Map\MapServer.sln" /t:Rebuild /p:Configuration=Debug_CHINA /p:Platform=Win32

:: 4. (可选) 重新编译客户端
msbuild "[Client]MH\MHClient.sln" /t:Rebuild /p:Configuration=Debug_CHINA /p:Platform=Win32
```

### 使用自动化脚本

```powershell
# 重新编译 Layer 3-5
.\build_all.ps1 -Phase 3 -Rebuild
.\build_all.ps1 -Phase 4 -Rebuild
.\build_all.ps1 -Phase 5 -Rebuild

# 或使用验证脚本
.\verify_system.ps1 -CheckLibs
```

### 验证清单

- [ ] 所有依赖模块编译成功
- [ ] 链接无 LNK2019/LNK2001 错误
- [ ] 所有库文件已更新

---

## Level 3: 功能验证

### 适用场景
- 修改了服务器业务逻辑
- 修改了客户端功能
- 修改了网络协议

### 执行步骤

```batch
:: 1. 停止现有服务器
cd Server
StopServers.bat

:: 2. 确保使用最新编译的程序
dir *.exe

:: 3. 按顺序启动服务器
StartServers.bat

:: 4. 等待服务器初始化
timeout /t 10

:: 5. 验证服务器状态
.\verify_system.ps1 -CheckServers

:: 6. 启动客户端测试
cd ..\[Client]MH\Debug
MHClient.exe
```

### 功能测试用例

#### 基础功能

| 测试项 | 操作 | 预期结果 |
|--------|------|---------|
| 登录 | 使用测试账号登录 | 成功进入角色选择 |
| 创建角色 | 创建新角色 | 角色创建成功 |
| 进入游戏 | 选择角色进入 | 成功进入游戏世界 |
| 移动 | WASD移动角色 | 角色正常移动 |
| 攻击 | 使用技能攻击怪物 | 伤害计算正确 |

#### 网络功能

| 测试项 | 操作 | 预期结果 |
|--------|------|---------|
| 聊天 | 发送聊天消息 | 消息正确显示 |
| 交易 | 与NPC交易 | 物品正确交换 |
| 组队 | 创建/加入队伍 | 队伍状态正确 |

### 验证清单

- [ ] 服务器正常启动
- [ ] 无崩溃或异常退出
- [ ] 客户端能够连接
- [ ] 受影响功能正常工作
- [ ] 日志无 ERROR 消息

---

## Level 4: 完整回归

### 适用场景
- 重大架构修改
- 版本发布前
- 长时间未更新后的首次编译

### 执行步骤

```batch
:: 1. 清理所有编译输出
del /q Lib\x86\Debug\*.lib 2>nul
del /q Lib\x86\Release\*.lib 2>nul
del /q Server\*.exe 2>nul

:: 2. 完整重新编译
.\build_all.ps1 -All -Rebuild -Verify

:: 3. 如果编译成功，验证输出
.\verify_system.ps1

:: 4. 启动服务器
cd Server
StopServers.bat
StartServers.bat

:: 5. 等待稳定
timeout /t 15

:: 6. 完整功能测试
.\verify_system.ps1 -CheckServers -ScanLogs
```

### 完整功能测试清单

**服务器测试**:
- [ ] MonitoringServer 正常启动
- [ ] AgentServer 正常启动
- [ ] MapServer 正常启动
- [ ] DistributeServer 正常启动 (可选)
- [ ] 所有端口正常监听
- [ ] 服务器间通信正常

**客户端测试**:
- [ ] 客户端正常启动
- [ ] 登录功能正常
- [ ] 角色创建/删除正常
- [ ] 进入游戏正常
- [ ] 基础移动正常
- [ ] 技能使用正常
- [ ] 物品系统正常
- [ ] NPC交互正常
- [ ] 聊天功能正常

**性能测试**:
- [ ] 服务器内存占用正常 (<2GB)
- [ ] CPU占用正常 (<50%空闲时)
- [ ] 网络延迟正常 (<100ms)

---

## 自动化回归测试

### PowerShell脚本

```powershell
# run_regression_test.ps1

param(
    [ValidateSet(1, 2, 3, 4)]
    [int]$Level = 1
)

function Test-Level1 {
    Write-Host "=== Level 1: 快速验证 ===" -ForegroundColor Cyan

    # 获取最近修改的文件
    $changedFiles = git diff --name-only HEAD~1

    foreach ($file in $changedFiles) {
        if ($file -match "\.cpp$|\.h$") {
            Write-Host "检查: $file"

            # 确定所属项目
            if ($file -match "\[Server\]Map") {
                $project = "[Server]Map\MapServer.sln"
            } elseif ($file -match "\[CC\]") {
                $project = $file -replace "^(.*)\\.*$", '$1\$1.sln'
            } else {
                continue
            }

            # 编译
            msbuild $project /p:Configuration=Debug /p:Platform=Win32 /m /v:minimal
            if ($LASTEXITCODE -ne 0) {
                Write-Host "[FAIL] $project 编译失败" -ForegroundColor Red
                return $false
            }
        }
    }

    Write-Host "[PASS] Level 1 验证通过" -ForegroundColor Green
    return $true
}

function Test-Level2 {
    Write-Host "=== Level 2: 依赖验证 ===" -ForegroundColor Cyan

    .\build_all.ps1 -Phase 3 -Rebuild
    if ($LASTEXITCODE -ne 0) { return $false }

    .\build_all.ps1 -Phase 4 -Rebuild
    if ($LASTEXITCODE -ne 0) { return $false }

    .\verify_system.ps1 -CheckLibs
    return $LASTEXITCODE -eq 0
}

function Test-Level3 {
    Write-Host "=== Level 3: 功能验证 ===" -ForegroundColor Cyan

    # 启动服务器
    cd Server
    .\StopServers.bat
    Start-Sleep -Seconds 2
    .\StartServers.bat
    Start-Sleep -Seconds 10
    cd ..

    # 验证
    .\verify_system.ps1 -CheckServers
    return $LASTEXITCODE -eq 0
}

function Test-Level4 {
    Write-Host "=== Level 4: 完整回归 ===" -ForegroundColor Cyan

    .\build_all.ps1 -All -Rebuild
    if ($LASTEXITCODE -ne 0) { return $false }

    .\verify_system.ps1
    if ($LASTEXITCODE -ne 0) { return $false }

    # 服务器测试
    cd Server
    .\StopServers.bat
    .\StartServers.bat
    Start-Sleep -Seconds 15
    cd ..

    .\verify_system.ps1 -CheckServers -ScanLogs
    return $LASTEXITCODE -eq 0
}

# 执行指定级别的测试
switch ($Level) {
    1 { $result = Test-Level1 }
    2 { $result = Test-Level2 }
    3 { $result = Test-Level3 }
    4 { $result = Test-Level4 }
}

if ($result) {
    Write-Host "`n回归测试通过!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`n回归测试失败!" -ForegroundColor Red
    exit 1
}
```

### 使用方法

```powershell
# Level 1 - 快速验证
.\run_regression_test.ps1 -Level 1

# Level 2 - 依赖验证
.\run_regression_test.ps1 -Level 2

# Level 3 - 功能验证
.\run_regression_test.ps1 -Level 3

# Level 4 - 完整回归
.\run_regression_test.ps1 -Level 4
```

---

## 回归测试决策矩阵

| 修改类型 | Level 1 | Level 2 | Level 3 | Level 4 |
|---------|:-------:|:-------:|:-------:|:-------:|
| 单个源文件 (.cpp) | ✓ | - | - | - |
| 单个头文件 (.h, 私有) | ✓ | - | - | - |
| 共享头文件 ([CC]Header/) | ✓ | ✓ | - | - |
| 基础库 ([Lib]*/) | ✓ | ✓ | - | - |
| 游戏核心库 ([CC]*/) | ✓ | ✓ | - | - |
| 服务器逻辑 | ✓ | - | ✓ | - |
| 网络协议 | ✓ | ✓ | ✓ | - |
| 数据库操作 | ✓ | - | ✓ | - |
| 客户端UI | ✓ | - | ✓ | - |
| 客户端逻辑 | ✓ | - | ✓ | - |
| 核心架构变更 | ✓ | ✓ | ✓ | ✓ |
| DirectX/渲染 | ✓ | ✓ | ✓ | ✓ |
| 版本发布前 | ✓ | ✓ | ✓ | ✓ |

---

## 常见问题

### Q: 修改了哪个文件需要执行哪个级别的测试?

A: 参考[回归测试决策矩阵](#回归测试决策矩阵)。基本原则:
- 任何修改至少执行 Level 1
- 修改共享代码执行 Level 2
- 修改业务逻辑执行 Level 3
- 重大修改执行 Level 4

### Q: Level 3 测试时服务器启动失败怎么办?

A:
1. 检查日志文件 (`Server/Log/`)
2. 检查端口占用 (`netstat -an`)
3. 检查数据库连接
4. 参考 [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

### Q: 如何快速判断修改是否影响了其他模块?

A:
```powershell
# 检查修改的文件
git diff --name-only HEAD~1

# 如果包含 [CC]Header/ 下的文件，则需要 Level 2
# 如果包含 [Lib]*/ 下的文件，则需要 Level 2
```

---

## 相关文档

- [TESTING_GUIDE.md](TESTING_GUIDE.md) - 测试指南
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - 故障排除
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 编译指南

---

*最后更新: 2026-02-20*
