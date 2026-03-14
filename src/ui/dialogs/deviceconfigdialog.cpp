/**
 * @file deviceconfigdialog.cpp
 * @brief 设备配置对话框实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "deviceconfigdialog.h"
#include "communication/serial/serialport.h"
#include "utils/log/logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSerialPortInfo>

namespace DeviceStudio {

DeviceConfigDialog::DeviceConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    setupConnections();
}

DeviceConfigDialog::DeviceConfigDialog(DeviceTypeConfig type, QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    setupConnections();
    
    // 设置设备类型
    deviceTypeCombo_->setCurrentIndex(static_cast<int>(type));
}

DeviceConfigDialog::~DeviceConfigDialog()
{
}

void DeviceConfigDialog::setupUI()
{
    setWindowTitle(tr("设备配置"));
    setMinimumWidth(500);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 设备基本信息
    QGroupBox* basicGroup = new QGroupBox(tr("基本信息"), this);
    QFormLayout* basicLayout = new QFormLayout(basicGroup);
    
    deviceNameEdit_ = new QLineEdit(this);
    deviceNameEdit_->setPlaceholderText(tr("请输入设备名称"));
    basicLayout->addRow(tr("设备名称:"), deviceNameEdit_);
    
    deviceTypeCombo_ = new QComboBox(this);
    deviceTypeCombo_->addItem(tr("串口设备"), static_cast<int>(DeviceTypeConfig::Serial));
    deviceTypeCombo_->addItem(tr("TCP客户端"), static_cast<int>(DeviceTypeConfig::TcpClient));
    deviceTypeCombo_->addItem(tr("TCP服务器"), static_cast<int>(DeviceTypeConfig::TcpServer));
    deviceTypeCombo_->addItem(tr("UDP通信"), static_cast<int>(DeviceTypeConfig::Udp));
    deviceTypeCombo_->addItem(tr("Modbus RTU"), static_cast<int>(DeviceTypeConfig::ModbusRtu));
    deviceTypeCombo_->addItem(tr("Modbus TCP"), static_cast<int>(DeviceTypeConfig::ModbusTcp));
    basicLayout->addRow(tr("设备类型:"), deviceTypeCombo_);
    
    mainLayout->addWidget(basicGroup);
    
    // 配置页面
    configStack_ = new QStackedWidget(this);
    
    setupSerialPage();
    setupTcpClientPage();
    setupTcpServerPage();
    setupUdpPage();
    setupModbusRtuPage();
    setupModbusTcpPage();
    
    mainLayout->addWidget(configStack_);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    testButton_ = new QPushButton(tr("测试连接"), this);
    buttonLayout->addWidget(testButton_);
    
    okButton_ = new QPushButton(tr("确定"), this);
    okButton_->setDefault(true);
    buttonLayout->addWidget(okButton_);
    
    cancelButton_ = new QPushButton(tr("取消"), this);
    buttonLayout->addWidget(cancelButton_);
    
    mainLayout->addLayout(buttonLayout);
}

void DeviceConfigDialog::setupSerialPage()
{
    QWidget* page = new QWidget();
    QFormLayout* layout = new QFormLayout(page);
    
    // 串口选择
    QHBoxLayout* portLayout = new QHBoxLayout();
    serialPortCombo_ = new QComboBox();
    QPushButton* refreshBtn = new QPushButton(tr("刷新"));
    portLayout->addWidget(serialPortCombo_);
    portLayout->addWidget(refreshBtn);
    layout->addRow(tr("串口:"), portLayout);
    
    // 波特率
    baudRateCombo_ = new QComboBox();
    baudRateCombo_->addItems({"1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"});
    baudRateCombo_->setCurrentText("115200");
    baudRateCombo_->setEditable(true);
    layout->addRow(tr("波特率:"), baudRateCombo_);
    
    // 数据位
    dataBitsCombo_ = new QComboBox();
    dataBitsCombo_->addItems({"5", "6", "7", "8"});
    dataBitsCombo_->setCurrentText("8");
    layout->addRow(tr("数据位:"), dataBitsCombo_);
    
    // 校验位
    parityCombo_ = new QComboBox();
    parityCombo_->addItem(tr("无"), static_cast<int>(QSerialPort::NoParity));
    parityCombo_->addItem(tr("奇校验"), static_cast<int>(QSerialPort::OddParity));
    parityCombo_->addItem(tr("偶校验"), static_cast<int>(QSerialPort::EvenParity));
    layout->addRow(tr("校验位:"), parityCombo_);
    
    // 停止位
    stopBitsCombo_ = new QComboBox();
    stopBitsCombo_->addItem("1", static_cast<int>(QSerialPort::OneStop));
    stopBitsCombo_->addItem("1.5", static_cast<int>(QSerialPort::OneAndHalfStop));
    stopBitsCombo_->addItem("2", static_cast<int>(QSerialPort::TwoStop));
    layout->addRow(tr("停止位:"), stopBitsCombo_);
    
    // 流控制
    flowControlCombo_ = new QComboBox();
    flowControlCombo_->addItem(tr("无"), static_cast<int>(QSerialPort::NoFlowControl));
    flowControlCombo_->addItem(tr("硬件"), static_cast<int>(QSerialPort::HardwareControl));
    flowControlCombo_->addItem(tr("软件"), static_cast<int>(QSerialPort::SoftwareControl));
    layout->addRow(tr("流控制:"), flowControlCombo_);
    
    configStack_->addWidget(page);
    
    // 刷新串口列表
    onRefreshSerialPorts();
    connect(refreshBtn, &QPushButton::clicked, this, &DeviceConfigDialog::onRefreshSerialPorts);
}

void DeviceConfigDialog::setupTcpClientPage()
{
    QWidget* page = new QWidget();
    QFormLayout* layout = new QFormLayout(page);
    
    tcpClientHostEdit_ = new QLineEdit("127.0.0.1");
    layout->addRow(tr("服务器地址:"), tcpClientHostEdit_);
    
    tcpClientPortSpin_ = new QSpinBox();
    tcpClientPortSpin_->setRange(1, 65535);
    tcpClientPortSpin_->setValue(8080);
    layout->addRow(tr("服务器端口:"), tcpClientPortSpin_);
    
    tcpClientTimeoutSpin_ = new QSpinBox();
    tcpClientTimeoutSpin_->setRange(100, 30000);
    tcpClientTimeoutSpin_->setValue(5000);
    tcpClientTimeoutSpin_->setSuffix(tr(" 毫秒"));
    layout->addRow(tr("连接超时:"), tcpClientTimeoutSpin_);
    
    tcpClientAutoReconnectCheck_ = new QCheckBox(tr("自动重连"));
    layout->addRow("", tcpClientAutoReconnectCheck_);
    
    configStack_->addWidget(page);
}

void DeviceConfigDialog::setupTcpServerPage()
{
    QWidget* page = new QWidget();
    QFormLayout* layout = new QFormLayout(page);
    
    tcpServerAddressEdit_ = new QLineEdit("0.0.0.0");
    layout->addRow(tr("监听地址:"), tcpServerAddressEdit_);
    
    tcpServerPortSpin_ = new QSpinBox();
    tcpServerPortSpin_->setRange(1, 65535);
    tcpServerPortSpin_->setValue(8080);
    layout->addRow(tr("监听端口:"), tcpServerPortSpin_);
    
    tcpServerMaxConnSpin_ = new QSpinBox();
    tcpServerMaxConnSpin_->setRange(1, 1000);
    tcpServerMaxConnSpin_->setValue(100);
    layout->addRow(tr("最大连接数:"), tcpServerMaxConnSpin_);
    
    configStack_->addWidget(page);
}

void DeviceConfigDialog::setupUdpPage()
{
    QWidget* page = new QWidget();
    QFormLayout* layout = new QFormLayout(page);
    
    udpLocalAddressEdit_ = new QLineEdit("0.0.0.0");
    layout->addRow(tr("本地地址:"), udpLocalAddressEdit_);
    
    udpLocalPortSpin_ = new QSpinBox();
    udpLocalPortSpin_->setRange(1, 65535);
    udpLocalPortSpin_->setValue(8080);
    layout->addRow(tr("本地端口:"), udpLocalPortSpin_);
    
    udpRemoteAddressEdit_ = new QLineEdit("127.0.0.1");
    layout->addRow(tr("远程地址:"), udpRemoteAddressEdit_);
    
    udpRemotePortSpin_ = new QSpinBox();
    udpRemotePortSpin_->setRange(1, 65535);
    udpRemotePortSpin_->setValue(8080);
    layout->addRow(tr("远程端口:"), udpRemotePortSpin_);
    
    udpMulticastCheck_ = new QCheckBox(tr("启用多播"));
    layout->addRow("", udpMulticastCheck_);
    
    udpMulticastGroupEdit_ = new QLineEdit("239.0.0.1");
    layout->addRow(tr("多播组:"), udpMulticastGroupEdit_);
    
    configStack_->addWidget(page);
}

void DeviceConfigDialog::setupModbusRtuPage()
{
    QWidget* page = new QWidget();
    QFormLayout* layout = new QFormLayout(page);
    
    // 串口选择（类似串口页面）
    modbusRtuPortCombo_ = new QComboBox();
    layout->addRow(tr("串口:"), modbusRtuPortCombo_);
    
    modbusRtuBaudRateCombo_ = new QComboBox();
    modbusRtuBaudRateCombo_->addItems({"9600", "19200", "38400", "57600", "115200"});
    layout->addRow(tr("波特率:"), modbusRtuBaudRateCombo_);
    
    modbusRtuSlaveIdSpin_ = new QSpinBox();
    modbusRtuSlaveIdSpin_->setRange(1, 247);
    modbusRtuSlaveIdSpin_->setValue(1);
    layout->addRow(tr("从站ID:"), modbusRtuSlaveIdSpin_);
    
    modbusRtuTimeoutSpin_ = new QSpinBox();
    modbusRtuTimeoutSpin_->setRange(100, 10000);
    modbusRtuTimeoutSpin_->setValue(1000);
    modbusRtuTimeoutSpin_->setSuffix(tr(" 毫秒"));
    layout->addRow(tr("超时:"), modbusRtuTimeoutSpin_);
    
    configStack_->addWidget(page);
}

void DeviceConfigDialog::setupModbusTcpPage()
{
    QWidget* page = new QWidget();
    QFormLayout* layout = new QFormLayout(page);
    
    modbusTcpHostEdit_ = new QLineEdit("127.0.0.1");
    layout->addRow(tr("服务器地址:"), modbusTcpHostEdit_);
    
    modbusTcpPortSpin_ = new QSpinBox();
    modbusTcpPortSpin_->setRange(1, 65535);
    modbusTcpPortSpin_->setValue(502);
    layout->addRow(tr("服务器端口:"), modbusTcpPortSpin_);
    
    modbusTcpSlaveIdSpin_ = new QSpinBox();
    modbusTcpSlaveIdSpin_->setRange(1, 247);
    modbusTcpSlaveIdSpin_->setValue(1);
    layout->addRow(tr("从站ID:"), modbusTcpSlaveIdSpin_);
    
    modbusTcpTimeoutSpin_ = new QSpinBox();
    modbusTcpTimeoutSpin_->setRange(100, 10000);
    modbusTcpTimeoutSpin_->setValue(1000);
    modbusTcpTimeoutSpin_->setSuffix(tr(" 毫秒"));
    layout->addRow(tr("超时:"), modbusTcpTimeoutSpin_);
    
    configStack_->addWidget(page);
}

void DeviceConfigDialog::setupConnections()
{
    connect(deviceTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeviceConfigDialog::onDeviceTypeChanged);
    connect(testButton_, &QPushButton::clicked, this, &DeviceConfigDialog::onTestConnection);
    connect(okButton_, &QPushButton::clicked, this, &DeviceConfigDialog::onAccept);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
}

void DeviceConfigDialog::onDeviceTypeChanged(int index)
{
    configStack_->setCurrentIndex(index);
}

void DeviceConfigDialog::onRefreshSerialPorts()
{
    serialPortCombo_->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& port : ports) {
        serialPortCombo_->addItem(port.portName());
    }
    
    if (modbusRtuPortCombo_) {
        modbusRtuPortCombo_->clear();
        for (const auto& port : ports) {
            modbusRtuPortCombo_->addItem(port.portName());
        }
    }
}

void DeviceConfigDialog::onTestConnection()
{
    QMessageBox::information(this, tr("测试连接"), tr("测试连接功能待实现"));
}

void DeviceConfigDialog::onAccept()
{
    // 验证设备名称
    if (deviceNameEdit_->text().isEmpty()) {
        QMessageBox::warning(this, tr("验证失败"), tr("请输入设备名称"));
        deviceNameEdit_->setFocus();
        return;
    }
    
    accept();
}

QString DeviceConfigDialog::deviceName() const
{
    return deviceNameEdit_->text();
}

DeviceTypeConfig DeviceConfigDialog::deviceType() const
{
    return static_cast<DeviceTypeConfig>(deviceTypeCombo_->currentData().toInt());
}

QVariantMap DeviceConfigDialog::configuration() const
{
    switch (deviceType()) {
        case DeviceTypeConfig::Serial:
            return getSerialConfig();
        case DeviceTypeConfig::TcpClient:
            return getTcpClientConfig();
        case DeviceTypeConfig::TcpServer:
            return getTcpServerConfig();
        case DeviceTypeConfig::Udp:
            return getUdpConfig();
        case DeviceTypeConfig::ModbusRtu:
            return getModbusRtuConfig();
        case DeviceTypeConfig::ModbusTcp:
            return getModbusTcpConfig();
        default:
            return QVariantMap();
    }
}

void DeviceConfigDialog::setConfiguration(const QVariantMap& config)
{
    // 根据设备类型设置配置
    DeviceTypeConfig type = static_cast<DeviceTypeConfig>(config.value("deviceType").toInt());
    deviceTypeCombo_->setCurrentIndex(static_cast<int>(type));
    
    switch (type) {
        case DeviceTypeConfig::Serial:
            setSerialConfig(config);
            break;
        case DeviceTypeConfig::TcpClient:
            setTcpClientConfig(config);
            break;
        case DeviceTypeConfig::TcpServer:
            setTcpServerConfig(config);
            break;
        case DeviceTypeConfig::Udp:
            setUdpConfig(config);
            break;
        case DeviceTypeConfig::ModbusRtu:
            setModbusRtuConfig(config);
            break;
        case DeviceTypeConfig::ModbusTcp:
            setModbusTcpConfig(config);
            break;
    }
}

QVariantMap DeviceConfigDialog::getSerialConfig() const
{
    QVariantMap config;
    config["portName"] = serialPortCombo_->currentText();
    config["baudRate"] = baudRateCombo_->currentText().toInt();
    config["dataBits"] = dataBitsCombo_->currentText().toInt();
    config["parity"] = parityCombo_->currentData().toInt();
    config["stopBits"] = stopBitsCombo_->currentData().toInt();
    config["flowControl"] = flowControlCombo_->currentData().toInt();
    return config;
}

QVariantMap DeviceConfigDialog::getTcpClientConfig() const
{
    QVariantMap config;
    config["hostAddress"] = tcpClientHostEdit_->text();
    config["port"] = tcpClientPortSpin_->value();
    config["timeout"] = tcpClientTimeoutSpin_->value();
    config["autoReconnect"] = tcpClientAutoReconnectCheck_->isChecked();
    return config;
}

QVariantMap DeviceConfigDialog::getTcpServerConfig() const
{
    QVariantMap config;
    config["listenAddress"] = tcpServerAddressEdit_->text();
    config["listenPort"] = tcpServerPortSpin_->value();
    config["maxConnections"] = tcpServerMaxConnSpin_->value();
    return config;
}

QVariantMap DeviceConfigDialog::getUdpConfig() const
{
    QVariantMap config;
    config["localAddress"] = udpLocalAddressEdit_->text();
    config["localPort"] = udpLocalPortSpin_->value();
    config["remoteAddress"] = udpRemoteAddressEdit_->text();
    config["remotePort"] = udpRemotePortSpin_->value();
    config["multicast"] = udpMulticastCheck_->isChecked();
    config["multicastGroup"] = udpMulticastGroupEdit_->text();
    return config;
}

QVariantMap DeviceConfigDialog::getModbusRtuConfig() const
{
    QVariantMap config;
    config["portName"] = modbusRtuPortCombo_->currentText();
    config["baudRate"] = modbusRtuBaudRateCombo_->currentText().toInt();
    config["slaveId"] = modbusRtuSlaveIdSpin_->value();
    config["timeout"] = modbusRtuTimeoutSpin_->value();
    config["protocol"] = "rtu";
    return config;
}

QVariantMap DeviceConfigDialog::getModbusTcpConfig() const
{
    QVariantMap config;
    config["hostAddress"] = modbusTcpHostEdit_->text();
    config["port"] = modbusTcpPortSpin_->value();
    config["slaveId"] = modbusTcpSlaveIdSpin_->value();
    config["timeout"] = modbusTcpTimeoutSpin_->value();
    config["protocol"] = "tcp";
    return config;
}

void DeviceConfigDialog::setSerialConfig(const QVariantMap& config)
{
    serialPortCombo_->setCurrentText(config.value("portName").toString());
    baudRateCombo_->setCurrentText(QString::number(config.value("baudRate", 115200).toInt()));
    dataBitsCombo_->setCurrentText(QString::number(config.value("dataBits", 8).toInt()));
}

void DeviceConfigDialog::setTcpClientConfig(const QVariantMap& config)
{
    tcpClientHostEdit_->setText(config.value("hostAddress").toString());
    tcpClientPortSpin_->setValue(config.value("port", 8080).toInt());
    tcpClientTimeoutSpin_->setValue(config.value("timeout", 5000).toInt());
    tcpClientAutoReconnectCheck_->setChecked(config.value("autoReconnect", false).toBool());
}

void DeviceConfigDialog::setTcpServerConfig(const QVariantMap& config)
{
    tcpServerAddressEdit_->setText(config.value("listenAddress").toString());
    tcpServerPortSpin_->setValue(config.value("listenPort", 8080).toInt());
    tcpServerMaxConnSpin_->setValue(config.value("maxConnections", 100).toInt());
}

void DeviceConfigDialog::setUdpConfig(const QVariantMap& config)
{
    udpLocalAddressEdit_->setText(config.value("localAddress").toString());
    udpLocalPortSpin_->setValue(config.value("localPort", 8080).toInt());
    udpRemoteAddressEdit_->setText(config.value("remoteAddress").toString());
    udpRemotePortSpin_->setValue(config.value("remotePort", 8080).toInt());
    udpMulticastCheck_->setChecked(config.value("multicast", false).toBool());
    udpMulticastGroupEdit_->setText(config.value("multicastGroup").toString());
}

void DeviceConfigDialog::setModbusRtuConfig(const QVariantMap& config)
{
    modbusRtuPortCombo_->setCurrentText(config.value("portName").toString());
    modbusRtuBaudRateCombo_->setCurrentText(QString::number(config.value("baudRate", 9600).toInt()));
    modbusRtuSlaveIdSpin_->setValue(config.value("slaveId", 1).toInt());
    modbusRtuTimeoutSpin_->setValue(config.value("timeout", 1000).toInt());
}

void DeviceConfigDialog::setModbusTcpConfig(const QVariantMap& config)
{
    modbusTcpHostEdit_->setText(config.value("hostAddress").toString());
    modbusTcpPortSpin_->setValue(config.value("port", 502).toInt());
    modbusTcpSlaveIdSpin_->setValue(config.value("slaveId", 1).toInt());
    modbusTcpTimeoutSpin_->setValue(config.value("timeout", 1000).toInt());
}

} // namespace DeviceStudio
