# MHGame 完整部署指南

> 完整可商用系统部署文档

---

## 目录

1. [系统要求](#系统要求)
2. [安装步骤](#安装步骤)
3. [数据库配置](#数据库配置)
4. [服务器配置](#服务器配置)
5. [客户端配置](#客户端配置)
6. [启动系统](#启动系统)
7. [故障排除](#故障排除)

---

## 系统要求

### 硬件要求

| 组件 | 最低要求 | 推荐配置 |
|------|---------|---------|
| CPU | 双核 2.0GHz | 四核 3.0GHz+ |
| 内存 | 4GB | 8GB+ |
| 硬盘 | 10GB | 20GB+ SSD |
| 网络 | 100Mbps | 1Gbps |

### 软件要求

| 软件 | 版本 | 用途 |
|------|------|------|
| Windows Server | 2012 R2+ / Windows 10+ | 操作系统 |
| SQL Server | Express 2019+ / Standard | 数据库 |
| DirectX | 9.0c / SDK June 2010 | 图形渲染 |
| Visual C++ Runtime | 2008, 2010, 2015-2022 | 运行库 |

---

## 安装步骤

### 1. 安装 SQL Server

```powershell
# 下载 SQL Server Express 2019
# https://www.microsoft.com/en-us/sql-server/sql-server-downloads

# 安装时选择:
# - 数据库引擎服务
# - SQL Server Management Studio (可选)
# - 混合模式认证 (Windows + SQL Server)
```

### 2. 安装 DirectX

```batch
# 运行 DirectX 安装程序
DXSETUP.exe
```

### 3. 安装 Visual C++ Runtime

```batch
# 安装以下运行库:
# - vcredist_x86_2008.exe
# - vcredist_x86_2010.exe
# - vcredist_x86_2015-2022.exe
```

---

## 数据库配置

### 方法1: 使用自动化脚本 (推荐)

```powershell
# 1. 启动 SQL Server
net start MSSQL$SQLEXPRESS

# 2. 运行数据库还原脚本
.\restore_database.ps1

# 3. 验证数据库
sqlcmd -S .\SQLEXPRESS -Q "SELECT name FROM sys.databases"
```

### 方法2: 手动还原

```sql
-- 在 SQL Server Management Studio 中执行:

-- 1. 还原 MHCMEMBER 数据库
RESTORE DATABASE MHCMEMBER
FROM DISK = 'D:\mxym\数据库\MHCMEMBER.bak'
WITH MOVE 'MHCMEMBER_Data' TO 'C:\Program Files\...\DATA\MHCMEMBER.mdf',
     MOVE 'MHCMEMBER_Log' TO 'C:\Program Files\...\DATA\MHCMEMBER_log.ldf',
     REPLACE;

-- 2. 还原 MHGAME 数据库
RESTORE DATABASE MHGAME
FROM DISK = 'D:\mxym\数据库\MHGAME.bak'
WITH MOVE 'MHGAME_Data' TO 'C:\Program Files\...\DATA\MHGAME.mdf',
     MOVE 'MHGAME_Log' TO 'C:\Program Files\...\DATA\MHGAME_log.ldf',
     REPLACE;

-- 3. 还原 MHLOG 数据库
RESTORE DATABASE MHLOG
FROM DISK = 'D:\mxym\数据库\MHLOG.bak'
WITH MOVE 'MHLOG_Data' TO 'C:\Program Files\...\DATA\MHLOG.mdf',
     MOVE 'MHLOG_Log' TO 'C:\Program Files\...\DATA\MHLOG_log.ldf',
     REPLACE;

-- 4. 创建登录用户
CREATE LOGIN mhgame WITH PASSWORD = 'MHGame@2026!';

-- 5. 授权
USE MHCMEMBER;
CREATE USER mhgame FOR LOGIN mhgame;
ALTER ROLE db_owner ADD MEMBER mhgame;

USE MHGAME;
CREATE USER mhgame FOR LOGIN mhgame;
ALTER ROLE db_owner ADD MEMBER mhgame;

USE MHLOG;
CREATE USER mhgame FOR LOGIN mhgame;
ALTER ROLE db_owner ADD MEMBER mhgame;
```

### 数据库信息

| 数据库 | 用途 | 大小 |
|--------|------|------|
| MHCMEMBER | 账号、角色、会员 | ~8MB |
| MHGAME | 游戏数据、物品、公会 | ~11MB |
| MHLOG | 日志记录 | ~800KB |

---

## 服务器配置

### 配置文件位置

| 文件 | 用途 |
|------|------|
| Server/masInfo.ini | 监控代理服务器 |
| Server/msInfo.ini | 监控服务器 |
| Server/asInfo.ini | 代理服务器 |
| Server/serverset/serverset.txt | 服务器集编号 |

### 数据库连接配置

所有服务器配置文件的数据库部分:

```ini
[Database]
DBIP=127.0.0.1
DBPort=1433
DBName=MHCMEMBER
DBUser=mhgame
DBPassword=MHGame@2026!
```

### 端口配置

| 服务器 | 端口 | 配置文件 |
|--------|------|---------|
| Monitoring Server | 19001 | msInfo.ini |
| Monitoring Agent | 20001 | masInfo.ini |
| Agent Server | 17001 | asInfo.ini |
| Map Server | 18001 | (内部配置) |

### 防火墙设置

```powershell
# 添加防火墙规则
netsh advfirewall firewall add rule name="MHGame Servers" dir=in action=allow protocol=tcp localport=17001-18001,19001,20001
```

---

## 客户端配置

### PlayDH/server.txt

```ini
[config]
name=MHGame Server
myip=127.0.0.1
home=http://localhost
regs=http://localhost/register
news=http://localhost/news
upup=Resource.zip|http://localhost/Resource.zip
```

### 游戏资源文件

确保以下 .pak 文件存在于 PlayDH/ 目录:

| 文件 | 大小 | 内容 |
|------|------|------|
| Character.pak | 240MB | 角色模型 |
| Effect.pak | 207MB | 特效 |
| Map.pak | 557MB | 地图数据 |
| monster.pak | 167MB | 怪物模型 |
| npc.pak | 36MB | NPC模型 |
| Pet.pak | 34MB | 宠物模型 |
| Titan.pak | 27MB | 泰坦模型 |

---

## 启动系统

### 自动启动 (推荐)

```powershell
# 启动完整系统 (服务器 + 客户端)
.\start_system.ps1

# 仅启动服务器
.\start_system.ps1 -SkipClient

# 仅启动客户端 (服务器已运行)
.\start_system.ps1 -SkipServers

# 停止系统
.\stop_system.ps1
```

### 手动启动

```batch
cd Server

:: 1. 启动监控服务器 (MAS模式)
MonitoringServer.exe 1

:: 2. 等待3秒

:: 3. 启动代理服务器
AgentServer.exe

:: 4. 等待2秒

:: 5. 启动地图服务器
MapServer.exe

:: 6. 启动客户端
cd ..\PlayDH
MHClient-Connect.exe
```

### 服务器启动顺序 (重要!)

```
1. MonitoringServer.exe (端口 20001)
   └─ 等待 3 秒

2. AgentServer.exe (端口 17001)
   └─ 等待 2 秒

3. MapServer.exe (端口 18001)

4. DistributeServer.exe (端口 16001) [可选]
```

---

## 验证系统

### 检查服务器状态

```powershell
.\verify_system.ps1
```

### 检查数据库连接

```powershell
sqlcmd -S .\SQLEXPRESS -U mhgame -P "MHGame@2026!" -Q "SELECT COUNT(*) FROM MHCMEMBER.dbo.USER_INFO"
```

### 检查端口监听

```batch
netstat -an | findstr "LISTENING" | findstr "17001 18001 20001"
```

---

## 故障排除

### 服务器无法连接数据库

1. 检查 SQL Server 服务状态:
```powershell
Get-Service MSSQL`$SQLEXPRESS
```

2. 检查混合模式认证是否启用:
```sql
-- 在 SSMS 中执行
EXEC xp_instance_regread N'HKEY_LOCAL_MACHINE',
    N'Software\Microsoft\MSSQLServer\MSSQLServer',
    N'LoginMode';
-- 应返回 2
```

3. 检查用户名密码是否正确

### 客户端无法连接服务器

1. 检查服务器是否运行:
```batch
tasklist | findstr "Server"
```

2. 检查防火墙:
```batch
netsh advfirewall show allprofiles
```

3. 检查 PlayDH/server.txt 中的 IP 配置

### 服务器启动失败

1. 查看日志文件:
```batch
type Server\Log\debug.txt
```

2. 检查端口占用:
```batch
netstat -ano | findstr "17001"
```

3. 检查配置文件路径

---

## 生产环境建议

### 安全建议

1. 修改默认数据库密码
2. 使用 Windows 防火墙限制访问
3. 启用 SQL Server 加密连接
4. 定期备份数据库

### 性能优化

1. 使用 SSD 存储数据库
2. 增加 SQL Server 内存配置
3. 定期重建索引
4. 监控服务器资源使用

### 运维建议

1. 设置日志轮转
2. 配置数据库自动备份
3. 监控服务器健康状态
4. 准备灾难恢复计划

---

## 联系支持

如遇问题，请检查:
1. 日志文件 (`Server/Log/`)
2. 事件查看器 (`eventvwr.msc`)
3. SQL Server 错误日志

---

*最后更新: 2026-02-20*
