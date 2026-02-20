# 故障排除指南 (Troubleshooting Guide)

> 编译错误 | 运行时错误 | 服务器问题 | 数据库问题

---

## 目录

1. [编译错误](#编译错误)
2. [链接错误](#链接错误)
3. [运行时错误](#运行时错误)
4. [服务器问题](#服务器问题)
5. [客户端问题](#客户端问题)
6. [数据库问题](#数据库问题)
7. [网络问题](#网络问题)
8. [性能问题](#性能问题)

---

## 编译错误

### C1083: 无法打开包含文件

**错误示例**:
```
fatal error C1083: Cannot open include file: 'd3dx9.h': No such file or directory
```

**原因**: DirectX SDK路径未正确配置

**解决方案**:
```batch
:: 1. 检查DirectX SDK是否安装
dir "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"

:: 2. 设置环境变量
setx DXSDK_DIR "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"

:: 3. 重启Visual Studio

:: 4. 验证项目设置
:: 项目属性 → VC++目录 → 包含目录: 添加 $(DXSDK_DIR)Include
:: 项目属性 → VC++目录 → 库目录: 添加 $(DXSDK_DIR)Lib\x86
```

### C2065: 未声明的标识符

**错误示例**:
```
error C2065: 'ASSERT': undeclared identifier
```

**原因**: 缺少必要的头文件包含

**解决方案**:
```cpp
// 确保包含核心定义头文件
#include "CommonDefine.h"
#include "CommonGameDefine.h"
```

**常见未声明标识符及对应头文件**:

| 标识符 | 头文件 |
|--------|--------|
| `ASSERT`, `SAFE_DELETE` | `CommonDefine.h` |
| `USINGTON`, `GAMERESRCMNGR` | `CommonDefine.h` |
| `random`, `DEGTORAD` | `CommonCalcFunc.h` |

### C2011: 类型重定义

**错误示例**:
```
error C2011: 'structName': 'struct' type redefinition
```

**原因**: 头文件重复包含，缺少包含保护

**解决方案**:
```cpp
// 方法1: 使用#pragma once
#pragma once
// ... 头文件内容

// 方法2: 使用传统的包含保护
#ifndef MYHEADER_H
#define MYHEADER_H
// ... 头文件内容
#endif
```

### C3861: 找不到标识符

**错误示例**:
```
error C3861: 'FunctionName': identifier not found
```

**原因**: 函数声明在使用之后，或缺少头文件

**解决方案**:
1. 确保函数声明在使用之前
2. 检查是否包含了正确的头文件
3. 检查命名空间

### C2512: 没有合适的默认构造函数

**错误示例**:
```
error C2512: 'ClassName': no appropriate default constructor available
```

**原因**: 类没有无参构造函数，但代码尝试默认构造

**解决方案**:
```cpp
// 添加默认构造函数
class ClassName {
public:
    ClassName() {}  // 默认构造函数
    ClassName(int value) : m_value(value) {}
private:
    int m_value = 0;  // 或使用成员初始化器
};
```

### C4996: 不安全的函数使用

**错误示例**:
```
warning C4996: 'strcpy': This function may be unsafe
```

**解决方案**:
```cpp
// 方法1: 使用安全版本
strcpy_s(dest, sizeof(dest), src);

// 方法2: 禁用该警告
#pragma warning(disable: 4996)

// 方法3: 定义宏
#define _CRT_SECURE_NO_WARNINGS
```

---

## 链接错误

### LNK2019: 无法解析的外部符号

**错误示例**:
```
error LNK2019: unresolved external symbol "void __cdecl FunctionName(...)"
```

**常见原因及解决方案**:

1. **依赖库未编译**
```batch
:: 检查库文件是否存在
dir Lib\x86\Debug\*.lib

:: 按顺序重新编译依赖
.\build_all.ps1 -Phase 1
.\build_all.ps1 -Phase 2
```

2. **库路径配置错误**
```batch
:: 检查项目设置
:: 项目属性 → 链接器 → 常规 → 附加库目录
:: 应包含: $(SolutionDir)..\Lib\x86\$(Configuration)
```

3. **函数声明与定义不匹配**
```cpp
// 检查声明和定义是否一致
// 声明: void Function(int a);
// 定义: void Function(int a) { ... }
```

### LNK2001: 无法解析的外部符号

**错误示例**:
```
error LNK2001: unresolved external symbol "public: static int ClassName::staticMember"
```

**原因**: 静态成员只有声明没有定义

**解决方案**:
```cpp
// 在.cpp文件中定义静态成员
// Header.h
class ClassName {
    static int staticMember;
};

// Implementation.cpp
int ClassName::staticMember = 0;  // 必须定义
```

### LNK2005: 符号重复定义

**错误示例**:
```
error LNK2005: "int globalVar" already defined in file.obj
```

**原因**: 全局变量在头文件中定义

**解决方案**:
```cpp
// Header.h
extern int globalVar;  // 声明

// Implementation.cpp
int globalVar = 0;  // 定义
```

### LNK1181: 无法打开输入文件

**错误示例**:
```
fatal error LNK1181: cannot open input file 'LibraryName.lib'
```

**解决方案**:
```batch
:: 检查库文件是否存在
dir Lib\x86\Debug\LibraryName.lib

:: 如果不存在，编译对应的库项目
msbuild "[Lib]LibraryName\LibraryName.sln" /p:Configuration=Debug /p:Platform=Win32
```

---

## 运行时错误

### 访问违规 (Access Violation)

**错误示例**:
```
Unhandled exception at 0x...: Access violation reading location 0x...
```

**常见原因**:
1. 空指针解引用
2. 数组越界
3. 使用已释放的内存

**调试方法**:
```cpp
// 1. 使用断言检查指针
ASSERT(ptr != NULL);

// 2. 使用调试输出
DEBUGMSG(ptr == NULL, "Pointer is NULL!");

// 3. 使用Visual Studio调试器
// 设置断点，检查调用堆栈
```

### 内存泄漏

**检测方法**:
```cpp
// 在程序开始处
_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

// 在特定位置检查
_CrtDumpMemoryLeaks();
```

**预防措施**:
```cpp
// 使用SAFE_DELETE宏
SAFE_DELETE(ptr);
SAFE_DELETE_ARRAY(arr);
SAFE_RELEASE(pInterface);

// 考虑使用智能指针 (C++11+)
std::unique_ptr<Type> ptr(new Type());
```

### 栈溢出

**错误示例**:
```
Stack overflow
```

**原因**:
1. 无限递归
2. 栈上分配大数组

**解决方案**:
```cpp
// 1. 检查递归终止条件
int RecursiveFunction(int n) {
    if (n <= 0) return 0;  // 终止条件
    return RecursiveFunction(n - 1);
}

// 2. 大数组使用堆分配
// 错误: int largeArray[10000000];
// 正确:
int* largeArray = new int[10000000];
// 使用后
delete[] largeArray;
```

---

## 服务器问题

### 服务器无法启动

**症状**: 双击服务器程序后立即退出或无响应

**诊断步骤**:
```batch
:: 1. 检查日志文件
type Server\Log\MS_Debug.txt
type Server\Log\MapServer.log

:: 2. 检查端口占用
netstat -ano | findstr "17001"
netstat -ano | findstr "18001"
netstat -ano | findstr "20001"

:: 3. 检查配置文件
type Server\serverset\serverset.txt
type Server\masInfo.ini

:: 4. 检查数据库连接
sc query MSSQLSERVER
```

**常见解决方案**:

1. **端口被占用**:
```batch
:: 查找占用端口的进程
netstat -ano | findstr ":17001"
:: 结果: TCP 0.0.0.0:17001 0.0.0.0:0 LISTENING 1234

:: 终止进程
taskkill /F /PID 1234
```

2. **配置文件错误**:
```batch
:: 检查serverset.txt中的配置编号
:: 应为1-4之间的数字
```

3. **缺少DLL**:
```batch
:: 检查依赖DLL是否存在
dir Server\*.dll

:: 常见需要的DLL:
:: - MSVCR90.dll, MSVCP90.dll (VC++ 2008)
:: - d3dx9_*.dll (DirectX)
```

### 服务器启动顺序错误

**症状**: 服务器之间无法通信

**正确启动顺序**:
```batch
:: 1. 监控服务器 (MAS模式)
MonitoringServer.exe 1

:: 等待5秒确认启动

:: 2. 代理服务器
AgentServer.exe

:: 等待5秒

:: 3. 地图服务器
MapServer.exe

:: 4. 分发服务器 (可选)
DistributeServer.exe
```

**或使用启动脚本**:
```batch
cd Server
StartServers.bat
```

### 服务器崩溃

**诊断步骤**:
```batch
:: 1. 检查是否有dump文件
dir Server\*.dmp

:: 2. 查看Windows事件日志
eventvwr.msc

:: 3. 使用调试器附加
devenv /debugexe Server\MapServer.exe
```

**常见崩溃原因**:
- 空指针访问
- 数据库查询失败
- 内存不足
- 配置数据异常

---

## 客户端问题

### 客户端无法连接服务器

**诊断步骤**:
```batch
:: 1. 确认服务器运行
tasklist | findstr "AgentServer"

:: 2. 检查网络连通性
ping <服务器IP>

:: 3. 检查端口
telnet <服务器IP> 17001

:: 4. 检查防火墙
netsh advfirewall show allprofiles
```

**解决方案**:

1. **防火墙设置**:
```batch
:: 添加防火墙规则
netsh advfirewall firewall add rule name="MHClient" dir=in action=allow program="D:\mxym\[Client]MH\MHClient.exe" enable=yes
```

2. **检查服务器IP配置**:
```batch
:: 确认serverInfo.ini中的IP正确
type Server\serverInfo.ini
```

### 客户端黑屏/闪退

**可能原因**:
1. DirectX初始化失败
2. 显卡驱动问题
3. 资源文件缺失

**解决方案**:
```batch
:: 1. 检查DirectX版本
dxdiag

:: 2. 更新显卡驱动

:: 3. 检查资源文件
dir *.pak
:: 应包含: Character.pak, Effect.pak, Map.pak等

:: 4. 以兼容模式运行
:: 右键 → 属性 → 兼容性 → Windows XP兼容模式
```

---

## 数据库问题

### 数据库连接失败

**错误示例**:
```
Cannot connect to database server
```

**诊断步骤**:
```batch
:: 1. 检查SQL Server服务
sc query MSSQLSERVER

:: 2. 如果未运行，启动服务
net start MSSQLSERVER

:: 3. 检查连接字符串
type Server\serverInfo.ini

:: 4. 测试连接
sqlcmd -S localhost -U sa -P password
```

**常见问题**:

1. **SQL Server未启动**:
```batch
net start MSSQLSERVER
```

2. **认证模式错误**:
```batch
:: 确保SQL Server启用了混合认证模式
:: SQL Server Management Studio → 服务器属性 → 安全性 → SQL Server和Windows身份验证模式
```

3. **防火墙阻止**:
```batch
:: 添加SQL Server防火墙规则
netsh advfirewall firewall add rule name="SQL Server" dir=in action=allow protocol=tcp localport=1433
```

### 数据库还原失败

**解决方案**:
```powershell
# 使用提供的还原脚本
.\RestoreDB_Final.ps1

# 或手动还原
sqlcmd -S localhost -Q "RESTORE DATABASE MHDB FROM DISK='D:\mxym\Database\MHDB.bak' WITH REPLACE"
```

---

## 网络问题

### 网络延迟高

**诊断**:
```batch
:: 检查网络状态
ping -t <服务器IP>

:: 检查网络利用率
netstat -e
```

**优化建议**:
1. 检查服务器网络带宽
2. 优化数据库查询
3. 减少不必要的网络消息

### 消息丢失

**可能原因**:
1. 网络不稳定
2. 服务器过载
3. 缓冲区溢出

**解决方案**:
1. 检查服务器日志中的错误
2. 增加网络缓冲区大小
3. 实现消息重发机制

---

## 性能问题

### 服务器CPU占用高

**诊断**:
```batch
:: 使用任务管理器查看
taskmgr

:: 使用性能监视器
perfmon
```

**优化方向**:
1. 检查AI计算复杂度
2. 优化数据库查询
3. 减少日志输出
4. 检查是否有死循环

### 内存占用过高

**诊断**:
```batch
:: 检查进程内存
tasklist /fi "imagename eq MapServer.exe" /fo table
```

**优化方向**:
1. 检查内存泄漏
2. 优化资源加载策略
3. 减少对象缓存

---

## 日志分析

### 关键日志文件

| 日志文件 | 位置 | 用途 |
|---------|------|------|
| MS_Debug.txt | Server/Log/ | 监控服务器日志 |
| MapServer_*.log | Server/Log/ | 地图服务器日志 |
| AgentServer_*.log | Server/Log/ | 代理服务器日志 |

### 常见日志错误

**"Bind Failed"**:
- 端口已被占用，更换端口或终止占用进程

**"Connect to DB Failed"**:
- 检查数据库服务和连接字符串

**"Accept Failed"**:
- 达到最大连接数，增加配置或检查资源

---

## 获取帮助

1. 查看 [QUICK_REFERENCE.md](QUICK_REFERENCE.md) 快速参考
2. 查看 [BUILD_GUIDE.md](BUILD_GUIDE.md) 编译指南
3. 查看 [TESTING_GUIDE.md](TESTING_GUIDE.md) 测试指南

---

*最后更新: 2026-02-20*
