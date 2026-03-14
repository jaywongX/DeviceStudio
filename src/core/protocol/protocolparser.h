/**
 * @file protocolparser.h
 * @brief 协议解析器
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include "protocoldefinition.h"
#include <QObject>
#include <QJsonObject>
#include <memory>

namespace DeviceStudio {

/**
 * @brief 协议解析器类
 * 
 * 负责解析协议定义文件和解析数据帧
 */
class ProtocolParser : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolParser(QObject* parent = nullptr);
    ~ProtocolParser() override = default;
    
    // ========== 协议定义管理 ==========
    
    /**
     * @brief 从JSON文件加载协议定义
     * @param filePath 文件路径
     * @return 是否成功
     */
    bool loadFromFile(const QString& filePath);
    
    /**
     * @brief 从JSON字符串加载协议定义
     * @param jsonString JSON字符串
     * @return 是否成功
     */
    bool loadFromJson(const QString& jsonString);
    
    /**
     * @brief 从JSON对象加载协议定义
     * @param json JSON对象
     * @return 是否成功
     */
    bool loadFromJsonObject(const QJsonObject& json);
    
    /**
     * @brief 设置协议定义
     * @param protocol 协议定义
     */
    void setProtocol(const ProtocolDefinition& protocol);
    
    /**
     * @brief 获取协议定义
     */
    const ProtocolDefinition& protocol() const { return m_protocol; }
    
    /**
     * @brief 是否有协议定义
     */
    bool hasProtocol() const { return m_protocol.isValid(); }
    
    // ========== 数据解析 ==========
    
    /**
     * @brief 解析数据
     * @param data 原始数据
     * @return 解析结果
     */
    ParseResult parse(const QByteArray& data);
    
    /**
     * @brief 在缓冲区中查找并解析帧
     * @param buffer 数据缓冲区(会被修改,已解析的数据会被移除)
     * @return 解析结果列表
     */
    QList<ParseResult> parseFromBuffer(QByteArray& buffer);
    
    /**
     * @brief 查找帧边界
     * @param data 数据
     * @param startPos 开始位置
     * @return 帧起始位置,-1表示未找到
     */
    int findFrameStart(const QByteArray& data, int startPos = 0) const;
    
    /**
     * @brief 查找帧结束
     * @param data 数据
     * @param frameStart 帧起始位置
     * @return 帧结束位置(不包含),-1表示未找到
     */
    int findFrameEnd(const QByteArray& data, int frameStart) const;
    
    /**
     * @brief 提取完整帧
     * @param data 数据
     * @param startPos 开始位置
     * @return 帧数据,如果没有完整帧则返回空
     */
    QByteArray extractFrame(const QByteArray& data, int startPos = 0) const;
    
    /**
     * @brief 验证帧
     * @param frameData 帧数据
     * @return 是否有效
     */
    bool validateFrame(const QByteArray& frameData) const;
    
    // ========== 数据打包 ==========
    
    /**
     * @brief 打包数据
     * @param values 字段值映射
     * @param config 打包配置
     * @return 打包后的数据
     */
    QByteArray pack(const QMap<QString, QVariant>& values, const PackConfig& config = PackConfig());
    
    /**
     * @brief 打包数据(使用默认值)
     * @return 打包后的数据(所有字段使用默认值或固定值)
     */
    QByteArray packDefault();
    
    // ========== 工具方法 ==========
    
    /**
     * @brief 清除协议定义
     */
    void clear();
    
    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const { return m_lastError; }

signals:
    /**
     * @brief 协议加载完成信号
     */
    void protocolLoaded(const QString& protocolName);
    
    /**
     * @brief 解析错误信号
     */
    void parseError(const QString& error);

private:
    // 解析字段
    QVariant parseField(const QByteArray& data, const ProtocolField& field);
    
    // 解析数值类型
    QVariant parseNumeric(const QByteArray& data, const ProtocolField& field);
    
    // 解析位域
    QVariant parseBitField(uint32_t value, const ProtocolField& field);
    
    // 应用缩放和单位
    QString formatDisplayValue(const QVariant& value, const ProtocolField& field);
    
    // 计算校验范围
    QByteArray getChecksumData(const QByteArray& frameData) const;
    
    // 打包字段
    QByteArray packField(const QVariant& value, const ProtocolField& field);
    
    // 解析JSON定义
    bool parseProtocolJson(const QJsonObject& json);
    ProtocolField parseFieldJson(const QJsonObject& json);
    ChecksumConfig parseChecksumJson(const QJsonObject& json);
    FrameConfig parseFrameJson(const QJsonObject& json);
    
    // 数据类型转换
    static DataType dataTypeFromString(const QString& str);
    static ByteOrder byteOrderFromString(const QString& str);
    static ChecksumType checksumTypeFromString(const QString& str);
    
private:
    ProtocolDefinition m_protocol;      ///< 协议定义
    QString m_lastError;                ///< 最后的错误信息
};

using ProtocolParserPtr = std::shared_ptr<ProtocolParser>;

} // namespace DeviceStudio
