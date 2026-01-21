# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个大型韩式MMORPG游戏开发项目，使用C++和Visual Studio开发。项目包含客户端、服务器端、游戏库和工具集等多个组件。

## 构建系统

### 核心构建工具
- **Visual Studio 2008/2010** - 主要IDE和构建系统
- **解决方案文件 (.sln)** - 每个模块都有独立的解决方案文件
- **项目文件 (.vcproj)** - Visual C++项目配置

### 主要构建配置

项目支持多种构建配置，针对不同地区：

**调试配置：**
- `Debug Connect` - 调试连接版本
- `Debug GM_TOOL` - GM工具调试版
- `Debug GM_TOOL China/Japan/HK/TL` - 各地区GM工具调试版

**发布配置：**
- `Release` - 标准发布版本
- `Semi Release` - 半发布版本
- `Semi ReleaseChina/Japan/HK/TL` - 各地区半发布版本

### 构建命令

使用Visual Studio IDE或命令行：
```
devenv [Client]MH\MHClient.sln /build "Debug Connect"
msbuild [Client]MH\MHClient.sln /p:Configuration="Debug Connect"
```

### 依赖库

**核心依赖：**
- DirectX 8.1 SDK
- Windows API
- Winsock2 (网络通信)
- DirectInput8 (输入处理)
- 自定义库：YHLibrary, BaseNetwork, DBThread

## 代码架构

### 模块化架构

项目采用清晰的模块化设计，按功能划分：

**客户端模块 (`[Client]MH/`)：**
- `MHClient.cpp` - 客户端入口点
- `MainGame.h/cpp` - 主游戏逻辑
- `Interface/` - UI组件系统
- `Input/` - 输入处理和IME支持

**服务器模块：**
- `[Server]Agent/` - 代理服务器
- `[Server]Distribute/` - 分发服务器
- `[Server]Map/` - 地图服务器
- `[Monitoring]Server/` - 监控服务器

**游戏核心 (`[CC]` 前缀目录)：**
- `[CC]Header/` - 共享头文件
- `[CC]Ability/` - 技能系统
- `[CC]BattleSystem/` - 战斗系统
- `[CC]Quest/` - 任务系统
- `[CC]Skill/` - 技能管理
- `[CC]ServerModule/` - 服务器核心模块

**工具库 (`[Lib]` 前缀目录)：**
- `[Lib]YHLibrary/` - 核心游戏库（链表、网络、文件I/O）
- `[Lib]BaseNetwork/` - 网络抽象层
- `[Lib]DBThread/` - 数据库线程处理
- `[Lib]HSEL/` - 哈希/加密工具
- `[Lib]ZipArchive/` - 压缩库

**工具集 (`[Tool]` 前缀目录)：**
- `[Tool]PackingMan/` - 资源打包工具
- `[Tool]Regen/` - 生成工具
- `[Tool]DS_RMTool/` - 数据库工具

### 核心系统

**GameResourceManager**
- 位置：`[CC]Header/GameResourceManager.h`
- 职责：管理游戏资源、NPC、怪物、物品数据
- 单例模式：使用 `GAMERESRCMNGR` 宏访问

**网络系统**
- 自定义网络协议实现
- 消息解析器：`DistributeNetworkMsgParser`
- 加密通信支持（MD5, 自定义加密）

**文件系统**
- 自定义文件格式：`MHFile.cpp`
- 资源打包系统
- 支持加密资源文件

## 编码规范

### 命名约定

**匈牙利 notation：**
- 成员变量：`m_` 前缀 (如 `m_bMonsterRegenLoad`)
- 指针：`p` 前缀 (如 `pObject`)
- 布尔值：`b` 前缀 (如 `m_bLoad`)
- 字：`w` 前缀 (如 `wExpPoint`)
- DWORD：`dw` 前缀

**类命名：**
- 全局类：`c` 前缀 (如 `cWindowManager`, `cGameClient`)
- 接口/抽象类：`I` 前缀

### 单例模式

使用自定义宏实现单例：
```cpp
// 声明
EXTERNGLOBALTON(ClassName);
// 定义
GLOBALTON(ClassName);
// 使用
USINGTON(ClassName)->Method();
```

