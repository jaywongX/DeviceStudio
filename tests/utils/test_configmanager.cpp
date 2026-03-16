#include <gtest/gtest.h>
#include "utils/config/configmanager.h"
#include <filesystem>
#include <fstream>

using namespace DeviceStudio;

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试配置文件
        std::filesystem::create_directories("test_config");
        
        std::ofstream file("test_config/test.json");
        file << R"({
    "appName": "TestApp",
    "version": "1.0.0",
    "settings": {
        "debug": true,
        "timeout": 30,
        "threshold": 0.85
    },
    "list": [1, 2, 3]
})";
        file.close();
    }
    
    void TearDown() override {
        // 清理测试配置目录
        if (std::filesystem::exists("test_config")) {
            std::filesystem::remove_all("test_config");
        }
    }
};

// 测试加载配置文件
TEST_F(ConfigManagerTest, LoadConfig) {
    EXPECT_TRUE(ConfigManager::instance().load("test_config/test.json"));
}

// 测试获取字符串值
TEST_F(ConfigManagerTest, GetString) {
    ConfigManager::instance().load("test_config/test.json");
    
    EXPECT_EQ(ConfigManager::instance().get<std::string>("appName"), "TestApp");
    EXPECT_EQ(ConfigManager::instance().get<std::string>("version"), "1.0.0");
}

// 测试获取整数值
TEST_F(ConfigManagerTest, GetInt) {
    ConfigManager::instance().load("test_config/test.json");
    
    EXPECT_EQ(ConfigManager::instance().get<int>("settings.timeout"), 30);
}

// 测试获取浮点值
TEST_F(ConfigManagerTest, GetDouble) {
    ConfigManager::instance().load("test_config/test.json");
    
    EXPECT_DOUBLE_EQ(ConfigManager::instance().get<double>("settings.threshold"), 0.85);
}

// 测试获取布尔值
TEST_F(ConfigManagerTest, GetBool) {
    ConfigManager::instance().load("test_config/test.json");
    
    EXPECT_TRUE(ConfigManager::instance().get<bool>("settings.debug"));
}

// 测试嵌套路径
TEST_F(ConfigManagerTest, NestedPath) {
    ConfigManager::instance().load("test_config/test.json");
    
    EXPECT_TRUE(ConfigManager::instance().has("settings.debug"));
    EXPECT_TRUE(ConfigManager::instance().has("settings.timeout"));
    EXPECT_TRUE(ConfigManager::instance().has("settings.threshold"));
    EXPECT_FALSE(ConfigManager::instance().has("settings.nonexistent"));
}

// 测试默认值
TEST_F(ConfigManagerTest, DefaultValue) {
    ConfigManager::instance().load("test_config/test.json");
    
    EXPECT_EQ(ConfigManager::instance().get<std::string>("nonexistent", "default"), "default");
    EXPECT_EQ(ConfigManager::instance().get<int>("nonexistent", 100), 100);
    EXPECT_DOUBLE_EQ(ConfigManager::instance().get<double>("nonexistent", 1.5), 1.5);
    EXPECT_FALSE(ConfigManager::instance().get<bool>("nonexistent", false));
}

// 测试设置值
TEST_F(ConfigManagerTest, SetValue) {
    ConfigManager::instance().load("test_config/test.json");
    
    ConfigManager::instance().set<std::string>("newKey", "newValue");
    EXPECT_EQ(ConfigManager::instance().get<std::string>("newKey"), "newValue");
    
    ConfigManager::instance().set<int>("settings.newInt", 42);
    EXPECT_EQ(ConfigManager::instance().get<int>("settings.newInt"), 42);
}

// 测试保存配置
TEST_F(ConfigManagerTest, SaveConfig) {
    ConfigManager::instance().load("test_config/test.json");
    
    ConfigManager::instance().set<std::string>("savedKey", "savedValue");
    EXPECT_TRUE(ConfigManager::instance().save("test_config/saved.json"));
    
    // 验证保存的文件可以被加载
    EXPECT_TRUE(ConfigManager::instance().load("test_config/saved.json"));
    EXPECT_EQ(ConfigManager::instance().get<std::string>("savedKey"), "savedValue");
}

// 测试加载不存在的文件
TEST_F(ConfigManagerTest, LoadNonexistentFile) {
    EXPECT_FALSE(ConfigManager::instance().load("test_config/nonexistent.json"));
}

// 测试空路径
TEST_F(ConfigManagerTest, EmptyPath) {
    ConfigManager::instance().load("test_config/test.json");
    
    EXPECT_FALSE(ConfigManager::instance().has(""));
}


