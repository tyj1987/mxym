@echo off
REM ========================================
REM MMORPG项目完整编译脚本
REM 使用方法: build_all.bat [Debug|Release] [clean]
REM ========================================

setlocal enabledelayedexpansion

REM 参数解析
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release
set ACTION=%2

REM 配置变量
set PROJECT_ROOT=%~dp0
set BUILD_LOG=%PROJECT_ROOT%build_log_%CONFIG%.txt
set PARALLEL=%3

REM 清理选项
if /i "%ACTION%"=="clean" (
    echo ========================================
    echo 清理输出目录...
    echo ========================================

    if exist "%PROJECT_ROOT%Bin" (
        echo 删除: Bin\
        rmdir /s /q "%PROJECT_ROOT%Bin"
    )
    if exist "%PROJECT_ROOT%Lib" (
        echo 删除: Lib\
        rmdir /s /q "%PROJECT_ROOT%Lib"
    )
    if exist "%PROJECT_ROOT%DLL" (
        echo 删除: DLL\
        rmdir /s /q "%PROJECT_ROOT%DLL"
    )
    if exist "%PROJECT_ROOT%Obj" (
        echo 删除: Obj\
        rmdir /s /q "%PROJECT_ROOT%Obj"
    )

    echo 清理完成!
    goto :eof
)

REM 检查MSBuild可用性
where msbuild >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到 MSBuild
    echo 请确保已安装 Visual Studio
    echo.
    echo 如果使用 VS2022，运行:
    echo   call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
    pause
    exit /b 1
)

REM 开始编译
echo ======================================== > %BUILD_LOG%
echo MMORPG项目完整编译 [%CONFIG%] >> %BUILD_LOG%
echo 开始时间: %date% %time% >> %BUILD_LOG%
echo ======================================== >> %BUILD_LOG%
echo.
echo 开始编译 MMORPG 项目 [%CONFIG% 配置]
echo 日志文件: %BUILD_LOG%
echo.

REM 编译函数
:compile_module
set MODULE_NAME=%1
set PROJECT_FILE=%2
set DEPENDENCY=%3

echo [%time%] 编译: %MODULE_NAME% >> %BUILD_LOG%
echo 编译: %MODULE_NAME%...

if defined DEPENDENCY (
    echo   依赖: %DEPENDENCY%
)

set MSBUILD_OPTIONS=/p:Configuration=%CONFIG% /t:Rebuild /v:n /nologo

if /i not "%PARALLEL%"=="serial" (
    set MSBUILD_OPTIONS=%MSBUILD_OPTIONS% /m
)

msbuild "%PROJECT_FILE%" %MSBUILD_OPTIONS% >> %BUILD_LOG% 2>&1

if errorlevel 1 (
    echo [错误] %MODULE_NAME% 编译失败 >> %BUILD_LOG%
    echo [错误] %MODULE_NAME% 编译失败，详见日志
    echo.
    exit /b 1
) else (
    echo [成功] %MODULE_NAME% >> %BUILD_LOG%
    echo [成功] %MODULE_NAME%
)
echo.
goto :eof

REM ========== 第1批: 基础静态库 ==========
echo ======================================== >> %BUILD_LOG%
echo 第1批: 基础静态库 >> %BUILD_LOG%
echo ======================================== >> %BUILD_LOG%
echo.
echo [第1批] 编译基础静态库...
echo.

set "P1=%PROJECT_ROOT%[Lib]YHLibrary\YHLibrary.vcproj"
set "P2=%PROJECT_ROOT%[Lib]HSEL\HSEL.vcproj"
set "P3=%PROJECT_ROOT%[Lib]ZipArchive\ZipArchive.vcproj"

if exist "%P1%" call :compile_module "YHLibrary" "%P1%"
if exist "%P2%" call :compile_module "HSEL" "%P2%"
if exist "%P3%" call :compile_module "ZipArchive" "%P3%"

REM ========== 第2批: 基础功能DLL ==========
echo ======================================== >> %BUILD_LOG%
echo 第2批: 基础功能DLL >> %BUILD_LOG%
echo ======================================== >> %BUILD_LOG%
echo.
echo [第2批] 编译基础功能DLL...
echo.

set "P4=%PROJECT_ROOT%[Lib]BaseNetwork\BaseNetwork.vcproj"
set "P5=%PROJECT_ROOT%[Lib]DBThread\DBThread.vcproj"

if exist "%P4%" call :compile_module "BaseNetwork" "%P4%" "YHLibrary"
if exist "%P5%" call :compile_module "DBThread" "%P5%" "YHLibrary"

REM ========== 第3批: 中间服务层 ==========
echo ======================================== >> %BUILD_LOG%
echo 第3批: 中间服务层DLL >> %BUILD_LOG%
echo ======================================== >> %BUILD_LOG%
echo.
echo [第3批] 编译中间服务层DLL...
echo.

set "P6=%PROJECT_ROOT%SoundLib\SoundLib.vcproj"
set "P7=%PROJECT_ROOT%4DyuchiGXGeometry\4DyuchiGXGeometry.vcproj"
set "P8=%PROJECT_ROOT%4DYUCHIGXEXECUTIVE\4DyuchiGXExecutive.vcproj"
set "P9=%PROJECT_ROOT%4DyuchiFileStorage\4DyuchiFileStorage.vcproj"
set "P10=%PROJECT_ROOT%4DyuchiNET_Latest\I4DyuchiNET.vcproj"
set "P11=%PROJECT_ROOT%4DYUCHIGX_RENDER\4DyuchiGX_Render.vcproj"

