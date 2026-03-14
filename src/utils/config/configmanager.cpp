/**
 * @file configmanager.cpp
 * @brief 配置管理器实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "configmanager.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace DeviceStudio {

ConfigManager& ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load(const std::string& configPath)
{
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << configPath << std::endl;
            return false;
        }
        
        file >> config_;
        currentConfigPath_ = configPath;
        
        return true;
        
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "JSON parse error: " << ex.what() << std::endl;
        return false;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to load config: " << ex.what() << std::endl;
        return false;
    }
}

bool ConfigManager::save(const std::string& configPath)
{
    try {
        std::string path = configPath.empty() ? currentConfigPath_ : configPath;
        if (path.empty()) {
            std::cerr << "No config file path specified" << std::endl;
            return false;
        }
        
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << path << std::endl;
            return false;
        }
        
        file << config_.dump(4);
        
        return true;
        
    } catch (const std::exception& ex) {
        std::cerr << "Failed to save config: " << ex.what() << std::endl;
        return false;
    }
}

bool ConfigManager::has(const std::string& key) const
{
    return getJsonNode(key) != nullptr;
}

void ConfigManager::remove(const std::string& key)
{
    // 分割键路径
    std::vector<std::string> keys;
    std::stringstream ss(key);
    std::string item;
    while (std::getline(ss, item, '.')) {
        keys.push_back(item);
    }
    
    if (keys.empty()) {
        return;
    }
    
    // 定位到父节点
    nlohmann::json* node = &config_;
    for (size_t i = 0; i < keys.size() - 1; ++i) {
        if (!node->is_object() || !node->contains(keys[i])) {
            return;
        }
        node = &(*node)[keys[i]];
    }
    
    // 删除最后一个键
    if (node->is_object()) {
        node->erase(keys.back());
    }
}

void ConfigManager::clear()
{
    config_.clear();
}

nlohmann::json* ConfigManager::getJsonNode(const std::string& key, bool create)
{
    // 分割键路径
    std::vector<std::string> keys;
    std::stringstream ss(key);
    std::string item;
    while (std::getline(ss, item, '.')) {
        keys.push_back(item);
    }
    
    if (keys.empty()) {
        return nullptr;
    }
    
    // 遍历键路径
    nlohmann::json* node = &config_;
    for (const auto& k : keys) {
        if (!node->is_object()) {
            if (create) {
                *node = nlohmann::json::object();
            } else {
                return nullptr;
            }
        }
        
        if (!node->contains(k)) {
            if (create) {
                (*node)[k] = nlohmann::json::object();
            } else {
                return nullptr;
            }
        }
        
        node = &(*node)[k];
    }
    
    return node;
}

const nlohmann::json* ConfigManager::getJsonNode(const std::string& key) const
{
    // 分割键路径
    std::vector<std::string> keys;
    std::stringstream ss(key);
    std::string item;
    while (std::getline(ss, item, '.')) {
        keys.push_back(item);
    }
    
    if (keys.empty()) {
        return nullptr;
    }
    
    // 遍历键路径
    const nlohmann::json* node = &config_;
    for (const auto& k : keys) {
        if (!node->is_object() || !node->contains(k)) {
            return nullptr;
        }
        node = &(*node)[k];
    }
    
    return node;
}

} // namespace DeviceStudio
