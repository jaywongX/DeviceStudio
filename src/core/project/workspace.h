/**
 * @file workspace.h
 * @brief 工作区管理
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QVariantMap>

namespace DeviceStudio {

/**
 * @brief 工作区配置
 */
struct WorkspaceConfig
{
    QString name;                   ///< 工作区名称
    QString description;            ///< 描述
    QString createTime;             ///< 创建时间
    QString modifyTime;             ///< 修改时间
    
    QStringList projectPaths;       ///< 包含的项目路径
    
    // UI配置
    QVariantMap windowGeometry;     ///< 窗口几何信息
    QVariantMap windowState;        ///< 窗口状态
    QVariantMap dockState;          ///< 停靠窗口状态
    QVariantMap toolbarState;       ///< 工具栏状态
    
    // 视图配置
    QString activeView;             ///< 当前激活的视图
    QStringList openViews;          ///< 打开的视图列表
    QVariantMap viewStates;         ///< 各视图状态
};

/**
 * @brief 工作区管理器
 * 
 * 管理工作区的创建、打开、保存等操作
 * 工作区可以包含多个项目，保存用户的UI布局和偏好设置
 */
class WorkspaceManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static WorkspaceManager* instance();
    
    /**
     * @brief 创建新工作区
     */
    bool createWorkspace(const QString& path, const WorkspaceConfig& config);
    
    /**
     * @brief 打开工作区
     */
    bool openWorkspace(const QString& path);
    
    /**
     * @brief 关闭工作区
     */
    void closeWorkspace();
    
    /**
     * @brief 保存工作区
     */
    bool saveWorkspace();
    
    /**
     * @brief 另存为
     */
    bool saveWorkspaceAs(const QString& path);
    
    /**
     * @brief 是否有打开的工作区
     */
    bool hasOpenWorkspace() const { return !m_workspacePath.isEmpty(); }
    
    /**
     * @brief 获取工作区路径
     */
    QString workspacePath() const { return m_workspacePath; }
    
    /**
     * @brief 获取工作区配置
     */
    WorkspaceConfig config() const { return m_config; }
    
    /**
     * @brief 更新工作区配置
     */
    void updateConfig(const WorkspaceConfig& config);
    
    /**
     * @brief 添加项目到工作区
     */
    void addProject(const QString& projectPath);
    
    /**
     * @brief 移除项目
     */
    void removeProject(const QString& projectPath);
    
    /**
     * @brief 获取项目列表
     */
    QStringList projects() const { return m_config.projectPaths; }
    
    /**
     * @brief 设置窗口状态
     */
    void setWindowState(const QVariantMap& state);
    
    /**
     * @brief 设置停靠窗口状态
     */
    void setDockState(const QVariantMap& state);
    
    /**
     * @brief 标记已修改
     */
    void markModified();
    
    /**
     * @brief 是否已修改
     */
    bool isModified() const { return m_modified; }
    
    /**
     * @brief 获取最近工作区列表
     */
    QStringList recentWorkspaces() const;
    
    /**
     * @brief 添加到最近工作区
     */
    void addToRecentWorkspaces(const QString& path);

signals:
    /**
     * @brief 工作区打开信号
     */
    void workspaceOpened(const QString& path);
    
    /**
     * @brief 工作区关闭信号
     */
    void workspaceClosed();
    
    /**
     * @brief 工作区保存信号
     */
    void workspaceSaved(const QString& path);
    
    /**
     * @brief 工作区修改信号
     */
    void workspaceModified(bool modified);
    
    /**
     * @brief 项目添加信号
     */
    void projectAdded(const QString& projectPath);
    
    /**
     * @brief 项目移除信号
     */
    void projectRemoved(const QString& projectPath);

private:
    WorkspaceManager(QObject* parent = nullptr);
    ~WorkspaceManager() = default;
    
    // 禁止拷贝
    WorkspaceManager(const WorkspaceManager&) = delete;
    WorkspaceManager& operator=(const WorkspaceManager&) = delete;
    
    /**
     * @brief 配置转JSON
     */
    QJsonObject configToJson() const;
    
    /**
     * @brief JSON转配置
     */
    WorkspaceConfig jsonToConfig(const QJsonObject& json) const;
    
    /**
     * @brief 保存最近工作区列表
     */
    void saveRecentWorkspaces();
    
    /**
     * @brief 加载最近工作区列表
     */
    void loadRecentWorkspaces();
    
    static WorkspaceManager* s_instance;
    
    QString m_workspacePath;
    WorkspaceConfig m_config;
    bool m_modified = false;
    QStringList m_recentWorkspaces;
    int m_maxRecentWorkspaces = 10;
};

} // namespace DeviceStudio
