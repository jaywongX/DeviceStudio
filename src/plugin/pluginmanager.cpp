/**
 * @file pluginmanager.cpp
 * @brief 插件管理器实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "pluginmanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

namespace DeviceStudio {

PluginManager* PluginManager::s_instance = nullptr;

PluginManager* PluginManager::instance()
{
    if (!s_instance) {
        s_instance = new PluginManager();
    }
    return s_instance;
}

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
    , m_loader(new PluginLoader(this))
{
    // 设置默认配置文件路径
    QString configDir = QCoreApplication::applicationDirPath() + "/config";
    m_configFilePath = configDir + "/plugins.json";
    
    // 连接加载器信号
    connect(m_loader, &PluginLoader::pluginLoaded,
            this, &PluginManager::onPluginLoaded);
    connect(m_loader, &PluginLoader::pluginUnloaded,
            this, &PluginManager::onPluginUnloaded);
    connect(m_loader, &PluginLoader::errorOccurred,
            this, &PluginManager::onPluginError);
}

PluginManager::~PluginManager()
{
    shutdown();
}

bool PluginManager::initialize(const QStringList& pluginPaths)
{
    if (m_initialized) {
        return true;
    }
    
    // 设置插件路径
    if (pluginPaths.isEmpty()) {
        // 使用默认路径
        QString appDir = QCoreApplication::applicationDirPath();
        m_loader->addPluginPath(appDir + "/plugins");
        m_loader->addPluginPath(appDir + "/../plugins");
    } else {
        m_loader->setPluginPaths(pluginPaths);
    }
    
    // 加载插件配置
    loadAllConfigurations();
    
    m_initialized = true;
    return true;
}

void PluginManager::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    // 保存配置
    saveAllConfigurations();
    
    // 卸载所有插件
    m_loader->unloadAllPlugins();
    
    m_initialized = false;
}

int PluginManager::loadAllPlugins()
{
    // 扫描插件
    int discovered = m_loader->scanPlugins();
    
    // 加载所有发现的插件
    int loaded = 0;
    for (const auto& pluginId : m_loader->pluginIds()) {
        if (loadPlugin(pluginId)) {
            ++loaded;
        }
    }
    
    emit allPluginsLoaded();
    return loaded;
}

bool PluginManager::loadPlugin(const QString& pluginId)
{
    // 检查是否已加载
    if (isPluginLoaded(pluginId)) {
        return true;
    }
    
    // 获取插件文件路径
    QString filePath = m_loader->getPluginFilePath(pluginId);
    if (filePath.isEmpty()) {
        m_lastError = tr("未找到插件: %1").arg(pluginId);
        return false;
    }
    
    // 加载插件
    auto plugin = m_loader->getPlugin(pluginId);
    if (!plugin) {
        m_lastError = tr("加载插件失败: %1").arg(pluginId);
        return false;
    }
    
    // 应用配置
    if (m_pluginConfigs.contains(pluginId)) {
        plugin->setConfiguration(m_pluginConfigs[pluginId]);
    }
    
    return true;
}

void PluginManager::unloadPlugin(const QString& pluginId)
{
    // 保存配置
    auto plugin = getPlugin(pluginId);
    if (plugin) {
        m_pluginConfigs[pluginId] = plugin->configuration();
    }
    
    m_loader->unloadPlugin(pluginId);
}

bool PluginManager::reloadPlugin(const QString& pluginId)
{
    // 先保存配置
    auto plugin = getPlugin(pluginId);
    QVariantMap config;
    if (plugin) {
        config = plugin->configuration();
    }
    
    // 重新加载
    auto newPlugin = m_loader->reloadPlugin(pluginId);
    if (newPlugin) {
        newPlugin->setConfiguration(config);
        return true;
    }
    
    return false;
}

IPluginPtr PluginManager::getPlugin(const QString& pluginId) const
{
    return m_loader->getPlugin(pluginId);
}

QList<IPluginPtr> PluginManager::getPluginsByType(PluginType type) const
{
    return m_loader->getPluginsByType(type);
}

QList<IPluginPtr> PluginManager::getAllPlugins() const
{
    return m_loader->getAllPlugins();
}

QStringList PluginManager::pluginIds() const
{
    return m_loader->pluginIds();
}

bool PluginManager::isPluginLoaded(const QString& pluginId) const
{
    return m_loader->isPluginLoaded(pluginId);
}

int PluginManager::pluginCount() const
{
    return m_loader->pluginCount();
}

// ========== 协议插件 ==========

IProtocolPluginPtr PluginManager::getProtocolPlugin(const QString& pluginId) const
{
    return m_loader->getProtocolPlugin(pluginId);
}

QList<IProtocolPluginPtr> PluginManager::getProtocolPlugins() const
{
    return m_loader->getProtocolPlugins();
}

IProtocolPluginPtr PluginManager::findProtocolPlugin(const QString& protocolName) const
{
    auto plugins = getProtocolPlugins();
    for (const auto& plugin : plugins) {
        if (plugin->supportsProtocol(protocolName)) {
            return plugin;
        }
    }
    return nullptr;
}

// ========== 通信插件 ==========

ICommunicationPluginPtr PluginManager::getCommunicationPlugin(const QString& pluginId) const
{
    return m_loader->getCommunicationPlugin(pluginId);
}

QList<ICommunicationPluginPtr> PluginManager::getCommunicationPlugins() const
{
    return m_loader->getCommunicationPlugins();
}

ICommunicationPluginPtr PluginManager::findCommunicationPlugin(const QString& commType) const
{
    auto plugins = getCommunicationPlugins();
    for (const auto& plugin : plugins) {
        if (plugin->communicationType() == commType) {
            return plugin;
        }
    }
    return nullptr;
}

QStringList PluginManager::availableCommunicationTypes() const
{
    QStringList types;
    auto plugins = getCommunicationPlugins();
    for (const auto& plugin : plugins) {
        QString type = plugin->communicationType();
        if (!types.contains(type)) {
            types.append(type);
        }
    }
    return types;
}

// ========== 视图插件 ==========

IViewPluginPtr PluginManager::getViewPlugin(const QString& pluginId) const
{
    return m_loader->getViewPlugin(pluginId);
}

QList<IViewPluginPtr> PluginManager::getViewPlugins() const
{
    QList<IViewPluginPtr> result;
    auto plugins = getPluginsByType(PluginType::View);
    for (const auto& plugin : plugins) {
        auto viewPlugin = std::dynamic_pointer_cast<IViewPlugin>(plugin);
        if (viewPlugin) {
            result.append(viewPlugin);
        }
    }
    return result;
}

// ========== 配置 ==========

void PluginManager::setPluginConfiguration(const QString& pluginId, const QVariantMap& config)
{
    m_pluginConfigs[pluginId] = config;
    
    // 更新已加载插件的配置
    auto plugin = getPlugin(pluginId);
    if (plugin) {
        plugin->setConfiguration(config);
    }
}

QVariantMap PluginManager::getPluginConfiguration(const QString& pluginId) const
{
    // 优先返回运行时配置
    auto plugin = getPlugin(pluginId);
    if (plugin) {
        return plugin->configuration();
    }
    
    // 返回缓存配置
    return m_pluginConfigs.value(pluginId);
}

void PluginManager::saveAllConfigurations()
{
    // 收集所有配置
    QJsonObject root;
    
    for (const auto& pluginId : pluginIds()) {
        auto plugin = getPlugin(pluginId);
        if (plugin) {
            QJsonObject configObj;
            QVariantMap config = plugin->configuration();
            for (auto it = config.begin(); it != config.end(); ++it) {
                configObj[it.key()] = QJsonValue::fromVariant(it.value());
            }
            root[pluginId] = configObj;
        }
    }
    
    // 也保存未加载插件的配置
    for (auto it = m_pluginConfigs.begin(); it != m_pluginConfigs.end(); ++it) {
        if (!root.contains(it.key())) {
            QJsonObject configObj;
            for (auto cit = it.value().begin(); cit != it.value().end(); ++cit) {
                configObj[cit.key()] = QJsonValue::fromVariant(cit.value());
            }
            root[it.key()] = configObj;
        }
    }
    
    // 写入文件
    QDir dir = QFileInfo(m_configFilePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(m_configFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson());
        file.close();
    }
}

void PluginManager::loadAllConfigurations()
{
    QFile file(m_configFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return;
    }
    
    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
        QString pluginId = it.key();
        QJsonObject configObj = it.value().toObject();
        
        QVariantMap config;
        for (auto cit = configObj.begin(); cit != configObj.end(); ++cit) {
            config[cit.key()] = cit.value().toVariant();
        }
        
        m_pluginConfigs[pluginId] = config;
    }
}

// ========== 私有槽函数 ==========

void PluginManager::onPluginLoaded(const QString& pluginId, const QString& pluginName)
{
    emit pluginLoaded(pluginId, pluginName);
}

void PluginManager::onPluginUnloaded(const QString& pluginId)
{
    emit pluginUnloaded(pluginId);
}

void PluginManager::onPluginError(const QString& error)
{
    m_lastError = error;
    emit errorOccurred(error);
}

} // namespace DeviceStudio
