# src/core/CMakeLists.txt - 核心模块

# 源文件
set(CORE_SOURCES
    devicemanager/device.cpp
    devicemanager/devicemanager.cpp
    datacenter/datastore.cpp
    datacenter/datarecorder.cpp
    datacenter/dataplayer.cpp
    scriptengine/luascriptengine.cpp
    scriptengine/scriptparser.cpp
    scriptengine/scriptexecutor.cpp
    protocol/checksum.cpp
    protocol/protocolparser.cpp
    protocol/protocolengine.cpp
    project/project.cpp
    project/workspace.cpp
)

# 头文件
set(CORE_HEADERS
    devicemanager/idevice.h
    devicemanager/idevicemanager.h
    devicemanager/device.h
    devicemanager/devicemanager.h
    datacenter/idatacenter.h
    datacenter/datastore.h
    datacenter/datarecorder.h
    datacenter/dataplayer.h
    scriptengine/luascriptengine.h
    scriptengine/scriptcommands.h
    scriptengine/scriptparser.h
    scriptengine/scriptexecutor.h
    protocol/protocoldefinition.h
    protocol/checksum.h
    protocol/protocolparser.h
    protocol/protocolengine.h
    project/project.h
    project/workspace.h
)

# 定义源文件分组（Visual Studio 过滤器）
GROUP_FILES(${CORE_SOURCES} ${CORE_HEADERS} "")

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
    lua_static
    sol2
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
)

# 导出头文件
target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
)
