/**
 * @file customprotocolplugin.h
 * @brief 自定义协议插件示例
 * @author DeviceStudio Team
 * @date 2026-03-14
 * 
 * 这是一个示例插件，展示如何创建自定义协议插件
 */

#pragma once

#include "../iplugin.h"
#include "../../core/protocol/protocoldefinition.h"
#include <QObject>
#include <QJsonObject>

namespace DeviceStudio {

/**
 * @brief 自定义协议插件实现
 * 
 * 实现了一个简单的自定义协议示例，用于演示插件机制
 */
class CustomProtocolPlugin : public QObject, public IProtocolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.devicestudio.IProtocolPlugin" FILE "customprotocol.json")
    Q_INTERFACES(DeviceStudio::IProtocolPlugin)

public:
    explicit CustomProtocolPlugin(QObject* parent = nullptr);
    ~CustomProtocolPlugin() override;

    // IPlugin 接口
    QString name() const override { return QStringLiteral("CustomProtocol"); }
    QString version() const override { return QStringLiteral("1.0.0"); }
    QString description() const override { return tr("自定义协议插件示例"); }
    QString author() const override { return QStringLiteral("DeviceStudio Team"); }
    
    bool initialize() override;
    void shutdown() override;
    
    QList<PluginDependency> dependencies() const override { return {}; }
    bool hasError() const override { return !m_lastError.isEmpty(); }
    QString lastError() const override { return m_lastError; }

    // IProtocolPlugin 接口
    QString protocolName() const override { return QStringLiteral("CustomProtocol"); }
    QString protocolVersion() const override { return QStringLiteral("1.0"); }
    
    bool canParse(const QByteArray& data) const override;
    QList<ProtocolField> parse(const QByteArray& data) override;
    QByteArray build(const QVariantMap& fields) override;
    ProtocolDefinition definition() const override;

private:
    /**
     * @brief 初始化协议定义
     */
    void initProtocolDefinition();
    
    /**
     * @brief 计算校验和
     */
    quint8 calculateChecksum(const QByteArray& data) const;

    ProtocolDefinition m_definition;
    QString m_lastError;
    
    // 协议常量
    static constexpr quint8 FRAME_HEADER = 0xAA;
    static constexpr quint8 FRAME_TAIL = 0x55;
    static constexpr int MIN_FRAME_SIZE = 6; // Header + Len + Cmd + Data(0) + Checksum + Tail
};

} // namespace DeviceStudio
