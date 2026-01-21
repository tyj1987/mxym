# Visual Studio项目升级指南

## 概述

本指南提供了将MMORPG项目从Visual Studio .NET 2003升级到现代版本的详细步骤。

## 升级路径选择

### 选项1: VS 2010 (推荐 - 最小改动)

**优点:**
- 与老项目兼容性最好
- .vcproj到.vcxproj转换相对平滑
- 仍可编译老代码
- 改动量最小

**缺点:**
- 不支持现代C++特性
- 不再受微软官方支持
- 部分过时API无警告

**适用场景:**
- 需要快速启动项目
- 保持现有代码不变
- 短期开发计划

### 选项2: VS 2022 (推荐 - 长期支持)

**优点:**
- 支持C++17/20标准
- 官方长期支持到2032年
- 更好的 IntelliSense 和调试工具
- 性能优化和安全性提升
- 现代 Windows SDK

**缺点:**
- 需要修改更多代码
- 需要更新Windows SDK
- 部分过时API需要替换

**适用场景:**
- 长期开发计划
- 希望使用现代C++特性
- 需要官方技术支持

## VS 2010 升级步骤

### 准备工作

1. **安装Visual Studio 2010 SP1**

   下载地址: https://my.visualstudio.com/Downloads?q=Visual%20Studio%202010

   必需组件:
   - Visual C++ 2010
   - Visual Studio 2010 SP1
   - Windows 7.1 SDK (如果需要)

2. **备份项目**

   ```batch
   xcopy D:\mxym D:\mxym_backup\ /E /I /H /Y
   ```

3. **安装DirectX SDK June 2010**

   下载: https://www.microsoft.com/en-us/download/details.aspx?id=6812

### 升级过程

#### 方法A: 使用Visual Studio升级向导

1. 打开 Visual Studio 2010
2. File → Open → Project/Solution
3. 选择 `.sln` 文件
4. VS会自动显示升级向导
5. 选择 "Yes, upgrade the projects"
6. 选择备份选项 (推荐创建备份)
7. 完成升级

#### 方法B: 命令行批量升级

使用 `devenv` 命令行工具:

```batch
REM 升级单个解决方案
devenv "D:\mxym\[Client]MH\MHClient.sln" /Upgrade

REM 批量升级所有解决方案
for /r D:\mxym %%f in (*.sln) do (
    echo 升级: %%f
    devenv "%%f" /Upgrade
)
```

### 升级后修复

#### 1. 项目文件格式变化

```xml
<!-- 旧格式 .vcproj -->
<?xml version="1.0" encoding="ks_c_5601-1987"?>
<VisualStudioProject
    ProjectType="Visual C++"
    Version="7.10"
    ...

<!-- 新格式 .vcxproj -->
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="4.0" ...
```

#### 2. 常见编译问题

**问题1: 字符集编码**

```
错误: error C1083: 无法打开包括文件: "stdafx.h"
```

解决: 所有源文件保存为UTF-8 with BOM或本地编码

**问题2: MFC/ATL版本**

```
错误: fatal error C1189: #error:  此文件需要_MFC_VER
```

解决: 项目属性 → Configuration Properties → General → Use of MFC
设置为: Use Standard Windows Libraries 或 Use MFC in a Shared DLL

## VS 2022 升级步骤

### 准备工作

1. **安装Visual Studio 2022**

   下载: https://visualstudio.microsoft.com/downloads/

   必需 Workloads:
   - 使用C++的桌面开发
   - C++ CMake tools for VS (可选)

   必需 Individual Components:
   - Windows 10/11 SDK (最新版本)
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - C++ ATL for latest build tools
   - C++ MFC for latest build tools (如果使用MFC)

2. **安装.NET Framework 3.5** (某些工具需要)

   控制面板 → 程序和功能 → 启用或关闭Windows功能
   勾选 .NET Framework 3.5 (包括 .NET 2.0 和 3.0)

3. **备份项目**

   同VS 2010

### 升级过程

#### 自动升级

