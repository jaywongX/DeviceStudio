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
# Lua - 脚本引擎
# ============================================================================
option(LUA_BUILD_LUA_COMPILER "Build lua compiler" OFF)
option(LUA_BUILD_LUA_INTERPRETER "Build lua interpreter" OFF)
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

# 为 sol2 创建接口库（如果不存在）
if(NOT TARGET sol2)
    add_library(sol2 INTERFACE)
    target_include_directories(sol2 INTERFACE
        ${sol2_SOURCE_DIR}/include
        ${lua_SOURCE_DIR}/include
    )
    target_link_libraries(sol2 INTERFACE lua_static)
endif()

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
message(STATUS "QCustomPlot: local (thirdparty/QCustomPlot)")
message(STATUS "Lua: v5.4.4")
message(STATUS "sol2: v3.3.0")
if(BUILD_TESTS)
    message(STATUS "Google Test: v1.14.0")
endif()
message(STATUS "====================")
