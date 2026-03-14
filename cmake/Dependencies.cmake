# Dependencies.cmake - 第三方依赖管理
# 使用 FetchContent 自动下载和管理依赖

include(FetchContent)

# ============================================================================
# nlohmann/json - JSON解析库
# ============================================================================
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(json)

# ============================================================================
# spdlog - 日志库
# ============================================================================
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# ============================================================================
# QCustomPlot - Qt图表库
# ============================================================================
FetchContent_Declare(
    qcustomplot
    GIT_REPOSITORY https://github.com/VSRonin/QCustomPlot.git
    GIT_TAG        v2.1.1
)
FetchContent_MakeAvailable(qcustomplot)

# ============================================================================
# Lua - 脚本引擎
# ============================================================================
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/walterschell/Lua.git
    GIT_TAG        v5.4.4
)
FetchContent_MakeAvailable(lua)

# ============================================================================
# sol2 - Lua C++绑定库
# ============================================================================
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG        v3.3.0
)
FetchContent_MakeAvailable(sol2)

# ============================================================================
# Google Test - 单元测试框架 (可选)
# ============================================================================
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    # Windows: 阻止 overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
endif()

# ============================================================================
# 输出依赖信息
# ============================================================================
message(STATUS "=== Dependencies ===")
message(STATUS "nlohmann/json: v3.11.2")
message(STATUS "spdlog: v1.12.0")
message(STATUS "QCustomPlot: v2.1.1")
message(STATUS "Lua: v5.4.4")
message(STATUS "sol2: v3.3.0")
if(BUILD_TESTS)
    message(STATUS "Google Test: v1.14.0")
endif()
message(STATUS "====================")
