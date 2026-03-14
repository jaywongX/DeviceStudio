/**
 * @file udpsocket.cpp
 * @brief UDP通信实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "udpsocket.h"
#include "utils/log/logger.h"

namespace DeviceStudio {

// ========== UdpConfig 实现 ==========

UdpConfig UdpConfig::fromVariantMap(const QVariantMap& map)
{
    UdpConfig config;
    config.localAddress = map.value("localAddress", "0.0.0.0").toString();
    config.localPort = map.value("localPort", 8080).toUInt();
    config.remoteAddress = map.value("remoteAddress", "127.0.0.1").toString();
    config.remotePort = map.value("remotePort", 8080).toUInt();
    config.multicast = map.value("multicast", false).toBool();
    config.multicastGroup = map.value("multicastGroup", "239.0.0.1").toString();
    return config;
}

QVariantMap UdpConfig::toVariantMap() const
{
    QVariantMap map;
    map["localAddress"] = localAddress;
    map["localPort"] = localPort;
    map["remoteAddress"] = remoteAddress;
    map["remotePort"] = remotePort;
    map["multicast"] = multicast;
    map["multicastGroup"] = multicastGroup;
    return map;
}

// ========== UdpSocket 实现 ==========

UdpSocket::UdpSocket(QObject* parent)
    : Device(DeviceType::Udp, parent)
{
    setDeviceName("UDP Socket");
    socket_ = new QUdpSocket(this);
    
    connect(socket_, &QUdpSocket::readyRead, this, &UdpSocket::onReadyRead);
    connect(socket_, &QUdpSocket::errorOccurred, this, &UdpSocket::onError);
}

UdpSocket::UdpSocket(quint16 localPort, QObject* parent)
    : UdpSocket(parent)
{
    config_.localPort = localPort;
    setDeviceName(QString("UDP Socket (:%1)").arg(localPort));
}

UdpSocket::~UdpSocket()
{
    if (isConnected()) {
        disconnect();
    }
}

bool UdpSocket::connect(const QVariantMap& config)
{
    config_ = UdpConfig::fromVariantMap(config);
    setConfiguration(config);
    
    QHostAddress address(config_.localAddress);
    
    // 绑定端口
    if (!socket_->bind(address, config_.localPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        QString error = QString("Failed to bind UDP socket: %1").arg(socket_->errorString());
        DS_LOG_ERROR("{}", error.toStdString());
        setDeviceState(DeviceState::Error);
        emit errorOccurred(error);
        return false;
    }
    
    // 如果是多播模式
    if (config_.multicast && !joinMulticastGroup(config_.multicastGroup)) {
        return false;
    }
    
    setDeviceState(DeviceState::Connected);
    DS_LOG_INFO("UDP socket bound to {}:{}",
                config_.localAddress.toStdString(), config_.localPort);
    return true;
}

void UdpSocket::disconnect()
{
    if (config_.multicast) {
        leaveMulticastGroup(config_.multicastGroup);
    }
    
    if (socket_->state() != QAbstractSocket::UnconnectedState) {
        socket_->abort();
    }
    
    setDeviceState(DeviceState::Disconnected);
    DS_LOG_INFO("UDP socket closed");
}

qint64 UdpSocket::send(const QByteArray& data)
{
    return sendTo(config_.remoteAddress, config_.remotePort, data);
}

QByteArray UdpSocket::receive()
{
    QByteArray data = receiveBuffer_;
    receiveBuffer_.clear();
    return data;
}

void UdpSocket::setConfig(const UdpConfig& config)
{
    config_ = config;
    if (isConnected()) {
        disconnect();
        connect(config_.toVariantMap());
    }
    emit configChanged(config_);
}

qint64 UdpSocket::sendTo(const QString& address, quint16 port, const QByteArray& data)
{
    QHostAddress targetAddress(address);
    qint64 bytesSent = socket_->writeDatagram(data, targetAddress, port);
    
    if (bytesSent > 0) {
        emit dataSent(data);
        DS_LOG_DEBUG("UDP sent {} bytes to {}:{}", bytesSent, address.toStdString(), port);
    } else {
        DS_LOG_ERROR("Failed to send UDP data: {}", socket_->errorString().toStdString());
    }
    
    return bytesSent;
}

bool UdpSocket::joinMulticastGroup(const QString& groupAddress)
{
    QHostAddress group(groupAddress);
    if (!socket_->joinMulticastGroup(group)) {
        QString error = QString("Failed to join multicast group %1: %2")
            .arg(groupAddress).arg(socket_->errorString());
        DS_LOG_ERROR("{}", error.toStdString());
        emit errorOccurred(error);
        return false;
    }
    
    DS_LOG_INFO("Joined multicast group: {}", groupAddress.toStdString());
    return true;
}

bool UdpSocket::leaveMulticastGroup(const QString& groupAddress)
{
    QHostAddress group(groupAddress);
    bool result = socket_->leaveMulticastGroup(group);
    if (result) {
        DS_LOG_INFO("Left multicast group: {}", groupAddress.toStdString());
    }
    return result;
}

QString UdpSocket::localAddress() const
{
    return socket_->localAddress().toString();
}

void UdpSocket::clearReceiveBuffer()
{
    receiveBuffer_.clear();
}

int UdpSocket::receiveBufferSize() const
{
    return receiveBuffer_.size();
}

void UdpSocket::onReadyRead()
{
    while (socket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket_->receiveDatagram();
        QByteArray data = datagram.data();
        
        receiveBuffer_.append(data);
        
        QString senderAddress = datagram.senderAddress().toString();
        quint16 senderPort = datagram.senderPort();
        
        emit datagramReceived(senderAddress, senderPort, data);
        emit dataReceived(data);
        
        DS_LOG_TRACE("UDP received {} bytes from {}:{}", 
                     data.size(), senderAddress.toStdString(), senderPort);
    }
}

void UdpSocket::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorString = socket_->errorString();
    DS_LOG_ERROR("UDP socket error: {}", errorString.toStdString());
    emit errorOccurred(errorString);
}

} // namespace DeviceStudio
