#include <gtest/gtest.h>
#include "utils/config/configmanager.h"
#include <filesystem>

using namespace DeviceStudio::Utils;

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
    ConfigManager config;
    EXPECT_TRUE(config.loadFromFile("test_config/test.json"));
}

// 测试获取字符串值
TEST_F(ConfigManagerTest, GetString) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    EXPECT_EQ(config.getString("appName"), "TestApp");
    EXPECT_EQ(config.getString("version"), "1.0.0");
}

// 测试获取整数值
TEST_F(ConfigManagerTest, GetInt) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    EXPECT_EQ(config.getInt("settings.timeout"), 30);
}

// 测试获取浮点值
TEST_F(ConfigManagerTest, GetDouble) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    EXPECT_DOUBLE_EQ(config.getDouble("settings.threshold"), 0.85);
}

// 测试获取布尔值
TEST_F(ConfigManagerTest, GetBool) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    EXPECT_TRUE(config.getBool("settings.debug"));
}

// 测试嵌套路径
TEST_F(ConfigManagerTest, NestedPath) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    EXPECT_TRUE(config.has("settings.debug"));
    EXPECT_TRUE(config.has("settings.timeout"));
    EXPECT_TRUE(config.has("settings.threshold"));
    EXPECT_FALSE(config.has("settings.nonexistent"));
}

// 测试默认值
TEST_F(ConfigManagerTest, DefaultValue) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    EXPECT_EQ(config.getString("nonexistent", "default"), "default");
    EXPECT_EQ(config.getInt("nonexistent", 100), 100);
    EXPECT_DOUBLE_EQ(config.getDouble("nonexistent", 1.5), 1.5);
    EXPECT_FALSE(config.getBool("nonexistent", false));
}

// 测试设置值
TEST_F(ConfigManagerTest, SetValue) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    config.setValue("newKey", "newValue");
    EXPECT_EQ(config.getString("newKey"), "newValue");
    
    config.setValue("settings.newInt", 42);
    EXPECT_EQ(config.getInt("settings.newInt"), 42);
}

// 测试保存配置
TEST_F(ConfigManagerTest, SaveConfig) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    config.setValue("savedKey", "savedValue");
    EXPECT_TRUE(config.saveToFile("test_config/saved.json"));
    
    // 验证保存的文件可以被加载
    ConfigManager config2;
    EXPECT_TRUE(config2.loadFromFile("test_config/saved.json"));
    EXPECT_EQ(config2.getString("savedKey"), "savedValue");
}

// 测试加载不存在的文件
TEST_F(ConfigManagerTest, LoadNonexistentFile) {
    ConfigManager config;
    EXPECT_FALSE(config.loadFromFile("test_config/nonexistent.json"));
}

// 测试空路径
TEST_F(ConfigManagerTest, EmptyPath) {
    ConfigManager config;
    config.loadFromFile("test_config/test.json");
    
    EXPECT_FALSE(config.has(""));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
