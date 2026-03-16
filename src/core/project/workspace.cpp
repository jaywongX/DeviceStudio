/**
 * @file workspace.cpp
 * @brief 工作区管理实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "workspace.h"
#include "config/configmanager.h"
#include "log/logger.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QDateTime>
#include <QJsonArray>

namespace DeviceStudio {

WorkspaceManager* WorkspaceManager::s_instance = nullptr;

WorkspaceManager* WorkspaceManager::instance()
{
    if (!s_instance) {
        s_instance = new WorkspaceManager();
    }
    return s_instance;
}

WorkspaceManager::WorkspaceManager(QObject* parent)
    : QObject(parent)
{
    loadRecentWorkspaces();
}

bool WorkspaceManager::createWorkspace(const QString& path, const WorkspaceConfig& config)
{
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(path)) {
            DS_LOG_ERROR("Failed to create workspace directory: " + path.toStdString());
            return false;
        }
    }
    
    m_config = config;
    m_config.createTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_config.modifyTime = m_config.createTime;
    
    m_workspacePath = path;
    if (!saveWorkspace()) {
        m_workspacePath.clear();
        return false;
    }
    
    m_modified = false;
    addToRecentWorkspaces(path);
    
    DS_LOG_INFO("Workspace created: " + path.toStdString());
    emit workspaceOpened(path);
    
    return true;
}

bool WorkspaceManager::openWorkspace(const QString& path)
{
    QString workspaceFile = path + "/workspace.json";
    QFile file(workspaceFile);
    if (!file.exists()) {
        DS_LOG_ERROR("Workspace file not found: " + workspaceFile.toStdString());
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        DS_LOG_ERROR("Failed to open workspace file: " + workspaceFile.toStdString());
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        DS_LOG_ERROR("Invalid workspace file format");
        return false;
    }
    
    m_config = jsonToConfig(doc.object());
    m_workspacePath = path;
    m_modified = false;
    
    addToRecentWorkspaces(path);
    
    DS_LOG_INFO("Workspace opened: " + path.toStdString());
    emit workspaceOpened(path);
    
    return true;
}

void WorkspaceManager::closeWorkspace()
{
    if (m_workspacePath.isEmpty()) {
        return;
    }
    
    DS_LOG_INFO("Workspace closed: " + m_workspacePath.toStdString());
    
    m_workspacePath.clear();
    m_config = WorkspaceConfig();
    m_modified = false;
    
    emit workspaceClosed();
}

bool WorkspaceManager::saveWorkspace()
{
    if (m_workspacePath.isEmpty()) {
        DS_LOG_ERROR("No workspace to save");
        return false;
    }
    
    m_config.modifyTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QString workspaceFile = m_workspacePath + "/workspace.json";
    QFile file(workspaceFile);
    if (!file.open(QIODevice::WriteOnly)) {
        DS_LOG_ERROR("Failed to save workspace file: " + workspaceFile.toStdString());
        return false;
    }
    
    QJsonDocument doc(configToJson());
    file.write(doc.toJson());
    file.close();
    
    m_modified = false;
    
    DS_LOG_INFO("Workspace saved: " + m_workspacePath.toStdString());
    emit workspaceSaved(m_workspacePath);
    
    return true;
}

bool WorkspaceManager::saveWorkspaceAs(const QString& path)
{
    QString oldPath = m_workspacePath;
    m_workspacePath = path;
    
    if (!saveWorkspace()) {
        m_workspacePath = oldPath;
        return false;
    }
    
    addToRecentWorkspaces(path);
    return true;
}

void WorkspaceManager::updateConfig(const WorkspaceConfig& config)
{
    m_config = config;
    markModified();
}

void WorkspaceManager::addProject(const QString& projectPath)
{
    if (!m_config.projectPaths.contains(projectPath)) {
        m_config.projectPaths.append(projectPath);
        markModified();
        emit projectAdded(projectPath);
        DS_LOG_DEBUG("Project added to workspace: " + projectPath.toStdString());
    }
}

void WorkspaceManager::removeProject(const QString& projectPath)
{
    if (m_config.projectPaths.removeOne(projectPath)) {
        markModified();
        emit projectRemoved(projectPath);
        DS_LOG_DEBUG("Project removed from workspace: " + projectPath.toStdString());
    }
}

void WorkspaceManager::setWindowState(const QVariantMap& state)
{
    m_config.windowState = state;
    markModified();
}

void WorkspaceManager::setDockState(const QVariantMap& state)
{
    m_config.dockState = state;
    markModified();
}

void WorkspaceManager::markModified()
{
    if (!m_modified) {
        m_modified = true;
        emit workspaceModified(true);
    }
}

QStringList WorkspaceManager::recentWorkspaces() const
{
    return m_recentWorkspaces;
}

void WorkspaceManager::addToRecentWorkspaces(const QString& path)
{
    m_recentWorkspaces.removeAll(path);
    m_recentWorkspaces.prepend(path);
    
    while (m_recentWorkspaces.size() > m_maxRecentWorkspaces) {
        m_recentWorkspaces.removeLast();
    }
    
    saveRecentWorkspaces();
}

QJsonObject WorkspaceManager::configToJson() const
{
    QJsonObject json;
    
    json["name"] = m_config.name;
    json["description"] = m_config.description;
    json["createTime"] = m_config.createTime;
    json["modifyTime"] = m_config.modifyTime;
    
    json["projects"] = QJsonArray::fromStringList(m_config.projectPaths);
    
    json["windowGeometry"] = QJsonObject::fromVariantMap(m_config.windowGeometry);
    json["windowState"] = QJsonObject::fromVariantMap(m_config.windowState);
    json["dockState"] = QJsonObject::fromVariantMap(m_config.dockState);
    json["toolbarState"] = QJsonObject::fromVariantMap(m_config.toolbarState);
    
    json["activeView"] = m_config.activeView;
    json["openViews"] = QJsonArray::fromStringList(m_config.openViews);
    json["viewStates"] = QJsonObject::fromVariantMap(m_config.viewStates);
    
    return json;
}

WorkspaceConfig WorkspaceManager::jsonToConfig(const QJsonObject& json) const
{
    WorkspaceConfig config;
    
    config.name = json["name"].toString();
    config.description = json["description"].toString();
    config.createTime = json["createTime"].toString();
    config.modifyTime = json["modifyTime"].toString();
    
    config.projectPaths.clear();
    for (const auto& val : json["projects"].toArray()) {
        config.projectPaths.append(val.toString());
    }
    
    config.windowGeometry = json["windowGeometry"].toObject().toVariantMap();
    config.windowState = json["windowState"].toObject().toVariantMap();
    config.dockState = json["dockState"].toObject().toVariantMap();
    config.toolbarState = json["toolbarState"].toObject().toVariantMap();
    
    config.activeView = json["activeView"].toString();
    
    config.openViews.clear();
    for (const auto& val : json["openViews"].toArray()) {
        config.openViews.append(val.toString());
    }
    
    config.viewStates = json["viewStates"].toObject().toVariantMap();
    
    return config;
}

void WorkspaceManager::saveRecentWorkspaces()
{
    // 转换 QStringList 到 std::vector<std::string>
    std::vector<std::string> recent;
    for (const QString& path : m_recentWorkspaces) {
        recent.push_back(path.toStdString());
    }
    ConfigManager::instance().set("workspace.recent", recent);
}

void WorkspaceManager::loadRecentWorkspaces()
{
    auto recent = ConfigManager::instance().get<std::vector<std::string>>("workspace.recent");
    m_recentWorkspaces.clear();
    for (const auto& path : recent) {
        m_recentWorkspaces.append(QString::fromStdString(path));
    }
}

} // namespace DeviceStudio
