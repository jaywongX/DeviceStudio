@ECHO OFF
SETLOCAL
@REM ===========================================
@REM 这是发布用的编译脚本
@REM ===========================================

@REM 这里修改实际编译时使用的VC工具集，这里默认使用本地最新工具集:
SET VC_TOOL_KIT=

@REM 构建工程的目录名，.sln 会生成在这里面
SET BUILD_DIR_NAME=build-windows

@REM 工程根目录
SET PROJECT_ROOT=%CD%

@REM 调用编译函数
CALL:Build Release x86
CALL:Build Release x64

GOTO:EOF

@REM 定义编译函数
:Build

@REM 编译配置，Debug/Release，由函数参数决定
SET BUILD_TYPE=%1

@REM 架构位数
SET ARCHIVE=%2

@REM 构建工程的目录路径
SET BUILD_DIR=%PROJECT_ROOT%\%BUILD_DIR_NAME%-%ARCHIVE%

@REM 指定编译产物的安装目录
SET INSTALL_DIR="%PROJECT_ROOT%/built/windows/%ARCHIVE%"

@REM 删除旧的构建目录
IF EXIST %BUILD_DIR% (
    RMDIR /s /q %BUILD_DIR%
)
MKDIR %BUILD_DIR%
CD /D %BUILD_DIR%

IF /i "%ARCHIVE%" EQU "x64" (
    SET ARCHIVE_PARAM=-A x64
) ELSE (
    SET ARCHIVE_PARAM=
)

IF DEFINED VC_TOOL_KIT (
    SET TOOL_KIT_PARAM=-T %VC_TOOL_KIT%
) ELSE (
    SET TOOL_KIT_PARAM=
)

@REM 开始构建工程
CALL cmake ..                                   ^
           -DCMAKE_BUILD_TYPE=%BUILD_TYPE%      ^
           -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR% ^
           -DCMAKE_VERBOSE_MAKEFILE=FALSE       ^
           %ARCHIVE_PARAM%                      ^
           %TOOL_KIT_PARAM%
            
CALL cmake --build . --config %BUILD_TYPE% --target INSTALL

CD %PROJECT_ROOT%

GOTO:EOF

ENDLOCAL