1. 打开 Visual Studio 2022
2. File → Open → Project/Solution
3. 选择 `.sln` 或 `.vcproj` 文件
4. VS会自动显示"One-way upgrade"对话框
5. 选择升级到最新工具集
6. 完成升级

#### 批量升级脚本

创建 `D:\mxym\upgrade_to_vs2022.ps1`:

```powershell
<#
    批量升级所有项目到VS 2022
#>

$ProjectRoot = "D:\mxym"
$DevEnv = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe"

Get-ChildItem -Path $ProjectRoot -Filter "*.sln" -Recurse | ForEach-Object {
    Write-Host "升级: $($_.FullName)"
    & $DevEnv $_.FullName /Upgrade
}

Get-ChildItem -Path $ProjectRoot -Filter "*.vcproj" -Recurse | ForEach-Object {
    Write-Host "升级: $($_.FullName)"
    & $DevEnv $_.FullName /Upgrade
}
```

### 代码修改清单

#### 1. 标准库冲突

```cpp
// 问题代码 (CommonDefine.h)
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

// 修改方案1: 使用宏保护
#ifndef NOMINMAX
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

// 修改方案2: 使用内联函数
template<typename T>
inline constexpr const T& mymax(const T& a, const T& b) {
    return a > b ? a : b;
}

template<typename T>
inline constexpr const T& mymin(const T& a, const T& b) {
    return a < b ? a : b;
}
```

#### 2. 不安全的CRT函数

```cpp
// 问题代码
strcpy(dest, src);
sprintf(buffer, "Value: %d", value);
strcat(dest, more);

// 解决方案A: 禁用警告
// 在项目属性中: C/C++ → Preprocessor → Preprocessor Definitions
添加: _CRT_SECURE_NO_WARNINGS

// 解决方案B: 使用安全版本
strcpy_s(dest, destSize, src);
sprintf_s(buffer, bufferSize, "Value: %d", value);
strcat_s(dest, destSize, more);
```

#### 3. for循环变量作用域

```cpp
// 问题代码 (C++标准变化)
for(int i = 0; i < 10; i++) { }
// i 在这里仍然可见 (老编译器)

// 修改 (现代C++)
for(int i = 0; i < 10; i++) { }
int result = i; // 错误! i 不可见

// 解决: 在循环外声明
int i;
for(i = 0; i < 10; i++) { }
int result = i; // 正确
```

#### 4. 异常规范

```cpp
// 问题代码 (已弃用的语法)
void Function() throw();  // 无异常
void Function() throw(int); // 抛出int异常

// 修改为现代C++
void Function() noexcept;  // C++11
void Function(); // 允许任何异常
```

#### 5. auto_ptr 替换

```cpp
// 问题代码
#include <memory>
std::auto_ptr<MyClass> ptr(new MyClass());

// 修改为C++11
#include <memory>
std::unique_ptr<MyClass> ptr(new MyClass());
// 或
std::shared_ptr<MyClass> ptr = std::make_shared<MyClass>();
```

#### 6. bind1st/bind2nd 替换

```cpp
// 问题代码
#include <functional>
std::bind1st(std::minus<int>(), 10)
std::bind2nd(std::greater<int>(), 5)

// 修改为C++11
std::bind(std::minus<int>(), std::placeholders::_1, 10)
std::bind(std::greater<int>(), std::placeholders::_1, 5)

// 或使用lambda (推荐)
[](int x) { return x - 10; }
[](int x) { return x > 5; }
```

#### 7. std::binary_function 废弃

```cpp
// 问题代码
struct MyCompare : std::binary_function<int, int, bool> {
    bool operator()(int a, int b) const {
        return a < b;
    }
};

// 修改为C++11
struct MyCompare {
    bool operator()(int a, int b) const {
        return a < b;
    }
};
```

#### 8. 随机数生成

```cpp
// 问题代码 (rand() 不推荐)
srand((unsigned)time(NULL));
int r = rand() % 100;

// 修改为C++11
#include <random>
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(0, 99);
int r = dis(gen);
```

#### 9. nullptr 替代 NULL

