# 快速参考手册 (Quick Reference)

> 5分钟快速上手指南 | 环境检查 | 常用命令 | 紧急故障排除

---

## 环境检查清单

### 必需环境

| 项目 | 检查命令 | 预期结果 |
|------|---------|---------|
| Visual Studio 2022 | `where devenv` | `C:\Program Files\Microsoft Visual Studio\2022\...` |
| DirectX SDK | `echo %DXSDK_DIR%` | `C:\Program Files (x86)\Microsoft DirectX SDK...` |
| Windows SDK | `dir "C:\Program Files (x86)\Windows Kits\10"` | 存在目录 |

### 快速环境检查 (PowerShell)

```powershell
# 检查VS2022
if (Test-Path "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe") {
    Write-Host "[OK] Visual Studio 2022"
} else { Write-Host "[MISSING] Visual Studio 2022" }

# 检查DirectX SDK
if ($env:DXSDK_DIR) {
    Write-Host "[OK] DirectX SDK: $env:DXSDK_DIR"
} else { Write-Host "[MISSING] DirectX SDK (DXSDK_DIR)" }

# 检查MSBuild
$msbuild = Get-Command msbuild -ErrorAction SilentlyContinue
if ($msbuild) {
    Write-Host "[OK] MSBuild: $($msbuild.Source)"
} else { Write-Host "[MISSING] MSBuild in PATH" }
```

---

## 编译命令速查表

### 基础库编译 (Layer 1)

```batch
:: 使用统一脚本
.\build_all.ps1 -Phase 1

:: 或手动编译
msbuild "[Lib]YHLibrary\YHLibrary.sln" /p:Configuration=Debug /p:Platform=Win32
msbuild "[Lib]HSEL\HSEL.sln" /p:Configuration=Debug /p:Platform=Win32
msbuild "[Lib]ZipArchive\ZipArchive.sln" /p:Configuration=Debug /p:Platform=Win32
```

### 功能DLL编译 (Layer 2)

```batch
.\build_all.ps1 -Phase 2

:: 或手动
msbuild "[Lib]BaseNetwork\BaseNetwork.sln" /p:Configuration=Debug /p:Platform=Win32
msbuild "[Lib]DBThread\DBThread.sln" /p:Configuration=Debug /p:Platform=Win32
```

### 游戏核心库 (Layer 3)

```batch
.\build_all.ps1 -Phase 3
```

### 服务器编译 (Layer 4)

```batch
.\build_all.ps1 -Phase 4
```

### 客户端编译 (Layer 5)

```batch
.\build_all.ps1 -Phase 5
```

### 全量编译

```batch
.\build_all.ps1 -All
```

### 清理重建

```batch
.\build_all.ps1 -All -Rebuild
```

---

## 服务器命令速查

### 启动服务器

```batch
:: 启动顺序 (重要!)
cd Server

:: 1. 监控服务器 (端口 20001)
MonitoringServer.exe 1

:: 2. 代理服务器 (端口 17001)
AgentServer.exe

:: 3. 地图服务器 (端口 18001)
MapServer.exe

:: 4. 分发服务器 (端口 16001) - 可选
DistributeServer.exe

:: 或一键启动
StartServers.bat
```

### 停止服务器

```batch
cd Server
StopServers.bat

:: 或强制终止
taskkill /F /IM MonitoringServer.exe
taskkill /F /IM AgentServer.exe
taskkill /F /IM MapServer.exe
taskkill /F /IM DistributeServer.exe
```

### 检查服务器状态

```batch
:: 检查进程
tasklist | findstr "Server"

:: 检查端口
netstat -an | findstr "LISTENING" | findstr "17001 18001 20001"

:: 使用验证脚本
.\verify_system.ps1 -CheckServers
```

---

## 端口速查表

| 服务器 | 端口 | 说明 |
|--------|------|------|
| Monitoring Server (MS) | 19001-19010 | 监控服务器 |
| Monitoring Agent (MAS) | 20001 | 监控代理 |
| Distribute Server | 16001 | 分发服务器 |
| Agent Server | 17001-17099 | 代理服务器 |
| Map Server | 18001-18099 | 地图服务器 |

---

## 配置文件速查

| 文件 | 用途 |
|------|------|
| `Server/serverset/serverset.txt` | 服务器配置集 (1-4) |
| `Server/masInfo.ini` | 监控代理配置 |
| `Server/msInfo.ini` | 监控服务器配置 |
| `Server/serverInfo.ini` | 服务器信息 |
| `Server/Log/` | 日志目录 |

---

## 输出目录速查

| 类型 | Debug路径 | Release路径 |
|------|-----------|-------------|
| 库文件 (.lib) | `Lib/x86/Debug/` | `Lib/x86/Release/` |
| DLL文件 | `DLL/x86/Debug/` | `DLL/x86/Release/` |
| 可执行文件 | `Server/` | `Server/` |

---

## 紧急故障排除

### 编译错误

**DirectX SDK未找到**
```batch
:: 设置环境变量
set DXSDK_DIR=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)
```

**链接错误 LNK2019**
```batch
:: 检查依赖库是否存在
dir Lib\x86\Debug\*.lib

:: 按顺序重新编译依赖
.\build_all.ps1 -Phase 1 -Rebuild
.\build_all.ps1 -Phase 2 -Rebuild
```

**C2065 未声明的标识符**
```batch
:: 检查头文件包含
:: 确保包含 CommonDefine.h
```

### 服务器无法启动

**端口被占用**
```batch
:: 查看端口占用
netstat -ano | findstr "17001"

:: 终止占用进程 (PID从上面命令获取)
taskkill /F /PID <PID>
```

**数据库连接失败**
```batch
:: 检查SQL Server服务
sc query MSSQLSERVER

:: 启动服务
net start MSSQLSERVER
```

**配置文件错误**
```batch
:: 检查配置文件
type Server\serverset\serverset.txt
```

### 客户端无法连接

1. 检查Agent Server是否运行: `tasklist | findstr AgentServer`
2. 检查防火墙: 控制面板 → Windows防火墙 → 允许应用
3. 检查服务器IP配置: `Server/serverInfo.ini`

---

## 常用宏速查

### 单例宏
```cpp
USINGTON(ClassName)->Method();    // 获取单例指针
GAMERESRCMNGR->LoadData();        // 游戏资源管理器
```

### 内存管理
```cpp
SAFE_DELETE(ptr);                 // 安全删除
SAFE_DELETE_ARRAY(arr);           // 安全删除数组
SAFE_RELEASE(pInterface);         // 安全释放COM
```

### 调试宏
```cpp
ASSERT(condition);
ASSERTMSG(condition, "message");
DEBUGMSG(condition, "debug info");
```

---

## 验证脚本

```powershell
# 完整系统验证
.\verify_system.ps1

# 仅检查编译输出
.\verify_system.ps1 -CheckLibs

# 仅检查服务器
.\verify_system.ps1 -CheckServers

# 扫描日志错误
.\verify_system.ps1 -ScanLogs
```

---

## 相关文档

- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 详细编译指南
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - 完整故障排除
- [TESTING_GUIDE.md](TESTING_GUIDE.md) - 测试验证指南
- [REGRESSION_TEST.md](REGRESSION_TEST.md) - 回归测试流程

---

*最后更新: 2026-02-20*
