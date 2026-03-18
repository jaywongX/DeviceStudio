/**
 * @file protocolengine.cpp
 * @brief 协议解析引擎实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "protocolengine.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace DeviceStudio {

ProtocolEngine* ProtocolEngine::s_instance = nullptr;

ProtocolEngine* ProtocolEngine::instance()
{
    if (!s_instance) {
        s_instance = new ProtocolEngine();
    }
    return s_instance;
}

ProtocolEngine::ProtocolEngine(QObject* parent)
    : QObject(parent)
{
}

ProtocolEngine::~ProtocolEngine()
{
    unloadAllProtocols();
}

bool ProtocolEngine::registerProtocol(ProtocolParserPtr parser)
{
    if (!parser || !parser->hasProtocol()) {
        emit errorOccurred(tr("无效的协议解析器"));
        return false;
    }
    
    QString protocolId = parser->protocol().id;
    if (m_protocols.contains(protocolId)) {
        // 协议已存在,替换
        m_protocols[protocolId] = parser;
    } else {
        m_protocols.insert(protocolId, parser);
    }
    
    emit protocolLoaded(protocolId, parser->protocol().name);
    return true;
}

bool ProtocolEngine::loadProtocol(const QString& filePath)
{
    auto parser = std::make_shared<ProtocolParser>(this);
    
    if (!parser->loadFromFile(filePath)) {
        emit errorOccurred(tr("加载协议失败: %1 - %2").arg(filePath, parser->lastError()));
        return false;
    }
    
    return registerProtocol(parser);
}

int ProtocolEngine::loadProtocolsFromDirectory(const QString& directoryPath)
{
    QDir dir(directoryPath);
    if (!dir.exists()) {
        emit errorOccurred(tr("协议目录不存在: %1").arg(directoryPath));
        return 0;
    }
    
    int count = 0;
    QStringList filters;
    filters << "*.json" << "*.protocol";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const auto& fileInfo : files) {
        if (loadProtocol(fileInfo.absoluteFilePath())) {
            ++count;
        }
    }
    
    return count;
}

void ProtocolEngine::unloadProtocol(const QString& protocolId)
{
    if (m_protocols.remove(protocolId) > 0) {
        emit protocolUnloaded(protocolId);
    }
}

void ProtocolEngine::unloadAllProtocols()
{
    QStringList ids = m_protocols.keys();
    for (const auto& id : ids) {
        unloadProtocol(id);
    }
}

ProtocolParserPtr ProtocolEngine::getProtocol(const QString& protocolId) const
{
    // 先按ID查找
    if (m_protocols.contains(protocolId)) {
        return m_protocols[protocolId];
    }
    
    // 按名称查找
    for (auto it = m_protocols.begin(); it != m_protocols.end(); ++it) {
        if (it.value()->protocol().name == protocolId) {
            return it.value();
        }
    }
    
    return nullptr;
}

QStringList ProtocolEngine::getProtocolIds() const
{
    return m_protocols.keys();
}

bool ProtocolEngine::hasProtocol(const QString& protocolId) const
{
    return m_protocols.contains(protocolId) || 
           std::any_of(m_protocols.begin(), m_protocols.end(),
                      [&protocolId](const ProtocolParserPtr& parser) {
                          return parser->protocol().name == protocolId;
                      });
}

ProtocolParseResult ProtocolEngine::parse(const QString& protocolId, const QByteArray& data)
{
    auto parser = getProtocol(protocolId);
    if (!parser) {
        ProtocolParseResult result;
        result.errorMessage = tr("未找到协议: %1").arg(protocolId);
        emit errorOccurred(result.errorMessage);
        return result;
    }

    ProtocolParseResult result = parser->parse(data);
    emit parseCompleted(result);
    return result;
}

QList<ProtocolParseResult> ProtocolEngine::autoParse(const QByteArray& data)
{
    QList<ProtocolParseResult> results;

    // 匹配可能的协议
    QList<ProtocolParserPtr> matchedParsers = matchProtocols(data);

    // 使用匹配的协议解析
    for (auto& parser : matchedParsers) {
        ProtocolParseResult result = parser->parse(data);
        if (result.success) {
            results.append(result);
        }
    }

    return results;
}

QList<ProtocolParseResult> ProtocolEngine::parseFromBuffer(const QString& protocolId, QByteArray& buffer)
{
    auto parser = getProtocol(protocolId);
    if (!parser) {
        emit errorOccurred(tr("未找到协议: %1").arg(protocolId));
        return {};
    }

    return parser->parseFromBuffer(buffer);
}

bool ProtocolEngine::validate(const QString& protocolId, const QByteArray& data)
{
    auto parser = getProtocol(protocolId);
    if (!parser) {
        return false;
    }

    ProtocolParseResult result = parser->parse(data);
    return result.success && result.checksumValid;
}

QByteArray ProtocolEngine::pack(const QString& protocolId, const QMap<QString, QVariant>& values,
                                const PackConfig& config)
{
    auto parser = getProtocol(protocolId);
    if (!parser) {
        emit errorOccurred(tr("未找到协议: %1").arg(protocolId));
        return QByteArray();
    }
    
    return parser->pack(values, config);
}

void ProtocolEngine::setProtocolDirectory(const QString& directory)
{
    m_protocolDirectory = directory;
}

void ProtocolEngine::reloadAllProtocols()
{
    unloadAllProtocols();
    
    if (!m_protocolDirectory.isEmpty()) {
        loadProtocolsFromDirectory(m_protocolDirectory);
    }
}

QList<ProtocolParserPtr> ProtocolEngine::matchProtocols(const QByteArray& data)
{
    QList<ProtocolParserPtr> matched;
    
    for (auto& parser : m_protocols) {
        const auto& protocol = parser->protocol();
        
        // 检查帧头匹配
        if (!protocol.frame.header.isEmpty()) {
            if (data.startsWith(protocol.frame.header)) {
                matched.append(parser);
            }
        } else {
            // 没有帧头的协议也加入候选
            matched.append(parser);
        }
    }
    
    return matched;
}

} // namespace DeviceStudio
