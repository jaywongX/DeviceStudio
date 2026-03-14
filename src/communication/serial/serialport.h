/**
 * @file serialport.h
 * @brief 串口通信类
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include "core/devicemanager/device.h"
#include <QSerialPort>
#include <QSerialPortInfo>

namespace DeviceStudio {

/**
 * @brief 串口设备类
 */
class SerialPort : public Device
{
    Q_OBJECT

public:
    explicit SerialPort(QObject* parent = nullptr);
    ~SerialPort() override;
    
    // ========== Device 接口实现 ==========
    bool connect(const QVariantMap& config) override;
    void disconnect() override;
    qint64 send(const QByteArray& data) override;
    QByteArray receive() override;
    
    // ========== 串口特定方法 ==========
    
    /**
     * @brief 获取可用串口列表
     */
    static QList<QSerialPortInfo> availablePorts();
    
    /**
     * @brief 设置串口参数
     */
    void setBaudRate(qint32 baudRate);
    void setDataBits(QSerialPort::DataBits dataBits);
    void setParity(QSerialPort::Parity parity);
    void setStopBits(QSerialPort::StopBits stopBits);
    void setFlowControl(QSerialPort::FlowControl flowControl);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort* serial_ = nullptr;
    QByteArray receiveBuffer_;
};

} // namespace DeviceStudio
