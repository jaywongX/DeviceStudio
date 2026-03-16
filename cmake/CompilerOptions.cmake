# CompilerOptions.cmake - 编译器选项配置
# 跨平台编译器选项设置

# ============================================================================
# C++17 标准
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# 平台特定设置
# ============================================================================
if(WIN32)
    # Windows MSVC 设置
    if(MSVC)
        # 设置 MSVC 运行时库
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        
        # MSVC 编译选项
        add_compile_options(
            /W4           # 警告级别4
            /utf-8        # UTF-8 源码
            /MP           # 多进程编译
            /permissive-  # 严格标准一致性
        )
        
        # 定义 Windows 版本
        add_definitions(-DWIN32_LEAN_AND_MEAN)
        add_definitions(-DNOMINMAX)
        
        # 调试/发布选项（使用生成器表达式支持 Visual Studio 多配置）
        add_compile_options(
            $<$<CONFIG:Debug>:/Od>    # Debug: 禁用优化
            $<$<CONFIG:Debug>:/Zi>    # Debug: 生成调试信息
            $<$<CONFIG:Release>:/O2>  # Release: 优化级别2
            $<$<CONFIG:RelWithDebInfo>:/O2>  # RelWithDebInfo: 优化级别2
            $<$<CONFIG:MinSizeRel>:/O1>       # MinSizeRel: 最小大小优化
        )
    endif()
    
elseif(UNIX AND NOT APPLE)
    # Linux GCC 设置
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror=return-type
        -Wno-unused-parameter
    )
    
    # 链接线程库
    find_package(Threads REQUIRED)
    
    # Linux 特定定义
    add_definitions(-DLINUX)
    
    # 调试选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0 -g)  # 禁用优化，生成调试信息
    else()
        add_compile_options(-O2)     # 优化级别2
    endif()
endif()

# ============================================================================
# 输出编译器信息
# ============================================================================
message(STATUS "=== Compiler Options ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(MSVC)
    message(STATUS "Compiler: MSVC ${MSVC_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Compiler: GCC ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Compiler: Clang ${CMAKE_CXX_COMPILER_VERSION}")
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "=========================")
