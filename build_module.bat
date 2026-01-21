@echo off
REM ========================================
REM 快速编译单个模块
REM ========================================
REM 用法: build_module.bat [模块名] [Debug|Release]
REM
REM 支持的模块名:
REM   YHLibrary, BaseNetwork, DBThread, HSEL, ZipArchive
REM   SoundLib, 4DyuchiGX_Render, 4DyuchiNET
REM   MHClient, MapServer, AgentServer, DistributeServer
REM   MHAutoPatch, RegenTool, PackingTool
REM
REM 示例:
REM   build_module.bat YHLibrary Release
REM   build_module.bat MHClient Debug
REM ========================================

setlocal enabledelayedexpansion

set MODULE=%1
set CONFIG=%2
if "%CONFIG%"=="" set CONFIG=Release
set PROJECT_ROOT=%~dp0

REM 检查参数
if "%MODULE%"=="" (
    echo 错误: 缺少模块名参数
    echo.
    echo 用法: build_module.bat [模块名] [Debug|Release]
    echo.
    echo 可用模块列表:
    echo   基础库: YHLibrary, HSEL, ZipArchive, BaseNetwork, DBThread
    echo   中间层: SoundLib, 4DyuchiGX_Render, 4DyuchiNET, 4DyuchiGXGeometry
    echo   服务器: MapServer, AgentServer, DistributeServer, MonitoringServer
    echo   客户端: MHClient, MHAutoPatch, Selupdate
    echo   工具: RegenTool, PackingTool, DS_RMTool, ServerTool
    pause
    exit /b 1
)

REM 模块映射
if /i "%MODULE%"=="YHLibrary" (
    set "PROJECT=[Lib]YHLibrary\YHLibrary.vcproj"
    set "DEPS=无"
) else if /i "%MODULE%"=="HSEL" (
    set "PROJECT=[Lib]HSEL\HSEL.vcproj"
    set "DEPS=无"
) else if /i "%MODULE%"=="ZipArchive" (
    set "PROJECT=[Lib]ZipArchive\ZipArchive.vcproj"
    set "DEPS=无"
) else if /i "%MODULE%"=="BaseNetwork" (
    set "PROJECT=[Lib]BaseNetwork\BaseNetwork.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="DBThread" (
    set "PROJECT=[Lib]DBThread\DBThread.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="SoundLib" (
    set "PROJECT=SoundLib\SoundLib.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="4DyuchiGX_Render" (
    set "PROJECT=4DYUCHIGX_RENDER\4DyuchiGX_Render.vcproj"
    set "DEPS=DirectX SDK"
) else if /i "%MODULE%"=="4DyuchiGXGeometry" (
    set "PROJECT=4DyuchiGXGeometry\4DyuchiGXGeometry.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="4DyuchiGXExecutive" (
    set "PROJECT=4DYUCHIGXEXECUTIVE\4DyuchiGXExecutive.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="4DyuchiFileStorage" (
    set "PROJECT=4DyuchiFileStorage\4DyuchiFileStorage.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="4DyuchiNET" (
    set "PROJECT=4DyuchiNET_Latest\I4DyuchiNET.vcproj"
    set "DEPS=YHLibrary, BaseNetwork"
) else if /i "%MODULE%"=="MapServer" (
    set "PROJECT=[Server]Map\MapServer.vcproj"
    set "DEPS=YHLibrary, BaseNetwork"
) else if /i "%MODULE%"=="AgentServer" (
    set "PROJECT=[Server]Agent\AgentServer.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="DistributeServer" (
    set "PROJECT=[Server]Distribute\DistributeServer.vcproj"
    set "DEPS=YHLibrary, BaseNetwork"
) else if /i "%MODULE%"=="MonitoringServer" (
    set "PROJECT=[Monitoring]Server\MonitoringServer.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="MHClient" (
    set "PROJECT=[Client]MH\MHClient.vcproj"
    set "DEPS=多项 (YHLibrary, DirectX, SoundLib等)"
) else if /i "%MODULE%"=="MHAutoPatch" (
    set "PROJECT=[Client]MHAutoPatch\MHAutoPatch.vcproj"
    set "DEPS=ZipArchive, SoundLib"
) else if /i "%MODULE%"=="Selupdate" (
    set "PROJECT=[Client]Selupdate\Selupdate.vcproj"
    set "DEPS=ZipArchive, SoundLib"
) else if /i "%MODULE%"=="RegenTool" (
    set "PROJECT=[Tool]Regen\RegenTool.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="PackingTool" (
    set "PROJECT=[Tool]PackingMan\PackingTool.vcproj"
    set "DEPS=未知"
) else if /i "%MODULE%"=="DS_RMTool" (
    set "PROJECT=[Tool]DS_RMTool\DS_RMTool.vcproj"
    set "DEPS=未知"
) else if /i "%MODULE%"=="ServerTool" (
    set "PROJECT=[Monitoring]Tool\ServerTool.vcproj"
    set "DEPS=YHLibrary"
) else if /i "%MODULE%"=="ModelView" (
    set "PROJECT=ModelView\ModelView.vcproj"
    set "DEPS=未知"
) else if /i "%MODULE%"=="MapEditor" (
    set "PROJECT=4DyuchiGXMapEditor\4DyuchiGXMapEditor.vcproj"
    set "DEPS=未知"
) else (
    echo 错误: 未知的模块 '%MODULE%'
    echo.
    echo 支持的模块:
    echo   基础库: YHLibrary, HSEL, ZipArchive, BaseNetwork, DBThread
    echo   中间层: SoundLib, 4DyuchiGX_Render, 4DyuchiNET
    echo   服务器: MapServer, AgentServer, DistributeServer
    echo   客户端: MHClient, MHAutoPatch
    echo   工具: RegenTool, PackingTool
    pause
    exit /b 1
)

set PROJECT_PATH=%PROJECT_ROOT%%PROJECT%

REM 检查项目文件是否存在
if not exist "%PROJECT_PATH%" (
    echo 错误: 找不到项目文件
    echo   路径: %PROJECT_PATH%
    pause
    exit /b 1
)

REM 显示编译信息
echo ========================================
echo 模块快速编译
echo ========================================
echo 模块名称: %MODULE%
echo 配置:     %CONFIG%
echo 项目文件: %PROJECT%
echo 依赖:     %DEPS%
echo ========================================
echo.

REM 检查MSBuild
where msbuild >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到 MSBuild
    echo 请确保已安装 Visual Studio
    pause
    exit /b 1
)

REM 执行编译
echo 开始编译...
echo.

msbuild "%PROJECT_PATH%" /p:Configuration=%CONFIG% /t:Rebuild /v:m /nologo

if errorlevel 1 (
    echo.
    echo ========================================
    echo [错误] 编译失败
    echo ========================================
    pause
    exit /b 1
) else (
    echo.
    echo ========================================
    echo [成功] 编译成功!
    echo ========================================
    pause
    exit /b 0
)
