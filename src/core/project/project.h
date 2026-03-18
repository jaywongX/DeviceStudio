/**
 * @file project.h
 * @brief 项目管理
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
 * @brief 项目配置
 */
struct ProjectConfig
{
    QString name;                   ///< 项目名称
    QString description;            ///< 项目描述
    QString author;                 ///< 作者
    QString version;                ///< 版本
    QString createTime;             ///< 创建时间
    QString modifyTime;             ///< 修改时间
    
    // 设备配置
    QVariantList devices;           ///< 设备列表
    
    // 协议配置
    QStringList protocols;          ///< 协议文件列表
    
    // 脚本配置
    QStringList scripts;            ///< 脚本文件列表
    
    // 数据配置
    QString dataPath;               ///< 数据存储路径
    
    // UI配置
    QVariantMap windowState;        ///< 窗口状态
    QVariantMap panelLayout;        ///< 面板布局
};

/**
 * @brief 项目管理器
 * 
 * 管理项目的创建、打开、保存等操作
 */
class ProjectManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static ProjectManager* instance();
    
    /**
     * @brief 创建新项目
     */
    bool createProject(const QString& path, const ProjectConfig& config);
    
    /**
     * @brief 打开项目
     */
    bool openProject(const QString& path);
    
    /**
     * @brief 关闭项目
     */
    void closeProject();
    
    /**
     * @brief 保存项目
     */
    bool saveProject();
    
    /**
     * @brief 另存为
     */
    bool saveProjectAs(const QString& path);
    
    /**
     * @brief 是否有打开的项目
     */
    bool hasOpenProject() const { return !m_projectPath.isEmpty(); }
    
    /**
     * @brief 获取项目路径
     */
    QString projectPath() const { return m_projectPath; }
    
    /**
     * @brief 获取项目配置
     */
    ProjectConfig config() const { return m_config; }
    
    /**
     * @brief 更新项目配置
     */
    void updateConfig(const ProjectConfig& config);
    
    /**
     * @brief 标记项目已修改
     */
    void markModified();
    
    /**
     * @brief 项目是否已修改
     */
    bool isModified() const { return m_modified; }
    
    /**
     * @brief 获取最近项目列表
     */
    QStringList recentProjects() const;
    
    /**
     * @brief 添加到最近项目
     */
    void addToRecentProjects(const QString& path);
    
    /**
     * @brief 清除最近项目列表
     */
    void clearRecentProjects();

signals:
    /**
     * @brief 项目打开信号
     */
    void projectOpened(const QString& path);
    
    /**
     * @brief 项目关闭信号
     */
    void projectClosed();
    
    /**
     * @brief 项目保存信号
     */
    void projectSaved(const QString& path);
    
    /**
     * @brief 项目修改信号
     */
    void projectModified(bool modified);

private:
    ProjectManager(QObject* parent = nullptr);
    ~ProjectManager() = default;
    
    // 禁止拷贝
    ProjectManager(const ProjectManager&) = delete;
    ProjectManager& operator=(const ProjectManager&) = delete;
    
    /**
     * @brief 项目配置转JSON
     */
    QJsonObject configToJson() const;
    
    /**
     * @brief JSON转项目配置
     */
    ProjectConfig jsonToConfig(const QJsonObject& json) const;
    
    /**
     * @brief 保存最近项目列表
     */
    void saveRecentProjects();
    
    /**
     * @brief 加载最近项目列表
     */
    void loadRecentProjects();
    
    static ProjectManager* s_instance;
    
    QString m_projectPath;
    ProjectConfig m_config;
    bool m_modified = false;
    QStringList m_recentProjects;
    int m_maxRecentProjects = 10;
};

} // namespace DeviceStudio
