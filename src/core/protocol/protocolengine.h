/**
 * @file protocolengine.h
 * @brief 协议解析引擎
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "protocoldefinition.h"
#include "protocolparser.h"
#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <memory>

namespace DeviceStudio {

/**
 * @brief 协议解析引擎
 * 
 * 管理多个协议解析器,提供协议注册、查找和数据解析功能
 */
class ProtocolEngine : public QObject
{
    Q_OBJECT

public:
    static ProtocolEngine* instance();
    
    explicit ProtocolEngine(QObject* parent = nullptr);
    ~ProtocolEngine() override;
    
    // ========== 协议管理 ==========
    
    /**
     * @brief 注册协议
     * @param parser 协议解析器
     * @return 是否成功
     */
    bool registerProtocol(ProtocolParserPtr parser);
    
    /**
     * @brief 从文件加载协议
     * @param filePath 协议文件路径
     * @return 是否成功
     */
    bool loadProtocol(const QString& filePath);
    
    /**
     * @brief 从目录加载所有协议
     * @param directoryPath 目录路径
     * @return 加载的协议数量
     */
    int loadProtocolsFromDirectory(const QString& directoryPath);
    
    /**
     * @brief 卸载协议
     * @param protocolId 协议ID
     */
    void unloadProtocol(const QString& protocolId);
    
    /**
     * @brief 卸载所有协议
     */
    void unloadAllProtocols();
    
    /**
     * @brief 获取协议
     * @param protocolId 协议ID或协议名称
     * @return 协议解析器
     */
    ProtocolParserPtr getProtocol(const QString& protocolId) const;
    
    /**
     * @brief 获取所有协议ID列表
     */
    QStringList getProtocolIds() const;
    
    /**
     * @brief 获取协议数量
     */
    int protocolCount() const { return m_protocols.size(); }
    
    /**
     * @brief 检查协议是否存在
     */
    bool hasProtocol(const QString& protocolId) const;
    
    // ========== 数据解析 ==========

    /**
     * @brief 使用指定协议解析数据
     * @param protocolId 协议ID
     * @param data 原始数据
     * @return 解析结果
     */
    ProtocolParseResult parse(const QString& protocolId, const QByteArray& data);

    /**
     * @brief 自动识别协议并解析数据
     * @param data 原始数据
     * @return 解析结果列表(可能匹配多个协议)
     */
    QList<ProtocolParseResult> autoParse(const QByteArray& data);

    /**
     * @brief 从缓冲区解析数据
     * @param protocolId 协议ID
     * @param buffer 数据缓冲区
     * @return 解析结果列表
     */
    QList<ProtocolParseResult> parseFromBuffer(const QString& protocolId, QByteArray& buffer);
    
    /**
     * @brief 验证数据是否匹配协议
     * @param protocolId 协议ID
     * @param data 数据
     * @return 是否匹配
     */
    bool validate(const QString& protocolId, const QByteArray& data);
    
    // ========== 数据打包 ==========
    
    /**
     * @brief 使用指定协议打包数据
     * @param protocolId 协议ID
     * @param values 字段值映射
     * @param config 打包配置
     * @return 打包后的数据
     */
    QByteArray pack(const QString& protocolId, const QMap<QString, QVariant>& values,
                   const PackConfig& config = PackConfig());
    
    // ========== 配置 ==========
    
    /**
     * @brief 设置协议文件目录
     */
    void setProtocolDirectory(const QString& directory);
    
    /**
     * @brief 获取协议文件目录
     */
    QString protocolDirectory() const { return m_protocolDirectory; }
    
    /**
     * @brief 重载所有协议
     */
    void reloadAllProtocols();

signals:
    /**
     * @brief 协议加载信号
     */
    void protocolLoaded(const QString& protocolId, const QString& protocolName);
    
    /**
     * @brief 协议卸载信号
     */
    void protocolUnloaded(const QString& protocolId);
    
    /**
     * @brief 解析完成信号
     */
    void parseCompleted(const ProtocolParseResult& result);
    
    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& error);

private:
    // 匹配协议
    QList<ProtocolParserPtr> matchProtocols(const QByteArray& data);

private:
    QMap<QString, ProtocolParserPtr> m_protocols;   ///< 协议映射表
    QString m_protocolDirectory;                    ///< 协议文件目录
    
    static ProtocolEngine* s_instance;
};

} // namespace DeviceStudio
