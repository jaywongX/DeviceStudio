/**
 * @file devicepanel.cpp
 * @brief 设备面板实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "devicepanel.h"
#include "core/devicemanager/devicemanager.h"
#include "core/devicemanager/idevice.h"
#include "log/logger.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

namespace DeviceStudio {

DevicePanel::DevicePanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refreshDeviceList();
}

void DevicePanel::setDeviceManager(std::shared_ptr<DeviceManager> manager)
{
    if (deviceManager_) {
        disconnect(deviceManager_.get(), nullptr, this, nullptr);
    }
    
    deviceManager_ = manager;
    
    if (deviceManager_) {
        // 连接设备管理器信号
        connect(deviceManager_.get(), &DeviceManager::deviceAdded,
                this, [this](const QString&) { refreshDeviceList(); });
        connect(deviceManager_.get(), &DeviceManager::deviceRemoved,
                this, [this](const QString&) { refreshDeviceList(); });
        connect(deviceManager_.get(), &DeviceManager::deviceStateChanged,
                this, [this](const QString&, DeviceState) { refreshDeviceList(); });
    }
    
    refreshDeviceList();
}

void DevicePanel::refreshDeviceList()
{
    deviceTree_->clear();
    
    if (!deviceManager_) {
        updateButtonStates();
        return;
    }
    
    // 按类型分组设备
    QMap<QString, QTreeWidgetItem*> categoryItems;
    
    auto devices = deviceManager_->getAllDevices();
    for (const auto& device : devices) {
        if (!device) continue;
        
        QString deviceType;
        switch (device->deviceType()) {
        case DeviceType::Serial:
            deviceType = tr("串口设备");
            break;
        case DeviceType::TcpClient:
        case DeviceType::TcpServer:
            deviceType = tr("网络设备");
            break;
        case DeviceType::Udp:
            deviceType = tr("UDP设备");
            break;
        case DeviceType::ModbusRtu:
        case DeviceType::ModbusTcp:
            deviceType = tr("Modbus设备");
            break;
        case DeviceType::Can:
            deviceType = tr("CAN设备");
            break;
        default:
            deviceType = tr("其他设备");
            break;
        }
        
        // 获取或创建分类节点
        QTreeWidgetItem* categoryItem = categoryItems.value(deviceType, nullptr);
        if (!categoryItem) {
            categoryItem = new QTreeWidgetItem(deviceTree_, QStringList{deviceType});
            categoryItem->setExpanded(true);
            categoryItems[deviceType] = categoryItem;
        }
        
        // 创建设备节点
        QString status;
        switch (device->deviceState()) {
        case DeviceState::Connected:
            status = tr("已连接");
            break;
        case DeviceState::Connecting:
            status = tr("连接中");
            break;
        case DeviceState::Disconnecting:
            status = tr("断开中");
            break;
        case DeviceState::Error:
            status = tr("错误");
            break;
        default:
            status = tr("已断开");
            break;
        }
        
        // 获取设备参数信息
        QString paramInfo;
        QVariantMap config = device->getConfiguration();
        if (config.contains("port")) {
            paramInfo = config["port"].toString();
            if (config.contains("baudRate")) {
                paramInfo += " " + config["baudRate"].toString();
            }
        } else if (config.contains("host")) {
            paramInfo = config["host"].toString();
            if (config.contains("port")) {
                paramInfo += ":" + config["port"].toString();
            }
        }
        
        QTreeWidgetItem* deviceItem = new QTreeWidgetItem(categoryItem, 
            QStringList{device->deviceName(), paramInfo, status});
        deviceItem->setData(0, Qt::UserRole, device->deviceId());
        
        // 根据状态设置颜色
        if (device->deviceState() == DeviceState::Connected) {
            deviceItem->setForeground(2, QColor(Qt::darkGreen));
        } else if (device->deviceState() == DeviceState::Error) {
            deviceItem->setForeground(2, QColor(Qt::red));
        }
    }
    
    updateButtonStates();
    DS_LOG_DEBUG("Device list refreshed, count: " + std::to_string(devices.size()));
}

void DevicePanel::onAddDevice()
{
    DS_LOG_INFO("Add device requested");
    emit addDeviceRequested();
}

void DevicePanel::onRemoveDevice()
{
    QString deviceId = getSelectedDeviceId();
    if (deviceId.isEmpty()) {
        return;
    }
    
    DS_LOG_INFO("Remove device requested: " + deviceId.toStdString());
    
    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("确认删除"),
        tr("确定要删除此设备吗？"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (deviceManager_ && deviceManager_->removeDevice(deviceId)) {
            DS_LOG_INFO("Device removed: " + deviceId.toStdString());
        } else {
            DS_LOG_ERROR("Failed to remove device: " + deviceId.toStdString());
        }
    }
    
    refreshDeviceList();
}

void DevicePanel::onConnectDevice()
{
    QString deviceId = getSelectedDeviceId();
    if (deviceId.isEmpty()) {
        return;
    }
    
    if (!deviceManager_) {
        return;
    }
    
    DS_LOG_INFO("Connect device requested: " + deviceId.toStdString());
    
    auto device = deviceManager_->getDevice(deviceId);
    if (device) {
        bool result = device->connect(device->getConfiguration());
        if (result) {
            DS_LOG_INFO("Device connected: " + deviceId.toStdString());
            emit deviceConnectionChanged(true);
        } else {
            DS_LOG_ERROR("Failed to connect device: " + deviceId.toStdString());
            QMessageBox::warning(this, tr("连接失败"), tr("无法连接到设备"));
        }
    }
    
    refreshDeviceList();
}

void DevicePanel::onDisconnectDevice()
{
    QString deviceId = getSelectedDeviceId();
    if (deviceId.isEmpty()) {
        return;
    }
    
    if (!deviceManager_) {
        return;
    }
    
    DS_LOG_INFO("Disconnect device requested: " + deviceId.toStdString());
    
    auto device = deviceManager_->getDevice(deviceId);
    if (device) {
        device->disconnect();
        DS_LOG_INFO("Device disconnected: " + deviceId.toStdString());
        emit deviceConnectionChanged(false);
    }
    
    refreshDeviceList();
}

void DevicePanel::onItemSelectionChanged()
{
    updateButtonStates();
    
    QString deviceId = getSelectedDeviceId();
    if (!deviceId.isEmpty()) {
        emit deviceSelected(deviceId);
    }
}

void DevicePanel::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    
    QString deviceId = item->data(0, Qt::UserRole).toString();
    if (!deviceId.isEmpty()) {
        DS_LOG_INFO("Device double-clicked: " + deviceId.toStdString());
        emit deviceSelected(deviceId);
    }
}

QString DevicePanel::deviceTypeToCategory(const QString& deviceType) const
{
    return deviceType;
}

void DevicePanel::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 设备树
    deviceTree_ = new QTreeWidget(this);
    deviceTree_->setHeaderLabels(QStringList{tr("设备"), tr("参数"), tr("状态")});
    deviceTree_->setRootIsDecorated(true);
    deviceTree_->setAlternatingRowColors(true);
    deviceTree_->header()->setStretchLastSection(true);
    
    connect(deviceTree_, &QTreeWidget::itemSelectionChanged, 
            this, &DevicePanel::onItemSelectionChanged);
    connect(deviceTree_, &QTreeWidget::itemDoubleClicked,
            this, &DevicePanel::onItemDoubleClicked);
    
    mainLayout->addWidget(deviceTree_);
    
    // 按钮栏
    QWidget* buttonWidget = new QWidget(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(4, 4, 4, 4);
    
    addButton_ = new QPushButton(tr("添加"), this);
    removeButton_ = new QPushButton(tr("删除"), this);
    connectButton_ = new QPushButton(tr("连接"), this);
    disconnectButton_ = new QPushButton(tr("断开"), this);
    refreshButton_ = new QPushButton(tr("刷新"), this);
    
    connect(addButton_, &QPushButton::clicked, this, &DevicePanel::onAddDevice);
    connect(removeButton_, &QPushButton::clicked, this, &DevicePanel::onRemoveDevice);
    connect(connectButton_, &QPushButton::clicked, this, &DevicePanel::onConnectDevice);
    connect(disconnectButton_, &QPushButton::clicked, this, &DevicePanel::onDisconnectDevice);
    connect(refreshButton_, &QPushButton::clicked, this, &DevicePanel::refreshDeviceList);
    
    buttonLayout->addWidget(addButton_);
    buttonLayout->addWidget(removeButton_);
    buttonLayout->addWidget(connectButton_);
    buttonLayout->addWidget(disconnectButton_);
    buttonLayout->addWidget(refreshButton_);
    
    mainLayout->addWidget(buttonWidget);
    
    updateButtonStates();
}

void DevicePanel::updateButtonStates()
{
    bool hasSelection = deviceTree_->currentItem() && 
                        !deviceTree_->currentItem()->data(0, Qt::UserRole).isNull();
    
    removeButton_->setEnabled(hasSelection);
    connectButton_->setEnabled(hasSelection);
    disconnectButton_->setEnabled(hasSelection);
}

QString DevicePanel::getSelectedDeviceId() const
{
    QTreeWidgetItem* item = deviceTree_->currentItem();
    if (item) {
        return item->data(0, Qt::UserRole).toString();
    }
    return QString();
}

} // namespace DeviceStudio
