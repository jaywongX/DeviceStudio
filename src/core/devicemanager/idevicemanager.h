/**
 * @file idevicemanager.h
 * @brief 设备管理器接口定义
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "idevice.h"
#include <QObject>
#include <QString>
#include <QList>
#include <functional>
#include <memory>

namespace DeviceStudio {

/**
 * @brief 设备管理器接口类
 * 
 * 负责设备的创建、管理和销毁
 */
class IDeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit IDeviceManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDeviceManager() = default;
    
    // ========== 设备创建 ==========
    
    /**
     * @brief 创建设备
     * @param type 设备类型
     * @param name 设备名称
     * @param config 设备配置
     * @return 设备指针，失败返回 nullptr
     */
    virtual IDevicePtr createDevice(DeviceType type, 
                                     const QString& name,
                                     const QVariantMap& config = QVariantMap()) = 0;
    
    // ========== 设备管理 ==========
    
    /**
     * @brief 添加设备
     * @param device 设备指针
     * @return 成功返回 true
     */
    virtual bool addDevice(IDevicePtr device) = 0;
    
    /**
     * @brief 移除设备
     * @param deviceId 设备ID
     * @return 成功返回 true
     */
    virtual bool removeDevice(const QString& deviceId) = 0;
    
    /**
     * @brief 获取设备
     * @param deviceId 设备ID
     * @return 设备指针
     */
    virtual IDevicePtr getDevice(const QString& deviceId) const = 0;
    
    /**
     * @brief 获取所有设备
     */
    virtual QList<IDevicePtr> getAllDevices() const = 0;
    
    /**
     * @brief 获取设备数量
     */
    virtual int deviceCount() const = 0;
    
    // ========== 设备查询 ==========
    
    /**
     * @brief 根据名称查找设备
     * @param name 设备名称
     * @return 设备列表
     */
    virtual QList<IDevicePtr> findDevicesByName(const QString& name) const = 0;
    
    /**
     * @brief 根据类型查找设备
     * @param type 设备类型
     * @return 设备列表
     */
    virtual QList<IDevicePtr> findDevicesByType(DeviceType type) const = 0;
    
    /**
     * @brief 根据状态查找设备
     * @param state 设备状态
     * @return 设备列表
     */
    virtual QList<IDevicePtr> findDevicesByState(DeviceState state) const = 0;
    
    // ========== 批量操作 ==========
    
    /**
     * @brief 连接所有设备
     */
    virtual void connectAll() = 0;
    
    /**
     * @brief 断开所有设备
     */
    virtual void disconnectAll() = 0;
    
    /**
     * @brief 移除所有设备
     */
    virtual void removeAllDevices() = 0;

signals:
    /**
     * @brief 设备添加信号
     */
    void deviceAdded(const QString& deviceId);
    
    /**
     * @brief 设备移除信号
     */
    void deviceRemoved(const QString& deviceId);
    
    /**
     * @brief 设备状态改变信号
     */
    void deviceStateChanged(const QString& deviceId, DeviceState state);
    
    /**
     * @brief 设备数据接收信号
     */
    void deviceDataReceived(const QString& deviceId, const QByteArray& data);
    
    /**
     * @brief 设备错误信号
     */
    void deviceError(const QString& deviceId, const QString& errorString);
};

// 设备管理器智能指针类型
using IDeviceManagerPtr = std::shared_ptr<IDeviceManager>;

} // namespace DeviceStudio
