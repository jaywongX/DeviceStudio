/**
 * @file pluginloader.h
 * @brief 插件加载器
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include "iplugin.h"
#include <QObject>
#include <QStringList>
#include <QMap>
#include <memory>

namespace DeviceStudio {

/**
 * @brief 插件加载器
 * 
 * 负责动态加载和管理插件
 */
class PluginLoader : public QObject
{
    Q_OBJECT

public:
    static PluginLoader* instance();
    
    explicit PluginLoader(QObject* parent = nullptr);
    ~PluginLoader() override;
    
    // ========== 插件目录管理 ==========
    
    /**
     * @brief 添加插件搜索路径
     * @param path 路径
     */
    void addPluginPath(const QString& path);
    
    /**
     * @brief 移除插件搜索路径
     * @param path 路径
     */
    void removePluginPath(const QString& path);
    
    /**
     * @brief 获取所有插件搜索路径
     */
    QStringList pluginPaths() const { return m_pluginPaths; }
    
    /**
     * @brief 设置插件搜索路径
     */
    void setPluginPaths(const QStringList& paths);
    
    // ========== 插件加载 ==========
    
    /**
     * @brief 扫描所有插件目录
     * @return 发现的插件数量
     */
    int scanPlugins();
    
    /**
     * @brief 加载插件
     * @param filePath 插件文件路径
     * @return 插件实例,失败返回nullptr
     */
    IPluginPtr loadPlugin(const QString& filePath);
    
    /**
     * @brief 卸载插件
     * @param pluginId 插件ID
     */
    void unloadPlugin(const QString& pluginId);
    
    /**
     * @brief 卸载所有插件
     */
    void unloadAllPlugins();
    
    /**
     * @brief 重新加载插件
     * @param pluginId 插件ID
     * @return 新的插件实例
     */
    IPluginPtr reloadPlugin(const QString& pluginId);
    
    // ========== 插件查询 ==========
    
    /**
     * @brief 获取插件
     * @param pluginId 插件ID
     * @return 插件实例
     */
    IPluginPtr getPlugin(const QString& pluginId) const;
    
    /**
     * @brief 获取指定类型的插件列表
     */
    QList<IPluginPtr> getPluginsByType(PluginType type) const;
    
    /**
     * @brief 获取所有已加载的插件
     */
    QList<IPluginPtr> getAllPlugins() const;
    
    /**
     * @brief 获取插件ID列表
     */
    QStringList pluginIds() const;
    
    /**
     * @brief 检查插件是否已加载
     */
    bool isPluginLoaded(const QString& pluginId) const;
    
    /**
     * @brief 获取已加载插件数量
     */
    int pluginCount() const { return m_plugins.size(); }
    
    // ========== 特定类型插件获取 ==========
    
    /**
     * @brief 获取协议插件
     */
    IProtocolPluginPtr getProtocolPlugin(const QString& pluginId) const;
    
    /**
     * @brief 获取通信插件
     */
    ICommunicationPluginPtr getCommunicationPlugin(const QString& pluginId) const;
    
    /**
     * @brief 获取视图插件
     */
    IViewPluginPtr getViewPlugin(const QString& pluginId) const;
    
    /**
     * @brief 获取所有协议插件
     */
    QList<IProtocolPluginPtr> getProtocolPlugins() const;
    
    /**
     * @brief 获取所有通信插件
     */
    QList<ICommunicationPluginPtr> getCommunicationPlugins() const;
    
    // ========== 插件信息 ==========
    
    /**
     * @brief 获取插件元数据
     */
    PluginMetaData getPluginMetaData(const QString& pluginId) const;
    
    /**
     * @brief 获取插件文件路径
     */
    QString getPluginFilePath(const QString& pluginId) const;
    
    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const { return m_lastError; }

signals:
    /**
     * @brief 插件加载完成信号
     */
    void pluginLoaded(const QString& pluginId, const QString& pluginName);
    
    /**
     * @brief 插件卸载信号
     */
    void pluginUnloaded(const QString& pluginId);
    
    /**
     * @brief 插件发现信号(扫描时)
     */
    void pluginDiscovered(const QString& filePath, const PluginMetaData& metaData);
    
    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& error);

private:
    // 查找插件文件
    QStringList findPluginFiles() const;
    
    // 获取插件元数据(不加载)
    PluginMetaData extractMetaData(const QString& filePath) const;
    
    // 验证插件
    bool validatePlugin(IPluginPtr plugin) const;

private:
    QStringList m_pluginPaths;                              ///< 插件搜索路径
    QMap<QString, IPluginPtr> m_plugins;                    ///< 已加载插件
    QMap<QString, QString> m_pluginFiles;                   ///< 插件文件路径映射
    mutable QString m_lastError;                            ///< 最后的错误信息
    
    static PluginLoader* s_instance;
};

} // namespace DeviceStudio
