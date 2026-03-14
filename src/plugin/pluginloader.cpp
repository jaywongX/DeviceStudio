/**
 * @file pluginloader.cpp
 * @brief 插件加载器实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "pluginloader.h"
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace DeviceStudio {

PluginLoader* PluginLoader::s_instance = nullptr;

PluginLoader* PluginLoader::instance()
{
    if (!s_instance) {
        s_instance = new PluginLoader();
    }
    return s_instance;
}

PluginLoader::PluginLoader(QObject* parent)
    : QObject(parent)
{
    // 添加默认插件路径
    QString appDir = QCoreApplication::applicationDirPath();
    m_pluginPaths.append(appDir + "/plugins");
    m_pluginPaths.append(appDir + "/../plugins");
}

PluginLoader::~PluginLoader()
{
    unloadAllPlugins();
}

void PluginLoader::addPluginPath(const QString& path)
{
    QString absPath = QDir(path).absolutePath();
    if (!m_pluginPaths.contains(absPath)) {
        m_pluginPaths.append(absPath);
    }
}

void PluginLoader::removePluginPath(const QString& path)
{
    QString absPath = QDir(path).absolutePath();
    m_pluginPaths.removeAll(absPath);
}

void PluginLoader::setPluginPaths(const QStringList& paths)
{
    m_pluginPaths.clear();
    for (const auto& path : paths) {
        addPluginPath(path);
    }
}

int PluginLoader::scanPlugins()
{
    QStringList files = findPluginFiles();
    
    for (const auto& file : files) {
        PluginMetaData metaData = extractMetaData(file);
        if (!metaData.id.isEmpty()) {
            emit pluginDiscovered(file, metaData);
        }
    }
    
    return files.size();
}

IPluginPtr PluginLoader::loadPlugin(const QString& filePath)
{
    // 检查文件是否存在
    if (!QFile::exists(filePath)) {
        m_lastError = tr("插件文件不存在: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return nullptr;
    }
    
    // 加载插件
    QPluginLoader loader(filePath);
    QObject* instance = loader.instance();
    
    if (!instance) {
        m_lastError = tr("加载插件失败: %1 - %2").arg(filePath, loader.errorString());
        emit errorOccurred(m_lastError);
        return nullptr;
    }
    
    // 转换为插件接口
    IPlugin* plugin = qobject_cast<IPlugin*>(instance);
    if (!plugin) {
        loader.unload();
        m_lastError = tr("无效的插件接口: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return nullptr;
    }
    
    // 验证插件
    if (!validatePlugin(IPluginPtr(plugin, [&loader](IPlugin*) { loader.unload(); }))) {
        loader.unload();
        return nullptr;
    }
    
    // 初始化插件
    if (!plugin->initialize()) {
        loader.unload();
        m_lastError = tr("插件初始化失败: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return nullptr;
    }
    
    // 创建共享指针
    IPluginPtr pluginPtr(plugin, [&loader](IPlugin*) { loader.unload(); });
    
    // 注册插件
    QString pluginId = plugin->id();
    m_plugins[pluginId] = pluginPtr;
    m_pluginFiles[pluginId] = filePath;
    
    emit pluginLoaded(pluginId, plugin->name());
    
    return pluginPtr;
}

void PluginLoader::unloadPlugin(const QString& pluginId)
{
    if (!m_plugins.contains(pluginId)) {
        return;
    }
    
    IPluginPtr plugin = m_plugins[pluginId];
    plugin->shutdown();
    
    m_plugins.remove(pluginId);
    m_pluginFiles.remove(pluginId);
    
    emit pluginUnloaded(pluginId);
}

void PluginLoader::unloadAllPlugins()
{
    QStringList ids = m_plugins.keys();
    for (const auto& id : ids) {
        unloadPlugin(id);
    }
}

IPluginPtr PluginLoader::reloadPlugin(const QString& pluginId)
{
    if (!m_pluginFiles.contains(pluginId)) {
        return nullptr;
    }
    
    QString filePath = m_pluginFiles[pluginId];
    unloadPlugin(pluginId);
    
    return loadPlugin(filePath);
}

IPluginPtr PluginLoader::getPlugin(const QString& pluginId) const
{
    return m_plugins.value(pluginId);
}

QList<IPluginPtr> PluginLoader::getPluginsByType(PluginType type) const
{
    QList<IPluginPtr> result;
    for (const auto& plugin : m_plugins) {
        if (plugin->type() == type) {
            result.append(plugin);
        }
    }
    return result;
}

QList<IPluginPtr> PluginLoader::getAllPlugins() const
{
    return m_plugins.values();
}

QStringList PluginLoader::pluginIds() const
{
    return m_plugins.keys();
}

bool PluginLoader::isPluginLoaded(const QString& pluginId) const
{
    return m_plugins.contains(pluginId);
}

IProtocolPluginPtr PluginLoader::getProtocolPlugin(const QString& pluginId) const
{
    IPluginPtr plugin = getPlugin(pluginId);
    if (!plugin || plugin->type() != PluginType::Protocol) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<IProtocolPlugin>(plugin);
}

ICommunicationPluginPtr PluginLoader::getCommunicationPlugin(const QString& pluginId) const
{
    IPluginPtr plugin = getPlugin(pluginId);
    if (!plugin || plugin->type() != PluginType::Communication) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<ICommunicationPlugin>(plugin);
}

IViewPluginPtr PluginLoader::getViewPlugin(const QString& pluginId) const
{
    IPluginPtr plugin = getPlugin(pluginId);
    if (!plugin || plugin->type() != PluginType::View) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<IViewPlugin>(plugin);
}

QList<IProtocolPluginPtr> PluginLoader::getProtocolPlugins() const
{
    QList<IProtocolPluginPtr> result;
    for (const auto& plugin : m_plugins) {
        if (plugin->type() == PluginType::Protocol) {
            auto protocolPlugin = std::dynamic_pointer_cast<IProtocolPlugin>(plugin);
            if (protocolPlugin) {
                result.append(protocolPlugin);
            }
        }
    }
    return result;
}

QList<ICommunicationPluginPtr> PluginLoader::getCommunicationPlugins() const
{
    QList<ICommunicationPluginPtr> result;
    for (const auto& plugin : m_plugins) {
        if (plugin->type() == PluginType::Communication) {
            auto commPlugin = std::dynamic_pointer_cast<ICommunicationPlugin>(plugin);
            if (commPlugin) {
                result.append(commPlugin);
            }
        }
    }
    return result;
}

PluginMetaData PluginLoader::getPluginMetaData(const QString& pluginId) const
{
    IPluginPtr plugin = getPlugin(pluginId);
    if (plugin) {
        return plugin->metaData();
    }
    return PluginMetaData();
}

QString PluginLoader::getPluginFilePath(const QString& pluginId) const
{
    return m_pluginFiles.value(pluginId);
}

QStringList PluginLoader::findPluginFiles() const
{
    QStringList files;
    
    // 根据平台确定插件扩展名
#ifdef Q_OS_WIN
    QStringList extensions = { "*.dll" };
#else
    QStringList extensions = { "*.so" };
#endif
    
    for (const auto& path : m_pluginPaths) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }
        
        for (const auto& ext : extensions) {
            QStringList found = dir.entryList(QStringList() << ext, QDir::Files);
            for (const auto& file : found) {
                files.append(dir.absoluteFilePath(file));
            }
        }
        
        // 递归搜索子目录
        QDirIterator it(path, extensions, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            if (!files.contains(it.filePath())) {
                files.append(it.filePath());
            }
        }
    }
    
    return files;
}

PluginMetaData PluginLoader::extractMetaData(const QString& filePath) const
{
    PluginMetaData metaData;
    
    QPluginLoader loader(filePath);
    QJsonObject json = loader.metaData();
    
    if (json.isEmpty()) {
        return metaData;
    }
    
    QJsonObject metaDataJson = json["MetaData"].toObject();
    
    metaData.id = metaDataJson["id"].toString();
    metaData.name = metaDataJson["name"].toString();
    metaData.version = metaDataJson["version"].toString();
    metaData.description = metaDataJson["description"].toString();
    metaData.author = metaDataJson["author"].toString();
    metaData.license = metaDataJson["license"].toString();
    metaData.website = metaDataJson["website"].toString();
    
    // 解析类型
    QString typeStr = metaDataJson["type"].toString("other").toLower();
    if (typeStr == "protocol") {
        metaData.type = PluginType::Protocol;
    } else if (typeStr == "communication") {
        metaData.type = PluginType::Communication;
    } else if (typeStr == "view") {
        metaData.type = PluginType::View;
    } else if (typeStr == "script") {
        metaData.type = PluginType::Script;
    } else {
        metaData.type = PluginType::Other;
    }
    
    // 解析依赖
    QJsonArray depends = metaDataJson["depends"].toArray();
    for (const auto& dep : depends) {
        metaData.depends.append(dep.toString());
    }
    
    // 解析提供
    QJsonArray provides = metaDataJson["provides"].toArray();
    for (const auto& prov : provides) {
        metaData.provides.append(prov.toString());
    }
    
    return metaData;
}

bool PluginLoader::validatePlugin(IPluginPtr plugin) const
{
    if (!plugin) {
        return false;
    }
    
    PluginMetaData metaData = plugin->metaData();
    
    // 检查必需字段
    if (metaData.id.isEmpty()) {
        m_lastError = tr("插件ID不能为空");
        return false;
    }
    
    if (metaData.name.isEmpty()) {
        m_lastError = tr("插件名称不能为空");
        return false;
    }
    
    // 检查依赖
    for (const auto& dep : metaData.depends) {
        if (!m_plugins.contains(dep)) {
            m_lastError = tr("缺少依赖插件: %1").arg(dep);
            return false;
        }
    }
    
    return true;
}

} // namespace DeviceStudio
