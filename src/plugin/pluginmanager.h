/**
 * @file pluginmanager.h
 * @brief 插件管理器
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "iplugin.h"
#include "pluginloader.h"
#include <QObject>
#include <QMap>

namespace DeviceStudio {

/**
 * @brief 插件管理器
 * 
 * 提供插件的统一管理和协调功能
 */
class PluginManager : public QObject
{
    Q_OBJECT

public:
    static PluginManager* instance();
    
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager() override;
    
    // ========== 初始化 ==========
    
    /**
     * @brief 初始化插件系统
     * @param pluginPaths 插件搜索路径列表
     * @return 是否成功
     */
    bool initialize(const QStringList& pluginPaths = QStringList());
    
    /**
     * @brief 关闭插件系统
     */
    void shutdown();
    
    /**
     * @brief 是否已初始化
     */
    bool isInitialized() const { return m_initialized; }
    
    // ========== 插件加载 ==========
    
    /**
     * @brief 加载所有插件
     * @return 成功加载的插件数量
     */
    int loadAllPlugins();
    
    /**
     * @brief 加载指定插件
     * @param pluginId 插件ID
     * @return 是否成功
     */
    bool loadPlugin(const QString& pluginId);
    
    /**
     * @brief 卸载指定插件
     * @param pluginId 插件ID
     */
    void unloadPlugin(const QString& pluginId);
    
    /**
     * @brief 重新加载插件
     * @param pluginId 插件ID
     * @return 是否成功
     */
    bool reloadPlugin(const QString& pluginId);
    
    // ========== 插件查询 ==========
    
    /**
     * @brief 获取插件
     */
    IPluginPtr getPlugin(const QString& pluginId) const;
    
    /**
     * @brief 获取指定类型的插件
     */
    QList<IPluginPtr> getPluginsByType(PluginType type) const;
    
    /**
     * @brief 获取所有插件
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
     * @brief 获取插件数量
     */
    int pluginCount() const;
    
    // ========== 协议插件 ==========
    
    /**
     * @brief 获取协议插件
     */
    IProtocolPluginPtr getProtocolPlugin(const QString& pluginId) const;
    
    /**
     * @brief 获取所有协议插件
     */
    QList<IProtocolPluginPtr> getProtocolPlugins() const;
    
    /**
     * @brief 查找支持指定协议的插件
     */
    IProtocolPluginPtr findProtocolPlugin(const QString& protocolName) const;
    
    // ========== 通信插件 ==========
    
    /**
     * @brief 获取通信插件
     */
    ICommunicationPluginPtr getCommunicationPlugin(const QString& pluginId) const;
    
    /**
     * @brief 获取所有通信插件
     */
    QList<ICommunicationPluginPtr> getCommunicationPlugins() const;
    
    /**
     * @brief 查找指定类型的通信插件
     */
    ICommunicationPluginPtr findCommunicationPlugin(const QString& commType) const;
    
    /**
     * @brief 获取可用的通信类型列表
     */
    QStringList availableCommunicationTypes() const;
    
    // ========== 视图插件 ==========
    
    /**
     * @brief 获取视图插件
     */
    IViewPluginPtr getViewPlugin(const QString& pluginId) const;
    
    /**
     * @brief 获取所有视图插件
     */
    QList<IViewPluginPtr> getViewPlugins() const;
    
    // ========== 配置 ==========
    
    /**
     * @brief 设置插件配置
     */
    void setPluginConfiguration(const QString& pluginId, const QVariantMap& config);
    
    /**
     * @brief 获取插件配置
     */
    QVariantMap getPluginConfiguration(const QString& pluginId) const;
    
    /**
     * @brief 保存所有插件配置
     */
    void saveAllConfigurations();
    
    /**
     * @brief 加载所有插件配置
     */
    void loadAllConfigurations();
    
    // ========== 工具方法 ==========
    
    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const { return m_lastError; }
    
    /**
     * @brief 获取插件加载器
     */
    PluginLoader* loader() { return m_loader; }

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
     * @brief 所有插件加载完成信号
     */
    void allPluginsLoaded();
    
    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& error);

private slots:
    void onPluginLoaded(const QString& pluginId, const QString& pluginName);
    void onPluginUnloaded(const QString& pluginId);
    void onPluginError(const QString& error);

private:
    bool m_initialized = false;
    PluginLoader* m_loader = nullptr;
    QMap<QString, QVariantMap> m_pluginConfigs; ///< 插件配置缓存
    QString m_configFilePath;                   ///< 配置文件路径
    QString m_lastError;
    
    static PluginManager* s_instance;
};

} // namespace DeviceStudio
