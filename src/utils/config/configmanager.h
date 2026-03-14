/**
 * @file configmanager.h
 * @brief 配置管理器接口
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace DeviceStudio {

/**
 * @brief 配置管理器类（单例模式）
 * 
 * 使用 JSON 格式管理应用配置
 */
class ConfigManager {
public:
    /**
     * @brief 获取单例实例
     */
    static ConfigManager& instance();
    
    /**
     * @brief 从文件加载配置
     * @param configPath 配置文件路径
     * @return 成功返回 true，失败返回 false
     */
    bool load(const std::string& configPath);
    
    /**
     * @brief 保存配置到文件
     * @param configPath 配置文件路径
     * @return 成功返回 true，失败返回 false
     */
    bool save(const std::string& configPath);
    
    /**
     * @brief 检查配置项是否存在
     * @param key 配置键（支持点分隔符访问嵌套值，如 "application.name"）
     * @return 存在返回 true，不存在返回 false
     */
    bool has(const std::string& key) const;
    
    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值，如果不存在则返回默认值
     */
    template<typename T>
    T get(const std::string& key, const T& defaultValue = T()) const;
    
    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     */
    template<typename T>
    void set(const std::string& key, const T& value);
    
    /**
     * @brief 删除配置项
     * @param key 配置键
     */
    void remove(const std::string& key);
    
    /**
     * @brief 清空所有配置
     */
    void clear();
    
    /**
     * @brief 获取原始 JSON 对象
     */
    const nlohmann::json& getJson() const { return config_; }
    
    /**
     * @brief 获取原始 JSON 对象（可修改）
     */
    nlohmann::json& getJson() { return config_; }

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    /**
     * @brief 从点分隔的键获取 JSON 节点
     * @param key 配置键
     * @param create 如果为 true，则创建不存在的节点
     * @return JSON 节点指针，失败返回 nullptr
     */
    nlohmann::json* getJsonNode(const std::string& key, bool create = false);
    const nlohmann::json* getJsonNode(const std::string& key) const;
    
    nlohmann::json config_;              ///< JSON 配置对象
    std::string currentConfigPath_;      ///< 当前配置文件路径
};

// ============================================================================
// 模板方法实现
// ============================================================================

template<typename T>
T ConfigManager::get(const std::string& key, const T& defaultValue) const
{
    const nlohmann::json* node = getJsonNode(key);
    if (node && !node->is_null()) {
        try {
            return node->get<T>();
        } catch (const nlohmann::json::exception&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

template<typename T>
void ConfigManager::set(const std::string& key, const T& value)
{
    nlohmann::json* node = getJsonNode(key, true);
    if (node) {
        *node = value;
    }
}

} // namespace DeviceStudio
