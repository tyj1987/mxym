# CLAUDE.md

> MMORPG项目开发指南 | 快速开始 | 核心架构 | 详细文档索引

---

## 快速开始

### 首次编译

```batch
# 方法1: 使用统一脚本 (推荐)
.\build_all.ps1 -All

# 方法2: 分阶段编译
.\build_all.ps1 -Phase 1   # 基础库
.\build_all.ps1 -Phase 2   # 功能DLL
.\build_all.ps1 -Phase 3   # 游戏核心
.\build_all.ps1 -Phase 4   # 服务器
.\build_all.ps1 -Phase 5   # 客户端
```

### 启动服务器

```batch
cd Server
StartServers.bat
```

### 验证系统

```powershell
.\verify_system.ps1
```

---

## 编译命令速查

### 环境要求

| 软件 | 版本 | 环境变量 |
|------|------|---------|
| Visual Studio | 2022 | - |
| DirectX SDK | June 2010 | `DXSDK_DIR` |
| Windows SDK | 10/11 | - |

### 编译顺序 (必须按此顺序)

```
┌─────────────────────────────────────────┐
│ Layer 1: 基础静态库                      │
│   YHLibrary → HSEL → ZipArchive         │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│ Layer 2: 基础功能DLL                     │
│   BaseNetwork → DBThread                │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│ Layer 3: 游戏核心库 [CC]                 │
│   Ability → Skill → BattleSystem        │
│   → Quest → Suryun                      │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│ Layer 4: 服务器应用                      │
│   Monitoring → Agent → Map → Distribute │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│ Layer 5: 客户端应用                      │
│   MHClient                              │
└─────────────────────────────────────────┘
```

### 常用命令

```batch
# MSBuild编译
msbuild project.sln /p:Configuration=Debug /p:Platform=Win32 /m

# 完全重建
msbuild project.sln /t:Rebuild /p:Configuration=Debug_CHINA /p:Platform=Win32

# 清理
msbuild project.sln /t:Clean
```

---

## 服务器启动顺序

```
1. MonitoringServer.exe (端口 20001)
2. AgentServer.exe      (端口 17001)
3. MapServer.exe        (端口 18001)
4. DistributeServer.exe (端口 16001) [可选]
```

### 端口分配

| 服务器 | 端口 | 说明 |
|--------|------|------|
| Monitoring (MS) | 19001-19010 | 监控服务器 |
| Monitoring (MAS) | 20001 | 监控代理 |
| Distribute | 16001 | 分发服务器 |
| Agent | 17001-17099 | 代理服务器 |
| Map | 18001-18099 | 地图服务器 |

---

## 项目结构

```
D:\mxym\
├── [CC]Header/         # 共享头文件
├── [CC]Ability/        # 能力系统
├── [CC]Skill/          # 技能系统
├── [CC]BattleSystem/   # 战斗系统
├── [CC]Quest/          # 任务系统
├── [CC]Suryun/         # 修练系统
├── [CC]ServerModule/   # 服务器模块
├── [Lib]YHLibrary/     # 核心工具库
├── [Lib]BaseNetwork/   # 网络抽象层
├── [Lib]DBThread/      # 数据库线程
├── [Lib]HSEL/          # 哈希/加密
├── [Lib]ZipArchive/    # 压缩库
├── [Server]Agent/      # 代理服务器
├── [Server]Map/        # 地图服务器
├── [Server]Distribute/ # 分发服务器
├── [Monitoring]Server/ # 监控服务器
├── [Client]MH/         # 客户端
├── [Tool]*/            # 工具集
├── Server/             # 部署目录
├── Lib/                # 库输出
└── DLL/                # DLL输出
```

---

## 核心系统

### GameResourceManager
- **位置**: `[CC]Header/GameResourceManager.h`
- **职责**: 管理游戏资源、NPC、怪物、物品数据
- **访问**: `GAMERESRCMNGR->Method()`