**常用单例宏：**
- `GAMERESRCMNGR` - GameResourceManager
- `USINGTON(className)` - 获取单例指针

### 调试宏

```cpp
ASSERT(condition);              // 断言
ASSERTMSG(condition, msg);      // 带消息断言
CRITICALASSERT(condition);      // 关键断言（仅Debug）
DEBUGMSG(condition, msg);       // 调试消息
DEBUG(code);                    // Debug代码块
```

### 内存管理

```cpp
SAFE_DELETE(pointer);           // 安全删除
SAFE_DELETE_ARRAY(array);       // 安全删除数组
SAFE_RELEASE(interface);        // 安全释放COM接口
```

### 预处理器定义

**地区相关：**
- `TAIWAN_LOCAL` - 台湾版本
- `CHINA_LOCAL` - 中国版本
- `JAPAN_LOCAL` - 日本版本
- `HK_LOCAL` - 香港版本

**功能开关：**
- `_GMTOOL_` - GM工具版本
- `_HACK_SHIELD_` - 反作弊系统
- `_NO_NPROTECT_START` - 禁用保护
- `_MHCLIENT_` - 客户端代码
- `_ASSERTENABLE_` - 启用断言（客户端）

### 字符串处理

特殊字符定义：
```cpp
#define TEXT_DELIMITER    '^'    // 分隔符
#define TEXT_NEWLINECHAR  'n'    // 换行符
#define TEXT_TABCHAR      't'    // 制表符
#define TEXT_SPACECHAR    's'    // 空格
#define TEXT_EMPTYCHAR    'e'    // 空字符
```

## 项目结构关键点

### 头文件包含路径

主要包含目录：
```
[CC]Header/         - 共享游戏头文件
[Lib]YHLibrary/     - 核心库头文件
[CC]BattleSystem/   - 战斗系统
[CC]Skill/          - 技能系统
[CC]Ability/        - 能力系统
```

### 资源管理

- 使用预编译头：`stdafx.h`/`StdAfx.cpp`
- 资源文件使用自定义格式
- 支持资源加密和压缩

### 多地区支持

项目通过预处理器定义支持多个亚洲地区：
- 编译时根据地区定义选择不同功能
- 地区特定的配置和资源
- 多语言支持（中文、日文等）

## 开发注意事项

### 版本控制
- 使用 **Perforce SCM** 进行版本控制
- 项目文件包含Perforce配置信息

### 反作弊系统
- 集成HackShield (`_HACK_SHIELD_`)
- 自定义安全措施
- 加密网络通信

### 性能优化
- 使用内存池（`CMemoryPoolTempl`）
- 哈希表（`CYHHashTable`）
- 预加载系统（特效、资源）
- 多线程数据库操作

### 调试技巧
- 使用Visual Studio调试器
- 通过预处理器定义控制调试输出
- Map文件生成用于发布版本调试

## 常用文件位置

**入口点：**
- 客户端：`[Client]MH/MHClient.cpp`
- 主游戏：`[Client]MH/MainGame.cpp`

**核心管理器：**
- 游戏资源：`[CC]Header/GameResourceManager.h`
- 用户管理：`[CC]Header/UserManager.h`
- 网络消息：`[Server]Distribute/DistributeNetworkMsgParser.h`

**工具：**
- 资源打包：`[Tool]PackingMan/`
- 地图编辑器：`4DyuchiGXMapEditor/`
- 模型查看器：`ModelView/`

## 地图和场景系统

**分辨率设置：**
- 中分辨率：1024x768 (`MIDRSLTN_W`, `MIDRSLTN_H`)
- 低分辨率：800x600 (`LOWRSLTN_W`, `LOWRSLTN_H`)
- 动态获取：`GET_MAINWIN_W`, `GET_MAINWIN_H`

## 数学工具

```cpp
DEGTORAD(deg)         // 角度转弧度
RADTODEG(rad)         // 弧度转角度
VECTORTORAD(vec)      // 向量转弧度
random(min, max)      // 随机数
LargeRandom(min, max) // 大范围随机数
```
