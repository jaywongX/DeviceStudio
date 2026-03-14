/**
 * @file devicemanager.cpp
 * @brief 设备管理器实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "devicemanager.h"
#include "device.h"
#include <algorithm>

namespace DeviceStudio {

DeviceManager::DeviceManager(QObject* parent)
    : IDeviceManager(parent)
{
}

IDevicePtr DeviceManager::createDevice(DeviceType type, 
                                        const QString& name,
                                        const QVariantMap& config)
{
    IDevicePtr device = std::make_shared<Device>(type);
    device->setDeviceName(name);
    
    if (!config.isEmpty()) {
        device->setConfiguration(config);
    }
    
    return device;
}

bool DeviceManager::addDevice(IDevicePtr device)
{
    if (!device) {
        return false;
    }
    
    QString deviceId = device->deviceId();
    if (devices_.contains(deviceId)) {
        return false;
    }
    
    devices_[deviceId] = device;
    
    // 连接信号
    connect(device.get(), &IDevice::stateChanged, this, [this, deviceId](DeviceState state) {
        emit deviceStateChanged(deviceId, state);
    });
    
    connect(device.get(), &IDevice::dataReceived, this, [this, deviceId](const QByteArray& data) {
        emit deviceDataReceived(deviceId, data);
    });
    
    connect(device.get(), &IDevice::errorOccurred, this, [this, deviceId](const QString& error) {
        emit deviceError(deviceId, error);
    });
    
    emit deviceAdded(deviceId);
    
    return true;
}

bool DeviceManager::removeDevice(const QString& deviceId)
{
    if (!devices_.contains(deviceId)) {
        return false;
    }
    
    IDevicePtr device = devices_[deviceId];
    if (device && device->isConnected()) {
        device->disconnect();
    }
    
    devices_.remove(deviceId);
    emit deviceRemoved(deviceId);
    
    return true;
}

IDevicePtr DeviceManager::getDevice(const QString& deviceId) const
{
    return devices_.value(deviceId);
}

QList<IDevicePtr> DeviceManager::getAllDevices() const
{
    return devices_.values();
}

int DeviceManager::deviceCount() const
{
    return devices_.size();
}

QList<IDevicePtr> DeviceManager::findDevicesByName(const QString& name) const
{
    QList<IDevicePtr> result;
    for (const auto& device : devices_) {
        if (device->deviceName() == name) {
            result.append(device);
        }
    }
    return result;
}

QList<IDevicePtr> DeviceManager::findDevicesByType(DeviceType type) const
{
    QList<IDevicePtr> result;
    for (const auto& device : devices_) {
        if (device->deviceType() == type) {
            result.append(device);
        }
    }
    return result;
}

QList<IDevicePtr> DeviceManager::findDevicesByState(DeviceState state) const
{
    QList<IDevicePtr> result;
    for (const auto& device : devices_) {
        if (device->deviceState() == state) {
            result.append(device);
        }
    }
    return result;
}

void DeviceManager::connectAll()
{
    for (auto& device : devices_) {
        if (!device->isConnected()) {
            device->connect(device->getConfiguration());
        }
    }
}

void DeviceManager::disconnectAll()
{
    for (auto& device : devices_) {
        if (device->isConnected()) {
            device->disconnect();
        }
    }
}

void DeviceManager::removeAllDevices()
{
    disconnectAll();
    devices_.clear();
}

} // namespace DeviceStudio
