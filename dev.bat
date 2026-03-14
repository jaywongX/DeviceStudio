@echo off
setlocal enabledelayedexpansion

echo ========================================
echo DeviceStudio Development Script
echo ========================================
echo.

::: 解析参数
set DEV_MODE=debug
set DEV_ARCH=x64
set GENERATOR="Visual Studio 17 2022"

:parse_args
if "%~1"=="" goto :end_parse
if /i "%~1"=="-mode" set DEV_MODE=%~2& shift
if /i "%~1"=="-arch" set DEV_ARCH=%~2& shift
if /i "%~1"=="-help" goto :show_help
shift
goto :parse_args

:show_help
echo 用法: dev.bat [选项]
echo.
echo 选项:
echo   -mode ^<debug^|release^|analyze^>  开发模式 (默认: debug)
echo   -arch ^<x86^|x64^>               目标架构 (默认: x64)
echo   -help                          显示帮助信息
echo.
echo 模式说明:
echo   debug   - 调试模式（包含调试符号，禁用优化）
echo   release - 发布模式（优化编译）
echo   analyze - 代码分析模式（启用静态分析）
echo.
echo 示例:
echo   dev.bat                    # 默认调试模式
echo   dev.bat -mode analyze      # 代码分析
exit /b 0

:end_parse

echo 开发配置:
echo   模式: %DEV_MODE%
echo   架构: %DEV_ARCH%
echo.

::: 设置 MSVC 环境
set MSVC_ARCH=x64
if /i "%DEV_ARCH%"=="x86" set MSVC_ARCH=x86

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat"
) else (
    echo ERROR: Visual Studio 2022 not found!
    pause
    exit /b 1
)

::: 设置构建目录
set BUILD_DIR=build_dev_%DEV_ARCH%

::: 创建构建目录
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

::: 配置 CMake 选项
set CMAKE_OPTIONS=-G %GENERATOR% -A %DEV_ARCH%
set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DBUILD_TESTS=ON

if /i "%DEV_MODE%"=="debug" (
    echo [配置] 调试模式构建...
    set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE=Debug
    set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_CXX_FLAGS_DEBUG="/MDd /Zi /Od /RTC1"
    set BUILD_CONFIG=Debug
) else if /i "%DEV_MODE%"=="release" (
    echo [配置] 发布模式构建...
    set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE=Release
    set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_CXX_FLAGS_RELEASE="/MD /O2 /Ob2"
    set BUILD_CONFIG=Release
) else if /i "%DEV_MODE%"=="analyze" (
    echo [配置] 代码分析模式...
    set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE=Debug
    set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_CXX_FLAGS_DEBUG="/MDd /Zi /Od /analyze"
    set BUILD_CONFIG=Debug
    set ENABLE_ANALYZE=1
)

echo.
echo [1/3] 配置项目...
cmake .. %CMAKE_OPTIONS%

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo [2/3] 构建项目...
cmake --build . --config %BUILD_CONFIG%

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo [3/3] 开发环境准备完成!
echo ========================================
echo 构建目录: %BUILD_DIR%
echo 可执行文件: %BUILD_DIR%\bin\%BUILD_CONFIG%\DeviceStudio.exe
echo ========================================
echo.
echo 可用操作:
echo   - 运行程序: %BUILD_DIR%\bin\%BUILD_CONFIG%\DeviceStudio.exe
echo   - 运行测试: cd %BUILD_DIR% ^&^& ctest -C %BUILD_CONFIG% --output-on-failure
echo   - 打开IDE: devenv %BUILD_DIR%\DeviceStudio.sln
echo.

::: 如果启用代码分析，显示提示
if defined ENABLE_ANALYZE (
    echo [代码分析] 已启用静态分析，请查看编译输出中的分析警告。
    echo.
)

::: 提供交互式菜单
echo 请选择操作:
echo   1. 运行程序
echo   2. 运行测试
echo   3. 打开Visual Studio
echo   4. 清理构建
echo   5. 退出
echo.
set /p choice=请输入选项 (1-5): 

if "%choice%"=="1" (
    echo 启动程序...
    start "" "%BUILD_DIR%\bin\%BUILD_CONFIG%\DeviceStudio.exe"
) else if "%choice%"=="2" (
    echo 运行测试...
    ctest -C %BUILD_CONFIG% --output-on-failure
    pause
) else if "%choice%"=="3" (
    echo 打开Visual Studio...
    devenv "%BUILD_DIR%\DeviceStudio.sln"
) else if "%choice%"=="4" (
    echo 清理构建目录...
    cd ..
    rd /s /q %BUILD_DIR%
    echo 清理完成。
    pause
) else (
    echo 退出。
)

pause
