@echo off
setlocal enabledelayedexpansion

echo ========================================
echo DeviceStudio Build Script for Windows
echo ========================================
echo.

::: 解析参数
set BUILD_ARCH=x64
set BUILD_TYPE=Release
set BUILD_TESTS=ON
set CLEAN_BUILD=0

:parse_args
if "%~1"=="" goto :end_parse
if /i "%~1"=="-arch" set BUILD_ARCH=%~2& shift
if /i "%~1"=="-config" set BUILD_TYPE=%~2& shift
if /i "%~1"=="-tests" set BUILD_TESTS=%~2& shift
if /i "%~1"=="-clean" set CLEAN_BUILD=1
if /i "%~1"=="-help" goto :show_help
shift
goto :parse_args

:show_help
echo 用法: build.bat [选项]
echo.
echo 选项:
echo   -arch ^<x86^|x64^>    目标架构 (默认: x64)
echo   -config ^<Release^|Debug^>  构建类型 (默认: Release)
echo   -tests ^<ON^|OFF^>    构建测试 (默认: ON)
echo   -clean              清理构建目录
echo   -help               显示帮助信息
echo.
echo 示例:
echo   build.bat                    # 默认64位Release构建
echo   build.bat -arch x86          # 32位构建
echo   build.bat -config Debug      # Debug构建
echo   build.bat -clean             # 清理后构建
exit /b 0

:end_parse

echo 配置:
echo   架构: %BUILD_ARCH%
echo   类型: %BUILD_TYPE%
echo   测试: %BUILD_TESTS%
echo   清理: %CLEAN_BUILD%
echo.

::: 设置 MSVC 环境
set MSVC_ARCH=x64
if /i "%BUILD_ARCH%"=="x86" set MSVC_ARCH=x86

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars%MSVC_ARCH%.bat"
) else (
    echo ERROR: Visual Studio 2022 not found!
    echo Please install Visual Studio 2022 or modify the script to point to your installation.
    pause
    exit /b 1
)

::: 设置构建目录
set BUILD_DIR=build_%BUILD_ARCH%_%BUILD_TYPE%

::: 清理构建目录
if %CLEAN_BUILD%==1 (
    echo [清理] 删除构建目录...
    if exist %BUILD_DIR% rd /s /q %BUILD_DIR%
)

::: 创建构建目录
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo.
echo [1/4] 配置 CMake...
cmake .. -G "Visual Studio 17 2022" -A %BUILD_ARCH% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_TESTS=%BUILD_TESTS%

if %ERRORLEVEL% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo [2/4] 构建项目...
cmake --build . --config %BUILD_TYPE%

if %ERRORLEVEL% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

if /i "%BUILD_TESTS%"=="ON" (
    echo.
    echo [3/4] 运行测试...
    ctest -C %BUILD_TYPE% --output-on-failure
)

echo.
echo [4/4] 构建完成!
echo ========================================
echo 可执行文件: %BUILD_DIR%\bin\%BUILD_TYPE%\DeviceStudio.exe
echo ========================================
pause
