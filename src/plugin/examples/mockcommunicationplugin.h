/**
 * @file mockcommunicationplugin.h
 * @brief Mock通信插件示例
 * @author DeviceStudio Team
 * @date 2026-03-14
 * 
 * 这是一个模拟通信插件，用于测试和演示
 */

#pragma once

#include "../iplugin.h"
#include <QObject>
#include <QTimer>
#include <QByteArray>

namespace DeviceStudio {

/**
 * @brief Mock通信插件实现
 * 
 * 模拟通信设备，可以配置自动发送测试数据
 */
class MockCommunicationPlugin : public QObject, public ICommunicationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.devicestudio.ICommunicationPlugin" FILE "mockcommunication.json")
    Q_INTERFACES(DeviceStudio::ICommunicationPlugin)

public:
    explicit MockCommunicationPlugin(QObject* parent = nullptr);
    ~MockCommunicationPlugin() override;

    // IPlugin 接口
    QString name() const override { return QStringLiteral("MockCommunication"); }
    QString version() const override { return QStringLiteral("1.0.0"); }
    QString description() const override { return tr("模拟通信插件（测试用）"); }
    QString author() const override { return QStringLiteral("DeviceStudio Team"); }
    
    bool initialize() override;
    void shutdown() override;
    
    QList<PluginDependency> dependencies() const override { return {}; }
    bool hasError() const override { return !m_lastError.isEmpty(); }
    QString lastError() const override { return m_lastError; }

    // ICommunicationPlugin 接口
    QString communicationType() const override { return QStringLiteral("Mock"); }
    QString communicationName() const override { return QStringLiteral("模拟通信"); }
    
    bool open(const QVariantMap& config) override;
    void close() override;
    bool isOpen() const override { return m_opened; }
    
    qint64 write(const QByteArray& data) override;
    QByteArray readAll() override;
    qint64 bytesAvailable() const override;
    void clear() override;
    
    QVariantMap configuration() const override;
    bool setConfiguration(const QVariantMap& config) override;
    QStringList availableDevices() const override;

signals:
    /**
     * @brief 数据接收信号
     */
    void dataReceived(const QByteArray& data);

private slots:
    /**
     * @brief 自动发送定时器
     */
    void onAutoSend();

private:
    bool m_opened = false;
    QByteArray m_receiveBuffer;
    QVariantMap m_config;
    QString m_lastError;
    QTimer* m_autoSendTimer = nullptr;
    bool m_autoSendEnabled = false;
    int m_autoSendInterval = 1000;
    QByteArray m_autoSendData;
};

} // namespace DeviceStudio
