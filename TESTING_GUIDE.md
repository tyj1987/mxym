# 测试验证指南 (Testing Guide)

> 编译验证 | 服务器验证 | 客户端测试 | 回归测试流程

---

## 目录

1. [快速验证](#快速验证)
2. [编译验证](#编译验证)
3. [服务器验证](#服务器验证)
4. [客户端测试](#客户端测试)
5. [回归测试流程](#回归测试流程)
6. [自动化验证脚本](#自动化验证脚本)

---

## 快速验证

### 一键验证命令

```powershell
# 运行完整系统验证
.\verify_system.ps1

# 或分步验证
.\verify_system.ps1 -CheckEnv      # 环境检查
.\verify_system.ps1 -CheckLibs     # 库文件检查
.\verify_system.ps1 -CheckServers  # 服务器检查
.\verify_system.ps1 -ScanLogs      # 日志扫描
```

### 验证检查清单

- [ ] 环境变量配置正确 (DXSDK_DIR)
- [ ] 基础库文件存在 (10+ .lib文件)
- [ ] 服务器可执行文件存在 (4+ .exe文件)
- [ ] 服务器能够正常启动
- [ ] 端口正常监听
- [ ] 客户端能够连接

---

## 编译验证

### 库文件验证

```batch
:: 检查基础库 (Layer 1)
dir /b Lib\x86\Debug\*.lib | find /c ".lib"
:: 预期: 3+ (YHLibrary.lib, HSEL.lib, ZipArchive_Debug.lib)

:: 检查功能DLL库 (Layer 2)
dir /b Lib\x86\Debug\BaseNetwork*.lib
dir /b Lib\x86\Debug\DBThread*.lib

:: 检查游戏核心库 (Layer 3)
dir /b Lib\x86\Debug\Ability*.lib
dir /b Lib\x86\Debug\Skill*.lib
dir /b Lib\x86\Debug\BattleSystem*.lib
dir /b Lib\x86\Debug\Quest*.lib
```

**预期库文件列表**:

| 层级 | 库名 | 预期路径 |
|------|------|---------|
| Layer 1 | YHLibrary.lib | Lib/x86/Debug/ |
| Layer 1 | HSEL.lib | Lib/x86/Debug/ |
| Layer 1 | ZipArchive_Debug.lib | Lib/x86/Debug/ |
| Layer 2 | BaseNetwork.lib | Lib/x86/Debug/ |
| Layer 2 | DBThread.lib | Lib/x86/Debug/ |
| Layer 3 | Ability.lib | Lib/x86/Debug/ |
| Layer 3 | Skill.lib | Lib/x86/Debug/ |
| Layer 3 | BattleSystem.lib | Lib/x86/Debug/ |
| Layer 3 | Quest.lib | Lib/x86/Debug/ |
| Layer 3 | Suryun.lib | Lib/x86/Debug/ |

### 服务器可执行文件验证

```batch
:: 检查服务器文件
dir /b Server\*.exe
:: 预期输出:
:: MonitoringServer.exe
:: AgentServer.exe
:: MapServer.exe
:: DistributeServer.exe (可选)
```

### 客户端验证

```batch
:: 检查客户端文件
dir /b "[Client]MH\Debug\*.exe"
:: 预期: MHClient.exe

:: 或发布版本
dir /b "[Client]MH\Release\*.exe"
```

---

## 服务器验证

### 启动验证

**步骤1: 启动监控服务器**
```batch
cd Server
MonitoringServer.exe 1
```

**预期结果**:
- 控制台显示启动信息
- 无错误消息
- 端口20001开始监听

**验证命令**:
```batch
netstat -an | findstr "20001"
:: 预期: TCP 0.0.0.0:20001 0.0.0.0:0 LISTENING
```

**步骤2: 启动代理服务器**
```batch
AgentServer.exe
```

**预期结果**:
- 显示连接到监控服务器
- 端口17001开始监听

**验证命令**:
```batch
netstat -an | findstr "17001"
:: 预期: TCP 0.0.0.0:17001 0.0.0.0:0 LISTENING
```

**步骤3: 启动地图服务器**
```batch
MapServer.exe
```

**预期结果**:
- 显示连接到监控服务器
- 加载地图数据
- 端口18001开始监听

**验证命令**:
```batch
netstat -an | findstr "18001"
:: 预期: TCP 0.0.0.0:18001 0.0.0.0:0 LISTENING
```

### 进程验证

```batch
:: 检查所有服务器进程
tasklist | findstr "Server"

:: 预期输出包含:
:: MonitoringServer.exe
:: AgentServer.exe
:: MapServer.exe
```

### 日志验证

```batch
:: 检查监控服务器日志
type Server\Log\MS_Debug.txt

:: 检查是否有ERROR关键字
findstr /i "error" Server\Log\*.log
```

### 端口监听验证

```batch
:: 检查所有服务器端口
netstat -an | findstr "LISTENING" | findstr "17001 18001 20001"

:: 预期输出:
:: TCP    0.0.0.0:17001    0.0.0.0:0    LISTENING
:: TCP    0.0.0.0:18001    0.0.0.0:0    LISTENING
:: TCP    0.0.0.0:20001    0.0.0.0:0    LISTENING
```

---

## 客户端测试

### 基本启动测试

```batch
:: 启动客户端
cd "[Client]MH\Debug"
MHClient.exe
```

**验证项**:
- [ ] 客户端正常启动
- [ ] 显示登录界面
- [ ] 无崩溃或黑屏

### 登录测试

**测试账号**: admin / admin123 (根据实际配置)

**验证项**:
- [ ] 能够输入账号密码
- [ ] 点击登录后有响应
- [ ] 显示服务器列表

### 连接测试

**前提**: 所有服务器已启动

**验证项**:
- [ ] 能够选择服务器
- [ ] 连接进度条正常
- [ ] 进入角色选择界面

### 游戏功能测试

**基础功能**:
- [ ] 创建角色
- [ ] 进入游戏世界
- [ ] 角色移动
- [ ] 使用技能
- [ ] NPC交互
- [ ] 物品使用

**网络功能**:
- [ ] 聊天消息
- [ ] 交易系统
- [ ] 组队功能
- [ ] 公会功能

---

## 回归测试流程

### Level 1: 快速验证 (必须)

**适用**: 任何代码修改后

**步骤**:
```batch
:: 1. 编译修改的模块
msbuild <修改的项目.sln> /p:Configuration=Debug /p:Platform=Win32

:: 2. 检查编译无错误
echo %ERRORLEVEL%
:: 预期: 0
```

**验证项**:
- [ ] 编译无错误
- [ ] 编译无新警告
- [ ] 输出文件已更新

### Level 2: 依赖验证

**适用**: 修改了被其他模块依赖的库

**步骤**:
```batch
:: 1. 识别依赖模块
:: 例如: 修改了[CC]Header中的头文件

:: 2. 重新编译所有依赖模块
.\build_all.ps1 -Phase 3 -Rebuild  :: 游戏核心
.\build_all.ps1 -Phase 4 -Rebuild  :: 服务器
.\build_all.ps1 -Phase 5 -Rebuild  :: 客户端

:: 3. 检查链接无错误
```

**验证项**:
- [ ] 所有依赖模块编译成功
- [ ] 链接无错误
- [ ] 所有可执行文件已更新

### Level 3: 功能验证

**适用**: 修改了服务器或客户端功能

**步骤**:
```batch
:: 1. 启动服务器
cd Server
StartServers.bat

:: 2. 等待服务器稳定
timeout /t 10

:: 3. 启动客户端测试
cd ..\[Client]MH\Debug
MHClient.exe

:: 4. 测试受影响的功能
```

**验证项**:
- [ ] 服务器正常启动
- [ ] 客户端能够连接
- [ ] 受影响功能正常工作
- [ ] 无新的崩溃或错误

### Level 4: 完整回归

**适用**: 重大架构修改、版本发布前

**步骤**:
```batch
:: 1. 清理所有输出
del /q Lib\x86\Debug\*.lib
del /q Server\*.exe

:: 2. 完整重新编译
.\build_all.ps1 -All -Rebuild

:: 3. 验证编译
.\verify_system.ps1 -CheckLibs

:: 4. 启动服务器
cd Server
StartServers.bat

:: 5. 完整功能测试
```

**验证项**:
- [ ] 所有模块编译成功
- [ ] 所有库文件存在
- [ ] 所有服务器可执行文件存在
- [ ] 服务器正常启动
- [ ] 客户端连接成功
- [ ] 核心功能全部正常
- [ ] 日志无错误

### 回归测试矩阵

| 修改类型 | Level 1 | Level 2 | Level 3 | Level 4 |
|---------|---------|---------|---------|---------|
| 单个源文件 | 必须 | - | - | - |
| 共享头文件 | 必须 | 必须 | 推荐 | - |
| 服务器逻辑 | 必须 | - | 必须 | - |
| 网络协议 | 必须 | 必须 | 必须 | 推荐 |
| 核心架构 | 必须 | 必须 | 必须 | 必须 |
| 版本发布前 | 必须 | 必须 | 必须 | 必须 |

---

## 自动化验证脚本

### verify_system.ps1 使用

```powershell
# 完整验证
.\verify_system.ps1

# 仅检查环境
.\verify_system.ps1 -CheckEnv

# 仅检查编译输出
.\verify_system.ps1 -CheckLibs

# 仅检查服务器
.\verify_system.ps1 -CheckServers

# 扫描日志错误
.\verify_system.ps1 -ScanLogs

# 详细输出
.\verify_system.ps1 -Verbose
```

### 预期输出示例

```
=== 系统验证报告 ===

[环境检查]
  [OK] Visual Studio 2022: C:\Program Files\Microsoft Visual Studio\2022\Community
  [OK] DirectX SDK: C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)
  [OK] MSBuild: C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe

[库文件检查]
  [OK] YHLibrary.lib
  [OK] HSEL.lib
  [OK] ZipArchive_Debug.lib
  [OK] BaseNetwork.lib
  [OK] DBThread.lib
  [OK] Ability.lib
  [OK] Skill.lib
  [OK] BattleSystem.lib
  [OK] Quest.lib
  [OK] Suryun.lib
  总计: 10 个库文件

[服务器检查]
  [OK] MonitoringServer.exe
  [OK] AgentServer.exe
  [OK] MapServer.exe
  [OK] DistributeServer.exe

[端口检查]
  [OK] 端口 17001 正在监听
  [OK] 端口 18001 正在监听
  [OK] 端口 20001 正在监听

[日志扫描]
  [OK] 未发现错误日志

=== 验证完成 ===
```

### 集成到CI/CD

```yaml
# 示例 Azure Pipelines 配置
steps:
- script: |
    .\build_all.ps1 -All
    .\verify_system.ps1 -CheckLibs
  displayName: 'Build and Verify'

- script: |
    cd Server
    StartServers.bat
    timeout /t 30
  displayName: 'Start Servers'

- script: |
    .\verify_system.ps1 -CheckServers
  displayName: 'Verify Servers'
```

---

## 测试数据

### 测试账号

| 账号 | 密码 | 权限 | 用途 |
|------|------|------|------|
| admin | admin123 | GM | 管理测试 |
| test1 | test123 | 普通 | 功能测试 |
| test2 | test123 | 普通 | 压力测试 |

### 测试场景

**登录测试**:
1. 正确账号密码登录
2. 错误密码登录
3. 不存在的账号登录
4. 重复登录

**角色测试**:
1. 创建新角色
2. 删除角色
3. 角色进入游戏
4. 角色属性检查

**战斗测试**:
1. 单体攻击
2. 范围技能
3. PK模式
4. 组队战斗

---

## 相关文档

- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - 快速参考
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 编译指南
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - 故障排除
- [REGRESSION_TEST.md](REGRESSION_TEST.md) - 回归测试详细流程

---

*最后更新: 2026-02-20*
