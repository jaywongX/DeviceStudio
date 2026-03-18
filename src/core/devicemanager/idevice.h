/**
 * @file idevice.h
 * @brief 设备接口定义
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantMap>
#include <memory>

namespace DeviceStudio {

/**
 * @brief 设备状态枚举
 */
enum class DeviceState {
    Disconnected,   ///< 已断开
    Connecting,     ///< 连接中
    Connected,      ///< 已连接
    Disconnecting,  ///< 断开中
    Error           ///< 错误状态
};

/**
 * @brief 设备类型枚举
 */
enum class DeviceType {
    Serial,         ///< 串口设备
    TcpClient,      ///< TCP客户端
    TcpServer,      ///< TCP服务器
    Udp,            ///< UDP通信
    ModbusRtu,      ///< Modbus RTU
    ModbusTcp,      ///< Modbus TCP
    Can,            ///< CAN总线
    Custom          ///< 自定义设备
};

/**
 * @brief 设备接口类
 * 
 * 所有设备实现都必须继承此接口
 */
class IDevice : public QObject
{
    Q_OBJECT

public:
    explicit IDevice(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDevice() = default;
    
    // ========== 基本信息 ==========
    
    /**
     * @brief 获取设备ID
     */
    virtual QString deviceId() const = 0;
    
    /**
     * @brief 获取设备名称
     */
    virtual QString deviceName() const = 0;
    
    /**
     * @brief 获取设备类型
     */
    virtual DeviceType deviceType() const = 0;
    
    /**
     * @brief 获取设备状态
     */
    virtual DeviceState deviceState() const = 0;
    
    // ========== 连接管理 ==========
    
    /**
     * @brief 连接设备
     * @param config 连接配置参数
     * @return 成功返回 true，失败返回 false
     */
    virtual bool connect(const QVariantMap& config) = 0;
    
    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief 是否已连接
     */
    virtual bool isConnected() const = 0;
    
    // ========== 数据收发 ==========
    
    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 实际发送的字节数，失败返回 -1
     */
    virtual qint64 send(const QByteArray& data) = 0;
    
    /**
     * @brief 接收数据（非阻塞）
     * @return 接收到的数据
     */
    virtual QByteArray receive() = 0;
    
    // ========== 配置管理 ==========
    
    /**
     * @brief 获取设备配置
     */
    virtual QVariantMap getConfiguration() const = 0;
    
    /**
     * @brief 设置设备配置
     */
    virtual void setConfiguration(const QVariantMap& config) = 0;

signals:
    /**
     * @brief 设备状态改变信号
     */
    void stateChanged(DeviceState state);
    
    /**
     * @brief 数据接收信号
     */
    void dataReceived(const QByteArray& data);
    
    /**
     * @brief 数据发送信号
     */
    void dataSent(const QByteArray& data);
    
    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& errorString);
};

// 设备智能指针类型
using IDevicePtr = std::shared_ptr<IDevice>;
using IDeviceWeakPtr = std::weak_ptr<IDevice>;

} // namespace DeviceStudio

// 元类型注册
Q_DECLARE_METATYPE(DeviceStudio::DeviceState)
Q_DECLARE_METATYPE(DeviceStudio::DeviceType)