if exist "%P6%" call :compile_module "SoundLib" "%P6%" "YHLibrary"
if exist "%P7%" call :compile_module "4DyuchiGXGeometry" "%P7%" "YHLibrary"
if exist "%P8%" call :compile_module "4DyuchiGXExecutive" "%P8%" "YHLibrary"
if exist "%P9%" call :compile_module "4DyuchiFileStorage" "%P9%" "YHLibrary"
if exist "%P10%" call :compile_module "4DyuchiNET" "%P10%" "YHLibrary, BaseNetwork"
if exist "%P11%" call :compile_module "4DyuchiGX_Render" "%P11%" "DirectX SDK"

REM ========== 第4批: 服务器端 ==========
echo ======================================== >> %BUILD_LOG%
echo 第4批: 服务器端应用 >> %BUILD_LOG%
echo ======================================== >> %BUILD_LOG%
echo.
echo [第4批] 编译服务器端应用...
echo.

set "P12=%PROJECT_ROOT%[Server]Map\MapServer.vcproj"
set "P13=%PROJECT_ROOT%[Server]Agent\AgentServer.vcproj"
set "P14=%PROJECT_ROOT%[Server]Distribute\DistributeServer.vcproj"
set "P15=%PROJECT_ROOT%[Monitoring]Server\MonitoringServer.vcproj"

if exist "%P12%" call :compile_module "MapServer" "%P12%" "YHLibrary, BaseNetwork"
if exist "%P13%" call :compile_module "AgentServer" "%P13%" "YHLibrary"
if exist "%P14%" call :compile_module "DistributeServer" "%P14%" "YHLibrary, BaseNetwork"
if exist "%P15%" call :compile_module "MonitoringServer" "%P15%" "YHLibrary"

REM ========== 第5批: 客户端 ==========
echo ======================================== >> %BUILD_LOG%
echo 第5批: 客户端应用 >> %BUILD_LOG%
echo ======================================== >> %BUILD_LOG%
echo.
echo [第5批] 编译客户端应用...
echo.

set "P16=%PROJECT_ROOT%[Client]MH\MHClient.vcproj"
set "P17=%PROJECT_ROOT%[Client]MHAutoPatch\MHAutoPatch.vcproj"
set "P18=%PROJECT_ROOT%[Client]Selupdate\Selupdate.vcproj"

if exist "%P16%" call :compile_module "MHClient" "%P16%" "多项依赖"
if exist "%P17%" call :compile_module "MHAutoPatch" "%P17%" "ZipArchive, SoundLib"
if exist "%P18%" call :compile_module "Selupdate" "%P18%" "ZipArchive, SoundLib"

REM ========== 第6批: 工具 (可选) ==========
echo ======================================== >> %BUILD_LOG%
echo 第6批: 工具和辅助程序 >> %BUILD_LOG%
echo ======================================== >> %BUILD_LOG%
echo.
echo [第6批] 编译工具和辅助程序...
echo.

set "P19=%PROJECT_ROOT%[Monitoring]Tool\ServerTool.vcproj"
set "P20=%PROJECT_ROOT%[Tool]PackingMan\PackingTool.vcproj"
set "P21=%PROJECT_ROOT%[Tool]Regen\RegenTool.vcproj"
set "P22=%PROJECT_ROOT%[Tool]DS_RMTool\DS_RMTool.vcproj"
set "P23=%PROJECT_ROOT%ModelView\ModelView.vcproj"
set "P24=%PROJECT_ROOT%4DyuchiGXMapEditor\4DyuchiGXMapEditor.vcproj"
set "P25=%PROJECT_ROOT%4DyuchiFilePack\4DyuchiFilePack.vcproj"

if exist "%P19%" call :compile_module "ServerTool" "%P19%"
if exist "%P20%" call :compile_module "PackingTool" "%P20%"
if exist "%P21%" call :compile_module "RegenTool" "%P21%"
if exist "%P22%" call :compile_module "DS_RMTool" "%P22%"
if exist "%P23%" call :compile_module "ModelView" "%P23%"
if exist "%P24%" call :compile_module "MapEditor" "%P24%"
if exist "%P25%" call :compile_module "FilePack" "%P25%"

REM 跳过需要3ds Max SDK的工具
echo 跳过需要3ds Max SDK的导出工具 >> %BUILD_LOG%
echo   - anmexp (动画导出)
echo   - maxexp (模型导出)
echo   - MtlExp (材质导出)
echo.

REM 编译完成
echo ======================================== >> %BUILD_LOG%
echo 编译完成! >> %BUILD_LOG%
echo 结束时间: %date% %time% >> %BUILD_LOG%
echo ======================================== >> %BUILD_LOG%
echo.
echo ========================================
echo 编译完成!
echo 查看完整日志: %BUILD_LOG%
echo ========================================
echo.

REM 显示日志摘要
findstr /C:"[成功]" %BUILD_LOG%
findstr /C:"[错误]" %BUILD_LOG%

endlocal
