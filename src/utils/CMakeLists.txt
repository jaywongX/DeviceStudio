# src/utils/CMakeLists.txt - 工具模块

# 源文件
set(UTILS_SOURCES
    log/logger.cpp
    config/configmanager.cpp
)

# 头文件
set(UTILS_HEADERS
    log/logger.h
    config/configmanager.h
)

# 定义源文件分组（Visual Studio 过滤器）
GROUP_FILES(${UTILS_SOURCES} ${UTILS_HEADERS} "")

# 创建静态库
add_library(utils STATIC
    ${UTILS_SOURCES}
    ${UTILS_HEADERS}
)

# 包含目录
target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接第三方库
target_link_libraries(utils PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

# Qt 模块
target_link_libraries(utils PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)

# 导出头文件
target_include_directories(utils PUBLIC
    ${CMAKE_SOURCE_DIR}/src/utils
)
