# src/visualization/CMakeLists.txt - 可视化模块

# 源文件
set(VISUALIZATION_SOURCES
    chart/chartwidget.cpp
    chart/channelmanager.cpp
    gauge/gaugewidget.cpp
    gauge/bargauge.cpp
    gauge/ledindicator.cpp
    gauge/switchindicator.cpp
)

# 头文件
set(VISUALIZATION_HEADERS
    chart/chartwidget.h
    chart/channelmanager.h
    gauge/gaugewidget.h
    gauge/bargauge.h
    gauge/ledindicator.h
    gauge/switchindicator.h
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
    qcustomplot
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
)

# 导出头文件
target_include_directories(visualization PUBLIC
    ${CMAKE_SOURCE_DIR}/src/visualization
)
