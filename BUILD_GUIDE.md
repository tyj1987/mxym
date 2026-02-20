# 编译指南 (Build Guide)

> 详细编译步骤 | 配置说明 | 输出位置 | 常见问题

---

## 目录

1. [环境准备](#环境准备)
2. [编译顺序](#编译顺序)
3. [编译命令详解](#编译命令详解)
4. [构建配置](#构建配置)
5. [输出文件位置](#输出文件位置)
6. [自动化编译](#自动化编译)
7. [常见编译问题](#常见编译问题)

---

## 环境准备

### 必需软件

| 软件 | 版本 | 用途 | 下载链接 |
|------|------|------|---------|
| Visual Studio | 2022 Community | 主要IDE | https://visualstudio.microsoft.com/ |
| DirectX SDK | June 2010 | 图形渲染 | https://www.microsoft.com/en-us/download/details.aspx?id=6812 |
| Windows SDK | 10/11 | 平台SDK | VS安装时包含 |

### Visual Studio工作负载

确保安装以下工作负载：
- **使用C++的桌面开发**
- **Windows 10/11 SDK**
- **C++ CMake tools for Windows** (可选)

### 环境变量设置

```batch
:: DirectX SDK
setx DXSDK_DIR "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"

:: 验证
echo %DXSDK_DIR%
```

### 项目配置

对于每个项目，确保VC++目录设置正确：

**包含目录**:
```
$(DXSDK_DIR)Include
$(SolutionDir)..\[CC]Header
$(SolutionDir)..\[Lib]YHLibrary
```

**库目录**:
```
$(DXSDK_DIR)Lib\x86
$(SolutionDir)..\Lib\x86\$(Configuration)
```

---

## 编译顺序

### 依赖关系图

```
┌─────────────────────────────────────────────────────────────────┐
│ Layer 5: 客户端应用                                              │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ [Client]MH/MHClient.sln                                     │ │
│ └─────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
                              ▲
┌─────────────────────────────────────────────────────────────────┐
│ Layer 4: 服务器应用                                              │
│ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐              │
│ │ Monitoring  │ │   Agent     │ │    Map      │ │ Distribute │  │
│ │   Server    │ │   Server    │ │   Server    │ │  Server    │  │
│ └──────────────┘ └──────────────┘ └──────────────┘ └──────────┘ │
└─────────────────────────────────────────────────────────────────┘
                              ▲
┌─────────────────────────────────────────────────────────────────┐
│ Layer 3: 游戏核心库 ([CC])                                       │
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐    │
│ │ Ability │ │  Skill  │ │ Battle  │ │  Quest  │ │ Suryun  │    │
│ └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘    │
└─────────────────────────────────────────────────────────────────┘
                              ▲
┌─────────────────────────────────────────────────────────────────┐
│ Layer 2: 基础功能DLL                                             │
│ ┌─────────────────────┐ ┌─────────────────────┐                 │
│ │   BaseNetwork.sln   │ │    DBThread.sln     │                 │
│ └─────────────────────┘ └─────────────────────┘                 │
└─────────────────────────────────────────────────────────────────┘
                              ▲
┌─────────────────────────────────────────────────────────────────┐
│ Layer 1: 基础静态库 (无依赖)                                     │
│ ┌───────────────┐ ┌───────────────┐ ┌───────────────┐          │
│ │  YHLibrary    │ │     HSEL      │ │  ZipArchive   │          │
│ └───────────────┘ └───────────────┘ └───────────────┘          │
└─────────────────────────────────────────────────────────────────┘
```

### 详细编译步骤

#### Layer 1: 基础静态库

```batch
:: 1. YHLibrary - 核心工具库
msbuild "[Lib]YHLibrary\YHLibrary.sln" /p:Configuration=Debug /p:Platform=Win32 /m

:: 2. HSEL - 哈希/加密库
msbuild "[Lib]HSEL\HSEL.sln" /p:Configuration=Debug /p:Platform=Win32 /m

:: 3. ZipArchive - 压缩库
msbuild "[Lib]ZipArchive\ZipArchive.sln" /p:Configuration=Debug /p:Platform=Win32 /m
```

**验证**:
```batch
dir Lib\x86\Debug\*.lib
:: 预期输出: YHLibrary.lib, HSEL.lib, ZipArchive_Debug.lib
```

#### Layer 2: 基础功能DLL

```batch
:: 1. BaseNetwork - 网络抽象层
msbuild "[Lib]BaseNetwork\BaseNetwork.sln" /p:Configuration=Debug /p:Platform=Win32 /m

:: 2. DBThread - 数据库线程库
msbuild "[Lib]DBThread\DBThread.sln" /p:Configuration=Debug /p:Platform=Win32 /m
```

**验证**:
```batch
dir Lib\x86\Debug\BaseNetwork*.lib
dir Lib\x86\Debug\DBThread*.lib
```

#### Layer 3: 游戏核心库

```batch
:: 1. Ability - 能力系统
msbuild "[CC]Ability\Ability.sln" /p:Configuration=Debug /p:Platform=Win32 /m

:: 2. Skill - 技能系统
msbuild "[CC]Skill\Skill.sln" /p:Configuration=Debug /p:Platform=Win32 /m

:: 3. BattleSystem - 战斗系统
msbuild "[CC]BattleSystem\BattleSystem.sln" /p:Configuration=Debug /p:Platform=Win32 /m

:: 4. Quest - 任务系统
msbuild "[CC]Quest\Quest.sln" /p:Configuration=Debug /p:Platform=Win32 /m

:: 5. Suryun - 修练系统
msbuild "[CC]Suryun\Suryun.sln" /p:Configuration=Debug /p:Platform=Win32 /m
```

#### Layer 4: 服务器应用

```batch
:: 1. MonitoringServer - 监控服务器
msbuild "[Monitoring]Server\MonitoringServer.sln" /p:Configuration=Debug_CHINA /p:Platform=Win32 /m

:: 2. AgentServer - 代理服务器
msbuild "[Server]Agent\AgentServer.sln" /p:Configuration=Debug_CHINA /p:Platform=Win32 /m

:: 3. MapServer - 地图服务器
msbuild "[Server]Map\MapServer.sln" /p:Configuration=Debug_CHINA /p:Platform=Win32 /m

:: 4. DistributeServer - 分发服务器
msbuild "[Server]Distribute\DistributeServer.sln" /p:Configuration=Debug_CHINA /p:Platform=Win32 /m
```

**验证**:
```batch
dir Server\*.exe
:: 预期输出: MonitoringServer.exe, AgentServer.exe, MapServer.exe, DistributeServer.exe
```

#### Layer 5: 客户端应用

```batch
:: MHClient - 主客户端
msbuild "[Client]MH\MHClient.sln" /p:Configuration=Debug_CHINA /p:Platform=Win32 /m
```

---

## 编译命令详解

### MSBuild常用参数

```batch
msbuild <project.sln> [参数]

常用参数:
  /p:Configuration=<配置>    - 构建配置 (Debug, Release, Debug_CHINA等)
  /p:Platform=<平台>         - 目标平台 (Win32, x64)
  /t:<目标>                  - 构建目标 (Build, Rebuild, Clean)
  /m                         - 多核并行编译
  /v:<级别>                  - 输出详细程度 (q, m, n, d, diag)
  /fl                        - 输出到文件
  /flp:logfile=<文件>        - 日志文件路径

示例:
  :: 增量编译
  msbuild project.sln /p:Configuration=Debug /p:Platform=Win32 /m

  :: 完全重建
  msbuild project.sln /p:Configuration=Debug /p:Platform=Win32 /t:Rebuild /m

  :: 详细输出到文件
  msbuild project.sln /p:Configuration=Debug /p:Platform=Win32 /v:d /fl /flp:logfile=build.log
```

### Devenv命令

```batch
:: 使用Visual Studio IDE编译
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" <project.sln> /build "<配置>|Win32"

示例:
  devenv project.sln /build "Debug_CHINA|Win32"
  devenv project.sln /rebuild "Release|Win32"
```

---

## 构建配置

### 调试配置

| 配置名 | 用途 | 预处理器 |
|--------|------|---------|
| `Debug` | 标准调试 | `_DEBUG` |
| `Debug_CHINA` | 中国版调试 | `_DEBUG`, `CHINA_LOCAL` |
| `Debug_JAPAN` | 日本版调试 | `_DEBUG`, `JAPAN_LOCAL` |
| `Debug_HK` | 香港版调试 | `_DEBUG`, `HK_LOCAL` |
| `Debug Connect` | 连接调试 | `_DEBUG`, `_CONNECT` |
| `Debug GM_TOOL` | GM工具调试 | `_DEBUG`, `_GMTOOL_` |

### 发布配置

| 配置名 | 用途 | 预处理器 |
|--------|------|---------|
| `Release` | 标准发布 | `NDEBUG` |
| `Semi Release` | 半发布 | `NDEBUG`, `_SEMI_RELEASE` |
| `Semi ReleaseChina` | 中国半发布 | `NDEBUG`, `CHINA_LOCAL` |
| `Semi ReleaseJapan` | 日本半发布 | `NDEBUG`, `JAPAN_LOCAL` |

### 配置特定编译

```batch
:: 编译中国版本
msbuild project.sln /p:Configuration=Debug_CHINA /p:Platform=Win32

:: 编译发布版本
msbuild project.sln /p:Configuration=Release /p:Platform=Win32
```

---

## 输出文件位置

### 库文件 (.lib)

| 配置 | 路径 |
|------|------|
| Debug | `Lib\x86\Debug\` |
| Release | `Lib\x86\Release\` |

### DLL文件

| 配置 | 路径 |
|------|------|
| Debug | `DLL\x86\Debug\` |
| Release | `DLL\x86\Release\` |

### 可执行文件

所有服务器和客户端可执行文件输出到 `Server\` 目录。

### 中间文件

- `Debug\` - 项目级中间文件
- `ipch\` - 预编译头缓存
- `*.obj` - 目标文件
- `*.pdb` - 调试符号

---

## 自动化编译

### 使用统一编译脚本

```powershell
# 编译所有模块
.\build_all.ps1 -All

# 按阶段编译
.\build_all.ps1 -Phase 1    # 基础库
.\build_all.ps1 -Phase 2    # 功能DLL
.\build_all.ps1 -Phase 3    # 游戏核心
.\build_all.ps1 -Phase 4    # 服务器
.\build_all.ps1 -Phase 5    # 客户端

# 完全重建
.\build_all.ps1 -All -Rebuild

# 指定配置
.\build_all.ps1 -All -Configuration Release

# 编译后验证
.\build_all.ps1 -All -Verify
```

### 脚本参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-Phase` | 编译阶段 (1-5) | - |
| `-All` | 编译所有阶段 | false |
| `-Rebuild` | 完全重建 | false |
| `-Configuration` | 构建配置 | Debug |
| `-Verify` | 编译后验证 | false |
| `-Parallel` | 并行编译 | true |
| `-Verbose` | 详细输出 | false |

---

## 常见编译问题

### DirectX SDK未找到

**错误信息**:
```
fatal error C1083: Cannot open include file: 'd3dx9.h'
```

**解决方案**:
```batch
:: 1. 检查环境变量
echo %DXSDK_DIR%

:: 2. 设置环境变量
setx DXSDK_DIR "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"

:: 3. 重启命令行或IDE
```

### 链接错误 LNK2019

**错误信息**:
```
error LNK2019: unresolved external symbol "..."
```

**解决方案**:
1. 检查依赖库是否已编译
2. 确保按正确顺序编译
3. 检查库文件路径配置

```batch
:: 检查库文件
dir Lib\x86\Debug\*.lib

:: 重新编译依赖
.\build_all.ps1 -Phase 1 -Rebuild
```

### 预编译头错误

**错误信息**:
```
fatal error C1083: Cannot open precompiled header file
```

**解决方案**:
```batch
:: 清理并重建
msbuild project.sln /t:Clean
msbuild project.sln /t:Rebuild
```

### 编码问题

**错误信息**:
```
warning C4819: The file contains a character that cannot be represented in the current code page
```

**解决方案**:
- 使用UTF-8 BOM编码保存源文件
- 或在项目中设置 `/source-charset:utf-8`

### 内存不足

**解决方案**:
```batch
:: 减少并行编译数
msbuild project.sln /m:2

:: 或使用32位MSBuild
:: (避免大项目地址空间耗尽)
```

---

## 编译时间优化

### 并行编译

```batch
:: 使用所有核心
msbuild project.sln /m

:: 限制核心数
msbuild project.sln /m:4
```

### 增量编译

- 只重新编译修改的文件
- 避免使用 `/t:Rebuild`
- 确保时间戳正确

### 预编译头

- 项目已配置预编译头 (`stdafx.h`)
- 首次编译较慢，后续大幅加速

### 分布式编译 (高级)

可考虑使用 Incredibuild 等工具实现分布式编译。

---

## 相关文档

- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - 快速参考
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - 故障排除
- [TESTING_GUIDE.md](TESTING_GUIDE.md) - 测试指南

---

*最后更新: 2026-02-20*
