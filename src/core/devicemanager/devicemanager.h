/**
 * @file devicemanager.h
 * @brief 设备管理器实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "idevicemanager.h"
#include <QMap>

namespace DeviceStudio {

/**
 * @brief 设备管理器类
 * 
 * 实现设备管理器接口
 */
class DeviceManager : public IDeviceManager
{
    Q_OBJECT

public:
    explicit DeviceManager(QObject* parent = nullptr);
    ~DeviceManager() override = default;
    
    // ========== IDeviceManager 接口实现 ==========
    
    IDevicePtr createDevice(DeviceType type, 
                             const QString& name,
                             const QVariantMap& config = QVariantMap()) override;
    
    bool addDevice(IDevicePtr device) override;
    bool removeDevice(const QString& deviceId) override;
    IDevicePtr getDevice(const QString& deviceId) const override;
    QList<IDevicePtr> getAllDevices() const override;
    int deviceCount() const override;
    
    QList<IDevicePtr> findDevicesByName(const QString& name) const override;
    QList<IDevicePtr> findDevicesByType(DeviceType type) const override;
    QList<IDevicePtr> findDevicesByState(DeviceState state) const override;
    
    void connectAll() override;
    void disconnectAll() override;
    void removeAllDevices() override;

private:
    QMap<QString, IDevicePtr> devices_;
};

} // namespace DeviceStudio
