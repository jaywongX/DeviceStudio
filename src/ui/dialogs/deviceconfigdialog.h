/**
 * @file deviceconfigdialog.h
 * @brief 设备配置对话框
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QStackedWidget>
#include <QPushButton>

namespace DeviceStudio {

/**
 * @brief 设备类型
 */
enum class DeviceTypeConfig {
    Serial,
    TcpClient,
    TcpServer,
    Udp,
    ModbusRtu,
    ModbusTcp
};

/**
 * @brief 设备配置对话框
 */
class DeviceConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceConfigDialog(QWidget* parent = nullptr);
    explicit DeviceConfigDialog(DeviceTypeConfig type, QWidget* parent = nullptr);
    ~DeviceConfigDialog() override;
    
    /**
     * @brief 获取设备名称
     */
    QString deviceName() const;
    
    /**
     * @brief 获取设备类型
     */
    DeviceTypeConfig deviceType() const;
    
    /**
     * @brief 获取配置参数
     */
    QVariantMap configuration() const;
    
    /**
     * @brief 设置配置参数
     */
    void setConfiguration(const QVariantMap& config);

private slots:
    void onDeviceTypeChanged(int index);
    void onRefreshSerialPorts();
    void onTestConnection();
    void onAccept();

private:
    void setupUI();
    void setupSerialPage();
    void setupTcpClientPage();
    void setupTcpServerPage();
    void setupUdpPage();
    void setupModbusRtuPage();
    void setupModbusTcpPage();
    void setupConnections();
    
    QVariantMap getSerialConfig() const;
    QVariantMap getTcpClientConfig() const;
    QVariantMap getTcpServerConfig() const;
    QVariantMap getUdpConfig() const;
    QVariantMap getModbusRtuConfig() const;
    QVariantMap getModbusTcpConfig() const;
    
    void setSerialConfig(const QVariantMap& config);
    void setTcpClientConfig(const QVariantMap& config);
    void setTcpServerConfig(const QVariantMap& config);
    void setUdpConfig(const QVariantMap& config);
    void setModbusRtuConfig(const QVariantMap& config);
    void setModbusTcpConfig(const QVariantMap& config);
    
    // 通用控件
    QLineEdit* deviceNameEdit_ = nullptr;
    QComboBox* deviceTypeCombo_ = nullptr;
    QStackedWidget* configStack_ = nullptr;
    
    // 串口配置
    QComboBox* serialPortCombo_ = nullptr;
    QComboBox* baudRateCombo_ = nullptr;
    QComboBox* dataBitsCombo_ = nullptr;
    QComboBox* parityCombo_ = nullptr;
    QComboBox* stopBitsCombo_ = nullptr;
    QComboBox* flowControlCombo_ = nullptr;
    
    // TCP客户端配置
    QLineEdit* tcpClientHostEdit_ = nullptr;
    QSpinBox* tcpClientPortSpin_ = nullptr;
    QSpinBox* tcpClientTimeoutSpin_ = nullptr;
    QCheckBox* tcpClientAutoReconnectCheck_ = nullptr;
    
    // TCP服务器配置
    QLineEdit* tcpServerAddressEdit_ = nullptr;
    QSpinBox* tcpServerPortSpin_ = nullptr;
    QSpinBox* tcpServerMaxConnSpin_ = nullptr;
    
    // UDP配置
    QLineEdit* udpLocalAddressEdit_ = nullptr;
    QSpinBox* udpLocalPortSpin_ = nullptr;
    QLineEdit* udpRemoteAddressEdit_ = nullptr;
    QSpinBox* udpRemotePortSpin_ = nullptr;
    QCheckBox* udpMulticastCheck_ = nullptr;
    QLineEdit* udpMulticastGroupEdit_ = nullptr;
    
    // Modbus RTU配置
    QComboBox* modbusRtuPortCombo_ = nullptr;
    QComboBox* modbusRtuBaudRateCombo_ = nullptr;
    QSpinBox* modbusRtuSlaveIdSpin_ = nullptr;
    QSpinBox* modbusRtuTimeoutSpin_ = nullptr;
    
    // Modbus TCP配置
    QLineEdit* modbusTcpHostEdit_ = nullptr;
    QSpinBox* modbusTcpPortSpin_ = nullptr;
    QSpinBox* modbusTcpSlaveIdSpin_ = nullptr;
    QSpinBox* modbusTcpTimeoutSpin_ = nullptr;
    
    // 按钮
    QPushButton* testButton_ = nullptr;
    QPushButton* okButton_ = nullptr;
    QPushButton* cancelButton_ = nullptr;
};

} // namespace DeviceStudio
