/**
 * @file udpsocket.h
 * @brief UDP通信类
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include "core/devicemanager/device.h"
#include <QUdpSocket>
#include <QNetworkDatagram>

namespace DeviceStudio {

/**
 * @brief UDP配置
 */
struct UdpConfig {
    QString localAddress = "0.0.0.0";    // 本地绑定地址
    quint16 localPort = 8080;             // 本地端口
    QString remoteAddress = "127.0.0.1";  // 远程地址
    quint16 remotePort = 8080;            // 远程端口
    bool multicast = false;               // 是否使用多播
    QString multicastGroup = "239.0.0.1"; // 多播组地址
    
    static UdpConfig fromVariantMap(const QVariantMap& map);
    QVariantMap toVariantMap() const;
};

/**
 * @brief UDP通信设备类
 */
class UdpSocket : public Device
{
    Q_OBJECT

public:
    explicit UdpSocket(QObject* parent = nullptr);
    explicit UdpSocket(quint16 localPort, QObject* parent = nullptr);
    ~UdpSocket() override;
    
    // ========== IDevice 接口实现 ==========
    bool connect(const QVariantMap& config) override;
    void disconnect() override;
    qint64 send(const QByteArray& data) override;
    QByteArray receive() override;
    
    // ========== UDP特定方法 ==========
    
    /**
     * @brief 设置配置
     */
    void setConfig(const UdpConfig& config);
    UdpConfig getConfig() const { return config_; }
    
    /**
     * @brief 发送数据到指定地址
     */
    qint64 sendTo(const QString& address, quint16 port, const QByteArray& data);
    
    /**
     * @brief 加入多播组
     */
    bool joinMulticastGroup(const QString& groupAddress);
    
    /**
     * @brief 离开多播组
     */
    bool leaveMulticastGroup(const QString& groupAddress);
    
    /**
     * @brief 获取本地地址
     */
    QString localAddress() const;
    
    /**
     * @brief 获取本地端口
     */
    quint16 localPort() const { return config_.localPort; }
    
    /**
     * @brief 清空接收缓冲区
     */
    void clearReceiveBuffer();
    
    /**
     * @brief 获取接收缓冲区大小
     */
    int receiveBufferSize() const;

signals:
    void datagramReceived(const QString& senderAddress, quint16 senderPort, const QByteArray& data);
    void configChanged(const UdpConfig& config);

private slots:
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QUdpSocket* socket_ = nullptr;
    QByteArray receiveBuffer_;
    UdpConfig config_;
};

} // namespace DeviceStudio
