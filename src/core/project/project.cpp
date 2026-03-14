/**
 * @file project.cpp
 * @brief 项目管理实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "project.h"
#include "config/configmanager.h"
#include "log/logger.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QDateTime>
#include <QStandardPaths>

namespace DeviceStudio {

ProjectManager* ProjectManager::s_instance = nullptr;

ProjectManager* ProjectManager::instance()
{
    if (!s_instance) {
        s_instance = new ProjectManager();
    }
    return s_instance;
}

ProjectManager::ProjectManager(QObject* parent)
    : QObject(parent)
{
    loadRecentProjects();
}

bool ProjectManager::createProject(const QString& path, const ProjectConfig& config)
{
    // 检查目录是否存在
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(path)) {
            DS_LOG_ERROR("Failed to create project directory: " + path.toStdString());
            return false;
        }
    }
    
    // 创建项目子目录
    dir.mkpath("data");
    dir.mkpath("scripts");
    dir.mkpath("protocols");
    dir.mkpath("logs");
    
    // 设置项目配置
    m_config = config;
    m_config.createTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_config.modifyTime = m_config.createTime;
    m_config.dataPath = "data";
    
    // 保存项目文件
    m_projectPath = path;
    if (!saveProject()) {
        m_projectPath.clear();
        return false;
    }
    
    m_modified = false;
    addToRecentProjects(path);
    
    DS_LOG_INFO("Project created: " + path.toStdString());
    emit projectOpened(path);
    
    return true;
}

bool ProjectManager::openProject(const QString& path)
{
    // 检查项目文件是否存在
    QString projectFile = path + "/project.json";
    QFile file(projectFile);
    if (!file.exists()) {
        DS_LOG_ERROR("Project file not found: " + projectFile.toStdString());
        return false;
    }
    
    // 读取项目文件
    if (!file.open(QIODevice::ReadOnly)) {
        DS_LOG_ERROR("Failed to open project file: " + projectFile.toStdString());
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        DS_LOG_ERROR("Invalid project file format");
        return false;
    }
    
    // 解析项目配置
    m_config = jsonToConfig(doc.object());
    m_projectPath = path;
    m_modified = false;
    
    addToRecentProjects(path);
    
    DS_LOG_INFO("Project opened: " + path.toStdString());
    emit projectOpened(path);
    
    return true;
}

void ProjectManager::closeProject()
{
    if (m_projectPath.isEmpty()) {
        return;
    }
    
    DS_LOG_INFO("Project closed: " + m_projectPath.toStdString());
    
    m_projectPath.clear();
    m_config = ProjectConfig();
    m_modified = false;
    
    emit projectClosed();
}

bool ProjectManager::saveProject()
{
    if (m_projectPath.isEmpty()) {
        DS_LOG_ERROR("No project to save");
        return false;
    }
    
    // 更新修改时间
    m_config.modifyTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // 保存项目文件
    QString projectFile = m_projectPath + "/project.json";
    QFile file(projectFile);
    if (!file.open(QIODevice::WriteOnly)) {
        DS_LOG_ERROR("Failed to save project file: " + projectFile.toStdString());
        return false;
    }
    
    QJsonDocument doc(configToJson());
    file.write(doc.toJson());
    file.close();
    
    m_modified = false;
    
    DS_LOG_INFO("Project saved: " + m_projectPath.toStdString());
    emit projectSaved(m_projectPath);
    
    return true;
}

bool ProjectManager::saveProjectAs(const QString& path)
{
    QString oldPath = m_projectPath;
    m_projectPath = path;
    
    if (!saveProject()) {
        m_projectPath = oldPath;
        return false;
    }
    
    addToRecentProjects(path);
    return true;
}

void ProjectManager::updateConfig(const ProjectConfig& config)
{
    m_config = config;
    markModified();
}

void ProjectManager::markModified()
{
    if (!m_modified) {
        m_modified = true;
        emit projectModified(true);
    }
}

QStringList ProjectManager::recentProjects() const
{
    return m_recentProjects;
}

void ProjectManager::addToRecentProjects(const QString& path)
{
    // 移除已存在的相同路径
    m_recentProjects.removeAll(path);
    
    // 添加到开头
    m_recentProjects.prepend(path);
    
    // 限制数量
    while (m_recentProjects.size() > m_maxRecentProjects) {
        m_recentProjects.removeLast();
    }
    
    saveRecentProjects();
}

void ProjectManager::clearRecentProjects()
{
    m_recentProjects.clear();
    saveRecentProjects();
}

QJsonObject ProjectManager::configToJson() const
{
    QJsonObject json;
    
    // 基本信息
    json["name"] = m_config.name;
    json["description"] = m_config.description;
    json["author"] = m_config.author;
    json["version"] = m_config.version;
    json["createTime"] = m_config.createTime;
    json["modifyTime"] = m_config.modifyTime;
    
    // 设备配置
    json["devices"] = QJsonArray::fromVariantList(m_config.devices);
    
    // 协议配置
    json["protocols"] = QJsonArray::fromStringList(m_config.protocols);
    
    // 脚本配置
    json["scripts"] = QJsonArray::fromStringList(m_config.scripts);
    
    // 数据路径
    json["dataPath"] = m_config.dataPath;
    
    // UI配置
    json["windowState"] = QJsonObject::fromVariantMap(m_config.windowState);
    json["panelLayout"] = QJsonObject::fromVariantMap(m_config.panelLayout);
    
    return json;
}

ProjectConfig ProjectManager::jsonToConfig(const QJsonObject& json) const
{
    ProjectConfig config;
    
    config.name = json["name"].toString();
    config.description = json["description"].toString();
    config.author = json["author"].toString();
    config.version = json["version"].toString();
    config.createTime = json["createTime"].toString();
    config.modifyTime = json["modifyTime"].toString();
    
    config.devices = json["devices"].toArray().toVariantList();
    
    config.protocols.clear();
    for (const auto& val : json["protocols"].toArray()) {
        config.protocols.append(val.toString());
    }
    
    config.scripts.clear();
    for (const auto& val : json["scripts"].toArray()) {
        config.scripts.append(val.toString());
    }
    
    config.dataPath = json["dataPath"].toString("data");
    
    config.windowState = json["windowState"].toObject().toVariantMap();
    config.panelLayout = json["panelLayout"].toObject().toVariantMap();
    
    return config;
}

void ProjectManager::saveRecentProjects()
{
    ConfigManager::instance()->setValue("project.recent", m_recentProjects);
}

void ProjectManager::loadRecentProjects()
{
    m_recentProjects = ConfigManager::instance()->value("project.recent").toStringList();
}

} // namespace DeviceStudio
