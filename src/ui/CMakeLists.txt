# src/ui/CMakeLists.txt - UI模块

# 源文件
set(UI_SOURCES
    mainwindow/mainwindow.cpp
    devicepanel/devicepanel.cpp
    terminal/terminalwidget.cpp
    scripteditor/scripteditor.cpp
)

# 头文件
set(UI_HEADERS
    mainwindow/mainwindow.h
    devicepanel/devicepanel.h
    terminal/terminalwidget.h
    scripteditor/scripteditor.h
)

# UI 文件 (Qt Designer)
set(UI_FORMS
    mainwindow/mainwindow.ui
)

# 创建静态库
add_library(ui STATIC
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${UI_FORMS}
)

# 包含目录
target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(ui PUBLIC
    utils
    core
    communication
    visualization
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Network
)

# 可选 Qt Charts
if(TARGET Qt${QT_VERSION_MAJOR}::Charts)
    target_link_libraries(ui PUBLIC Qt${QT_VERSION_MAJOR}::Charts)
endif()

# 导出头文件
target_include_directories(ui PUBLIC
    ${CMAKE_SOURCE_DIR}/src/ui
)
