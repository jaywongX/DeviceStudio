/**
 * @file tcpserver.cpp
 * @brief TCP服务器实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "tcpserver.h"
#include "utils/log/logger.h"

namespace DeviceStudio {

// ========== TcpServerConfig 实现 ==========

TcpServerConfig TcpServerConfig::fromVariantMap(const QVariantMap& map)
{
    TcpServerConfig config;
    config.listenAddress = map.value("listenAddress", "0.0.0.0").toString();
    config.listenPort = map.value("listenPort", 8080).toUInt();
    config.maxConnections = map.value("maxConnections", 100).toInt();
    return config;
}

QVariantMap TcpServerConfig::toVariantMap() const
{
    QVariantMap map;
    map["listenAddress"] = listenAddress;
    map["listenPort"] = listenPort;
    map["maxConnections"] = maxConnections;
    return map;
}

// ========== TcpServer 实现 ==========

TcpServer::TcpServer(QObject* parent)
    : Device(DeviceType::TcpServer, parent)
{
    setDeviceName("TCP Server");
    server_ = new QTcpServer(this);

    QObject::connect(server_, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
}

TcpServer::TcpServer(quint16 port, QObject* parent)
    : TcpServer(parent)
{
    config_.listenPort = port;
    setDeviceName(QString("TCP Server (:%1)").arg(port));
}

TcpServer::~TcpServer()
{
    if (isConnected()) {
        disconnect();
    }
}

bool TcpServer::connect(const QVariantMap& config)
{
    config_ = TcpServerConfig::fromVariantMap(config);
    setConfiguration(config);
    
    QHostAddress address(config_.listenAddress);
    
    if (!server_->listen(address, config_.listenPort)) {
        QString error = QString("Failed to start TCP server: %1").arg(server_->errorString());
        DS_LOG_ERROR("{}", error.toStdString());
        setDeviceState(DeviceState::Error);
        emit errorOccurred(error);
        return false;
    }
    
    setDeviceState(DeviceState::Connected);
    DS_LOG_INFO("TCP server listening on {}:{}",
                config_.listenAddress.toStdString(), config_.listenPort);
    return true;
}

void TcpServer::disconnect()
{
    // 断开所有客户端
    for (auto client : clients_) {
        client->disconnectFromHost();
    }
    clients_.clear();
    
    // 停止监听
    if (server_->isListening()) {
        server_->close();
    }
    
    setDeviceState(DeviceState::Disconnected);
    DS_LOG_INFO("TCP server stopped");
}

qint64 TcpServer::send(const QByteArray& data)
{
    return broadcast(data);
}

QByteArray TcpServer::receive()
{
    QByteArray data = receiveBuffer_;
    receiveBuffer_.clear();
    return data;
}

void TcpServer::setConfig(const TcpServerConfig& config)
{
    config_ = config;
    if (isConnected()) {
        disconnect();
        connect(config_.toVariantMap());
    }
}

QString TcpServer::serverAddress() const
{
    if (server_->isListening()) {
        return server_->serverAddress().toString();
    }
    return QString();
}

QString TcpServer::clientAddress(int clientId) const
{
    if (clients_.contains(clientId)) {
        QTcpSocket* client = clients_[clientId];
        return QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
    }
    return QString();
}

qint64 TcpServer::sendToClient(int clientId, const QByteArray& data)
{
    if (!clients_.contains(clientId)) {
        DS_LOG_WARN("Client {} not found", clientId);
        return -1;
    }
    
    QTcpSocket* client = clients_[clientId];
    qint64 bytesWritten = client->write(data);
    
    if (bytesWritten > 0) {
        client->flush();
        emit dataSent(data);
        DS_LOG_DEBUG("Sent {} bytes to client {}", bytesWritten, clientId);
    }
    
    return bytesWritten;
}

qint64 TcpServer::broadcast(const QByteArray& data)
{
    if (clients_.isEmpty()) {
        DS_LOG_WARN("No clients connected");
        return 0;
    }
    
    qint64 totalBytes = 0;
    for (auto client : clients_) {
        qint64 bytes = client->write(data);
        if (bytes > 0) {
            client->flush();
            totalBytes += bytes;
        }
    }
    
    if (totalBytes > 0) {
        emit dataSent(data);
        DS_LOG_DEBUG("Broadcast {} bytes to {} clients", data.size(), clients_.size());
    }
    
    return totalBytes;
}

void TcpServer::disconnectClient(int clientId)
{
    if (clients_.contains(clientId)) {
        QTcpSocket* client = clients_[clientId];
        client->disconnectFromHost();
        DS_LOG_INFO("Disconnected client {}", clientId);
    }
}

void TcpServer::clearReceiveBuffer()
{
    receiveBuffer_.clear();
}

void TcpServer::onNewConnection()
{
    while (server_->hasPendingConnections()) {
        QTcpSocket* client = server_->nextPendingConnection();
        
        // 检查最大连接数
        if (clients_.size() >= config_.maxConnections) {
            DS_LOG_WARN("Maximum connections reached, rejecting new connection");
            client->disconnectFromHost();
            client->deleteLater();
            continue;
        }
        
        // 分配客户端ID
        int clientId = nextClientId_++;
        clients_[clientId] = client;
        
        // 连接信号
        QObject::connect(client, &QTcpSocket::readyRead, this, &TcpServer::onClientReadyRead);
        QObject::connect(client, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);
        QObject::connect(client, &QTcpSocket::errorOccurred, this, &TcpServer::onClientError);
        
        QString address = QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
        DS_LOG_INFO("Client {} connected: {}", clientId, address.toStdString());
        
        emit clientConnected(clientId, address);
    }
}

void TcpServer::onClientReadyRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    // 查找客户端ID
    int clientId = clients_.key(client, -1);
    if (clientId == -1) return;
    
    QByteArray data = client->readAll();
    receiveBuffer_.append(data);
    
    emit dataReceivedFromClient(clientId, data);
    emit dataReceived(data);
    
    DS_LOG_TRACE("Received {} bytes from client {}", data.size(), clientId);
}

void TcpServer::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    int clientId = clients_.key(client, -1);
    if (clientId != -1) {
        clients_.remove(clientId);
        DS_LOG_INFO("Client {} disconnected", clientId);
        emit clientDisconnected(clientId);
    }
    
    client->deleteLater();
}

void TcpServer::onClientError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    int clientId = clients_.key(client, -1);
    DS_LOG_ERROR("Client {} error: {}", clientId, client->errorString().toStdString());
}

} // namespace DeviceStudio
