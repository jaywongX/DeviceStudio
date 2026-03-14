# src/communication/CMakeLists.txt - 通信模块

# 源文件
set(COMMUNICATION_SOURCES
    serial/serialport.cpp
    tcp/tcpclient.cpp
    tcp/tcpserver.cpp
    udp/udpsocket.cpp
    modbus/modbusdevice.cpp
    can/canbus.cpp
)

# 头文件
set(COMMUNICATION_HEADERS
    serial/serialport.h
    tcp/tcpclient.h
    tcp/tcpserver.h
    udp/udpsocket.h
    modbus/modbusdevice.h
    can/canbus.h
)

# 创建静态库
add_library(communication STATIC
    ${COMMUNICATION_SOURCES}
    ${COMMUNICATION_HEADERS}
)

# 包含目录
target_include_directories(communication PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(communication PUBLIC
    utils
    core
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::SerialBus
)

# 导出头文件
target_include_directories(communication PUBLIC
    ${CMAKE_SOURCE_DIR}/src/communication
)
