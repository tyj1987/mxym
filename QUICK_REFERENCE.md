# MMORPG项目快速参考卡

## 快速开始

### 编译整个项目
```batch
# Release配置
build_all.bat Release

# Debug配置
build_all.bat Debug

# 清理后编译
build_all.bat Release clean
```

### 编译单个模块
```batch
# 编译基础库
build_module.bat YHLibrary Release

# 编译客户端
build_module.bat MHClient Release

# 编译服务器
build_module.bat MapServer Debug
```

### PowerShell高级编译
```powershell
# 并行编译所有模块
.\build_all.ps1 -Configuration Release -Parallel

# 从第3批开始编译
.\build_all.ps1 -Configuration Release -StartFrom 3
```

## 模块编译顺序

```
第1批 (基础库) → 第2批 (基础DLL) → 第3批 (中间服务层)
→ 第4批 (服务器) → 第5批 (客户端) → 第6批 (工具)
```

### 快速查找模块

| 批次 | 模块 | 用途 | 输出 |
|------|------|------|------|
| 1 | YHLibrary | 核心基础库 | YHLibrary.lib |
| 1 | HSEL | 加密库 | HSEL.lib |
| 1 | ZipArchive | 压缩库 | ZipArchive.lib |
| 2 | BaseNetwork | 网络库 | BaseNetwork.dll |
| 2 | DBThread | 数据库线程 | DBThread.dll |
| 3 | SoundLib | 音频库 | SoundLib.lib |
| 3 | 4DyuchiGX_Render | 渲染引擎 | 4DyuchiGXRenderer.dll |
| 3 | 4DyuchiNET | 网络服务 | Inetwork.dll |
| 4 | MapServer | 地图服务器 | MapServer.exe |
| 4 | AgentServer | 代理服务器 | AgentServer.exe |
| 4 | DistributeServer | 分发服务器 | DistributeServer.exe |
| 5 | MHClient | 游戏客户端 | MHClient-Connect.exe |
| 5 | MHAutoPatch | 自动更新 | MHAutoPatch.exe |
| 6 | RegenTool | 生成工具 | RegenTool.exe |
| 6 | PackingTool | 打包工具 | PackingTool.exe |

## 关键路径

```
项目根目录: D:\mxym
源代码: [Client]MH, [Server]*, [CC]*, [Lib]*
输出: Bin, Lib, DLL, Obj
配置: config\build.props
文档: BUILD_AND_UPDATE_PLAN.md, UPGRADE_GUIDE.md
```

## 常用配置

### 运行时库设置
- Debug: `/MDd` (Multi-threaded Debug DLL)
- Release: `/MD` (Multi-threaded DLL)

### DirectX SDK
- 版本: June 2010
- 路径: `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)`

### Visual Studio
- 推荐: VS 2022 或 VS 2010
- 必需组件: C++桌面开发, Windows 10/11 SDK

## 常见问题

### Q: 编译失败 "找不到d3d8.h"
**A:** 安装DirectX SDK June 2010

### Q: "MSB8040 Spectre-mitigated libraries"
**A:** 项目属性 → Spectre Mitigation = No

### Q: "LNK2038 RuntimeLibrary mismatch"
**A:** 统一所有项目使用 /MD 或 /MDd

### Q: 找不到SS3DGFunc.lib
**A:** 缺失的自定义库，需联系原开发团队

## 代码修改要点

### 不安全的CRT函数
```cpp
// 方案A: 禁用警告 (快速)
#define _CRT_SECURE_NO_WARNINGS

// 方案B: 使用安全版本 (推荐)
strcpy_s(dest, size, src);
```

### min/max冲突
```cpp
// 方案A: 使用宏保护
#ifndef NOMINMAX
#define max(x,y) ...
#endif

// 方案B: 取消定义
#undef min
#undef max
#include <algorithm>
```

### NULL替换为nullptr
```cpp
// 旧代码
void* ptr = NULL;

// 新代码 (C++11)
void* ptr = nullptr;
```

## 开发环境检查清单

- [ ] Visual Studio 2022 或 2010 已安装
- [ ] Windows 10/11 SDK 已安装
- [ ] DirectX SDK June 2010 已安装
- [ ] MFC/ATL 组件已安装 (如果使用)
- [ ] 项目已升级到现代VS格式
- [ ] 所有.vcproj已转换为.vcxproj
- [ ] build.props配置已正确设置
- [ ] 运行时库设置统一为/MD
- [ ] 基础库能够编译
- [ ] DirectX SDK路径正确

## 编译输出结构

```
D:\mxym\
├── Bin\
│   ├── x86\
│   │   ├── Debug\
│   │   │   ├── Client\MHClient-Connect.exe
│   │   │   ├── Server\MapServer.exe
│   │   │   └── Tools\RegenTool.exe
│   │   └── Release\
│   └── x64\
├── Lib\
│   ├── x86\
│   │   ├── Debug\YHLibrary.lib
│   │   └── Release\YHLibrary.lib
│   └── x64\
├── DLL\
│   ├── x86\
│   │   ├── Debug\BaseNetwork.dll
│   │   ├── Debug\4DyuchiGXRenderer.dll
│   │   └── Release\
│   └── x64\
└── Obj\
    ├── YHLibrary\
    ├── BaseNetwork\
    └── ...
```

## 性能优化建议

1. **启用并行编译**
   - MSBuild参数: `/m`
   - 最大并行任务数: `/m:8` (8核)

2. **使用SSD**
   - 将项目放在SSD上
   - 编译速度提升3-5倍

3. **增加内存**
   - 推荐: 16GB RAM
   - 最佳: 32GB RAM

4. **增量编译**
   - 只编译修改过的文件
   - 保持预编译头文件

5. **使用RAM磁盘**
   - 将Obj目录放在RAM磁盘
   - 进一步提升编译速度

## 调试技巧

### 生成调试符号
- Release版本也生成PDB文件
- 项目设置: Linker → Debugging → Generate Debug Info = Yes

### Map文件
- 用于崩溃分析
- 项目设置: Linker → Debugging → Generate Map File = Yes

### Visual Studio调试
- F5: 开始调试
- F9: 设置断点
- F10: 单步跳过
- F11: 单步进入
- Shift+F11: 单步跳出

### 日志系统
- 使用项目自带的DEBUG宏
- 查看 OutputDebugString 输出
- 使用DebugView工具查看日志

## 版本控制建议

### .gitignore
```
# 编译输出
Bin/
Lib/
DLL/
Obj/
*.obj
*.lib
*.dll
*.exe
*.pdb
*.ilk
*.exp

# IDE文件
.vs/
*.suo
*.user
*.vcxproj.filters

# 日志
build_log*.txt
*.log

# 临时文件
*.tmp
*.cache
```

### 提交前检查
- [ ] 项目能够成功编译
- [ ] 没有编译警告
- [ ] 测试通过
- [ ] 更新文档
- [ ] 提交信息清晰

## 获取帮助

### 文档
- `BUILD_AND_UPDATE_PLAN.md` - 完整编译方案
- `UPGRADE_GUIDE.md` - VS升级指南
- `CLAUDE.md` - 项目架构说明

### 在线资源
- Microsoft Docs: https://docs.microsoft.com/cpp
- DirectX Docs: https://docs.microsoft.com/windows/win32/directx
- Stack Overflow: 搜索错误信息

### 团队联系
- 技术负责人: [联系方式]
- 架构师: [联系方式]
- 项目经理: [联系方式]

---

**版本:** 1.0
**更新日期:** 2026-01-21
