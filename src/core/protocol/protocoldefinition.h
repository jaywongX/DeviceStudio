/**
 * @file protocoldefinition.h
 * @brief 协议定义数据结构
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QList>
#include <QMap>
#include <optional>

namespace DeviceStudio {

/**
 * @brief 数据类型枚举
 */
enum class DataType {
    UInt8,      ///< 无符号8位
    Int8,       ///< 有符号8位
    UInt16,     ///< 无符号16位
    Int16,      ///< 有符号16位
    UInt32,     ///< 无符号32位
    Int32,      ///< 有符号32位
    UInt64,     ///< 无符号64位
    Int64,      ///< 有符号64位
    Float,      ///< 浮点数(32位)
    Double,     ///< 双精度浮点(64位)
    String,     ///< 字符串
    Bytes,      ///< 字节数组
    Bool        ///< 布尔值
};

/**
 * @brief 字节序枚举
 */
enum class ByteOrder {
    LittleEndian,   ///< 小端序
    BigEndian       ///< 大端序
};

/**
 * @brief 校验和类型枚举
 */
enum class ChecksumType {
    None,       ///< 无校验
    Xor,        ///< 异或校验
    Sum,        ///< 求和校验(取低字节)
    Sum16,      ///< 求和校验(取低字)
    Crc8,       ///< CRC8校验
    Crc16,      ///< CRC16校验
    Crc16Modbus,///< CRC16 Modbus
    Crc32       ///< CRC32校验
};

/**
 * @brief 校验和范围类型
 */
enum class ChecksumRange {
    All,        ///< 全部数据
    Header,     ///< 从帧头开始
    Data        ///< 仅数据部分
};

/**
 * @brief 协议字段定义
 */
struct ProtocolField {
    QString name;                       ///< 字段名称
    QString displayName;                ///< 显示名称
    int offset = 0;                     ///< 字节偏移
    int length = 1;                     ///< 字节长度
    DataType type = DataType::UInt8;    ///< 数据类型
    ByteOrder byteOrder = ByteOrder::BigEndian; ///< 字节序
    
    double scale = 1.0;                 ///< 缩放因子
    QString unit;                       ///< 单位
    QVariant fixedValue;                ///< 固定值(用于校验)
    QMap<int, QString> valueMap;        ///< 值映射表
    
    int bitOffset = 0;                  ///< 位偏移(用于位域)
    int bitLength = 0;                  ///< 位长度(0表示不使用位域)
    
    /**
     * @brief 是否有固定值
     */
    bool hasFixedValue() const {
        return fixedValue.isValid() && !fixedValue.isNull();
    }
    
    /**
     * @brief 是否是位域字段
     */
    bool isBitField() const {
        return bitLength > 0;
    }
};

/**
 * @brief 校验和配置
 */
struct ChecksumConfig {
    bool enabled = false;               ///< 是否启用校验
    ChecksumType type = ChecksumType::None; ///< 校验类型
    ChecksumRange range = ChecksumRange::All; ///< 校验范围
    int startOffset = 0;                ///< 起始偏移(相对于范围)
    int endOffset = -1;                 ///< 结束偏移(-1表示到校验和前一字节)
    int checksumOffset = -1;            ///< 校验和位置偏移
    int checksumLength = 1;             ///< 校验和长度(字节)
};

/**
 * @brief 帧识别配置
 */
struct FrameConfig {
    QByteArray header;                  ///< 帧头
    QByteArray tail;                    ///< 帧尾
    int minLength = 0;                  ///< 最小帧长
    int maxLength = 4096;               ///< 最大帧长
    int lengthFieldOffset = -1;         ///< 长度字段偏移
    int lengthFieldSize = 0;            ///< 长度字段大小
    int lengthAdjust = 0;               ///< 长度调整值
};

/**
 * @brief 协议定义
 */
struct ProtocolDefinition {
    QString name;                       ///< 协议名称
    QString version;                    ///< 协议版本
    QString description;                ///< 协议描述
    
    FrameConfig frame;                  ///< 帧配置
    ChecksumConfig checksum;            ///< 校验配置
    
    QList<ProtocolField> fields;        ///< 字段列表
    
    QString id;                         ///< 协议ID(自动生成)
    
    /**
     * @brief 获取帧长度
     */
    int frameLength() const {
        int len = frame.header.size() + frame.tail.size();
        for (const auto& field : fields) {
            len += field.length;
        }
        return len;
    }
    
    /**
     * @brief 验证协议定义是否有效
     */
    bool isValid() const {
        if (name.isEmpty()) return false;
        if (fields.isEmpty()) return false;
        return true;
    }
};

/**
 * @brief 协议解析结果
 */
struct ProtocolParseResult {
    bool success = false;               ///< 是否成功
    QString errorMessage;               ///< 错误信息
    QByteArray rawData;                 ///< 原始数据
    QString protocolName;               ///< 协议名称
    
    QMap<QString, QVariant> values;     ///< 解析后的值
    QMap<QString, QString> displayValues; ///< 显示值(应用缩放和单位)
    
    int frameStart = -1;                ///< 帧起始位置
    int frameEnd = -1;                  ///< 帧结束位置
    bool checksumValid = false;         ///< 校验和是否正确
    
    /**
     * @brief 获取字段值
     */
    QVariant getValue(const QString& fieldName) const {
        return values.value(fieldName);
    }
    
    /**
     * @brief 获取显示值
     */
    QString getDisplayValue(const QString& fieldName) const {
        return displayValues.value(fieldName);
    }
};

/**
 * @brief 数据打包配置
 */
struct PackConfig {
    bool calculateChecksum = true;      ///< 是否自动计算校验和
    bool appendHeader = true;           ///< 是否添加帧头
    bool appendTail = true;             ///< 是否添加帧尾
};

// 元类型注册
Q_DECLARE_METATYPE(DataType)
Q_DECLARE_METATYPE(ByteOrder)
Q_DECLARE_METATYPE(ChecksumType)
Q_DECLARE_METATYPE(ChecksumRange)
Q_DECLARE_METATYPE(ProtocolField)
Q_DECLARE_METATYPE(ChecksumConfig)
Q_DECLARE_METATYPE(FrameConfig)
Q_DECLARE_METATYPE(ProtocolDefinition)
Q_DECLARE_METATYPE(ProtocolParseResult)

} // namespace DeviceStudio
