# src/plugin/examples/CMakeLists.txt - 示例插件构建配置

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 自定义协议插件
add_library(CustomProtocolPlugin MODULE
    customprotocolplugin.cpp
    customprotocolplugin.h
)

target_link_libraries(CustomProtocolPlugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    core
)

target_include_directories(CustomProtocolPlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugin
)

set_target_properties(CustomProtocolPlugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "customprotocol"
)

# Mock通信插件
add_library(MockCommunicationPlugin MODULE
    mockcommunicationplugin.cpp
    mockcommunicationplugin.h
)

target_link_libraries(MockCommunicationPlugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    core
)

target_include_directories(MockCommunicationPlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugin
)

set_target_properties(MockCommunicationPlugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "mockcommunication"
)

# 复制插件到输出目录
if(WIN32)
    add_custom_command(TARGET CustomProtocolPlugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:CustomProtocolPlugin>"
        "${CMAKE_BINARY_DIR}/plugins/"
    )
    add_custom_command(TARGET MockCommunicationPlugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:MockCommunicationPlugin>"
        "${CMAKE_BINARY_DIR}/plugins/"
    )
else()
    add_custom_command(TARGET CustomProtocolPlugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:CustomProtocolPlugin>"
        "${CMAKE_BINARY_DIR}/plugins/"
    )
    add_custom_command(TARGET MockCommunicationPlugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:MockCommunicationPlugin>"
        "${CMAKE_BINARY_DIR}/plugins/"
    )
endif()

# 复制JSON元数据文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/customprotocol.json ${CMAKE_BINARY_DIR}/plugins/customprotocol.json COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mockcommunication.json ${CMAKE_BINARY_DIR}/plugins/mockcommunication.json COPYONLY)
