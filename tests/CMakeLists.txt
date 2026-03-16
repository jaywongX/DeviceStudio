# tests/CMakeLists.txt - 测试配置

# 测试源文件
set(TEST_SOURCES
    main.cpp
    utils/test_logger.cpp
    utils/test_configmanager.cpp
    core/test_device.cpp
    test_protocol.cpp
)

# 创建测试可执行文件
add_executable(tests ${TEST_SOURCES})

# 链接依赖
target_link_libraries(tests PRIVATE
    utils
    core
    gtest
    Qt${QT_VERSION_MAJOR}::Core
)

# 注册测试
include(GoogleTest)
gtest_discover_tests(tests)
