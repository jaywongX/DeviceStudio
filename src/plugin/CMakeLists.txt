# src/plugin/CMakeLists.txt - 插件系统模块

# 源文件
set(PLUGIN_SOURCES
    pluginloader.cpp
    pluginmanager.cpp
)

# 头文件
set(PLUGIN_HEADERS
    iplugin.h
    pluginloader.h
    pluginmanager.h
)

# 定义源文件分组（Visual Studio 过滤器）
GROUP_FILES(${PLUGIN_SOURCES} ${PLUGIN_HEADERS} "")

# 创建静态库
add_library(plugin STATIC
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

# 包含目录
target_include_directories(plugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(plugin PUBLIC
    utils
    core
    Qt${QT_VERSION_MAJOR}::Core
)

# 导出头文件
target_include_directories(plugin PUBLIC
    ${CMAKE_SOURCE_DIR}/src/plugin
)

# 添加示例插件子目录
add_subdirectory(examples)