### 网络架构
- **4DyuchiNET_Latest/** - 核心网络库 (IOCP)
- **BaseNetwork** - 网络抽象层 (COM接口)
- 自定义二进制协议，加密通信

### 渲染系统
- **传统**: DirectX 8.1 (4DyuchiGX_*)
- **现代**: DirectX 12 (4DYUCHIGX_RENDER_D3D12/)

---

## 编码规范

### 命名约定

| 类型 | 前缀 | 示例 |
|------|------|------|
| 成员变量 | `m_` | `m_bActive` |
| 布尔值 | `b` | `bIsRunning` |
| 指针 | `p` | `pObject` |
| 字 | `w` | `wCount` |
| DWORD | `dw` | `dwSize` |
| 类 | `c` | `cGameManager` |
| 接口 | `I` | `INetwork` |

### 常用宏

```cpp
// 单例
USINGTON(ClassName)->Method();
GAMERESRCMNGR->LoadData();

// 内存管理
SAFE_DELETE(ptr);
SAFE_DELETE_ARRAY(arr);
SAFE_RELEASE(pInterface);

// 调试
ASSERT(condition);
ASSERTMSG(condition, "message");
DEBUGMSG(condition, "info");
```

### 预处理器定义

**地区**: `CHINA_LOCAL`, `JAPAN_LOCAL`, `HK_LOCAL`, `TAIWAN_LOCAL`
**功能**: `_GMTOOL_`, `_HACK_SHIELD_`, `_MHCLIENT_`, `_DEBUG`

---

## 配置文件

| 文件 | 用途 |
|------|------|
| `Server/serverset/serverset.txt` | 服务器配置集 (1-4) |
| `Server/masInfo.ini` | 监控代理配置 |
| `Server/msInfo.ini` | 监控服务器配置 |
| `Server/serverInfo.ini` | 服务器信息 |
| `Server/Log/` | 日志目录 |

---

## 输出目录

| 类型 | Debug | Release |
|------|-------|---------|
| 库文件 | `Lib/x86/Debug/` | `Lib/x86/Release/` |
| DLL | `DLL/x86/Debug/` | `DLL/x86/Release/` |
| 可执行文件 | `Server/` | `Server/` |

---

## 常见问题速查

### DirectX SDK未找到
```batch
setx DXSDK_DIR "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"
```

### 链接错误 LNK2019
```batch
# 检查依赖库
dir Lib\x86\Debug\*.lib
# 按顺序重新编译
.\build_all.ps1 -Phase 1 -Rebuild
```

### 端口被占用
```batch
netstat -ano | findstr "17001"
taskkill /F /PID <PID>
```

### 服务器无法启动
1. 检查日志: `Server/Log/`
2. 检查端口: `netstat -an`
3. 检查数据库服务

---

## 详细文档索引

| 文档 | 内容 |
|------|------|
| [QUICK_REFERENCE.md](QUICK_REFERENCE.md) | 环境检查、常用命令、紧急故障排除 |
| [BUILD_GUIDE.md](BUILD_GUIDE.md) | 详细编译步骤、配置说明、输出位置 |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | 编译错误、运行时错误、服务器问题 |
| [TESTING_GUIDE.md](TESTING_GUIDE.md) | 编译验证、服务器验证、客户端测试 |
| [REGRESSION_TEST.md](REGRESSION_TEST.md) | 4级回归测试流程 |

---

## 自动化脚本

| 脚本 | 用途 |
|------|------|
| `build_all.ps1` | 统一编译脚本 |
| `verify_system.ps1` | 系统验证脚本 |
| `Server/StartServers.bat` | 启动所有服务器 |
| `Server/StopServers.bat` | 停止所有服务器 |

### build_all.ps1 参数

```powershell
.\build_all.ps1 -Phase 1..5     # 指定阶段
.\build_all.ps1 -All            # 编译全部
.\build_all.ps1 -Rebuild        # 完全重建
.\build_all.ps1 -Configuration Release  # 指定配置
.\build_all.ps1 -Verify         # 编译后验证
```

### verify_system.ps1 参数

```powershell
.\verify_system.ps1             # 完整验证
.\verify_system.ps1 -CheckEnv   # 环境检查
.\verify_system.ps1 -CheckLibs  # 库文件检查
.\verify_system.ps1 -CheckServers # 服务器检查
.\verify_system.ps1 -ScanLogs   # 日志扫描
```

---

## 开发工作流

### 添加新功能
1. 确定功能所属模块
2. 在相应目录添加代码
3. 更新头文件和消息解析器
4. 按依赖顺序重新编译
5. 执行回归测试

### 代码修改后验证
```powershell
# Level 1: 快速验证 (任何修改)
msbuild <project.sln> /p:Configuration=Debug

# Level 2: 依赖验证 (修改共享代码)
.\build_all.ps1 -Phase 3 -Rebuild

# Level 3: 功能验证 (修改业务逻辑)
cd Server && StartServers.bat
# 测试受影响功能

# Level 4: 完整回归 (重大修改)
.\build_all.ps1 -All -Rebuild -Verify
```

---

## 关键文件位置

**入口点**:
- 客户端: `[Client]MH/MHClient.cpp`
- 地图服务器: `[Server]Map/ServerSystem.cpp`

**核心头文件**:
- `[CC]Header/CommonDefine.h` - 核心宏定义
- `[CC]Header/CommonGameDefine.h` - 游戏定义
- `[CC]Header/GameResourceManager.h` - 资源管理

**网络消息**:
- `[Server]Map/MapNetworkMsgParser.h` - 地图服务器消息
- `[Server]Distribute/DistributeNetworkMsgParser.h` - 分发服务器消息

---

## 性能优化

- **内存**: 内存池 (`CMemoryPoolTempl`)
- **查找**: 哈希表 (`CYHHashTable`)
- **网络**: IOCP异步模型
- **数据库**: 多线程操作 (`DBThread`)

---

*文档维护: 开发团队 | 最后更新: 2026-02-20 | 项目状态: 活跃开发中*
