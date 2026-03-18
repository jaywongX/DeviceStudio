/**
 * @file tcpserver.h
 * @brief TCP服务器类
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "core/devicemanager/device.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>

namespace DeviceStudio {

/**
 * @brief TCP服务器配置
 */
struct TcpServerConfig {
    QString listenAddress = "0.0.0.0";  // 监听地址
    quint16 listenPort = 8080;           // 监听端口
    int maxConnections = 100;            // 最大连接数
    
    static TcpServerConfig fromVariantMap(const QVariantMap& map);
    QVariantMap toVariantMap() const;
};

/**
 * @brief TCP服务器设备类
 */
class TcpServer : public Device
{
    Q_OBJECT

public:
    explicit TcpServer(QObject* parent = nullptr);
    explicit TcpServer(quint16 port, QObject* parent = nullptr);
    ~TcpServer() override;
    
    // ========== IDevice 接口实现 ==========
    bool connect(const QVariantMap& config) override;
    void disconnect() override;
    qint64 send(const QByteArray& data) override;
    QByteArray receive() override;
    
    // ========== TCP服务器特定方法 ==========
    
    /**
     * @brief 设置配置
     */
    void setConfig(const TcpServerConfig& config);
    TcpServerConfig getConfig() const { return config_; }
    
    /**
     * @brief 获取监听地址
     */
    QString serverAddress() const;
    
    /**
     * @brief 获取监听端口
     */
    quint16 serverPort() const { return config_.listenPort; }
    
    /**
     * @brief 获取客户端数量
     */
    int clientCount() const { return clients_.size(); }
    
    /**
     * @brief 获取客户端列表
     */
    QList<QTcpSocket*> clients() const { return clients_.values(); }
    
    /**
     * @brief 获取客户端地址
     */
    QString clientAddress(int clientId) const;
    
    /**
     * @brief 向指定客户端发送数据
     */
    qint64 sendToClient(int clientId, const QByteArray& data);
    
    /**
     * @brief 向所有客户端广播数据
     */
    qint64 broadcast(const QByteArray& data);
    
    /**
     * @brief 断开指定客户端
     */
    void disconnectClient(int clientId);
    
    /**
     * @brief 清空接收缓冲区
     */
    void clearReceiveBuffer();

signals:
    void clientConnected(int clientId, const QString& address);
    void clientDisconnected(int clientId);
    void dataReceivedFromClient(int clientId, const QByteArray& data);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();
    void onClientError(QAbstractSocket::SocketError error);

private:
    QTcpServer* server_ = nullptr;
    QMap<int, QTcpSocket*> clients_;
    QByteArray receiveBuffer_;
    TcpServerConfig config_;
    int nextClientId_ = 1;
};

} // namespace DeviceStudio
