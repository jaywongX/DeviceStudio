/**
 * @file customprotocolplugin.cpp
 * @brief 自定义协议插件实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "customprotocolplugin.h"
#include "log/logger.h"
#include <QByteArray>
#include <QDebug>

namespace DeviceStudio {

CustomProtocolPlugin::CustomProtocolPlugin(QObject* parent)
    : QObject(parent)
{
    initProtocolDefinition();
}

CustomProtocolPlugin::~CustomProtocolPlugin()
{
    shutdown();
}

bool CustomProtocolPlugin::initialize()
{
    DS_LOG_INFO("CustomProtocolPlugin initializing...");
    initProtocolDefinition();
    DS_LOG_INFO("CustomProtocolPlugin initialized successfully");
    return true;
}

void CustomProtocolPlugin::shutdown()
{
    DS_LOG_INFO("CustomProtocolPlugin shutdown");
}

void CustomProtocolPlugin::initProtocolDefinition()
{
    m_definition.name = "CustomProtocol";
    m_definition.version = "1.0";
    m_definition.description = "自定义协议示例";
    
    // 帧配置
    m_definition.frameConfig.header = {FRAME_HEADER};
    m_definition.frameConfig.tail = {FRAME_TAIL};
    m_definition.frameConfig.lengthFieldOffset = 1;
    m_definition.frameConfig.lengthFieldSize = 1;
    m_definition.frameConfig.lengthIncludesHeader = false;
    m_definition.frameConfig.checksumType = ChecksumType::XOR;
    m_definition.frameConfig.checksumOffset = -2;  // 倒数第二个字节
    
    // 定义字段
    ProtocolField cmdField;
    cmdField.name = "command";
    cmdField.type = ProtocolFieldType::UInt8;
    cmdField.offset = 2;
    cmdField.description = "命令码";
    cmdField.valueMapping = {
        {0x01, "Read"},
        {0x02, "Write"},
        {0x03, "Response"},
        {0x04, "Error"}
    };
    m_definition.fields.append(cmdField);
    
    ProtocolField dataField;
    dataField.name = "data";
    dataField.type = ProtocolFieldType::Bytes;
    dataField.offset = 3;
    dataField.lengthField = "length";
    dataField.lengthAdjust = -4;  // 减去header, len, cmd, checksum, tail
    dataField.description = "数据内容";
    m_definition.fields.append(dataField);
}

bool CustomProtocolPlugin::canParse(const QByteArray& data) const
{
    if (data.size() < MIN_FRAME_SIZE) {
        return false;
    }
    
    // 检查帧头
    if (static_cast<quint8>(data[0]) != FRAME_HEADER) {
        return false;
    }
    
    // 检查帧尾
    if (static_cast<quint8>(data[data.size() - 1]) != FRAME_TAIL) {
        return false;
    }
    
    // 检查长度字段
    quint8 length = static_cast<quint8>(data[1]);
    if (length != data.size() - 2) {  // 不包括header和tail
        return false;
    }
    
    // 校验和验证
    quint8 expectedChecksum = calculateChecksum(data.mid(0, data.size() - 2));
    quint8 actualChecksum = static_cast<quint8>(data[data.size() - 2]);
    
    return expectedChecksum == actualChecksum;
}

QList<ProtocolField> CustomProtocolPlugin::parse(const QByteArray& data)
{
    QList<ProtocolField> result;
    
    if (!canParse(data)) {
        m_lastError = "Invalid frame format";
        return result;
    }
    
    // 解析命令码
    ProtocolField cmdField = m_definition.fields[0];
    cmdField.rawValue = static_cast<quint8>(data[2]);
    
    // 命令码映射
    QString cmdName = QString("Unknown(0x%1)").arg(cmdField.rawValue.toInt(), 2, 16, QChar('0'));
    for (const auto& mapping : m_definition.fields[0].valueMapping.toStdMap()) {
        if (mapping.first == cmdField.rawValue) {
            cmdName = mapping.second;
            break;
        }
    }
    cmdField.stringValue = cmdName;
    result.append(cmdField);
    
    // 解析数据
    ProtocolField dataField = m_definition.fields[1];
    int dataLength = data.size() - 6;  // 总长度减去header(1)+len(1)+cmd(1)+checksum(1)+tail(1)
    if (dataLength > 0) {
        dataField.rawValue = data.mid(3, dataLength);
        dataField.stringValue = dataField.rawValue.toByteArray().toHex(' ').toUpper();
    } else {
        dataField.rawValue = QByteArray();
        dataField.stringValue = "";
    }
    result.append(dataField);
    
    m_lastError.clear();
    return result;
}

QByteArray CustomProtocolPlugin::build(const QVariantMap& fields)
{
    QByteArray result;
    
    // 帧头
    result.append(FRAME_HEADER);
    
    // 长度字段占位
    int lengthPos = result.size();
    result.append(static_cast<char>(0));
    
    // 命令码
    int cmd = fields.value("command", 0x01).toInt();
    result.append(static_cast<char>(cmd));
    
    // 数据
    QByteArray data = fields.value("data").toByteArray();
    result.append(data);
    
    // 计算并填充长度
    int length = result.size() + 1;  // 加上checksum
    result[lengthPos] = static_cast<char>(length);
    
    // 校验和
    quint8 checksum = calculateChecksum(result);
    result.append(static_cast<char>(checksum));
    
    // 帧尾
    result.append(FRAME_TAIL);
    
    return result;
}

ProtocolDefinition CustomProtocolPlugin::definition() const
{
    return m_definition;
}

quint8 CustomProtocolPlugin::calculateChecksum(const QByteArray& data) const
{
    quint8 checksum = 0;
    for (char byte : data) {
        checksum ^= static_cast<quint8>(byte);
    }
    return checksum;
}

} // namespace DeviceStudio
