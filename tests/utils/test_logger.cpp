#include <gtest/gtest.h>
#include "utils/log/logger.h"
#include <fstream>
#include <filesystem>

using namespace DeviceStudio::Utils;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前清理日志目录
        if (std::filesystem::exists("logs")) {
            std::filesystem::remove_all("logs");
        }
    }
    
    void TearDown() override {
        // 关闭日志系统
        Logger::shutdown();
    }
};

// 测试日志初始化
TEST_F(LoggerTest, Initialize) {
    EXPECT_NO_THROW(Logger::init("DeviceStudio", "logs"));
    EXPECT_TRUE(std::filesystem::exists("logs"));
}

// 测试基本日志输出
TEST_F(LoggerTest, BasicLogging) {
    Logger::init("DeviceStudio", "logs");
    
    EXPECT_NO_THROW(Logger::trace("Trace message"));
    EXPECT_NO_THROW(Logger::debug("Debug message"));
    EXPECT_NO_THROW(Logger::info("Info message"));
    EXPECT_NO_THROW(Logger::warn("Warn message"));
    EXPECT_NO_THROW(Logger::error("Error message"));
    EXPECT_NO_THROW(Logger::critical("Critical message"));
}

// 测试格式化日志
TEST_F(LoggerTest, FormattedLogging) {
    Logger::init("DeviceStudio", "logs");
    
    EXPECT_NO_THROW(Logger::info("Formatted message: {}", 42));
    EXPECT_NO_THROW(Logger::info("Multiple args: {}, {}, {}", 1, 2.0, "test"));
}

// 测试日志级别设置
TEST_F(LoggerTest, SetLogLevel) {
    Logger::init("DeviceStudio", "logs");
    
    EXPECT_NO_THROW(Logger::setLevel(LogLevel::Debug));
    EXPECT_NO_THROW(Logger::setLevel(LogLevel::Info));
    EXPECT_NO_THROW(Logger::setLevel(LogLevel::Warn));
    EXPECT_NO_THROW(Logger::setLevel(LogLevel::Error));
}

// 测试重复初始化（应该不会崩溃）
TEST_F(LoggerTest, DoubleInit) {
    EXPECT_NO_THROW(Logger::init("DeviceStudio", "logs"));
    EXPECT_NO_THROW(Logger::init("DeviceStudio", "logs"));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