```cpp
// 问题代码
void* ptr = NULL;
MyClass* obj = NULL;

// 修改为C++11
void* ptr = nullptr;
MyClass* obj = nullptr;
```

#### 10. override 和 final 关键字

```cpp
// 问题代码
class Derived : public Base {
    void VirtualFunction();
};

// 修改为C++11 (添加override确保重写)
class Derived : public Base {
    void VirtualFunction() override;
};

// 防止进一步继承
class FinalClass final : public Base {
};
```

### DirectX升级到9.0c

如果选择升级DirectX:

#### 头文件变更

```cpp
// 旧版
#include <d3d8.h>
#include <d3dx8.h>

// 新版
#include <d3d9.h>
#include <d3dx9.h>
```

#### 接口名称变更

```cpp
// 全局替换
LPDIRECT3D8         → LPDIRECT3D9
LPDIRECT3DDEVICE8   → LPDIRECT3DDEVICE9
LPDIRECT3DTEXTURE8  → LPDIRECT3DTEXTURE9
LPDIRECT3DVERTEXBUFFER8  → LPDIRECT3DVERTEXBUFFER9
LPDIRECT3DINDEXBUFFER8   → LPDIRECT3DINDEXBUFFER9
```

#### 创建设备变更

```cpp
// 旧版
LPDIRECT3D8 g_pD3D = Direct3DCreate8(D3D_SDK_VERSION);

// 新版
LPDIRECT3D9 g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
```

#### Present参数变更

```cpp
// 旧版
g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

// 新版 (参数不变, 但建议明确)
g_pd3dDevice->Present(0, 0, 0, 0);
```

#### 链接库变更

```xml
<!-- 项目设置 -->
<!-- 旧版 -->
<AdditionalDependencies>d3dx8.lib d3d8.lib</AdditionalDependencies>

<!-- 新版 -->
<AdditionalDependencies>d3dx9.lib d3d9.lib</AdditionalDependencies>
```

#### 代码批量替换

使用Visual Studio的查找替换 (Ctrl+H):

1. 查找: `d3d8\.h`, 替换: `d3d9.h`
2. 查找: `d3dx8\.h`, 替换: `d3dx9.h`
3. 查找: `D3D8_`, 替换: `D3D9_`
4. 查找: `IDirect3D8`, 替换: `IDirect3D9`
5. 查找: `IDirect3DDevice8`, 替换: `IDirect3DDevice9`
6. 查找: `Direct3DCreate8`, 替换: `Direct3DCreate9`
7. 查找: `d3dx8\.lib`, 替换: `d3dx9.lib`
8. 查找: `d3d8\.lib`, 替换: `d3d9.lib`

### 运行时库统一

**统一所有项目使用动态运行时库:**

| 配置 | 运行时库 | 预处理器定义 |
|------|----------|--------------|
| Debug | Multi-threaded Debug DLL (/MDd) | _DEBUG;_MT |
| Release | Multi-threaded DLL (/MD) | NDEBUG;_MT |

**修改步骤:**

1. 打开项目属性
2. Configuration Properties → C/C++ → Code Generation
3. Runtime Library:
   - Debug: Multi-threaded Debug DLL (/MDd)
   - Release: Multi-threaded DLL (/MD)
4. Apply to all configurations

**批量修改PowerShell脚本:**

```powershell
<#
    统一运行时库设置为 /MD
#>

$ProjectRoot = "D:\mxym"

Get-ChildItem -Path $ProjectRoot -Filter "*.vcxproj" -Recurse | ForEach-Object {
    [xml]$proj = Get-Content $_.FullName

    $proj.Project.ItemDefinitionGroup.ClCompile.RuntimeLibrary | ForEach-Object {
        if ($_.Parent.Parent.Condition -match "Debug") {
            $_."#text" = "MultiThreadedDebugDLL"
        } else {
            $_."#text" = "MultiThreadedDLL"
        }
    }

    $proj.Save($_.FullName)
    Write-Host "已更新: $($_.FullName)"
}
```

## 路径问题修复

### 创建统一配置

