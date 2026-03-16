/**
 * @file tcpclient.cpp
 * @brief TCP客户端实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "tcpclient.h"
#include "utils/log/logger.h"

namespace DeviceStudio {

// ========== TcpClientConfig 实现 ==========

TcpClientConfig TcpClientConfig::fromVariantMap(const QVariantMap& map)
{
    TcpClientConfig config;
    config.hostAddress = map.value("hostAddress", "127.0.0.1").toString();
    config.port = map.value("port", 8080).toUInt();
    config.timeoutMs = map.value("timeout", 5000).toInt();
    config.autoReconnect = map.value("autoReconnect", false).toBool();
    config.reconnectInterval = map.value("reconnectInterval", 3000).toInt();
    return config;
}

QVariantMap TcpClientConfig::toVariantMap() const
{
    QVariantMap map;
    map["hostAddress"] = hostAddress;
    map["port"] = port;
    map["timeout"] = timeoutMs;
    map["autoReconnect"] = autoReconnect;
    map["reconnectInterval"] = reconnectInterval;
    return map;
}

// ========== TcpClient 实现 ==========

TcpClient::TcpClient(QObject* parent)
    : Device(DeviceType::TcpClient, parent)
{
    setDeviceName("TCP Client");
    socket_ = new QTcpSocket(this);
    reconnectTimer_ = new QTimer(this);
    
    QObject::connect(socket_, &QTcpSocket::connected, this, &TcpClient::onConnected);
    QObject::connect(socket_, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    QObject::connect(socket_, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    QObject::connect(socket_, &QTcpSocket::errorOccurred, this, &TcpClient::onError);
    QObject::connect(reconnectTimer_, &QTimer::timeout, this, &TcpClient::onReconnectTimer);
}

TcpClient::TcpClient(const QString& host, quint16 port, QObject* parent)
    : TcpClient(parent)
{
    config_.hostAddress = host;
    config_.port = port;
    setDeviceName(QString("TCP Client (%1:%2)").arg(host).arg(port));
}

TcpClient::~TcpClient()
{
    if (isConnected()) {
        disconnect();
    }
}

bool TcpClient::connect(const QVariantMap& config)
{
    config_ = TcpClientConfig::fromVariantMap(config);
    setConfiguration(config);
    
    if (config_.hostAddress.isEmpty()) {
        DS_LOG_ERROR("TCP client host address is empty");
        emit errorOccurred("Host address is empty");
        return false;
    }
    
    setDeviceState(DeviceState::Connecting);
    DS_LOG_INFO("TCP client connecting to {}:{}", config_.hostAddress.toStdString(), config_.port);
    
    socket_->connectToHost(config_.hostAddress, config_.port);
    
    // 等待连接
    if (socket_->waitForConnected(config_.timeoutMs)) {
        return true;
    } else {
        setDeviceState(DeviceState::Disconnected);
        QString error = QString("Connection timeout: %1:%2").arg(config_.hostAddress).arg(config_.port);
        DS_LOG_ERROR("{}", error.toStdString());
        emit errorOccurred(error);
        
        // 如果启用了自动重连
        if (config_.autoReconnect) {
            reconnectTimer_->start(config_.reconnectInterval);
        }
        
        return false;
    }
}

void TcpClient::disconnect()
{
    if (reconnectTimer_->isActive()) {
        reconnectTimer_->stop();
    }
    
    if (socket_->state() != QAbstractSocket::UnconnectedState) {
        setDeviceState(DeviceState::Disconnecting);
        socket_->disconnectFromHost();
    }
}

qint64 TcpClient::send(const QByteArray& data)
{
    if (!isConnected()) {
        DS_LOG_WARN("TCP client not connected");
        return -1;
    }
    
    qint64 bytesWritten = socket_->write(data);
    if (bytesWritten > 0) {
        socket_->flush();
        emit dataSent(data);
        DS_LOG_DEBUG("TCP client sent {} bytes", bytesWritten);
    } else {
        DS_LOG_ERROR("Failed to send data: {}", socket_->errorString().toStdString());
    }
    
    return bytesWritten;
}

QByteArray TcpClient::receive()
{
    QByteArray data = receiveBuffer_;
    receiveBuffer_.clear();
    return data;
}

void TcpClient::setConfig(const TcpClientConfig& config)
{
    config_ = config;
    if (isConnected()) {
        disconnect();
        connect(config_.toVariantMap());
    }
    emit configChanged(config_);
}

QString TcpClient::localAddress() const
{
    return socket_->localAddress().toString();
}

quint16 TcpClient::localPort() const
{
    return socket_->localPort();
}

void TcpClient::clearReceiveBuffer()
{
    receiveBuffer_.clear();
}

int TcpClient::receiveBufferSize() const
{
    return receiveBuffer_.size();
}

void TcpClient::setAutoReconnect(bool enabled, int intervalMs)
{
    config_.autoReconnect = enabled;
    config_.reconnectInterval = intervalMs;
    
    if (!enabled && reconnectTimer_->isActive()) {
        reconnectTimer_->stop();
    }
}

void TcpClient::onConnected()
{
    setDeviceState(DeviceState::Connected);
    DS_LOG_INFO("TCP client connected to {}:{}", config_.hostAddress.toStdString(), config_.port);
    emit connected();
    
    if (reconnectTimer_->isActive()) {
        reconnectTimer_->stop();
    }
}

void TcpClient::onDisconnected()
{
    setDeviceState(DeviceState::Disconnected);
    DS_LOG_INFO("TCP client disconnected");
    emit disconnected();
    
    // 如果启用了自动重连
    if (config_.autoReconnect) {
        reconnectTimer_->start(config_.reconnectInterval);
    }
}

void TcpClient::onReadyRead()
{
    QByteArray data = socket_->readAll();
    receiveBuffer_.append(data);
    emit dataReceived(data);
    DS_LOG_TRACE("TCP client received {} bytes", data.size());
}

void TcpClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorString = socket_->errorString();
    DS_LOG_ERROR("TCP client error: {}", errorString.toStdString());
    emit errorOccurred(errorString);
}

void TcpClient::onReconnectTimer()
{
    DS_LOG_INFO("TCP client attempting to reconnect...");
    socket_->connectToHost(config_.hostAddress, config_.port);
}

} // namespace DeviceStudio
