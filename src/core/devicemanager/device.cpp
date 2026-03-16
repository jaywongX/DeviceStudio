/**
 * @file device.cpp
 * @brief 设备基类实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "device.h"

namespace DeviceStudio {

Device::Device(DeviceType type, QObject* parent)
    : IDevice(parent)
    , deviceType_(type)
{
    deviceId_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
    deviceName_ = "Device";
}

bool Device::connect(const QVariantMap& config)
{
    setConfiguration(config);
    setDeviceState(DeviceState::Connecting);
    
    // 子类实现具体连接逻辑
    // 这里仅作为示例返回 false
    return false;
}

void Device::disconnect()
{
    setDeviceState(DeviceState::Disconnecting);
    
    // 子类实现具体断开逻辑
    
    setDeviceState(DeviceState::Disconnected);
}

bool Device::isConnected() const
{
    return deviceState_ == DeviceState::Connected;
}

qint64 Device::send(const QByteArray& data)
{
    Q_UNUSED(data);
    
    // 子类实现具体发送逻辑
    return -1;
}

QByteArray Device::receive()
{
    // 子类实现具体接收逻辑
    return QByteArray();
}

void Device::setDeviceState(DeviceState state)
{
    if (deviceState_ != state) {
        deviceState_ = state;
        emit stateChanged(state);
    }
}

void Device::setConfiguration(const QVariantMap& config)
{
    configuration_ = config;
}

} // namespace DeviceStudio
