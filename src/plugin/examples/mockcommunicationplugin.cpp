/**
 * @file mockcommunicationplugin.cpp
 * @brief Mock通信插件实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "mockcommunicationplugin.h"
#include "log/logger.h"
#include <QDateTime>
#include <QUuid>

namespace DeviceStudio {

MockCommunicationPlugin::MockCommunicationPlugin(QObject* parent)
    : QObject(parent)
    , m_autoSendTimer(new QTimer(this))
{
    connect(m_autoSendTimer, &QTimer::timeout, this, &MockCommunicationPlugin::onAutoSend);
}

MockCommunicationPlugin::~MockCommunicationPlugin()
{
    close();
}

bool MockCommunicationPlugin::initialize()
{
    DS_LOG_INFO("MockCommunicationPlugin initializing...");
    
    // 设置默认配置
    m_config["autoSend"] = false;
    m_config["autoSendInterval"] = 1000;
    m_config["autoSendData"] = "AA 01 00 01 55";
    
    DS_LOG_INFO("MockCommunicationPlugin initialized successfully");
    return true;
}

void MockCommunicationPlugin::shutdown()
{
    close();
    DS_LOG_INFO("MockCommunicationPlugin shutdown");
}

bool MockCommunicationPlugin::open(const QVariantMap& config)
{
    if (m_opened) {
        return true;
    }
    
    // 应用配置
    setConfiguration(config);
    
    m_opened = true;
    m_receiveBuffer.clear();
    
    // 启动自动发送
    if (m_autoSendEnabled) {
        m_autoSendTimer->start(m_autoSendInterval);
        DS_LOG_INFO("MockCommunication auto-send enabled, interval: " + std::to_string(m_autoSendInterval) + "ms");
    }
    
    DS_LOG_INFO("MockCommunication opened");
    return true;
}

void MockCommunicationPlugin::close()
{
    if (!m_opened) {
        return;
    }
    
    m_autoSendTimer->stop();
    m_opened = false;
    m_receiveBuffer.clear();
    
    DS_LOG_INFO("MockCommunication closed");
}

qint64 MockCommunicationPlugin::write(const QByteArray& data)
{
    if (!m_opened) {
        m_lastError = "Device not opened";
        return -1;
    }
    
    DS_LOG_DEBUG("MockCommunication sent " + std::to_string(data.size()) + " bytes");
    
    // Echo back the data (模拟回显)
    m_receiveBuffer.append(data);
    
    // 添加时间戳前缀
    QByteArray timestampedData = "[" + QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toUtf8() + "] ";
    timestampedData.append(data);
    m_receiveBuffer.append(timestampedData);
    
    emit dataReceived(timestampedData);
    
    return data.size();
}

QByteArray MockCommunicationPlugin::readAll()
{
    QByteArray data = m_receiveBuffer;
    m_receiveBuffer.clear();
    return data;
}

qint64 MockCommunicationPlugin::bytesAvailable() const
{
    return m_receiveBuffer.size();
}

void MockCommunicationPlugin::clear()
{
    m_receiveBuffer.clear();
}

QVariantMap MockCommunicationPlugin::configuration() const
{
    return m_config;
}

bool MockCommunicationPlugin::setConfiguration(const QVariantMap& config)
{
    m_config = config;
    
    // 更新自动发送配置
    m_autoSendEnabled = config.value("autoSend", false).toBool();
    m_autoSendInterval = config.value("autoSendInterval", 1000).toInt();
    QString hexData = config.value("autoSendData", "AA 01 00 01 55").toString();
    
    // 解析十六进制数据
    m_autoSendData.clear();
    QStringList hexBytes = hexData.split(' ', Qt::SkipEmptyParts);
    for (const QString& hex : hexBytes) {
        bool ok;
        quint8 byte = hex.toUInt(&ok, 16);
        if (ok) {
            m_autoSendData.append(static_cast<char>(byte));
        }
    }
    
    // 如果已打开，更新定时器
    if (m_opened && m_autoSendEnabled) {
        m_autoSendTimer->setInterval(m_autoSendInterval);
    }
    
    return true;
}

QStringList MockCommunicationPlugin::availableDevices() const
{
    // 返回模拟设备列表
    return {
        "MockDevice://virtual1",
        "MockDevice://virtual2",
        "MockDevice://virtual3"
    };
}

void MockCommunicationPlugin::onAutoSend()
{
    if (!m_opened || m_autoSendData.isEmpty()) {
        return;
    }
    
    // 发送数据
    m_receiveBuffer.append(m_autoSendData);
    emit dataReceived(m_autoSendData);
    
    DS_LOG_DEBUG("MockCommunication auto-sent " + std::to_string(m_autoSendData.size()) + " bytes");
}

} // namespace DeviceStudio
