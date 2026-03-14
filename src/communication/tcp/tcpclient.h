/**
 * @file tcpclient.h
 * @brief TCP客户端类
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include "core/devicemanager/device.h"
#include <QTcpSocket>
#include <QTimer>

namespace DeviceStudio {

/**
 * @brief TCP客户端配置
 */
struct TcpClientConfig {
    QString hostAddress = "127.0.0.1";  // 服务器地址
    quint16 port = 8080;                 // 服务器端口
    int timeoutMs = 5000;                // 连接超时
    bool autoReconnect = false;          // 自动重连
    int reconnectInterval = 3000;        // 重连间隔（毫秒）
    
    static TcpClientConfig fromVariantMap(const QVariantMap& map);
    QVariantMap toVariantMap() const;
};

/**
 * @brief TCP客户端设备类
 */
class TcpClient : public Device
{
    Q_OBJECT

public:
    explicit TcpClient(QObject* parent = nullptr);
    explicit TcpClient(const QString& host, quint16 port, QObject* parent = nullptr);
    ~TcpClient() override;
    
    // ========== IDevice 接口实现 ==========
    bool connect(const QVariantMap& config) override;
    void disconnect() override;
    qint64 send(const QByteArray& data) override;
    QByteArray receive() override;
    
    // ========== TCP客户端特定方法 ==========
    
    /**
     * @brief 设置配置
     */
    void setConfig(const TcpClientConfig& config);
    TcpClientConfig getConfig() const { return config_; }
    
    /**
     * @brief 获取服务器地址
     */
    QString serverAddress() const { return config_.hostAddress; }
    
    /**
     * @brief 获取服务器端口
     */
    quint16 serverPort() const { return config_.port; }
    
    /**
     * @brief 获取本地地址
     */
    QString localAddress() const;
    
    /**
     * @brief 获取本地端口
     */
    quint16 localPort() const;
    
    /**
     * @brief 清空接收缓冲区
     */
    void clearReceiveBuffer();
    
    /**
     * @brief 获取接收缓冲区大小
     */
    int receiveBufferSize() const;
    
    /**
     * @brief 设置自动重连
     */
    void setAutoReconnect(bool enabled, int intervalMs = 3000);

signals:
    void configChanged(const TcpClientConfig& config);
    void connected();
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void onReconnectTimer();

private:
    QTcpSocket* socket_ = nullptr;
    QByteArray receiveBuffer_;
    TcpClientConfig config_;
    QTimer* reconnectTimer_ = nullptr;
};

} // namespace DeviceStudio
