#include <gtest/gtest.h>
#include "utils/log/logger.h"
#include <filesystem>

using namespace DeviceStudio;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前清理日志目录
        if (std::filesystem::exists("logs")) {
            std::filesystem::remove_all("logs");
        }
    }
    
    void TearDown() override {
        // 日志系统是单例，不需要显式关闭
    }
};

// 测试日志初始化
TEST_F(LoggerTest, Initialize) {
    EXPECT_NO_THROW(Logger::instance().init("logs/test.log"));
    EXPECT_TRUE(std::filesystem::exists("logs"));
}

// 测试基本日志输出
TEST_F(LoggerTest, BasicLogging) {
    Logger::instance().init("logs/test.log");
    
    EXPECT_NO_THROW(DS_LOG_TRACE("Trace message"));
    EXPECT_NO_THROW(DS_LOG_DEBUG("Debug message"));
    EXPECT_NO_THROW(DS_LOG_INFO("Info message"));
    EXPECT_NO_THROW(DS_LOG_WARN("Warn message"));
    EXPECT_NO_THROW(DS_LOG_ERROR("Error message"));
}

// 测试格式化日志
TEST_F(LoggerTest, FormattedLogging) {
    Logger::instance().init("logs/test.log");
    
    EXPECT_NO_THROW(DS_LOG_INFO("Formatted message: {}", 42));
    EXPECT_NO_THROW(DS_LOG_INFO("Multiple args: {}, {}, {}", 1, 2.0, "test"));
}

// 测试日志级别设置
TEST_F(LoggerTest, SetLogLevel) {
    Logger::instance().init("logs/test.log");
    
    EXPECT_NO_THROW(Logger::instance().setLevel(LogLevel::Debug));
    EXPECT_NO_THROW(Logger::instance().setLevel(LogLevel::Info));
    EXPECT_NO_THROW(Logger::instance().setLevel(LogLevel::Warning));
    EXPECT_NO_THROW(Logger::instance().setLevel(LogLevel::Error));
}

// 测试重复初始化（应该不会崩溃）
TEST_F(LoggerTest, DoubleInit) {
    EXPECT_NO_THROW(Logger::instance().init("logs/test1.log"));
    EXPECT_NO_THROW(Logger::instance().init("logs/test2.log"));
}


