/**
 * @file modbusdevice.cpp
 * @brief Modbus设备实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "modbusdevice.h"
#include "utils/log/logger.h"
#include <QEventLoop>
#include <QTimer>

namespace DeviceStudio {

// ========== ModbusConfig 实现 ==========

ModbusConfig ModbusConfig::fromVariantMap(const QVariantMap& map)
{
    ModbusConfig config;
    
    // 协议类型
    QString protocol = map.value("protocol", "rtu").toString().toLower();
    config.protocol = (protocol == "tcp") ? ModbusProtocol::Tcp : ModbusProtocol::Rtu;
    
    // RTU配置
    config.portName = map.value("portName").toString();
    config.baudRate = map.value("baudRate", 9600).toInt();
    
    QString parity = map.value("parity", "even").toString().toLower();
    config.parity = (parity == "none") ? QSerialPort::NoParity :
                    (parity == "odd") ? QSerialPort::OddParity : QSerialPort::EvenParity;
    
    config.dataBits = static_cast<QSerialPort::DataBits>(map.value("dataBits", 8).toInt());
    config.stopBits = static_cast<QSerialPort::StopBits>(map.value("stopBits", 1).toInt());
    
    // TCP配置
    config.hostAddress = map.value("hostAddress", "127.0.0.1").toString();
    config.port = map.value("port", 502).toUInt();
    
    // 通用配置
    config.timeout = map.value("timeout", 1000).toInt();
    config.retries = map.value("retries", 3).toInt();
    config.slaveId = map.value("slaveId", 1).toInt();
    
    return config;
}

QVariantMap ModbusConfig::toVariantMap() const
{
    QVariantMap map;
    
    map["protocol"] = (protocol == ModbusProtocol::Tcp) ? "tcp" : "rtu";
    map["portName"] = portName;
    map["baudRate"] = baudRate;
    map["parity"] = (parity == QSerialPort::NoParity) ? "none" :
                    (parity == QSerialPort::OddParity) ? "odd" : "even";
    map["dataBits"] = dataBits;
    map["stopBits"] = stopBits;
    map["hostAddress"] = hostAddress;
    map["port"] = port;
    map["timeout"] = timeout;
    map["retries"] = retries;
    map["slaveId"] = slaveId;
    
    return map;
}

// ========== ModbusDevice 实现 ==========

ModbusDevice::ModbusDevice(ModbusProtocol protocol, QObject* parent)
    : Device((protocol == ModbusProtocol::Rtu) ? DeviceType::ModbusRtu : DeviceType::ModbusTcp, parent)
{
    config_.protocol = protocol;
    setDeviceName("Modbus Device");
}

ModbusDevice::ModbusDevice(QObject* parent)
    : ModbusDevice(ModbusProtocol::Rtu, parent)
{
}

ModbusDevice::~ModbusDevice()
{
    if (isConnected()) {
        disconnect();
    }
}

bool ModbusDevice::connect(const QVariantMap& config)
{
    config_ = ModbusConfig::fromVariantMap(config);
    setConfiguration(config);
    
    // 根据协议创建客户端
    if (config_.protocol == ModbusProtocol::Tcp) {
        if (!tcpClient_) {
            tcpClient_ = new QModbusTcpClient(this);
        }
        
        tcpClient_->setConnectionParameter(QModbusDevice::NetworkAddressParameter, config_.hostAddress);
        tcpClient_->setConnectionParameter(QModbusDevice::NetworkPortParameter, config_.port);
        tcpClient_->setTimeout(config_.timeout);
        tcpClient_->setNumberOfRetries(config_.retries);
        
        if (!tcpClient_->connectDevice()) {
            lastError_ = QString("Failed to connect to Modbus TCP server: %1:%2")
                .arg(config_.hostAddress).arg(config_.port);
            DS_LOG_ERROR("{}", lastError_.toStdString());
            setDeviceState(DeviceState::Error);
            emit errorOccurred(lastError_);
            return false;
        }
        
        setDeviceState(DeviceState::Connected);
        DS_LOG_INFO("Modbus TCP connected to {}:{}", config_.hostAddress.toStdString(), config_.port);
        return true;
    }
    else {
        // Modbus RTU
        if (!rtuMaster_) {
            rtuMaster_ = new QModbusRtuSerialMaster(this);
        }
        
        rtuMaster_->setConnectionParameter(QModbusDevice::SerialPortNameParameter, config_.portName);
        rtuMaster_->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, config_.baudRate);
        rtuMaster_->setConnectionParameter(QModbusDevice::SerialParityParameter, config_.parity);
        rtuMaster_->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, config_.dataBits);
        rtuMaster_->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, config_.stopBits);
        rtuMaster_->setTimeout(config_.timeout);
        rtuMaster_->setNumberOfRetries(config_.retries);
        
        if (!rtuMaster_->connectDevice()) {
            lastError_ = QString("Failed to connect to Modbus RTU device: %1").arg(config_.portName);
            DS_LOG_ERROR("{}", lastError_.toStdString());
            setDeviceState(DeviceState::Error);
            emit errorOccurred(lastError_);
            return false;
        }
        
        setDeviceState(DeviceState::Connected);
        DS_LOG_INFO("Modbus RTU connected to {} at {} baud", 
                    config_.portName.toStdString(), config_.baudRate);
        return true;
    }
}

void ModbusDevice::disconnect()
{
    if (tcpClient_ && tcpClient_->state() == QModbusDevice::ConnectedState) {
        tcpClient_->disconnectDevice();
    }
    
    if (rtuMaster_ && rtuMaster_->state() == QModbusDevice::ConnectedState) {
        rtuMaster_->disconnectDevice();
    }
    
    setDeviceState(DeviceState::Disconnected);
    DS_LOG_INFO("Modbus device disconnected");
}

qint64 ModbusDevice::send(const QByteArray& data)
{
    Q_UNUSED(data);
    DS_LOG_WARN("Direct send not supported for Modbus, use specific read/write methods");
    return -1;
}

QByteArray ModbusDevice::receive()
{
    DS_LOG_WARN("Direct receive not supported for Modbus, use specific read/write methods");
    return QByteArray();
}

void ModbusDevice::setConfig(const ModbusConfig& config)
{
    config_ = config;
    if (isConnected()) {
        disconnect();
        connect(config_.toVariantMap());
    }
}

QVector<bool> ModbusDevice::readCoils(int address, int count)
{
    QModbusDataUnit unit(QModbusDataUnit::Coils, address, count);
    
    if (sendRequest(unit, true)) {
        QVector<bool> result;
        for (int i = 0; i < unit.valueCount(); ++i) {
            result.append(unit.value(i) != 0);
        }
        emit dataRead(address, {});
        return result;
    }
    
    return QVector<bool>();
}

QVector<bool> ModbusDevice::readDiscreteInputs(int address, int count)
{
    QModbusDataUnit unit(QModbusDataUnit::DiscreteInputs, address, count);
    
    if (sendRequest(unit, true)) {
        QVector<bool> result;
        for (int i = 0; i < unit.valueCount(); ++i) {
            result.append(unit.value(i) != 0);
        }
        return result;
    }
    
    return QVector<bool>();
}

QVector<uint16_t> ModbusDevice::readHoldingRegisters(int address, int count)
{
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, address, count);
    
    if (sendRequest(unit, true)) {
        QVector<uint16_t> result;
        for (int i = 0; i < unit.valueCount(); ++i) {
            result.append(static_cast<uint16_t>(unit.value(i)));
        }
        emit dataRead(address, result);
        return result;
    }
    
    return QVector<uint16_t>();
}

QVector<uint16_t> ModbusDevice::readInputRegisters(int address, int count)
{
    QModbusDataUnit unit(QModbusDataUnit::InputRegisters, address, count);
    
    if (sendRequest(unit, true)) {
        QVector<uint16_t> result;
        for (int i = 0; i < unit.valueCount(); ++i) {
            result.append(static_cast<uint16_t>(unit.value(i)));
        }
        return result;
    }
    
    return QVector<uint16_t>();
}

bool ModbusDevice::writeSingleCoil(int address, bool value)
{
    QModbusDataUnit unit(QModbusDataUnit::Coils, address, 1);
    unit.setValue(0, value ? 1 : 0);
    
    if (sendRequest(unit, false)) {
        emit dataWritten(address, 1);
        return true;
    }
    
    return false;
}

bool ModbusDevice::writeSingleRegister(int address, uint16_t value)
{
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, address, 1);
    unit.setValue(0, value);
    
    if (sendRequest(unit, false)) {
        emit dataWritten(address, 1);
        return true;
    }
    
    return false;
}

bool ModbusDevice::writeMultipleCoils(int address, const QVector<bool>& values)
{
    QModbusDataUnit unit(QModbusDataUnit::Coils, address, values.size());
    
    for (int i = 0; i < values.size(); ++i) {
        unit.setValue(i, values[i] ? 1 : 0);
    }
    
    if (sendRequest(unit, false)) {
        emit dataWritten(address, values.size());
        return true;
    }
    
    return false;
}

bool ModbusDevice::writeMultipleRegisters(int address, const QVector<uint16_t>& values)
{
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, address, values.size());
    
    for (int i = 0; i < values.size(); ++i) {
        unit.setValue(i, values[i]);
    }
    
    if (sendRequest(unit, false)) {
        emit dataWritten(address, values.size());
        return true;
    }
    
    return false;
}

void ModbusDevice::setSlaveId(int slaveId)
{
    config_.slaveId = slaveId;
}

bool ModbusDevice::sendRequest(QModbusDataUnit& unit, bool isRead)
{
    QModbusClient* client = (config_.protocol == ModbusProtocol::Tcp) ? 
                            static_cast<QModbusClient*>(tcpClient_) : 
                            static_cast<QModbusClient*>(rtuMaster_);
    
    if (!client || client->state() != QModbusDevice::ConnectedState) {
        lastError_ = "Modbus device not connected";
        DS_LOG_ERROR("{}", lastError_.toStdString());
        emit errorOccurred(lastError_);
        return false;
    }
    
    QModbusReply* reply = nullptr;
    
    if (isRead) {
        reply = client->sendReadRequest(unit, config_.slaveId);
    } else {
        reply = client->sendWriteRequest(unit, config_.slaveId);
    }
    
    if (!reply) {
        lastError_ = "Failed to send Modbus request";
        DS_LOG_ERROR("{}", lastError_.toStdString());
        emit errorOccurred(lastError_);
        return false;
    }
    
    // 等待响应
    if (!reply->isFinished()) {
        QEventLoop loop;
        QObject::connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }
    
    bool success = false;
    
    if (reply->error() == QModbusDevice::NoError) {
        if (isRead) {
            unit = reply->result();
        }
        success = true;
        DS_LOG_DEBUG("Modbus request successful");
    } else {
        lastError_ = QString("Modbus error: %1").arg(reply->errorString());
        DS_LOG_ERROR("{}", lastError_.toStdString());
        emit errorOccurred(lastError_);
    }
    
    reply->deleteLater();
    return success;
}

} // namespace DeviceStudio
