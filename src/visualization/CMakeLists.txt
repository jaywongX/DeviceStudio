# src/visualization/CMakeLists.txt - 可视化模块

# QCustomPlot 路径
set(QCUSTOMPLOT_DIR ${CMAKE_SOURCE_DIR}/thirdparty/QCustomPlot)

# 源文件
set(VISUALIZATION_SOURCES
    chart/chartwidget.cpp
    chart/channelmanager.cpp
    gauge/gaugewidget.cpp
    gauge/bargauge.cpp
    gauge/ledindicator.cpp
    gauge/switchindicator.cpp
    ${QCUSTOMPLOT_DIR}/qcustomplot.cpp
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

# 定义源文件分组（Visual Studio 过滤器）
GROUP_FILES(${VISUALIZATION_SOURCES} ${VISUALIZATION_HEADERS} "")

# 创建静态库
add_library(visualization STATIC
    ${VISUALIZATION_SOURCES}
    ${VISUALIZATION_HEADERS}
)

# 包含目录
target_include_directories(visualization PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QCUSTOMPLOT_DIR}
)

# 链接依赖
target_link_libraries(visualization PUBLIC
    utils
    core
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
)

# 禁用 qcustomplot 的编译警告
if(MSVC)
    set_source_files_properties(${QCUSTOMPLOT_DIR}/qcustomplot.cpp PROPERTIES COMPILE_FLAGS /W0)
endif()
