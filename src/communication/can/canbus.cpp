/**
 * @file canbus.cpp
 * @brief CAN总线通信实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "canbus.h"
#include "log/logger.h"
#include <QCanBusDeviceInfo>
#include <QDateTime>

namespace DeviceStudio {

// ==================== CanBusConfig ====================

CanBusConfig CanBusConfig::fromVariantMap(const QVariantMap& map)
{
    CanBusConfig config;
    config.pluginName = map.value("pluginName", "socketcan").toString();
    config.interfaceName = map.value("interfaceName", "can0").toString();
    config.bitrate = map.value("bitrate", 500000).toInt();
    config.loopback = map.value("loopback", false).toBool();
    config.receiveOwn = map.value("receiveOwn", false).toBool();
    config.canFd = map.value("canFd", false).toBool();
    config.dataBitrate = map.value("dataBitrate", 2000000).toInt();
    return config;
}

QVariantMap CanBusConfig::toVariantMap() const
{
    QVariantMap map;
    map["pluginName"] = pluginName;
    map["interfaceName"] = interfaceName;
    map["bitrate"] = bitrate;
    map["loopback"] = loopback;
    map["receiveOwn"] = receiveOwn;
    map["canFd"] = canFd;
    map["dataBitrate"] = dataBitrate;
    return map;
}

// ==================== CanFrame ====================

CanFrame CanFrame::fromQCanBusFrame(const QCanBusFrame& frame)
{
    CanFrame cf;
    cf.id = frame.frameId();
    cf.extended = frame.hasExtendedFrameFormat();
    cf.rtr = frame.frameType() == QCanBusFrame::RemoteRequestFrame;
    cf.error = frame.frameType() == QCanBusFrame::ErrorFrame;
    cf.fdFormat = frame.hasFlexibleDataRateFormat();
    cf.bitrateSwitch = frame.hasBitrateSwitch();
    cf.payload = frame.payload();
    cf.timestamp = QDateTime::fromMSecsSinceEpoch(frame.timeStamp().microSeconds() / 1000);
    return cf;
}

QCanBusFrame CanFrame::toQCanBusFrame() const
{
    QCanBusFrame frame;
    frame.setFrameId(id);
    frame.setExtendedFrameFormat(extended);
    
    if (rtr) {
        frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
        frame.setPayload(QByteArray());
    } else {
        frame.setFrameType(QCanBusFrame::DataFrame);
        frame.setPayload(payload);
    }
    
    if (fdFormat) {
        frame.setFlexibleDataRateFormat(true);
        frame.setBitrateSwitch(bitrateSwitch);
    }
    
    return frame;
}

QString CanFrame::toString() const
{
    QString typeStr;
    if (error) typeStr = "ERR";
    else if (rtr) typeStr = "RTR";
    else typeStr = extended ? "EXT" : "STD";
    
    QString dataStr;
    if (!rtr && !error) {
        dataStr = payload.toHex(' ').toUpper();
    }
    
    return QString("[%1] %2 0x%3 [%4] %5")
        .arg(timestamp.toString("hh:mm:ss.zzz"))
        .arg(typeStr)
        .arg(id, extended ? 8 : 3, 16, QChar('0'))
        .arg(payload.size())
        .arg(dataStr);
}

// ==================== CanBus ====================

CanBus::CanBus(QObject* parent)
    : Device(DeviceType::Can, parent)
{
    setDeviceName(tr("CAN总线"));
}

CanBus::CanBus(const QString& pluginName, const QString& interfaceName, QObject* parent)
    : Device(DeviceType::Can, parent)
{
    config_.pluginName = pluginName;
    config_.interfaceName = interfaceName;
    setDeviceName(QString("CAN @ %1").arg(interfaceName));
}

CanBus::~CanBus()
{
    disconnect();
}

QStringList CanBus::availablePlugins()
{
    QStringList plugins;
    for (const QString& name : QCanBus::instance()->plugins()) {
        plugins.append(name);
    }
    return plugins;
}

QStringList CanBus::availableInterfaces(const QString& pluginName)
{
    QStringList interfaces;
    QString error;
    QCanBus* canBus = QCanBus::instance();
    for (const QCanBusDeviceInfo& info : canBus->availableDevices(pluginName, &error)) {
        interfaces.append(info.name());
    }
    return interfaces;
}

bool CanBus::connect(const QVariantMap& config)
{
    config_ = CanBusConfig::fromVariantMap(config);
    return applyConfig();
}

void CanBus::disconnect()
{
    if (canDevice_) {
        canDevice_->disconnectDevice();
        delete canDevice_;
        canDevice_ = nullptr;
        setDeviceState(DeviceState::Disconnected);
        DS_LOG_INFO("CAN bus disconnected");
    }
}

bool CanBus::applyConfig()
{
    // 检查插件是否存在
    if (!QCanBus::instance()->plugins().contains(config_.pluginName)) {
        lastError_ = tr("CAN插件不存在: %1").arg(config_.pluginName);
        DS_LOG_ERROR(lastError_.toStdString());
        emit errorOccurred(lastError_);
        return false;
    }
    
    // 创建设备
    QString error;
    canDevice_ = QCanBus::instance()->createDevice(config_.pluginName, config_.interfaceName, &error);
    if (!canDevice_) {
        lastError_ = tr("创建CAN设备失败: %1").arg(error);
        DS_LOG_ERROR(lastError_.toStdString());
        emit errorOccurred(lastError_);
        return false;
    }
    
    // 连接信号
    connect(canDevice_, &QCanBusDevice::framesReceived, this, &CanBus::onFramesReceived);
    connect(canDevice_, &QCanBusDevice::errorOccurred, this, &CanBus::onErrorOccurred);
    connect(canDevice_, &QCanBusDevice::stateChanged, this, &CanBus::onStateChanged);
    
    // 配置参数
    canDevice_->setConfigurationParameter(QCanBusDevice::LoopbackKey, config_.loopback);
    canDevice_->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, config_.receiveOwn);
    
    if (canDevice_->hasCanFd() && config_.canFd) {
        canDevice_->setConfigurationParameter(QCanBusDevice::CanFdKey, true);
        canDevice_->setConfigurationParameter(QCanBusDevice::DataBitRateKey, config_.dataBitrate);
    }
    
    // 设置波特率（部分插件支持）
    if (canDevice_->configurationParameter(QCanBusDevice::BitRateKey).isValid()) {
        canDevice_->setConfigurationParameter(QCanBusDevice::BitRateKey, config_.bitrate);
    }
    
    // 连接设备
    setDeviceState(DeviceState::Connecting);
    if (!canDevice_->connectDevice()) {
        lastError_ = tr("连接CAN设备失败: %1").arg(canDevice_->errorString());
        DS_LOG_ERROR(lastError_.toStdString());
        emit errorOccurred(lastError_);
        delete canDevice_;
        canDevice_ = nullptr;
        setDeviceState(DeviceState::Error);
        return false;
    }
    
    setDeviceState(DeviceState::Connected);
    DS_LOG_INFO("CAN bus connected: " + config_.pluginName.toStdString() + "/" + config_.interfaceName.toStdString());
    return true;
}

qint64 CanBus::send(const QByteArray& data)
{
    if (!isConnected() || !canDevice_) {
        return -1;
    }
    
    // 默认发送为标准帧，ID从数据前2字节解析
    if (data.size() < 2) {
        return -1;
    }
    
    quint32 id = static_cast<quint8>(data[0]) << 8 | static_cast<quint8>(data[1]);
    QByteArray payload = data.mid(2);
    
    if (sendStandardFrame(id, payload)) {
        return data.size();
    }
    return -1;
}

QByteArray CanBus::receive()
{
    QByteArray data = receiveBuffer_;
    receiveBuffer_.clear();
    return data;
}

bool CanBus::sendFrame(const CanFrame& frame)
{
    if (!isConnected() || !canDevice_) {
        lastError_ = tr("设备未连接");
        return false;
    }
    
    QCanBusFrame qframe = frame.toQCanBusFrame();
    bool ok = canDevice_->writeFrame(qframe);
    
    if (ok) {
        statistics_.framesSent++;
        statistics_.bytesSent += frame.payload.size();
        emit frameSent(frame);
        DS_LOG_DEBUG("CAN frame sent: ID=0x" + QString::number(frame.id, 16).toStdString());
    } else {
        statistics_.errors++;
        lastError_ = canDevice_->errorString();
        DS_LOG_ERROR("Send CAN frame failed: " + lastError_.toStdString());
    }
    
    return ok;
}

bool CanBus::sendStandardFrame(quint32 id, const QByteArray& data)
{
    if (id > 0x7FF) {
        lastError_ = tr("标准帧ID超过范围");
        return false;
    }
    
    CanFrame frame;
    frame.id = id;
    frame.extended = false;
    frame.payload = data.left(8);  // 标准CAN最多8字节
    
    return sendFrame(frame);
}

bool CanBus::sendExtendedFrame(quint32 id, const QByteArray& data)
{
    if (id > 0x1FFFFFFF) {
        lastError_ = tr("扩展帧ID超过范围");
        return false;
    }
    
    CanFrame frame;
    frame.id = id;
    frame.extended = true;
    frame.payload = config_.canFd ? data.left(64) : data.left(8);
    
    return sendFrame(frame);
}

bool CanBus::sendRemoteFrame(quint32 id, int dlc)
{
    CanFrame frame;
    frame.id = id;
    frame.extended = (id > 0x7FF);
    frame.rtr = true;
    frame.payload = QByteArray(dlc > 0 ? dlc : 0, 0);
    
    return sendFrame(frame);
}

bool CanBus::setFilter(quint32 id, quint32 mask, bool extended)
{
    if (!canDevice_) {
        return false;
    }
    
    QCanBusDevice::Filter filter;
    filter.frameId = id;
    filter.frameIdMask = mask;
    filter.format = extended ? QCanBusDevice::Filter::MatchExtendedFormat : QCanBusDevice::Filter::MatchBaseFormat;
    filter.type = QCanBusFrame::DataFrame;
    filter.typeMask = QCanBusFrame::RemoteRequestFrame;
    
    QList<QCanBusDevice::Filter> filters = canDevice_->configurationParameter(QCanBusDevice::UserKey).value<QList<QCanBusDevice::Filter>>();
    filters.append(filter);
    
    return canDevice_->setConfigurationParameter(QCanBusDevice::UserKey, QVariant::fromValue(filters));
}

void CanBus::clearFilters()
{
    if (canDevice_) {
        canDevice_->setConfigurationParameter(QCanBusDevice::UserKey, QVariant());
    }
}

void CanBus::resetStatistics()
{
    statistics_ = Statistics();
}

QString CanBus::lastError() const
{
    return lastError_;
}

void CanBus::setConfig(const CanBusConfig& config)
{
    config_ = config;
    emit configChanged(config);
}

void CanBus::onFramesReceived()
{
    if (!canDevice_) return;
    
    while (canDevice_->framesAvailable()) {
        QCanBusFrame qframe = canDevice_->readFrame();
        processFrame(qframe);
    }
}

void CanBus::processFrame(const QCanBusFrame& qframe)
{
    CanFrame frame = CanFrame::fromQCanBusFrame(qframe);
    
    if (frame.error) {
        statistics_.errors++;
        emit errorFrameReceived(frame);
        lastError_ = tr("收到错误帧");
        DS_LOG_WARNING("CAN error frame received");
        return;
    }
    
    statistics_.framesReceived++;
    statistics_.bytesReceived += frame.payload.size();
    
    // 添加到接收缓冲区（格式：ID(2字节) + 数据）
    receiveBuffer_.append(static_cast<char>(frame.id >> 8));
    receiveBuffer_.append(static_cast<char>(frame.id & 0xFF));
    receiveBuffer_.append(frame.payload);
    
    emit frameReceived(frame);
    emit dataReceived(receiveBuffer_);
    
    DS_LOG_DEBUG("CAN frame received: " + frame.toString().toStdString());
}

void CanBus::onErrorOccurred(QCanBusDevice::CanBusError error)
{
    if (error == QCanBusDevice::NoError) return;
    
    statistics_.errors++;
    lastError_ = canDevice_ ? canDevice_->errorString() : tr("未知错误");
    
    DS_LOG_ERROR("CAN bus error: " + lastError_.toStdString());
    emit errorOccurred(lastError_);
}

void CanBus::onStateChanged(QCanBusDevice::CanBusDeviceState state)
{
    switch (state) {
        case QCanBusDevice::UnconnectedState:
            setDeviceState(DeviceState::Disconnected);
            break;
        case QCanBusDevice::ConnectingState:
            setDeviceState(DeviceState::Connecting);
            break;
        case QCanBusDevice::ConnectedState:
            setDeviceState(DeviceState::Connected);
            break;
        case QCanBusDevice::ClosingState:
            setDeviceState(DeviceState::Disconnecting);
            break;
        default:
            break;
    }
}

} // namespace DeviceStudio
