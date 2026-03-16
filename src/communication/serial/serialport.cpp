/**
 * @file serialport.cpp
 * @brief 串口通信实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "serialport.h"
#include "utils/log/logger.h"

namespace DeviceStudio {

// ========== SerialPortConfig 实现 ==========

SerialPortConfig SerialPortConfig::fromVariantMap(const QVariantMap& map)
{
    SerialPortConfig config;
    config.portName = map.value("portName").toString();
    config.baudRate = map.value("baudRate", 115200).toInt();
    
    // 数据位
    int dataBits = map.value("dataBits", 8).toInt();
    switch (dataBits) {
        case 5: config.dataBits = QSerialPort::Data5; break;
        case 6: config.dataBits = QSerialPort::Data6; break;
        case 7: config.dataBits = QSerialPort::Data7; break;
        case 8: default: config.dataBits = QSerialPort::Data8; break;
    }
    
    // 校验位
    QString parity = map.value("parity", "none").toString().toLower();
    if (parity == "none") config.parity = QSerialPort::NoParity;
    else if (parity == "odd") config.parity = QSerialPort::OddParity;
    else if (parity == "even") config.parity = QSerialPort::EvenParity;
    else if (parity == "mark") config.parity = QSerialPort::MarkParity;
    else if (parity == "space") config.parity = QSerialPort::SpaceParity;
    
    // 停止位
    double stopBits = map.value("stopBits", 1.0).toDouble();
    if (stopBits == 1.0) config.stopBits = QSerialPort::OneStop;
    else if (stopBits == 1.5) config.stopBits = QSerialPort::OneAndHalfStop;
    else if (stopBits == 2.0) config.stopBits = QSerialPort::TwoStop;
    
    // 流控制
    QString flowControl = map.value("flowControl", "none").toString().toLower();
    if (flowControl == "none") config.flowControl = QSerialPort::NoFlowControl;
    else if (flowControl == "hardware") config.flowControl = QSerialPort::HardwareControl;
    else if (flowControl == "software") config.flowControl = QSerialPort::SoftwareControl;
    
    return config;
}

QVariantMap SerialPortConfig::toVariantMap() const
{
    QVariantMap map;
    map["portName"] = portName;
    map["baudRate"] = baudRate;
    
    // 数据位
    map["dataBits"] = (dataBits == QSerialPort::Data5) ? 5 :
                      (dataBits == QSerialPort::Data6) ? 6 :
                      (dataBits == QSerialPort::Data7) ? 7 : 8;
    
    // 校验位
    QStringList parityList = {"none", "odd", "even", "mark", "space"};
    map["parity"] = parityList.value(static_cast<int>(parity), "none");
    
    // 停止位
    map["stopBits"] = (stopBits == QSerialPort::OneStop) ? 1.0 :
                      (stopBits == QSerialPort::OneAndHalfStop) ? 1.5 : 2.0;
    
    // 流控制
    QStringList flowControlList = {"none", "hardware", "software"};
    map["flowControl"] = flowControlList.value(static_cast<int>(flowControl), "none");
    
    return map;
}

// ========== SerialPort 实现 ==========

SerialPort::SerialPort(QObject* parent)
    : Device(DeviceType::Serial, parent)
{
    setDeviceName("Serial Port");
    serial_ = new QSerialPort(this);
    
    QObject::connect(serial_, &QSerialPort::readyRead, this, &SerialPort::onReadyRead);
    QObject::connect(serial_, &QSerialPort::errorOccurred, this, &SerialPort::onErrorOccurred);
}

SerialPort::SerialPort(const QString& portName, QObject* parent)
    : SerialPort(parent)
{
    config_.portName = portName;
    setDeviceName(portName);
}

SerialPort::~SerialPort()
{
    if (isConnected()) {
        disconnect();
    }
}

bool SerialPort::connect(const QVariantMap& config)
{
    // 解析配置
    config_ = SerialPortConfig::fromVariantMap(config);
    setConfiguration(config);
    
    if (config_.portName.isEmpty()) {
        DS_LOG_ERROR("Serial port name is empty");
        emit errorOccurred("Serial port name is empty");
        return false;
    }
    
    // 设置端口名称
    serial_->setPortName(config_.portName);
    
    // 应用配置
    if (!applyConfig()) {
        return false;
    }
    
    // 打开串口
    if (serial_->open(QIODevice::ReadWrite)) {
        setDeviceState(DeviceState::Connected);
        DS_LOG_INFO("Serial port connected: {} at {} baud", 
                    config_.portName.toStdString(), config_.baudRate);
        return true;
    } else {
        QString errorString = serial_->errorString();
        DS_LOG_ERROR("Failed to open serial port: {}", errorString.toStdString());
        setDeviceState(DeviceState::Error);
        emit errorOccurred(errorString);
        return false;
    }
}

void SerialPort::disconnect()
{
    if (serial_ && serial_->isOpen()) {
        serial_->close();
        setDeviceState(DeviceState::Disconnected);
        DS_LOG_INFO("Serial port disconnected: {}", config_.portName.toStdString());
    }
}

qint64 SerialPort::send(const QByteArray& data)
{
    if (!isConnected() || !serial_) {
        DS_LOG_WARN("Serial port not connected");
        return -1;
    }
    
    qint64 bytesWritten = serial_->write(data);
    if (bytesWritten > 0) {
        serial_->flush();
        emit dataSent(data);
        DS_LOG_DEBUG("Serial port sent {} bytes", bytesWritten);
    } else {
        DS_LOG_ERROR("Failed to send data: {}", serial_->errorString().toStdString());
    }
    
    return bytesWritten;
}

QByteArray SerialPort::receive()
{
    QByteArray data = receiveBuffer_;
    receiveBuffer_.clear();
    return data;
}

QList<QSerialPortInfo> SerialPort::availablePorts()
{
    return QSerialPortInfo::availablePorts();
}

QStringList SerialPort::availablePortNames()
{
    QStringList names;
    const auto ports = availablePorts();
    for (const auto& port : ports) {
        names << port.portName();
    }
    return names;
}

void SerialPort::setConfig(const SerialPortConfig& config)
{
    config_ = config;
    if (isConnected()) {
        applyConfig();
    }
    emit configChanged(config_);
}

bool SerialPort::applyConfig()
{
    if (!serial_) return false;
    
    serial_->setBaudRate(config_.baudRate);
    serial_->setDataBits(config_.dataBits);
    serial_->setParity(config_.parity);
    serial_->setStopBits(config_.stopBits);
    serial_->setFlowControl(config_.flowControl);
    
    return true;
}

void SerialPort::setDTR(bool enabled)
{
    if (serial_) {
        serial_->setDataTerminalReady(enabled);
    }
}

void SerialPort::setRTS(bool enabled)
{
    if (serial_) {
        serial_->setRequestToSend(enabled);
    }
}

bool SerialPort::isCTS() const
{
    return serial_ ? serial_->pinoutSignals() & QSerialPort::ClearToSendSignal : false;
}

bool SerialPort::isDSR() const
{
    return serial_ ? serial_->pinoutSignals() & QSerialPort::DataSetReadySignal : false;
}

void SerialPort::clearReceiveBuffer()
{
    receiveBuffer_.clear();
    if (serial_) {
        serial_->clear(QSerialPort::Input);
    }
}

void SerialPort::flushSendBuffer()
{
    if (serial_) {
        serial_->flush();
        serial_->clear(QSerialPort::Output);
    }
}

int SerialPort::receiveBufferSize() const
{
    return receiveBuffer_.size();
}

void SerialPort::setTimeout(int timeoutMs)
{
    timeoutMs_ = timeoutMs;
}

void SerialPort::onReadyRead()
{
    if (serial_) {
        QByteArray data = serial_->readAll();
        receiveBuffer_.append(data);
        emit dataReceived(data);
        DS_LOG_TRACE("Serial port received {} bytes", data.size());
    }
}

void SerialPort::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError) {
        QString errorString = serial_->errorString();
        DS_LOG_ERROR("Serial port error: {}", errorString.toStdString());
        setDeviceState(DeviceState::Error);
        emit errorOccurred(errorString);
    }
}

} // namespace DeviceStudio
