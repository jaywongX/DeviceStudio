/**
 * @file protocolparser.cpp
 * @brief 协议解析器实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "protocolparser.h"
#include "checksum.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QtEndian>
#include <cmath>

namespace DeviceStudio {

ProtocolParser::ProtocolParser(QObject* parent)
    : QObject(parent)
{
}

bool ProtocolParser::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = tr("无法打开文件: %1").arg(filePath);
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    return loadFromJson(QString::fromUtf8(data));
}

bool ProtocolParser::loadFromJson(const QString& jsonString)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        m_lastError = tr("JSON解析错误: %1").arg(error.errorString());
        return false;
    }
    
    return loadFromJsonObject(doc.object());
}

bool ProtocolParser::loadFromJsonObject(const QJsonObject& json)
{
    return parseProtocolJson(json);
}

void ProtocolParser::setProtocol(const ProtocolDefinition& protocol)
{
    m_protocol = protocol;
    // 生成协议ID
    if (m_protocol.id.isEmpty()) {
        m_protocol.id = m_protocol.name + "_" + m_protocol.version;
    }
    emit protocolLoaded(m_protocol.name);
}

ProtocolParseResult ProtocolParser::parse(const QByteArray& data)
{
    ProtocolParseResult result;
    result.rawData = data;
    
    if (!hasProtocol()) {
        result.errorMessage = tr("未加载协议定义");
        return result;
    }
    
    if (data.isEmpty()) {
        result.errorMessage = tr("数据为空");
        return result;
    }
    
    // 查找帧起始位置
    int frameStart = findFrameStart(data);
    if (frameStart < 0) {
        result.errorMessage = tr("未找到帧头");
        return result;
    }
    
    // 查找帧结束位置
    int frameEnd = findFrameEnd(data, frameStart);
    if (frameEnd < 0) {
        result.errorMessage = tr("未找到完整帧");
        return result;
    }
    
    result.frameStart = frameStart;
    result.frameEnd = frameEnd;
    
    // 提取帧数据
    QByteArray frameData = data.mid(frameStart, frameEnd - frameStart);
    
    // 验证帧长度
    if (m_protocol.frame.minLength > 0 && frameData.size() < m_protocol.frame.minLength) {
        result.errorMessage = tr("帧长度不足: %1 < %2")
            .arg(frameData.size()).arg(m_protocol.frame.minLength);
        return result;
    }
    
    if (m_protocol.frame.maxLength > 0 && frameData.size() > m_protocol.frame.maxLength) {
        result.errorMessage = tr("帧长度超出: %1 > %2")
            .arg(frameData.size()).arg(m_protocol.frame.maxLength);
        return result;
    }
    
    // 验证校验和
    if (m_protocol.checksum.enabled) {
        result.checksumValid = validateFrame(frameData);
        if (!result.checksumValid) {
            result.errorMessage = tr("校验和验证失败");
            // 继续解析但标记校验失败
        }
    } else {
        result.checksumValid = true;
    }
    
    // 解析字段
    for (const auto& field : m_protocol.fields) {
        int offset = field.offset;
        
        // 检查偏移是否有效
        if (offset < 0 || offset + field.length > frameData.size()) {
            continue;
        }
        
        // 解析字段值
        QVariant value = parseField(frameData, field);
        result.values[field.name] = value;
        
        // 格式化显示值
        result.displayValues[field.name] = formatDisplayValue(value, field);
    }
    
    result.success = true;
    result.protocolName = m_protocol.name;
    
    return result;
}

QList<ProtocolParseResult> ProtocolParser::parseFromBuffer(QByteArray& buffer)
{
    QList<ProtocolParseResult> results;
    
    while (!buffer.isEmpty()) {
        int frameStart = findFrameStart(buffer);
        if (frameStart < 0) {
            // 未找到帧头,清空缓冲区
            buffer.clear();
            break;
        }
        
        // 移除帧头之前的数据
        if (frameStart > 0) {
            buffer.remove(0, frameStart);
        }
        
        int frameEnd = findFrameEnd(buffer, 0);
        if (frameEnd < 0) {
            // 没有完整的帧,等待更多数据
            break;
        }
        
        // 提取并解析帧
        QByteArray frameData = buffer.left(frameEnd);
        buffer.remove(0, frameEnd);

        ProtocolParseResult result = parse(frameData);
        results.append(result);
    }
    
    return results;
}

int ProtocolParser::findFrameStart(const QByteArray& data, int startPos) const
{
    if (!hasProtocol()) {
        return -1;
    }
    
    if (m_protocol.frame.header.isEmpty()) {
        return startPos;
    }
    
    return data.indexOf(m_protocol.frame.header, startPos);
}

int ProtocolParser::findFrameEnd(const QByteArray& data, int frameStart) const
{
    if (!hasProtocol()) {
        return -1;
    }
    
    // 如果有帧尾,查找帧尾
    if (!m_protocol.frame.tail.isEmpty()) {
        int tailPos = data.indexOf(m_protocol.frame.tail, frameStart + m_protocol.frame.header.size());
        if (tailPos >= 0) {
            return tailPos + m_protocol.frame.tail.size();
        }
        return -1;
    }
    
    // 如果有长度字段,根据长度字段确定帧结束
    if (m_protocol.frame.lengthFieldOffset >= 0) {
        int lengthOffset = frameStart + m_protocol.frame.lengthFieldOffset;
        if (lengthOffset + m_protocol.frame.lengthFieldSize <= data.size()) {
            int length = 0;
            if (m_protocol.frame.lengthFieldSize == 1) {
                length = static_cast<uint8_t>(data[lengthOffset]);
            } else if (m_protocol.frame.lengthFieldSize == 2) {
                length = qFromBigEndian<uint16_t>(data.constData() + lengthOffset);
            }
            int frameEnd = frameStart + length + m_protocol.frame.lengthAdjust;
            if (frameEnd <= data.size()) {
                return frameEnd;
            }
        }
        return -1;
    }
    
    // 固定长度帧
    int frameLen = m_protocol.frameLength();
    if (frameStart + frameLen <= data.size()) {
        return frameStart + frameLen;
    }
    
    return -1;
}

QByteArray ProtocolParser::extractFrame(const QByteArray& data, int startPos) const
{
    int frameStart = findFrameStart(data, startPos);
    if (frameStart < 0) {
        return QByteArray();
    }
    
    int frameEnd = findFrameEnd(data, frameStart);
    if (frameEnd < 0) {
        return QByteArray();
    }
    
    return data.mid(frameStart, frameEnd - frameStart);
}

bool ProtocolParser::validateFrame(const QByteArray& frameData) const
{
    if (!m_protocol.checksum.enabled) {
        return true;
    }
    
    int checksumOffset = m_protocol.checksum.checksumOffset;
    int checksumLength = m_protocol.checksum.checksumLength;
    
    if (checksumOffset < 0) {
        // 校验和在帧尾之前
        if (!m_protocol.frame.tail.isEmpty()) {
            checksumOffset = frameData.size() - m_protocol.frame.tail.size() - checksumLength;
        } else {
            checksumOffset = frameData.size() - checksumLength;
        }
    }
    
    // 获取校验数据范围
    int startOffset = m_protocol.checksum.startOffset;
    int endOffset = m_protocol.checksum.endOffset;
    
    if (endOffset < 0) {
        endOffset = checksumOffset;
    }
    
    QByteArray checksumData = frameData.mid(startOffset, endOffset - startOffset);
    
    // 计算校验和
    QByteArray calculated = ChecksumCalculator::calculate(checksumData, m_protocol.checksum.type);
    QByteArray original = frameData.mid(checksumOffset, checksumLength);
    
    return calculated == original;
}

QByteArray ProtocolParser::pack(const QMap<QString, QVariant>& values, const PackConfig& config)
{
    if (!hasProtocol()) {
        return QByteArray();
    }
    
    QByteArray result;
    
    // 添加帧头
    if (config.appendHeader && !m_protocol.frame.header.isEmpty()) {
        result.append(m_protocol.frame.header);
    }
    
    // 按字段顺序打包
    for (const auto& field : m_protocol.fields) {
        QVariant value = values.value(field.name);
        
        // 如果没有提供值,使用固定值或默认值
        if (!value.isValid() && field.hasFixedValue()) {
            value = field.fixedValue;
        }
        
        QByteArray fieldData = packField(value, field);
        result.append(fieldData);
    }
    
    // 添加校验和
    if (config.calculateChecksum && m_protocol.checksum.enabled) {
        QByteArray checksumData;
        
        // 计算校验数据范围
        int startOffset = m_protocol.checksum.startOffset;
        int endOffset = m_protocol.checksum.endOffset;
        
        // 根据范围获取数据
        if (m_protocol.checksum.range == ChecksumRange::All) {
            checksumData = result;
        } else if (m_protocol.checksum.range == ChecksumRange::Header && config.appendHeader) {
            checksumData = result;
        } else {
            // 仅数据部分
            int headerSize = config.appendHeader ? m_protocol.frame.header.size() : 0;
            checksumData = result.mid(headerSize);
        }
        
        QByteArray checksum = ChecksumCalculator::calculate(checksumData, m_protocol.checksum.type);
        result.append(checksum);
    }
    
    // 添加帧尾
    if (config.appendTail && !m_protocol.frame.tail.isEmpty()) {
        result.append(m_protocol.frame.tail);
    }
    
    return result;
}

QByteArray ProtocolParser::packDefault()
{
    QMap<QString, QVariant> values;
    return pack(values);
}

void ProtocolParser::clear()
{
    m_protocol = ProtocolDefinition();
    m_lastError.clear();
}

QVariant ProtocolParser::parseField(const QByteArray& data, const ProtocolField& field)
{
    if (field.isBitField()) {
        // 位域处理
        uint32_t value = 0;
        for (int i = 0; i < field.length && i < 4; ++i) {
            value = (value << 8) | static_cast<uint8_t>(data[field.offset + i]);
        }
        return parseBitField(value, field);
    }
    
    switch (field.type) {
    case DataType::UInt8:
    case DataType::Int8:
    case DataType::UInt16:
    case DataType::Int16:
    case DataType::UInt32:
    case DataType::Int32:
    case DataType::UInt64:
    case DataType::Int64:
    case DataType::Float:
    case DataType::Double:
        return parseNumeric(data, field);
        
    case DataType::String:
        return QString::fromUtf8(data.mid(field.offset, field.length));
        
    case DataType::Bytes:
        return data.mid(field.offset, field.length);
        
    case DataType::Bool:
        return data[field.offset] != 0;
        
    default:
        return QVariant();
    }
}

QVariant ProtocolParser::parseNumeric(const QByteArray& data, const ProtocolField& field)
{
    const char* ptr = data.constData() + field.offset;
    double value = 0;
    
    switch (field.type) {
    case DataType::UInt8:
        value = static_cast<uint8_t>(ptr[0]);
        break;
    case DataType::Int8:
        value = static_cast<int8_t>(ptr[0]);
        break;
    case DataType::UInt16:
        if (field.byteOrder == ByteOrder::BigEndian) {
            value = qFromBigEndian<uint16_t>(ptr);
        } else {
            value = qFromLittleEndian<uint16_t>(ptr);
        }
        break;
    case DataType::Int16:
        if (field.byteOrder == ByteOrder::BigEndian) {
            value = static_cast<int16_t>(qFromBigEndian<uint16_t>(ptr));
        } else {
            value = static_cast<int16_t>(qFromLittleEndian<uint16_t>(ptr));
        }
        break;
    case DataType::UInt32:
        if (field.byteOrder == ByteOrder::BigEndian) {
            value = qFromBigEndian<uint32_t>(ptr);
        } else {
            value = qFromLittleEndian<uint32_t>(ptr);
        }
        break;
    case DataType::Int32:
        if (field.byteOrder == ByteOrder::BigEndian) {
            value = static_cast<int32_t>(qFromBigEndian<uint32_t>(ptr));
        } else {
            value = static_cast<int32_t>(qFromLittleEndian<uint32_t>(ptr));
        }
        break;
    case DataType::UInt64:
        if (field.byteOrder == ByteOrder::BigEndian) {
            value = static_cast<double>(qFromBigEndian<uint64_t>(ptr));
        } else {
            value = static_cast<double>(qFromLittleEndian<uint64_t>(ptr));
        }
        break;
    case DataType::Int64:
        if (field.byteOrder == ByteOrder::BigEndian) {
            value = static_cast<double>(static_cast<int64_t>(qFromBigEndian<uint64_t>(ptr)));
        } else {
            value = static_cast<double>(static_cast<int64_t>(qFromLittleEndian<uint64_t>(ptr)));
        }
        break;
    case DataType::Float:
        if (field.byteOrder == ByteOrder::BigEndian) {
            uint32_t tmp = qFromBigEndian<uint32_t>(ptr);
            memcpy(&value, &tmp, 4);
        } else {
            uint32_t tmp = qFromLittleEndian<uint32_t>(ptr);
            memcpy(&value, &tmp, 4);
        }
        break;
    case DataType::Double:
        if (field.byteOrder == ByteOrder::BigEndian) {
            uint64_t tmp = qFromBigEndian<uint64_t>(ptr);
            memcpy(&value, &tmp, 8);
        } else {
            uint64_t tmp = qFromLittleEndian<uint64_t>(ptr);
            memcpy(&value, &tmp, 8);
        }
        break;
    default:
        break;
    }
    
    // 应用缩放因子
    if (field.scale != 1.0 && field.scale != 0.0) {
        value *= field.scale;
    }
    
    return value;
}

QVariant ProtocolParser::parseBitField(uint32_t value, const ProtocolField& field)
{
    // 提取位域
    uint32_t mask = (1 << field.bitLength) - 1;
    uint32_t result = (value >> field.bitOffset) & mask;
    
    // 根据数据类型返回
    if (field.type == DataType::Bool) {
        return result != 0;
    }
    
    return QVariant(result);
}

QString ProtocolParser::formatDisplayValue(const QVariant& value, const ProtocolField& field)
{
    QString display;
    
    // 检查值映射
    if (value.canConvert<int>() && !field.valueMap.isEmpty()) {
        int intVal = value.toInt();
        if (field.valueMap.contains(intVal)) {
            return field.valueMap[intVal];
        }
    }
    
    // 格式化数值
    if (value.typeId() == QMetaType::Double) {
        display = QString::number(value.toDouble(), 'f', 2);
    } else {
        display = value.toString();
    }
    
    // 添加单位
    if (!field.unit.isEmpty()) {
        display += field.unit;
    }
    
    return display;
}

QByteArray ProtocolParser::getChecksumData(const QByteArray& frameData) const
{
    int startOffset = m_protocol.checksum.startOffset;
    int endOffset = m_protocol.checksum.endOffset;
    
    if (endOffset < 0) {
        // 到校验和前一字节
        int checksumOffset = m_protocol.checksum.checksumOffset;
        if (checksumOffset < 0) {
            if (!m_protocol.frame.tail.isEmpty()) {
                checksumOffset = frameData.size() - m_protocol.frame.tail.size() - m_protocol.checksum.checksumLength;
            } else {
                checksumOffset = frameData.size() - m_protocol.checksum.checksumLength;
            }
        }
        endOffset = checksumOffset;
    }
    
    return frameData.mid(startOffset, endOffset - startOffset);
}

QByteArray ProtocolParser::packField(const QVariant& value, const ProtocolField& field)
{
    QByteArray result(field.length, 0);
    
    if (!value.isValid()) {
        return result;
    }
    
    switch (field.type) {
    case DataType::UInt8: {
        uint8_t v = value.toUInt();
        result[0] = static_cast<char>(v);
        break;
    }
    case DataType::Int8: {
        int8_t v = value.toInt();
        result[0] = static_cast<char>(v);
        break;
    }
    case DataType::UInt16: {
        uint16_t v = value.toUInt();
        if (field.byteOrder == ByteOrder::BigEndian) {
            qToBigEndian(v, result.data());
        } else {
            qToLittleEndian(v, result.data());
        }
        break;
    }
    case DataType::Int16: {
        int16_t v = value.toInt();
        if (field.byteOrder == ByteOrder::BigEndian) {
            qToBigEndian(static_cast<uint16_t>(v), result.data());
        } else {
            qToLittleEndian(static_cast<uint16_t>(v), result.data());
        }
        break;
    }
    case DataType::UInt32: {
        uint32_t v = value.toUInt();
        if (field.byteOrder == ByteOrder::BigEndian) {
            qToBigEndian(v, result.data());
        } else {
            qToLittleEndian(v, result.data());
        }
        break;
    }
    case DataType::Int32: {
        int32_t v = value.toInt();
        if (field.byteOrder == ByteOrder::BigEndian) {
            qToBigEndian(static_cast<uint32_t>(v), result.data());
        } else {
            qToLittleEndian(static_cast<uint32_t>(v), result.data());
        }
        break;
    }
    case DataType::Float: {
        float v = value.toFloat();
        uint32_t tmp;
        memcpy(&tmp, &v, 4);
        if (field.byteOrder == ByteOrder::BigEndian) {
            qToBigEndian(tmp, result.data());
        } else {
            qToLittleEndian(tmp, result.data());
        }
        break;
    }
    case DataType::Double: {
        double v = value.toDouble();
        uint64_t tmp;
        memcpy(&tmp, &v, 8);
        if (field.byteOrder == ByteOrder::BigEndian) {
            qToBigEndian(tmp, result.data());
        } else {
            qToLittleEndian(tmp, result.data());
        }
        break;
    }
    case DataType::String: {
        QByteArray strData = value.toString().toUtf8();
        int len = qMin(strData.size(), field.length);
        result = result.leftJustified(len, '\0');
        for (int i = 0; i < len; ++i) {
            result[i] = strData[i];
        }
        break;
    }
    case DataType::Bytes: {
        QByteArray bytes = value.toByteArray();
        int len = qMin(bytes.size(), field.length);
        result = result.leftJustified(len, '\0');
        for (int i = 0; i < len; ++i) {
            result[i] = bytes[i];
        }
        break;
    }
    case DataType::Bool: {
        result[0] = value.toBool() ? 1 : 0;
        break;
    }
    default:
        break;
    }
    
    return result;
}

bool ProtocolParser::parseProtocolJson(const QJsonObject& json)
{
    m_protocol = ProtocolDefinition();
    
    // 基本信息
    m_protocol.name = json["protocol_name"].toString();
    m_protocol.version = json["protocol_version"].toString("1.0");
    m_protocol.description = json["description"].toString();
    
    if (m_protocol.name.isEmpty()) {
        m_lastError = tr("协议名称不能为空");
        return false;
    }
    
    // 生成协议ID
    m_protocol.id = m_protocol.name + "_" + m_protocol.version;
    
    // 帧配置
    if (json.contains("frame_header")) {
        QString headerStr = json["frame_header"].toString();
        m_protocol.frame.header = QByteArray::fromHex(headerStr.toLatin1());
    }
    if (json.contains("frame_tail")) {
        QString tailStr = json["frame_tail"].toString();
        m_protocol.frame.tail = QByteArray::fromHex(tailStr.toLatin1());
    }
    m_protocol.frame.minLength = json["min_length"].toInt(0);
    m_protocol.frame.maxLength = json["max_length"].toInt(4096);
    
    // 长度字段
    if (json.contains("length_field")) {
        QJsonObject lengthField = json["length_field"].toObject();
        m_protocol.frame.lengthFieldOffset = lengthField["offset"].toInt(-1);
        m_protocol.frame.lengthFieldSize = lengthField["size"].toInt(0);
        m_protocol.frame.lengthAdjust = lengthField["adjust"].toInt(0);
    }
    
    // 校验配置
    if (json.contains("checksum")) {
        m_protocol.checksum = parseChecksumJson(json["checksum"].toObject());
    }
    
    // 字段列表
    if (json.contains("fields")) {
        QJsonArray fieldsArray = json["fields"].toArray();
        for (const auto& fieldVal : fieldsArray) {
            ProtocolField field = parseFieldJson(fieldVal.toObject());
            m_protocol.fields.append(field);
        }
    }
    
    if (m_protocol.fields.isEmpty()) {
        m_lastError = tr("协议字段不能为空");
        return false;
    }
    
    emit protocolLoaded(m_protocol.name);
    return true;
}

ProtocolField ProtocolParser::parseFieldJson(const QJsonObject& json)
{
    ProtocolField field;
    
    field.name = json["name"].toString();
    field.displayName = json["display_name"].toString(field.name);
    field.offset = json["offset"].toInt(0);
    field.length = json["length"].toInt(1);
    field.type = dataTypeFromString(json["type"].toString("uint8"));
    field.byteOrder = byteOrderFromString(json["byte_order"].toString("big"));
    field.scale = json["scale"].toDouble(1.0);
    field.unit = json["unit"].toString();
    field.bitOffset = json["bit_offset"].toInt(0);
    field.bitLength = json["bit_length"].toInt(0);
    
    // 固定值
    if (json.contains("fixed_value")) {
        QString fixedStr = json["fixed_value"].toString();
        field.fixedValue = QByteArray::fromHex(fixedStr.toLatin1());
    }
    
    // 值映射
    if (json.contains("value_map")) {
        QJsonObject valueMap = json["value_map"].toObject();
        for (auto it = valueMap.begin(); it != valueMap.end(); ++it) {
            field.valueMap[it.key().toInt()] = it.value().toString();
        }
    }
    
    return field;
}

ChecksumConfig ProtocolParser::parseChecksumJson(const QJsonObject& json)
{
    ChecksumConfig config;
    
    config.enabled = json["enabled"].toBool(false);
    config.type = checksumTypeFromString(json["type"].toString("xor"));
    config.startOffset = json["start_offset"].toInt(0);
    config.endOffset = json["end_offset"].toInt(-1);
    config.checksumOffset = json["checksum_offset"].toInt(-1);
    config.checksumLength = json["checksum_length"].toInt(1);
    
    // 解析范围类型
    QString rangeStr = json["range"].toString("all");
    if (rangeStr == "all") {
        config.range = ChecksumRange::All;
    } else if (rangeStr == "header") {
        config.range = ChecksumRange::Header;
    } else if (rangeStr == "data") {
        config.range = ChecksumRange::Data;
    }
    
    return config;
}

FrameConfig ProtocolParser::parseFrameJson(const QJsonObject& json)
{
    FrameConfig config;
    
    if (json.contains("header")) {
        QString headerStr = json["header"].toString();
        config.header = QByteArray::fromHex(headerStr.toLatin1());
    }
    if (json.contains("tail")) {
        QString tailStr = json["tail"].toString();
        config.tail = QByteArray::fromHex(tailStr.toLatin1());
    }
    config.minLength = json["min_length"].toInt(0);
    config.maxLength = json["max_length"].toInt(4096);
    
    return config;
}

DataType ProtocolParser::dataTypeFromString(const QString& str)
{
    QString lower = str.toLower();
    
    if (lower == "uint8" || lower == "u8") return DataType::UInt8;
    if (lower == "int8" || lower == "i8") return DataType::Int8;
    if (lower == "uint16" || lower == "u16") return DataType::UInt16;
    if (lower == "int16" || lower == "i16") return DataType::Int16;
    if (lower == "uint32" || lower == "u32") return DataType::UInt32;
    if (lower == "int32" || lower == "i32") return DataType::Int32;
    if (lower == "uint64" || lower == "u64") return DataType::UInt64;
    if (lower == "int64" || lower == "i64") return DataType::Int64;
    if (lower == "float" || lower == "f32") return DataType::Float;
    if (lower == "double" || lower == "f64") return DataType::Double;
    if (lower == "string" || lower == "str") return DataType::String;
    if (lower == "bytes" || lower == "raw") return DataType::Bytes;
    if (lower == "bool" || lower == "boolean") return DataType::Bool;
    
    return DataType::UInt8;
}

ByteOrder ProtocolParser::byteOrderFromString(const QString& str)
{
    QString lower = str.toLower();
    
    if (lower == "little" || lower == "little_endian" || lower == "le") {
        return ByteOrder::LittleEndian;
    }
    return ByteOrder::BigEndian;
}

ChecksumType ProtocolParser::checksumTypeFromString(const QString& str)
{
    QString lower = str.toLower();
    
    if (lower == "xor") return ChecksumType::Xor;
    if (lower == "sum") return ChecksumType::Sum;
    if (lower == "sum16") return ChecksumType::Sum16;
    if (lower == "crc8") return ChecksumType::Crc8;
    if (lower == "crc16") return ChecksumType::Crc16;
    if (lower == "crc16_modbus" || lower == "crc16modbus") return ChecksumType::Crc16Modbus;
    if (lower == "crc32") return ChecksumType::Crc32;
    
    return ChecksumType::None;
}

} // namespace DeviceStudio
