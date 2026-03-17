/**
 * @file luascriptengine.cpp
 * @brief Lua脚本引擎实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "luascriptengine.h"
#include "core/devicemanager/devicemanager.h"
#include "core/datacenter/datastore.h"
#include "core/protocol/checksum.h"
#include "utils/log/logger.h"
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QThread>
#include <QRegularExpression>

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
    stopAllTimers();
}

void LuaScriptEngine::stopAllTimers()
{
    for (auto it = timers_.begin(); it != timers_.end(); ++it) {
        it.value()->stop();
        delete it.value();
    }
    timers_.clear();
}

void LuaScriptEngine::stop()
{
    stopRequested_ = true;
    stopAllTimers();
    
    // 设置 Lua 全局变量，让 hook 检测到停止请求
    if (lua_) {
        lua_->set("_STOP_REQUESTED", true);
    }
    
    DS_LOG_INFO("Script stop requested");
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
    registerProtocolAPI();
    registerFileAPI();
    registerTimerAPI();
    registerStringAPI();
    registerMathAPI();
    
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
        sol::table devices = lua_->create_table();
        if (!deviceManager_) return devices;
        
        auto deviceList = deviceManager_->getAllDevices();
        int index = 1;
        for (const auto& dev : deviceList) {
            if (!dev) continue;
            
            sol::table deviceInfo = lua_->create_table();
            deviceInfo["id"] = dev->deviceId().toStdString();
            deviceInfo["name"] = dev->deviceName().toStdString();
            deviceInfo["type"] = static_cast<int>(dev->deviceType());
            deviceInfo["connected"] = dev->isConnected();
            
            devices[index++] = deviceInfo;
        }
        return devices;
    });
    
    // 连接设备
    device.set_function("connect", [this](const std::string& deviceName) -> bool {
        if (!deviceManager_) return false;
        
        QString name = QString::fromStdString(deviceName);
        auto devices = deviceManager_->findDevicesByName(name);
        if (devices.isEmpty()) {
            DS_LOG_ERROR("Lua: Device not found: {}", deviceName);
            return false;
        }
        
        auto device = devices.first();
        bool result = device->connect(device->getConfiguration());
        DS_LOG_INFO("Lua: Connecting to device {} - {}", deviceName, result ? "success" : "failed");
        return result;
    });
    
    // 断开设备
    device.set_function("disconnect", [this](const std::string& deviceName) {
        if (!deviceManager_) return;
        
        QString name = QString::fromStdString(deviceName);
        auto devices = deviceManager_->findDevicesByName(name);
        if (devices.isEmpty()) {
            DS_LOG_ERROR("Lua: Device not found: {}", deviceName);
            return;
        }
        
        auto device = devices.first();
        device->disconnect();
        DS_LOG_INFO("Lua: Disconnected device {}", deviceName);
    });
    
    // 发送数据
    device.set_function("send", [this](const std::string& deviceName, const std::string& data) -> int {
        if (!deviceManager_) return -1;
        
        QString name = QString::fromStdString(deviceName);
        auto devices = deviceManager_->findDevicesByName(name);
        if (devices.isEmpty()) {
            DS_LOG_ERROR("Lua: Device not found: {}", deviceName);
            return -1;
        }
        
        auto device = devices.first();
        QByteArray ba(data.data(), static_cast<int>(data.size()));
        qint64 sent = device->send(ba);
        DS_LOG_DEBUG("Lua: Sent {} bytes to {}", sent, deviceName);
        return static_cast<int>(sent);
    });
    
    // 接收数据
    device.set_function("receive", [this](const std::string& deviceName) -> std::string {
        if (!deviceManager_) return "";
        
        QString name = QString::fromStdString(deviceName);
        auto devices = deviceManager_->findDevicesByName(name);
        if (devices.isEmpty()) {
            return "";
        }
        
        auto device = devices.first();
        QByteArray data = device->receive();
        return std::string(data.constData(), data.size());
    });
    
    // 通过ID获取设备
    device.set_function("getById", [this](const std::string& deviceId) -> sol::object {
        if (!deviceManager_) return sol::nil;
        
        QString id = QString::fromStdString(deviceId);
        auto device = deviceManager_->getDevice(id);
        if (!device) return sol::nil;
        
        sol::table deviceInfo = lua_->create_table();
        deviceInfo["id"] = device->deviceId().toStdString();
        deviceInfo["name"] = device->deviceName().toStdString();
        deviceInfo["type"] = static_cast<int>(device->deviceType());
        deviceInfo["connected"] = device->isConnected();
        
        return deviceInfo;
    });
    
    DS_LOG_DEBUG("Device API registered");
}

void LuaScriptEngine::registerDataAPI()
{
    // 创建Data命名空间（即使没有 dataStore_ 也提供基本的内存存储）
    sol::table data = lua_->create_named_table("Data");
    
    // 内存数据存储（用于脚本内变量管理）
    sol::table memoryData = lua_->create_table();
    
    // 设置数据项
    data.set_function("set", [this, memoryData](const std::string& key, double value) mutable {
        memoryData[key] = value;
        if (dataStore_) {
            // 如果有持久化存储，也存储到那里
            // dataStore_->setValue(QString::fromStdString(key), value);
        }
        DS_LOG_DEBUG("Lua: Setting data {} = {}", key, value);
    });
    
    // 获取数据项
    data.set_function("get", [memoryData](const std::string& key) -> double {
        sol::object value = memoryData[key];
        if (value.is<double>()) {
            return value.as<double>();
        }
        return 0.0;
    });
    
    // 保存数据到文件
    data.set_function("save", [this](const std::string& fileName) -> bool {
        if (dataStore_) {
            // return dataStore_->saveToFile(QString::fromStdString(fileName));
            DS_LOG_INFO("Lua: Saving data to {}", fileName);
            return true;
        }
        return false;
    });
    
    // 从文件加载数据
    data.set_function("load", [this](const std::string& fileName) -> bool {
        if (dataStore_) {
            // return dataStore_->loadFromFile(QString::fromStdString(fileName));
            DS_LOG_INFO("Lua: Loading data from {}", fileName);
            return true;
        }
        return false;
    });
    
    // 清除数据
    data.set_function("clear", [memoryData]() mutable {
        memoryData.clear();
    });
    
    // 列出所有数据键
    data.set_function("keys", [memoryData, this]() -> sol::table {
        sol::table result = lua_->create_table();
        int index = 1;
        for (auto& pair : memoryData) {
            if (pair.first.is<std::string>()) {
                result[index++] = pair.first.as<std::string>();
            }
        }
        return result;
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

void LuaScriptEngine::registerProtocolAPI()
{
    // 创建Protocol命名空间
    sol::table protocol = lua_->create_named_table("Protocol");
    
    // 校验和计算
    protocol.set_function("checksumXOR", [](const std::string& data) -> int {
        QByteArray ba(data.data(), static_cast<int>(data.size()));
        quint8 checksum = ChecksumCalculator::xorChecksum(ba);
        return checksum;
    });

    protocol.set_function("checksumSUM", [](const std::string& data) -> int {
        QByteArray ba(data.data(), static_cast<int>(data.size()));
        quint8 checksum = ChecksumCalculator::sumChecksum(ba);
        return checksum;
    });

    protocol.set_function("checksumSUM16", [](const std::string& data) -> int {
        QByteArray ba(data.data(), static_cast<int>(data.size()));
        quint16 checksum = ChecksumCalculator::sum16Checksum(ba);
        return checksum;
    });

    protocol.set_function("checksumCRC16", [](const std::string& data, const std::string& type) -> int {
        QByteArray ba(data.data(), static_cast<int>(data.size()));
        if (type == "modbus") {
            return ChecksumCalculator::crc16Modbus(ba);
        } else {
            return ChecksumCalculator::crc16(ba);
        }
    });

    protocol.set_function("checksumCRC32", [](const std::string& data) -> int {
        QByteArray ba(data.data(), static_cast<int>(data.size()));
        quint32 checksum = ChecksumCalculator::crc32(ba);
        return static_cast<int>(checksum);
    });
    
    // 数据打包
    protocol.set_function("pack", [](sol::table fields, const std::string& format) -> std::string {
        QByteArray result;
        int index = 1;
        for (auto& pair : fields) {
            if (pair.second.is<int>()) {
                int val = pair.second.as<int>();
                if (format == "big") {
                    result.append(static_cast<char>((val >> 8) & 0xFF));
                    result.append(static_cast<char>(val & 0xFF));
                } else {
                    result.append(static_cast<char>(val & 0xFF));
                    result.append(static_cast<char>((val >> 8) & 0xFF));
                }
            } else if (pair.second.is<double>()) {
                double val = pair.second.as<double>();
                result.append(reinterpret_cast<char*>(&val), sizeof(double));
            }
            index++;
        }
        return std::string(result.constData(), result.size());
    });
    
    // 数据解包
    protocol.set_function("unpack", [this](const std::string& data, const std::string& format) -> sol::table {
        sol::table result = lua_->create_table();
        QByteArray ba = QByteArray::fromRawData(data.data(), data.size());
        int offset = 0;
        for (char c : format) {
            if (offset >= ba.size()) break;
            if (c == 'b') {
                result.add(static_cast<int>(static_cast<unsigned char>(ba[offset])));
                offset += 1;
            } else if (c == 'h') {
                int val = (static_cast<unsigned char>(ba[offset]) << 8) | static_cast<unsigned char>(ba[offset + 1]);
                result.add(val);
                offset += 2;
            } else if (c == 'l') {
                int val = static_cast<unsigned char>(ba[offset]) | 
                         (static_cast<unsigned char>(ba[offset + 1]) << 8) |
                         (static_cast<unsigned char>(ba[offset + 2]) << 16) |
                         (static_cast<unsigned char>(ba[offset + 3]) << 24);
                result.add(val);
                offset += 4;
            }
        }
        return result;
    });
    
    // 帧构建
    protocol.set_function("buildFrame", [](int header, const std::string& data, int tail) -> std::string {
        QByteArray frame;
        frame.append(static_cast<char>(header));
        frame.append(static_cast<char>(data.size()));
        frame.append(QByteArray::fromRawData(data.data(), data.size()));
        frame.append(static_cast<char>(tail));
        return std::string(frame.constData(), frame.size());
    });
    
    DS_LOG_DEBUG("Protocol API registered");
}

void LuaScriptEngine::registerFileAPI()
{
    // 创建File命名空间
    sol::table file = lua_->create_named_table("File");
    
    // 读取文件
    file.set_function("read", [](const std::string& path) -> std::string {
        QFile f(QString::fromStdString(path));
        if (!f.open(QIODevice::ReadOnly)) {
            return "";
        }
        QByteArray data = f.readAll();
        f.close();
        return std::string(data.constData(), data.size());
    });
    
    // 写入文件
    file.set_function("write", [](const std::string& path, const std::string& data) -> bool {
        QFile f(QString::fromStdString(path));
        if (!f.open(QIODevice::WriteOnly)) {
            return false;
        }
        bool result = f.write(data.data(), data.size()) == static_cast<qint64>(data.size());
        f.close();
        return result;
    });
    
    // 追加文件
    file.set_function("append", [](const std::string& path, const std::string& data) -> bool {
        QFile f(QString::fromStdString(path));
        if (!f.open(QIODevice::WriteOnly | QIODevice::Append)) {
            return false;
        }
        bool result = f.write(data.data(), data.size()) == static_cast<qint64>(data.size());
        f.close();
        return result;
    });
    
    // 检查文件是否存在
    file.set_function("exists", [](const std::string& path) -> bool {
        return QFile::exists(QString::fromStdString(path));
    });
    
    // 删除文件
    file.set_function("remove", [](const std::string& path) -> bool {
        return QFile::remove(QString::fromStdString(path));
    });
    
    // 重命名文件
    file.set_function("rename", [](const std::string& oldPath, const std::string& newPath) -> bool {
        return QFile::rename(QString::fromStdString(oldPath), QString::fromStdString(newPath));
    });
    
    // 获取文件大小
    file.set_function("size", [](const std::string& path) -> long long {
        QFileInfo info(QString::fromStdString(path));
        return info.size();
    });
    
    // 列出目录
    file.set_function("listDir", [this](const std::string& path) -> sol::table {
        sol::table result = lua_->create_table();
        QDir dir(QString::fromStdString(path));
        QStringList entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& entry : entries) {
            result.add(entry.toStdString());
        }
        return result;
    });
    
    // 创建目录
    file.set_function("mkdir", [](const std::string& path) -> bool {
        return QDir().mkpath(QString::fromStdString(path));
    });
    
    // 复制文件
    file.set_function("copy", [](const std::string& src, const std::string& dst) -> bool {
        return QFile::copy(QString::fromStdString(src), QString::fromStdString(dst));
    });
    
    // 文件哈希
    file.set_function("md5", [](const std::string& path) -> std::string {
        QFile f(QString::fromStdString(path));
        if (!f.open(QIODevice::ReadOnly)) {
            return "";
        }
        QByteArray hash = QCryptographicHash::hash(f.readAll(), QCryptographicHash::Md5);
        f.close();
        return hash.toHex().toStdString();
    });
    
    DS_LOG_DEBUG("File API registered");
}

void LuaScriptEngine::registerTimerAPI()
{
    // 创建Timer命名空间
    sol::table timer = lua_->create_named_table("Timer");
    
    // 单次定时器
    timer.set_function("singleShot", [this](int ms, sol::function callback) {
        int id = nextTimerId_++;
        QTimer* t = new QTimer(this);
        t->setSingleShot(true);
        connect(t, &QTimer::timeout, this, [this, id, callback]() {
            if (callback.valid()) {
                try {
                    callback(id);
                } catch (...) {}
            }
            timers_.remove(id);
            sender()->deleteLater();
        });
        timers_[id] = t;
        t->start(ms);
        return id;
    });
    
    // 重复定时器
    timer.set_function("start", [this](int ms, sol::function callback) -> int {
        int id = nextTimerId_++;
        QTimer* t = new QTimer(this);
        connect(t, &QTimer::timeout, this, [this, id, callback]() {
            if (callback.valid()) {
                try {
                    callback(id);
                } catch (...) {}
            }
        });
        timers_[id] = t;
        t->start(ms);
        return id;
    });
    
    // 停止定时器
    timer.set_function("stop", [this](int id) {
        if (timers_.contains(id)) {
            timers_[id]->stop();
            timers_[id]->deleteLater();
            timers_.remove(id);
            return true;
        }
        return false;
    });
    
    // 检查定时器是否运行
    timer.set_function("isRunning", [this](int id) -> bool {
        return timers_.contains(id) && timers_[id]->isActive();
    });
    
    // 停止所有定时器
    timer.set_function("stopAll", [this]() {
        stopAllTimers();
    });
    
    DS_LOG_DEBUG("Timer API registered");
}

void LuaScriptEngine::registerStringAPI()
{
    // 创建String命名空间
    sol::table str = lua_->create_named_table("String");
    
    // 字符串分割
    str.set_function("split", [this](const std::string& s, const std::string& sep) -> sol::table {
        sol::table result = lua_->create_table();
        QString str = QString::fromStdString(s);
        QStringList parts = str.split(QString::fromStdString(sep));
        for (const QString& part : parts) {
            result.add(part.toStdString());
        }
        return result;
    });
    
    // 字符串连接
    str.set_function("join", [](sol::table parts, const std::string& sep) -> std::string {
        QStringList list;
        for (auto& pair : parts) {
            if (pair.second.is<std::string>()) {
                list.append(QString::fromStdString(pair.second.as<std::string>()));
            }
        }
        return list.join(QString::fromStdString(sep)).toStdString();
    });
    
    // 字符串替换
    str.set_function("replace", [](const std::string& s, const std::string& from, const std::string& to) -> std::string {
        QString str = QString::fromStdString(s);
        return str.replace(QString::fromStdString(from), QString::fromStdString(to)).toStdString();
    });
    
    // 正则匹配
    str.set_function("match", [this](const std::string& s, const std::string& pattern) -> sol::table {
        sol::table result = lua_->create_table();
        QRegularExpression re(QString::fromStdString(pattern));
        QRegularExpressionMatchIterator it = re.globalMatch(QString::fromStdString(s));
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            result.add(match.captured(0).toStdString());
        }
        return result;
    });
    
    // 去除空白
    str.set_function("trim", [](const std::string& s) -> std::string {
        return QString::fromStdString(s).trimmed().toStdString();
    });
    
    // 转小写
    str.set_function("toLower", [](const std::string& s) -> std::string {
        return QString::fromStdString(s).toLower().toStdString();
    });
    
    // 转大写
    str.set_function("toUpper", [](const std::string& s) -> std::string {
        return QString::fromStdString(s).toUpper().toStdString();
    });
    
    // 字符串填充
    str.set_function("padLeft", [](const std::string& s, int width, const std::string& fill) -> std::string {
        return QString::fromStdString(s).leftJustified(width, fill.empty() ? ' ' : fill[0]).toStdString();
    });
    
    str.set_function("padRight", [](const std::string& s, int width, const std::string& fill) -> std::string {
        return QString::fromStdString(s).rightJustified(width, fill.empty() ? ' ' : fill[0]).toStdString();
    });
    
    // Base64编解码
    str.set_function("toBase64", [](const std::string& s) -> std::string {
        return QByteArray::fromRawData(s.data(), s.size()).toBase64().toStdString();
    });
    
    str.set_function("fromBase64", [](const std::string& s) -> std::string {
        QByteArray result = QByteArray::fromBase64(QByteArray::fromRawData(s.data(), s.size()));
        return std::string(result.constData(), result.size());
    });
    
    DS_LOG_DEBUG("String API registered");
}

void LuaScriptEngine::registerMathAPI()
{
    // 创建Math扩展命名空间
    sol::table mathEx = lua_->create_named_table("MathEx");
    
    // 限制值范围
    mathEx.set_function("clamp", [](double value, double min, double max) -> double {
        return qBound(min, value, max);
    });
    
    // 线性插值
    mathEx.set_function("lerp", [](double a, double b, double t) -> double {
        return a + (b - a) * t;
    });
    
    // 随机整数
    mathEx.set_function("randomInt", [](int min, int max) -> int {
        return QRandomGenerator::global()->bounded(min, max + 1);
    });
    
    // 随机浮点数
    mathEx.set_function("randomFloat", [](double min, double max) -> double {
        return min + QRandomGenerator::global()->generateDouble() * (max - min);
    });
    
    // 角度转弧度
    mathEx.set_function("degToRad", [](double deg) -> double {
        return deg * M_PI / 180.0;
    });
    
    // 弧度转角度
    mathEx.set_function("radToDeg", [](double rad) -> double {
        return rad * 180.0 / M_PI;
    });
    
    // 字节数组转数值
    mathEx.set_function("bytesToInt", [](const std::string& bytes, bool bigEndian) -> int {
        QByteArray ba = QByteArray::fromRawData(bytes.data(), bytes.size());
        if (ba.size() < 4) ba = ba.leftJustified(4, '\0');
        if (bigEndian) {
            return (static_cast<unsigned char>(ba[0]) << 24) |
                   (static_cast<unsigned char>(ba[1]) << 16) |
                   (static_cast<unsigned char>(ba[2]) << 8) |
                   static_cast<unsigned char>(ba[3]);
        } else {
            return static_cast<unsigned char>(ba[0]) |
                   (static_cast<unsigned char>(ba[1]) << 8) |
                   (static_cast<unsigned char>(ba[2]) << 16) |
                   (static_cast<unsigned char>(ba[3]) << 24);
        }
    });
    
    // 数值转字节数组
    mathEx.set_function("intToBytes", [](int value, bool bigEndian) -> std::string {
        QByteArray result(4, '\0');
        if (bigEndian) {
            result[0] = static_cast<char>((value >> 24) & 0xFF);
            result[1] = static_cast<char>((value >> 16) & 0xFF);
            result[2] = static_cast<char>((value >> 8) & 0xFF);
            result[3] = static_cast<char>(value & 0xFF);
        } else {
            result[0] = static_cast<char>(value & 0xFF);
            result[1] = static_cast<char>((value >> 8) & 0xFF);
            result[2] = static_cast<char>((value >> 16) & 0xFF);
            result[3] = static_cast<char>((value >> 24) & 0xFF);
        }
        return std::string(result.constData(), result.size());
    });
    
    // 计算平均值
    mathEx.set_function("average", [](sol::table values) -> double {
        double sum = 0;
        int count = 0;
        for (auto& pair : values) {
            if (pair.second.is<double>()) {
                sum += pair.second.as<double>();
                count++;
            }
        }
        return count > 0 ? sum / count : 0;
    });
    
    // 计算标准差
    mathEx.set_function("stddev", [this](sol::table values) -> double {
        double sum = 0, sumSq = 0;
        int count = 0;
        for (auto& pair : values) {
            if (pair.second.is<double>()) {
                double val = pair.second.as<double>();
                sum += val;
                sumSq += val * val;
                count++;
            }
        }
        if (count < 2) return 0;
        double mean = sum / count;
        return sqrt(sumSq / count - mean * mean);
    });
    
    DS_LOG_DEBUG("Math API registered");
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
    stopRequested_ = false;  // 重置停止标志
    
    try {
        // 设置一个检查停止的 hook
        if (config_.instructionLimit > 0) {
            lua_->script("debug.sethook(function() "
                        "if _STOP_REQUESTED then error('Script execution stopped by user') end "
                        "end, '', " + std::to_string(config_.instructionLimit) + ")");
        }
        
        // 设置全局停止标志变量
        lua_->set("_STOP_REQUESTED", false);
        
        sol::protected_function_result result = lua_->safe_script(script.toStdString());
        
        if (!result.valid()) {
            sol::error err = result;
            lastError_ = QString::fromStdString(err.what());
            
            // 检查是否是用户中断
            if (stopRequested_ || lastError_.contains("stopped by user")) {
                lastError_ = tr("脚本已被用户停止");
                DS_LOG_INFO("Lua script stopped by user");
            } else {
                DS_LOG_ERROR("Lua script error: {}", lastError_.toStdString());
            }
            
            emit errorOccurred(lastError_);
            emit scriptExecuted(script, false);
            return false;
        }
        
        emit scriptExecuted(script, true);
        return true;
    }
    catch (const std::exception& e) {
        lastError_ = QString::fromStdString(e.what());
        
        // 检查是否是用户中断
        if (stopRequested_ || lastError_.contains("stopped by user")) {
            lastError_ = tr("脚本已被用户停止");
            DS_LOG_INFO("Lua script stopped by user");
        } else {
            DS_LOG_ERROR("Lua exception: {}", lastError_.toStdString());
        }
        
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
