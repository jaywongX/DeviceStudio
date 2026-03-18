/**
 * @file devicepanel.h
 * @brief 设备面板类
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>

namespace DeviceStudio {

class DeviceManager;

/**
 * @brief 设备面板类
 * 
 * 显示设备列表，支持设备添加、删除、连接和断开
 */
class DevicePanel : public QWidget
{
    Q_OBJECT

public:
    explicit DevicePanel(QWidget* parent = nullptr);
    ~DevicePanel() override = default;
    
    /**
     * @brief 设置设备管理器
     */
    void setDeviceManager(std::shared_ptr<DeviceManager> manager);
    
    /**
     * @brief 刷新设备列表
     */
    void refreshDeviceList();

signals:
    /**
     * @brief 设备连接状态改变信号
     */
    void deviceConnectionChanged(bool connected);
    
    /**
     * @brief 设备选中信号
     */
    void deviceSelected(const QString& deviceId);
    
    /**
     * @brief 添加设备请求信号
     */
    void addDeviceRequested();

private slots:
    /**
     * @brief 添加设备
     */
    void onAddDevice();
    
    /**
     * @brief 删除设备
     */
    void onRemoveDevice();
    
    /**
     * @brief 连接设备
     */
    void onConnectDevice();
    
    /**
     * @brief 断开设备
     */
    void onDisconnectDevice();
    
    /**
     * @brief 设备项选中
     */
    void onItemSelectionChanged();
    
    /**
     * @brief 设备项双击
     */
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    /**
     * @brief 设置UI
     */
    void setupUi();
    
    /**
     * @brief 更新按钮状态
     */
    void updateButtonStates();
    
    /**
     * @brief 获取选中的设备ID
     */
    QString getSelectedDeviceId() const;
    
    /**
     * @brief 设备类型转换为显示名称
     */
    QString deviceTypeToCategory(const QString& deviceType) const;

private:
    std::shared_ptr<DeviceManager> deviceManager_;
    QTreeWidget* deviceTree_ = nullptr;
    QPushButton* addButton_ = nullptr;
    QPushButton* removeButton_ = nullptr;
    QPushButton* connectButton_ = nullptr;
    QPushButton* disconnectButton_ = nullptr;
    QPushButton* refreshButton_ = nullptr;
};

} // namespace DeviceStudio