创建 `D:\mxym\config\build.props`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <!-- 项目根目录 -->
    <ProjectRoot>$(MSBuildProjectDirectory)\..</ProjectRoot>

    <!-- 输出目录 -->
    <OutDir>$(ProjectRoot)\Bin\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>$(ProjectRoot)\Obj\$(ProjectName)\$(Platform)\$(Configuration)\</IntDir>

    <!-- 库目录 -->
    <LibDir>$(ProjectRoot)\Lib\$(Platform)\$(Configuration)\</LibDir>
    <DllDir>$(ProjectRoot)\DLL\$(Platform)\$(Configuration)\</DllDir>

    <!-- 第三方SDK -->
    <DirectXSdkPath>$(Registry:HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\DirectX SDK@InstalledPath)</DirectXSdkPath>

    <!-- 如果注册表查找失败，使用默认路径 -->
    <DirectXSdkPath Condition="'$(DirectXSdkPath)'==''">C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)</DirectXSdkPath>
  </PropertyGroup>

  <ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>$(ProjectRoot)\[CC]Header;$(ProjectRoot)\[Lib]YHLibrary;$(DirectXSdkPath)\Include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
      <AdditionalLibraryDirectories>$(LibDir);$(DllDir);$(DirectXSdkPath)\Lib\x86;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
    </Link>
  </ItemDefinitionGroup>
</Project>
```

### 在项目中引用

在每个 `.vcxproj` 文件末尾添加:

```xml
  <ImportGroup Label="ExtensionTargets">
    <Import Project="..\..\config\build.props" />
  </ImportGroup>
</Project>
```

## 测试编译

### 分阶段测试

```
阶段1: 基础库
  □ YHLibrary
  □ HSEL
  □ ZipArchive

阶段2: 基础DLL
  □ BaseNetwork
  □ DBThread

阶段3: 中间服务层
  □ SoundLib
  □ 4DyuchiGXGeometry
  □ 4DyuchiGXExecutive
  □ 4DyuchiFileStorage
  □ 4DyuchiNET
  □ 4DyuchiGX_Render

阶段4: 服务器
  □ MapServer
  □ AgentServer
  □ DistributeServer
  □ MonitoringServer

阶段5: 客户端
  □ MHClient
  □ MHAutoPatch
  □ Selupdate

阶段6: 工具
  □ RegenTool
  □ PackingTool
  □ ServerTool
```

### 常见错误和解决方案

#### 错误: MSB8040

```
MSB8040: Spectre-mitigated libraries are required for this project.
```

解决:
1. 安装 Spectre 缓解库
2. 或禁用: 项目属性 → C/C++ → Code Generation → Spectre Mitigation = No

#### 错误: LNK2038

```
mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
```

解决: 统一所有项目的 RuntimeLibrary 设置

#### 错误: C1083

```
fatal error C1083: 无法打开包括文件: 'd3d8.h': No such file or directory
```

解决:
1. 安装DirectX SDK
2. 检查项目包含路径设置

## 升级验证清单

完成升级后，验证以下项目:

- [ ] 所有解决方案成功升级
- [ ] 所有项目文件(.vcxproj)格式正确
- [ ] 基础库能够编译通过
- [ ] 中间层DLL能够编译通过
- [ ] 服务器程序能够编译通过
- [ ] 客户端程序能够编译通过
- [ ] 运行时库设置一致
- [ ] 包含路径正确
- [ ] 库路径正确
- [ ] 输出目录正确
- [ ] 无重大编译警告
- [ ] 程序能够正常启动
- [ ] 基本功能测试通过

## 回滚计划

如果升级失败:

```batch
REM 恢复备份
rd /s /q D:\mxym
robocopy D:\mxym_backup\ D:\mxym\ /E /XO

REM 或使用Git (如果有版本控制)
cd D:\mxym
git reset --hard HEAD
```

## 技术支持

- Visual Studio文档: https://docs.microsoft.com/en-us/cpp/
- DirectX文档: https://docs.microsoft.com/en-us/windows/win32/directx
- Stack Overflow: 搜索 "visual studio upgrade" + 具体错误信息
