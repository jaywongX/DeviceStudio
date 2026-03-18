/**
 * @file device.h
 * @brief 设备基类实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "idevice.h"
#include <QUuid>

namespace DeviceStudio {

/**
 * @brief 设备基类
 * 
 * 提供设备接口的默认实现
 */
class Device : public IDevice
{
    Q_OBJECT

public:
    explicit Device(DeviceType type, QObject* parent = nullptr);
    virtual ~Device() = default;
    
    // ========== IDevice 接口实现 ==========
    
    QString deviceId() const override { return deviceId_; }
    QString deviceName() const override { return deviceName_; }
    DeviceType deviceType() const override { return deviceType_; }
    DeviceState deviceState() const override { return deviceState_; }
    
    bool connect(const QVariantMap& config) override;
    void disconnect() override;
    bool isConnected() const override;
    
    qint64 send(const QByteArray& data) override;
    QByteArray receive() override;
    
    QVariantMap getConfiguration() const override { return configuration_; }
    void setConfiguration(const QVariantMap& config) override;
    
    /**
     * @brief 设置设备名称（公共接口）
     */
    void setName(const QString& name) { deviceName_ = name; }

protected:
    void setDeviceId(const QString& id) { deviceId_ = id; }
    void setDeviceName(const QString& name) { deviceName_ = name; }
    void setDeviceState(DeviceState state);
    
    QString deviceId_;
    QString deviceName_;
    DeviceType deviceType_;
    DeviceState deviceState_ = DeviceState::Disconnected;
    QVariantMap configuration_;
};

} // namespace DeviceStudio
