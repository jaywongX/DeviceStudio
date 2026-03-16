# CMakeLists.txt - DeviceStudio 根目录配置
# CMake 最低版本
cmake_minimum_required(VERSION 3.16)

# ============================================================================
# 项目信息
# ============================================================================
project(DeviceStudio 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Device Studio - Universal Device Debugging and Data Analysis Platform"
)

# ============================================================================
# 全局设置
# ============================================================================
# 自动处理 Qt MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# 加载模块
# ============================================================================
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CompilerOptions)
include(Dependencies)
include(utils)

# ============================================================================
# 查找 Qt
# ============================================================================
# 优先 Qt6，回退 Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets SerialPort Network SerialBus PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets SerialPort Network SerialBus PrintSupport)

# 可选 Qt 模块
if(Qt${QT_VERSION_MAJOR}Charts_FOUND)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Charts)
endif()

message(STATUS "=== Qt Configuration ===")
message(STATUS "Qt Version: ${QT_VERSION}")
message(STATUS "Qt Dir: ${Qt${QT_VERSION_MAJOR}_DIR}")
message(STATUS "========================")

# ============================================================================
# 包含目录
# ============================================================================
include_directories(${CMAKE_SOURCE_DIR}/src)

# ============================================================================
# 子目录
# ============================================================================
add_subdirectory(src/utils)
add_subdirectory(src/core)
add_subdirectory(src/communication)
add_subdirectory(src/visualization)
add_subdirectory(src/ui)
add_subdirectory(src/plugin)

# 测试目录
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ============================================================================
# 可执行文件
# ============================================================================
add_executable(${PROJECT_NAME}
    src/main/main.cpp
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    # 项目模块
    utils
    core
    communication
    visualization
    ui
    plugin
    
    # Qt 模块
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Network
)

# 可选 Qt 模块
if(TARGET Qt${QT_VERSION_MAJOR}::Charts)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_QTCHARTS)
endif()

# Linux 链接线程库
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

# ============================================================================
# 安装规则
# ============================================================================
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# 复制资源文件
install(DIRECTORY resources/ DESTINATION resources)
install(DIRECTORY config/ DESTINATION config)

# ============================================================================
# 输出构建信息
# ============================================================================
message(STATUS "")
message(STATUS "=== DeviceStudio Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "=========================================")
message(STATUS "")
