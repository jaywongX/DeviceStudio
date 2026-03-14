/**
 * @file serialport.cpp
 * @brief 串口通信实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "serialport.h"
#include "log/logger.h"

namespace DeviceStudio {

SerialPort::SerialPort(QObject* parent)
    : Device(DeviceType::Serial, parent)
{
    setDeviceName("Serial Port");
    serial_ = new QSerialPort(this);
    
    connect(serial_, &QSerialPort::readyRead, this, &SerialPort::onReadyRead);
    connect(serial_, &QSerialPort::errorOccurred, this, &SerialPort::onErrorOccurred);
}

SerialPort::~SerialPort()
{
    if (isConnected()) {
        disconnect();
    }
}

bool SerialPort::connect(const QVariantMap& config)
{
    if (!serial_) {
        return false;
    }
    
    // 从配置中获取参数
    QString portName = config.value("port").toString();
    qint32 baudRate = config.value("baudrate", 115200).toInt();
    
    if (portName.isEmpty()) {
        DS_LOG_ERROR("Serial port name is empty");
        return false;
    }
    
    // 设置串口参数
    serial_->setPortName(portName);
    serial_->setBaudRate(baudRate);
    serial_->setDataBits(QSerialPort::Data8);
    serial_->setParity(QSerialPort::NoParity);
    serial_->setStopBits(QSerialPort::OneStop);
    serial_->setFlowControl(QSerialPort::NoFlowControl);
    
    // 打开串口
    if (serial_->open(QIODevice::ReadWrite)) {
        setDeviceState(DeviceState::Connected);
        DS_LOG_INFO("Serial port connected: " + portName.toStdString());
        return true;
    } else {
        DS_LOG_ERROR("Failed to open serial port: " + serial_->errorString().toStdString());
        setDeviceState(DeviceState::Error);
        return false;
    }
}

void SerialPort::disconnect()
{
    if (serial_ && serial_->isOpen()) {
        serial_->close();
        setDeviceState(DeviceState::Disconnected);
        DS_LOG_INFO("Serial port disconnected");
    }
}

qint64 SerialPort::send(const QByteArray& data)
{
    if (!isConnected() || !serial_) {
        return -1;
    }
    
    qint64 bytesWritten = serial_->write(data);
    if (bytesWritten > 0) {
        serial_->flush();
        emit dataSent(data);
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

void SerialPort::setBaudRate(qint32 baudRate)
{
    if (serial_) {
        serial_->setBaudRate(baudRate);
    }
}

void SerialPort::setDataBits(QSerialPort::DataBits dataBits)
{
    if (serial_) {
        serial_->setDataBits(dataBits);
    }
}

void SerialPort::setParity(QSerialPort::Parity parity)
{
    if (serial_) {
        serial_->setParity(parity);
    }
}

void SerialPort::setStopBits(QSerialPort::StopBits stopBits)
{
    if (serial_) {
        serial_->setStopBits(stopBits);
    }
}

void SerialPort::setFlowControl(QSerialPort::FlowControl flowControl)
{
    if (serial_) {
        serial_->setFlowControl(flowControl);
    }
}

void SerialPort::onReadyRead()
{
    if (serial_) {
        QByteArray data = serial_->readAll();
        receiveBuffer_.append(data);
        emit dataReceived(data);
    }
}

void SerialPort::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError) {
        QString errorString = serial_->errorString();
        DS_LOG_ERROR("Serial port error: " + errorString.toStdString());
        emit errorOccurred(errorString);
    }
}

} // namespace DeviceStudio
