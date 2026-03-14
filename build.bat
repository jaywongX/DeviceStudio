@echo off
setlocal

echo ========================================
echo DeviceStudio Build Script for Windows
echo ========================================
echo.

:: 设置 MSVC 环境 (根据实际安装路径调整)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo ERROR: Visual Studio 2022 not found!
    echo Please install Visual Studio 2022 or modify the script to point to your installation.
    pause
    exit /b 1
)

echo.
echo [1/4] Creating build directory...
if not exist build mkdir build
cd build

echo.
echo [2/4] Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_TESTS=ON

if %ERRORLEVEL% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo [3/4] Building project...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo [4/4] Running tests...
ctest -C Release --output-on-failure

echo.
echo ========================================
echo Build complete!
echo Executable: build\bin\Release\DeviceStudio.exe
echo ========================================
pause
