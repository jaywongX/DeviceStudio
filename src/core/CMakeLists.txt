# src/core/CMakeLists.txt - 核心模块

# 源文件
set(CORE_SOURCES
    devicemanager/device.cpp
    devicemanager/devicemanager.cpp
    datacenter/datastore.cpp
    scriptengine/luascriptengine.cpp
)

# 头文件
set(CORE_HEADERS
    devicemanager/idevice.h
    devicemanager/idevicemanager.h
    devicemanager/device.h
    devicemanager/devicemanager.h
    datacenter/idatacenter.h
    datacenter/datastore.h
    scriptengine/luascriptengine.h
)

# 创建静态库
add_library(core STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

# 包含目录
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(core PUBLIC
    utils
    lua
    sol2
    Qt${QT_VERSION_MAJOR}::Core
)

# 导出头文件
target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
)
