# src/visualization/CMakeLists.txt - 可视化模块

# 源文件
set(VISUALIZATION_SOURCES
    chart/chartwidget.cpp
    dashboard/gaugewidget.cpp
    monitor/monitortable.cpp
)

# 头文件
set(VISUALIZATION_HEADERS
    chart/chartwidget.h
    dashboard/gaugewidget.h
    monitor/monitortable.h
)

# 创建静态库
add_library(visualization STATIC
    ${VISUALIZATION_SOURCES}
    ${VISUALIZATION_HEADERS}
)

# 包含目录
target_include_directories(visualization PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(visualization PUBLIC
    utils
    core
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

# 可选 Qt Charts
if(TARGET Qt${QT_VERSION_MAJOR}::Charts)
    target_link_libraries(visualization PUBLIC Qt${QT_VERSION_MAJOR}::Charts)
endif()

# 导出头文件
target_include_directories(visualization PUBLIC
    ${CMAKE_SOURCE_DIR}/src/visualization
)
