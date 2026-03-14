/**
 * @file devicepanel.cpp
 * @brief 设备面板实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "devicepanel.h"
#include "log/logger.h"

#include <QHBoxLayout>
#include <QHeaderView>

namespace DeviceStudio {

DevicePanel::DevicePanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refreshDeviceList();
}

void DevicePanel::refreshDeviceList()
{
    deviceTree_->clear();
    
    // TODO: 从设备管理器获取设备列表
    // 这里添加示例数据
    QTreeWidgetItem* serialItem = new QTreeWidgetItem(deviceTree_, QStringList{tr("串口设备")});
    serialItem->setExpanded(true);
    
    QTreeWidgetItem* com3Item = new QTreeWidgetItem(serialItem, QStringList{"COM3", "115200", tr("已断开")});
    com3Item->setData(0, Qt::UserRole, "device_001");
    
    QTreeWidgetItem* com4Item = new QTreeWidgetItem(serialItem, QStringList{"COM4", "9600", tr("已断开")});
    com4Item->setData(0, Qt::UserRole, "device_002");
    
    QTreeWidgetItem* networkItem = new QTreeWidgetItem(deviceTree_, QStringList{tr("网络设备")});
    networkItem->setExpanded(true);
    
    QTreeWidgetItem* tcpItem = new QTreeWidgetItem(networkItem, QStringList{"192.168.1.100:5000", "TCP", tr("已断开")});
    tcpItem->setData(0, Qt::UserRole, "device_003");
    
    updateButtonStates();
    
    DS_LOG_INFO("Device list refreshed");
}

void DevicePanel::onAddDevice()
{
    DS_LOG_INFO("Add device requested");
    emit addDeviceRequested();
    
    // TODO: 打开添加设备对话框
}

void DevicePanel::onRemoveDevice()
{
    QString deviceId = getSelectedDeviceId();
    if (deviceId.isEmpty()) {
        return;
    }
    
    DS_LOG_INFO("Remove device requested: " + deviceId.toStdString());
    
    // TODO: 从设备管理器移除设备
    
    refreshDeviceList();
}

void DevicePanel::onConnectDevice()
{
    QString deviceId = getSelectedDeviceId();
    if (deviceId.isEmpty()) {
        return;
    }
    
    DS_LOG_INFO("Connect device requested: " + deviceId.toStdString());
    
    // TODO: 连接设备
    
    emit deviceConnectionChanged(true);
    refreshDeviceList();
}

void DevicePanel::onDisconnectDevice()
{
    QString deviceId = getSelectedDeviceId();
    if (deviceId.isEmpty()) {
        return;
    }
    
    DS_LOG_INFO("Disconnect device requested: " + deviceId.toStdString());
    
    // TODO: 断开设备
    
    emit deviceConnectionChanged(false);
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
        // TODO: 打开设备详情或切换到数据终端
    }
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
