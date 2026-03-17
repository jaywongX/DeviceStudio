/**
 * @file luascriptengine.h
 * @brief Lua脚本引擎
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QTimer>
#include <sol/sol.hpp>
#include <memory>
#include <functional>
#include <atomic>

namespace DeviceStudio {

class DeviceManager;
class DataStore;

/**
 * @brief Lua脚本引擎配置
 */
struct LuaEngineConfig {
    bool enableIO = true;           // 是否允许IO操作
    bool enableOS = false;          // 是否允许OS操作（危险）
    int instructionLimit = 1000000; // 指令限制（防止死循环）
    QString scriptPath = "./scripts"; // 脚本路径
};

/**
 * @brief 定时器信息
 */
struct LuaTimer {
    int id;
    std::function<void()> callback;
    bool singleShot;
};

/**
 * @brief Lua脚本引擎类
 * 
 * 提供Lua脚本执行环境，支持设备操作和数据处理
 */
class LuaScriptEngine : public QObject
{
    Q_OBJECT

public:
    explicit LuaScriptEngine(QObject* parent = nullptr);
    explicit LuaScriptEngine(const LuaEngineConfig& config, QObject* parent = nullptr);
    ~LuaScriptEngine() override;
    
    // ========== 配置 ==========
    
    /**
     * @brief 设置配置
     */
    void setConfig(const LuaEngineConfig& config);
    LuaEngineConfig getConfig() const { return config_; }
    
    /**
     * @brief 设置设备管理器
     */
    void setDeviceManager(DeviceManager* manager);
    
    /**
     * @brief 设置数据存储
     */
    void setDataStore(DataStore* store);
    
    // ========== 脚本操作 ==========
    
    /**
     * @brief 加载脚本文件
     */
    bool loadScript(const QString& filePath);
    
    /**
     * @brief 执行脚本字符串
     */
    bool executeScript(const QString& script);
    
    /**
     * @brief 调用脚本函数
     */
    QVariant callFunction(const QString& functionName, const QVariantList& args = QVariantList());
    
    /**
     * @brief 检查函数是否存在
     */
    bool hasFunction(const QString& functionName);
    
    /**
     * @brief 注册全局变量
     */
    void setGlobalVariable(const QString& name, const QVariant& value);
    
    /**
     * @brief 获取全局变量
     */
    QVariant getGlobalVariable(const QString& name);
    
    /**
     * @brief 注册C++函数
     */
    template<typename Func>
    void registerFunction(const QString& name, Func func) {
        lua_->set(name.toStdString(), func);
    }
    
    // ========== 错误处理 ==========
    
    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const { return lastError_; }
    
    /**
     * @brief 清除错误
     */
    void clearError();
    
    /**
     * @brief 停止所有定时器
     */
    void stopAllTimers();
    
    /**
     * @brief 请求停止脚本执行
     */
    void stop();
    
    /**
     * @brief 是否已请求停止
     */
    bool isStopRequested() const { return stopRequested_; }

signals:
    /**
     * @brief 脚本执行完成信号
     */
    void scriptExecuted(const QString& script, bool success);
    
    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& error);
    
    /**
     * @brief 打印输出信号
     */
    void printOutput(const QString& message);
    
    /**
     * @brief 数据发送请求信号
     */
    void sendDataRequested(const QString& deviceName, const QByteArray& data);

private:
    void initializeLua();
    void registerStandardLibraries();
    void registerDeviceAPI();
    void registerDataAPI();
    void registerUtilityAPI();
    void registerProtocolAPI();
    void registerFileAPI();
    void registerTimerAPI();
    void registerStringAPI();
    void registerMathAPI();
    
    LuaEngineConfig config_;
    std::unique_ptr<sol::state> lua_;
    DeviceManager* deviceManager_ = nullptr;
    DataStore* dataStore_ = nullptr;
    QString lastError_;
    
    // 定时器管理
    QMap<int, QTimer*> timers_;
    int nextTimerId_ = 1;
    
    // 停止标志
    std::atomic<bool> stopRequested_{false};
};

} // namespace DeviceStudio
