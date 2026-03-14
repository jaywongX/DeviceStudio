/**
 * @file luascriptengine.cpp
 * @brief Lua脚本引擎实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "luascriptengine.h"
#include "core/devicemanager/devicemanager.h"
#include "core/datacenter/datastore.h"
#include "utils/log/logger.h"
#include <QFile>
#include <QDateTime>

namespace DeviceStudio {

LuaScriptEngine::LuaScriptEngine(QObject* parent)
    : QObject(parent)
{
    initializeLua();
}

LuaScriptEngine::LuaScriptEngine(const LuaEngineConfig& config, QObject* parent)
    : QObject(parent)
    , config_(config)
{
    initializeLua();
}

LuaScriptEngine::~LuaScriptEngine()
{
}

void LuaScriptEngine::setConfig(const LuaEngineConfig& config)
{
    config_ = config;
    // 重新初始化以应用新配置
    initializeLua();
}

void LuaScriptEngine::setDeviceManager(DeviceManager* manager)
{
    deviceManager_ = manager;
    registerDeviceAPI();
}

void LuaScriptEngine::setDataStore(DataStore* store)
{
    dataStore_ = store;
    registerDataAPI();
}

void LuaScriptEngine::initializeLua()
{
    // 创建Lua状态机
    lua_ = std::make_unique<sol::state>();
    
    // 开启基础库
    lua_->open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
    
    // 根据配置开启其他库
    if (config_.enableIO) {
        lua_->open_libraries(sol::lib::io);
    }
    
    if (config_.enableOS) {
        DS_LOG_WARN("OS library enabled - this may be a security risk");
        lua_->open_libraries(sol::lib::os);
    }
    
    // 设置指令限制
    if (config_.instructionLimit > 0) {
        lua_->script("debug.sethook(function() error('Script execution timeout') end, '', " + 
                    std::to_string(config_.instructionLimit) + ")");
    }
    
    // 注册标准库
    registerStandardLibraries();
    registerUtilityAPI();
    
    DS_LOG_INFO("Lua script engine initialized");
}

void LuaScriptEngine::registerStandardLibraries()
{
    // 注册print函数，重定向到Qt信号
    lua_->set_function("print", [this](sol::variadic_args args) {
        QString message;
        for (auto arg : args) {
            if (!message.isEmpty()) message += " ";
            
            if (arg.is<std::string>()) {
                message += QString::fromStdString(arg.as<std::string>());
            } else if (arg.is<int>()) {
                message += QString::number(arg.as<int>());
            } else if (arg.is<double>()) {
                message += QString::number(arg.as<double>());
            } else if (arg.is<bool>()) {
                message += arg.as<bool>() ? "true" : "false";
            }
        }
        emit printOutput(message);
        DS_LOG_DEBUG("Lua print: {}", message.toStdString());
    });
    
    // 注册日志函数
    lua_->set_function("log", [this](const std::string& level, const std::string& message) {
        QString msg = QString::fromStdString(message);
        if (level == "info") {
            DS_LOG_INFO("{}", msg.toStdString());
        } else if (level == "warn") {
            DS_LOG_WARN("{}", msg.toStdString());
        } else if (level == "error") {
            DS_LOG_ERROR("{}", msg.toStdString());
        } else {
            DS_LOG_DEBUG("{}", msg.toStdString());
        }
    });
}

void LuaScriptEngine::registerDeviceAPI()
{
    if (!deviceManager_) return;
    
    // 创建Device命名空间
    sol::table device = lua_->create_named_table("Device");
    
    // 设备列表
    device.set_function("list", [this]() {
        // 返回设备列表
        sol::table devices = lua_->create_table();
        // TODO: 实现设备列表
        return devices;
    });
    
    // 连接设备
    device.set_function("connect", [this](const std::string& deviceName) -> bool {
        if (!deviceManager_) return false;
        // TODO: 实现设备连接
        DS_LOG_INFO("Lua: Connecting to device {}", deviceName);
        return true;
    });
    
    // 断开设备
    device.set_function("disconnect", [this](const std::string& deviceName) {
        if (!deviceManager_) return;
        // TODO: 实现设备断开
        DS_LOG_INFO("Lua: Disconnecting device {}", deviceName);
    });
    
    // 发送数据
    device.set_function("send", [this](const std::string& deviceName, const std::string& data) -> int {
        if (!deviceManager_) return -1;
        // TODO: 实现数据发送
        DS_LOG_DEBUG("Lua: Sending {} bytes to {}", data.size(), deviceName);
        return static_cast<int>(data.size());
    });
    
    // 接收数据
    device.set_function("receive", [this](const std::string& deviceName) -> std::string {
        if (!deviceManager_) return "";
        // TODO: 实现数据接收
        return "";
    });
    
    DS_LOG_DEBUG("Device API registered");
}

void LuaScriptEngine::registerDataAPI()
{
    if (!dataStore_) return;
    
    // 创建Data命名空间
    sol::table data = lua_->create_named_table("Data");
    
    // 设置数据项
    data.set_function("set", [this](const std::string& key, double value) {
        if (!dataStore_) return;
        // TODO: 实现数据设置
        DS_LOG_DEBUG("Lua: Setting data {} = {}", key, value);
    });
    
    // 获取数据项
    data.set_function("get", [this](const std::string& key) -> double {
        if (!dataStore_) return 0.0;
        // TODO: 实现数据获取
        return 0.0;
    });
    
    // 保存数据
    data.set_function("save", [this](const std::string& fileName) -> bool {
        if (!dataStore_) return false;
        // TODO: 实现数据保存
        DS_LOG_INFO("Lua: Saving data to {}", fileName);
        return true;
    });
    
    // 加载数据
    data.set_function("load", [this](const std::string& fileName) -> bool {
        if (!dataStore_) return false;
        // TODO: 实现数据加载
        DS_LOG_INFO("Lua: Loading data from {}", fileName);
        return true;
    });
    
    DS_LOG_DEBUG("Data API registered");
}

void LuaScriptEngine::registerUtilityAPI()
{
    // 创建Util命名空间
    sol::table util = lua_->create_named_table("Util");
    
    // 延时函数
    util.set_function("sleep", [](int ms) {
        QThread::msleep(ms);
    });
    
    // 获取当前时间
    util.set_function("now", []() -> double {
        return QDateTime::currentMSecsSinceEpoch() / 1000.0;
    });
    
    // 格式化时间
    util.set_function("formatTime", [](double timestamp) -> std::string {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timestamp * 1000));
        return dt.toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString();
    });
    
    // 字符串转换
    util.set_function("toHex", [](const std::string& data) -> std::string {
        QString result;
        for (unsigned char c : data) {
            result += QString("%1 ").arg(c, 2, 16, QChar('0')).toUpper();
        }
        return result.trimmed().toStdString();
    });
    
    util.set_function("fromHex", [](const std::string& hex) -> std::string {
        QString hexStr = QString::fromStdString(hex).remove(' ');
        QByteArray data = QByteArray::fromHex(hexStr.toLatin1());
        return std::string(data.constData(), data.size());
    });
    
    DS_LOG_DEBUG("Utility API registered");
}

bool LuaScriptEngine::loadScript(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        lastError_ = QString("Failed to open script file: %1").arg(filePath);
        DS_LOG_ERROR("{}", lastError_.toStdString());
        emit errorOccurred(lastError_);
        return false;
    }
    
    QString script = QString::fromUtf8(file.readAll());
    file.close();
    
    bool success = executeScript(script);
    if (success) {
        DS_LOG_INFO("Script loaded: {}", filePath.toStdString());
    }
    
    return success;
}

bool LuaScriptEngine::executeScript(const QString& script)
{
    clearError();
    
    try {
        sol::protected_function_result result = lua_->safe_script(script.toStdString());
        
        if (!result.valid()) {
            sol::error err = result;
            lastError_ = QString::fromStdString(err.what());
            DS_LOG_ERROR("Lua script error: {}", lastError_.toStdString());
            emit errorOccurred(lastError_);
            emit scriptExecuted(script, false);
            return false;
        }
        
        emit scriptExecuted(script, true);
        return true;
    }
    catch (const std::exception& e) {
        lastError_ = QString::fromStdString(e.what());
        DS_LOG_ERROR("Lua exception: {}", lastError_.toStdString());
        emit errorOccurred(lastError_);
        emit scriptExecuted(script, false);
        return false;
    }
}

QVariant LuaScriptEngine::callFunction(const QString& functionName, const QVariantList& args)
{
    clearError();
    
    try {
        sol::protected_function func = (*lua_)[functionName.toStdString()];
        
        if (!func.valid()) {
            lastError_ = QString("Function not found: %1").arg(functionName);
            DS_LOG_ERROR("{}", lastError_.toStdString());
            emit errorOccurred(lastError_);
            return QVariant();
        }
        
        // 调用函数
        sol::protected_function_result result;
        
        if (args.isEmpty()) {
            result = func();
        } else {
            // 转换参数
            std::vector<sol::object> luaArgs;
            for (const QVariant& arg : args) {
                if (arg.typeId() == QMetaType::Int || arg.typeId() == QMetaType::LongLong) {
                    luaArgs.push_back(sol::make_object(*lua_, arg.toInt()));
                } else if (arg.typeId() == QMetaType::Double) {
                    luaArgs.push_back(sol::make_object(*lua_, arg.toDouble()));
                } else if (arg.typeId() == QMetaType::Bool) {
                    luaArgs.push_back(sol::make_object(*lua_, arg.toBool()));
                } else if (arg.typeId() == QMetaType::QString) {
                    luaArgs.push_back(sol::make_object(*lua_, arg.toString().toStdString()));
                }
            }
            result = func(sol::as_args(luaArgs));
        }
        
        if (!result.valid()) {
            sol::error err = result;
            lastError_ = QString::fromStdString(err.what());
            DS_LOG_ERROR("Lua function call error: {}", lastError_.toStdString());
            emit errorOccurred(lastError_);
            return QVariant();
        }
        
        // 转换返回值
        if (result.return_count() == 0) {
            return QVariant();
        }
        
        sol::object returnValue = result;
        if (returnValue.is<int>()) {
            return returnValue.as<int>();
        } else if (returnValue.is<double>()) {
            return returnValue.as<double>();
        } else if (returnValue.is<bool>()) {
            return returnValue.as<bool>();
        } else if (returnValue.is<std::string>()) {
            return QString::fromStdString(returnValue.as<std::string>());
        }
        
        return QVariant();
    }
    catch (const std::exception& e) {
        lastError_ = QString::fromStdString(e.what());
        DS_LOG_ERROR("Lua exception: {}", lastError_.toStdString());
        emit errorOccurred(lastError_);
        return QVariant();
    }
}

bool LuaScriptEngine::hasFunction(const QString& functionName)
{
    sol::object obj = (*lua_)[functionName.toStdString()];
    return obj.is<sol::function>();
}

void LuaScriptEngine::setGlobalVariable(const QString& name, const QVariant& value)
{
    if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::LongLong) {
        lua_->set(name.toStdString(), value.toInt());
    } else if (value.typeId() == QMetaType::Double) {
        lua_->set(name.toStdString(), value.toDouble());
    } else if (value.typeId() == QMetaType::Bool) {
        lua_->set(name.toStdString(), value.toBool());
    } else if (value.typeId() == QMetaType::QString) {
        lua_->set(name.toStdString(), value.toString().toStdString());
    }
}

QVariant LuaScriptEngine::getGlobalVariable(const QString& name)
{
    sol::object obj = (*lua_)[name.toStdString()];
    
    if (obj.is<int>()) {
        return obj.as<int>();
    } else if (obj.is<double>()) {
        return obj.as<double>();
    } else if (obj.is<bool>()) {
        return obj.as<bool>();
    } else if (obj.is<std::string>()) {
        return QString::fromStdString(obj.as<std::string>());
    }
    
    return QVariant();
}

void LuaScriptEngine::clearError()
{
    lastError_.clear();
}

} // namespace DeviceStudio
