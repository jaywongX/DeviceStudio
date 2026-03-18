/**
 * @file canbus.h
 * @brief CAN总线通信类
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "core/devicemanager/device.h"
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QTimer>
#include <QDateTime>

namespace DeviceStudio {

/**
 * @brief CAN总线配置结构体
 */
struct CanBusConfig {
    QString pluginName;         // 插件名称 (socketcan, peakcan, vectorcan, etc.)
    QString interfaceName;      // 接口名称 (can0, COM1, etc.)
    qint32 bitrate = 500000;    // 波特率 (bps)
    bool loopback = false;      // 回环模式
    bool receiveOwn = false;    // 接收自己发送的帧
    bool canFd = false;         // CAN FD模式
    int dataBitrate = 2000000;  // CAN FD数据相位波特率
    
    // 从QVariantMap转换
    static CanBusConfig fromVariantMap(const QVariantMap& map);
    QVariantMap toVariantMap() const;
};

/**
 * @brief CAN帧结构体
 */
struct CanFrame {
    quint32 id = 0;                 // 帧ID (标准11位或扩展29位)
    bool extended = false;          // 扩展帧标志
    bool rtr = false;               // 远程帧标志
    bool error = false;             // 错误帧标志
    bool fdFormat = false;          // CAN FD格式
    bool bitrateSwitch = false;     // CAN FD波特率切换
    QByteArray payload;             // 数据负载 (最多8字节或CAN FD 64字节)
    QDateTime timestamp;            // 时间戳
    
    // 便捷方法
    static CanFrame fromQCanBusFrame(const QCanBusFrame& frame);
    QCanBusFrame toQCanBusFrame() const;
    QString toString() const;
};

/**
 * @brief CAN总线设备类
 */
class CanBus : public Device
{
    Q_OBJECT

public:
    explicit CanBus(QObject* parent = nullptr);
    explicit CanBus(const QString& pluginName, const QString& interfaceName, QObject* parent = nullptr);
    ~CanBus() override;
    
    // ========== IDevice 接口实现 ==========
    bool connect(const QVariantMap& config) override;
    void disconnect() override;
    qint64 send(const QByteArray& data) override;
    QByteArray receive() override;
    
    // ========== CAN特定方法 ==========
    
    /**
     * @brief 获取可用的CAN插件列表
     */
    static QStringList availablePlugins();
    
    /**
     * @brief 获取指定插件的可用接口
     */
    static QStringList availableInterfaces(const QString& pluginName);
    
    /**
     * @brief 设置CAN配置
     */
    void setConfig(const CanBusConfig& config);
    CanBusConfig getConfig() const { return config_; }
    
    /**
     * @brief 发送CAN帧
     */
    bool sendFrame(const CanFrame& frame);
    
    /**
     * @brief 发送标准帧
     * @param id 帧ID (11位)
     * @param data 数据 (最多8字节)
     */
    bool sendStandardFrame(quint32 id, const QByteArray& data);
    
    /**
     * @brief 发送扩展帧
     * @param id 帧ID (29位)
     * @param data 数据 (最多8字节)
     */
    bool sendExtendedFrame(quint32 id, const QByteArray& data);
    
    /**
     * @brief 发送远程帧
     */
    bool sendRemoteFrame(quint32 id, int dlc = 0);
    
    /**
     * @brief 设置接收过滤器
     * @param id 起始ID
     * @param mask 掩码
     * @param extended 是否扩展帧
     */
    bool setFilter(quint32 id, quint32 mask, bool extended = false);
    
    /**
     * @brief 清除所有过滤器
     */
    void clearFilters();
    
    /**
     * @brief 获取统计信息
     */
    struct Statistics {
        quint64 framesReceived = 0;
        quint64 framesSent = 0;
        quint64 framesDropped = 0;
        quint64 bytesReceived = 0;
        quint64 bytesSent = 0;
        quint64 errors = 0;
    };
    Statistics getStatistics() const { return statistics_; }
    
    /**
     * @brief 重置统计信息
     */
    void resetStatistics();
    
    /**
     * @brief 获取最近的错误信息
     */
    QString lastError() const;

signals:
    /**
     * @brief 接收到CAN帧
     */
    void frameReceived(const CanFrame& frame);
    
    /**
     * @brief 发送了CAN帧
     */
    void frameSent(const CanFrame& frame);
    
    /**
     * @brief 错误帧接收
     */
    void errorFrameReceived(const CanFrame& frame);
    
    /**
     * @brief 配置改变
     */
    void configChanged(const CanBusConfig& config);

private slots:
    void onFramesReceived();
    void onErrorOccurred(QCanBusDevice::CanBusError error);
    void onStateChanged(QCanBusDevice::CanBusDeviceState state);

private:
    bool applyConfig();
    void processFrame(const QCanBusFrame& frame);
    
    QCanBusDevice* canDevice_ = nullptr;
    CanBusConfig config_;
    Statistics statistics_;
    QString lastError_;
    QByteArray receiveBuffer_;
};

} // namespace DeviceStudio
