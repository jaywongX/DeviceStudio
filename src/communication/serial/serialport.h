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
#include <QTimer>

namespace DeviceStudio {

/**
 * @brief 串口配置结构体
 */
struct SerialPortConfig {
    QString portName;                               // 端口名称 (COM1, /dev/ttyUSB0)
    qint32 baudRate = 115200;                       // 波特率
    QSerialPort::DataBits dataBits = QSerialPort::Data8;      // 数据位
    QSerialPort::Parity parity = QSerialPort::NoParity;       // 校验位
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;    // 停止位
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl; // 流控制
    
    // 从QVariantMap转换
    static SerialPortConfig fromVariantMap(const QVariantMap& map);
    QVariantMap toVariantMap() const;
};

/**
 * @brief 串口设备类
 */
class SerialPort : public Device
{
    Q_OBJECT

public:
    explicit SerialPort(QObject* parent = nullptr);
    explicit SerialPort(const QString& portName, QObject* parent = nullptr);
    ~SerialPort() override;
    
    // ========== IDevice 接口实现 ==========
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
     * @brief 获取可用串口名称列表
     */
    static QStringList availablePortNames();
    
    /**
     * @brief 设置串口配置
     */
    void setConfig(const SerialPortConfig& config);
    SerialPortConfig getConfig() const { return config_; }
    
    /**
     * @brief 设置DTR信号
     */
    void setDTR(bool enabled);
    
    /**
     * @brief 设置RTS信号
     */
    void setRTS(bool enabled);
    
    /**
     * @brief 获取CTS信号状态
     */
    bool isCTS() const;
    
    /**
     * @brief 获取DSR信号状态
     */
    bool isDSR() const;
    
    /**
     * @brief 清空接收缓冲区
     */
    void clearReceiveBuffer();
    
    /**
     * @brief 清空发送缓冲区
     */
    void flushSendBuffer();
    
    /**
     * @brief 获取接收缓冲区大小
     */
    int receiveBufferSize() const;
    
    /**
     * @brief 设置超时时间（毫秒）
     */
    void setTimeout(int timeoutMs);

signals:
    /**
     * @brief 串口配置改变信号
     */
    void configChanged(const SerialPortConfig& config);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    bool applyConfig();
    
    QSerialPort* serial_ = nullptr;
    QByteArray receiveBuffer_;
    SerialPortConfig config_;
    int timeoutMs_ = 1000;
};

} // namespace DeviceStudio
