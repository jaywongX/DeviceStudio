/**
 * @file modbusdevice.h
 * @brief Modbus设备类
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "core/devicemanager/device.h"
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QModbusDataUnit>
#include <QSerialPort>

namespace DeviceStudio {

/**
 * @brief Modbus协议类型
 */
enum class ModbusProtocol {
    Rtu,    // Modbus RTU (串口)
    Tcp     // Modbus TCP (网络)
};

/**
 * @brief Modbus配置
 */
struct ModbusConfig {
    ModbusProtocol protocol = ModbusProtocol::Rtu;
    
    // RTU配置
    QString portName;
    qint32 baudRate = 9600;
    QSerialPort::Parity parity = QSerialPort::EvenParity;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    
    // TCP配置
    QString hostAddress = "127.0.0.1";
    quint16 port = 502;
    
    // 通用配置
    int timeout = 1000;         // 超时时间（毫秒）
    int retries = 3;            // 重试次数
    int slaveId = 1;            // 从站ID
    
    static ModbusConfig fromVariantMap(const QVariantMap& map);
    QVariantMap toVariantMap() const;
};

/**
 * @brief Modbus设备类
 */
class ModbusDevice : public Device
{
    Q_OBJECT

public:
    explicit ModbusDevice(ModbusProtocol protocol, QObject* parent = nullptr);
    explicit ModbusDevice(QObject* parent = nullptr);
    ~ModbusDevice() override;
    
    // ========== IDevice 接口实现 ==========
    bool connect(const QVariantMap& config) override;
    void disconnect() override;
    qint64 send(const QByteArray& data) override;
    QByteArray receive() override;
    
    // ========== Modbus特定方法 ==========
    
    /**
     * @brief 设置配置
     */
    void setConfig(const ModbusConfig& config);
    ModbusConfig getConfig() const { return config_; }
    
    /**
     * @brief 读取线圈状态 (FC01)
     */
    QVector<bool> readCoils(int address, int count);
    
    /**
     * @brief 读取离散输入 (FC02)
     */
    QVector<bool> readDiscreteInputs(int address, int count);
    
    /**
     * @brief 读取保持寄存器 (FC03)
     */
    QVector<uint16_t> readHoldingRegisters(int address, int count);
    
    /**
     * @brief 读取输入寄存器 (FC04)
     */
    QVector<uint16_t> readInputRegisters(int address, int count);
    
    /**
     * @brief 写单个线圈 (FC05)
     */
    bool writeSingleCoil(int address, bool value);
    
    /**
     * @brief 写单个寄存器 (FC06)
     */
    bool writeSingleRegister(int address, uint16_t value);
    
    /**
     * @brief 写多个线圈 (FC15)
     */
    bool writeMultipleCoils(int address, const QVector<bool>& values);
    
    /**
     * @brief 写多个寄存器 (FC16)
     */
    bool writeMultipleRegisters(int address, const QVector<uint16_t>& values);
    
    /**
     * @brief 设置从站ID
     */
    void setSlaveId(int slaveId);
    
    /**
     * @brief 获取最后错误
     */
    QString lastError() const { return lastError_; }

signals:
    void dataRead(int address, const QVector<uint16_t>& data);
    void dataWritten(int address, int count);
    void errorOccurred(const QString& error);

private:
    bool sendRequest(QModbusDataUnit& unit, bool isRead = true);
    
    ModbusConfig config_;
    QModbusTcpClient* tcpClient_ = nullptr;
    QModbusRtuSerialMaster* rtuMaster_ = nullptr;
    QString lastError_;
};

} // namespace DeviceStudio
