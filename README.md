# MHGame - 韩式MMORPG游戏系统

## 快速开始

### 1. 系统检查
```batch
D:\mxym\启动完整系统.bat
```

### 2. 测试客户端（离线模式）
```batch
双击: D:\mxym\PlayDH\MHExecuter.exe
```

### 3. GM管理工具
```batch
python D:\mxym\GMTools\GM命令系统.py
```

### 4. 自定义配置
```batch
python D:\mxym\CustomLauncher\launcher_config.py
```

## 测试账号

| 账号 | 密码 | GM等级 | 说明 |
|------|------|--------|------|
| admin | admin123 | 9 | 超级管理员 |
| gm | gm123 | 5 | GM助手 |
| test | test123 | 0 | 普通玩家 |

## 系统要求

- Windows 7/10/11
- DirectX 9.0c
- SQL Server Express (服务器端，可选)
- Python 3.8+ (工具脚本)

## 文档

- [完整项目文档](完整项目文档.md) - 详细说明
- [项目构建方案](PROJECT_PLAN.md) - 开发计划
- [CLAUDE.md](CLAUDE.md) - 代码架构

## 功能特性

### ✅ 已实现
- 客户端编译和运行
- SQLite数据库系统
- GM命令系统
- 自定义启动器配置
- 官网和注册页面定制

### 🔄 进行中
- SQL Server数据库集成
- 服务器完整启动
- 客户端-服务器连接

### 📋 计划中
- Web注册页面
- GM Web管理面板
- 自动更新系统
- 游戏监控面板

## 目录结构

```
D:\mxym\
├── PlayDH/          # 客户端运行目录
├── Server/          # 服务器运行目录
├── Database/        # 数据库
├── GMTools/         # GM工具
├── CustomLauncher/  # 自定义启动器
├── [Client]MH/      # 客户端源代码
├── [Server]*/       # 服务器源代码
├── [CC]*/           # 共享代码
└── [Lib]*/          # 库文件
```

## 故障排除

### 服务器无法启动
- 检查数据库连接 (Server/masInfo.ini)
- 安装SQL Server Express
- 查看 Server/Log/ 日志

### 客户端无法连接
- 检查服务器是否运行
- 检查 PlayDH/server.txt 配置
- 检查防火墙设置

### GM命令不工作
- 验证GM账号等级
- 检查命令拼写
- 查看服务器日志

## 开发团队

- 源代码: 韩式MMORPG
- 编译修复: Claude AI
- 系统集成: Claude AI

## 许可证

私有项目，仅供学习使用。

---

**最后更新**: 2026-02-10
**状态**: 核心系统完成，等待数据库集成
