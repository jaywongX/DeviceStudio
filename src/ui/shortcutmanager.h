/**
 * @file shortcutmanager.h
 * @brief 快捷键管理器
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QObject>
#include <QKeySequence>
#include <QMap>
#include <QJsonObject>
#include <functional>

class QAction;
class QWidget;

namespace DeviceStudio {

/**
 * @brief 快捷键信息
 */
struct ShortcutInfo
{
    QString id;                 ///< 快捷键ID
    QString name;               ///< 显示名称
    QString description;        ///< 描述
    QKeySequence defaultKey;    ///< 默认快捷键
    QKeySequence currentKey;    ///< 当前快捷键
    QString category;           ///< 分类
    bool enabled = true;        ///< 是否启用
};

/**
 * @brief 快捷键管理器
 * 
 * 管理应用程序的所有快捷键，支持自定义快捷键
 */
class ShortcutManager : public QObject
{
    Q_OBJECT

public:
    using ShortcutCallback = std::function<void()>;
    
    /**
     * @brief 获取单例实例
     */
    static ShortcutManager* instance();
    
    /**
     * @brief 注册快捷键
     */
    void registerShortcut(const QString& id, 
                         const QString& name,
                         const QKeySequence& defaultKey,
                         const QString& description = QString(),
                         const QString& category = "General");
    
    /**
     * @brief 注册快捷键并绑定回调
     */
    void registerShortcut(const QString& id,
                         const QString& name,
                         const QKeySequence& defaultKey,
                         ShortcutCallback callback,
                         const QString& description = QString(),
                         const QString& category = "General");
    
    /**
     * @brief 注销快捷键
     */
    void unregisterShortcut(const QString& id);
    
    /**
     * @brief 绑定快捷键回调
     */
    void bindCallback(const QString& id, ShortcutCallback callback);
    
    /**
     * @brief 绑定快捷键到 QAction
     */
    void bindAction(const QString& id, QAction* action);
    
    /**
     * @brief 绑定快捷键到 QWidget（用于全局快捷键）
     */
    void bindWidget(const QString& id, QWidget* widget);
    
    /**
     * @brief 设置快捷键
     */
    bool setShortcut(const QString& id, const QKeySequence& key);
    
    /**
     * @brief 重置快捷键为默认值
     */
    void resetShortcut(const QString& id);
    
    /**
     * @brief 重置所有快捷键
     */
    void resetAllShortcuts();
    
    /**
     * @brief 获取快捷键信息
     */
    ShortcutInfo shortcut(const QString& id) const;
    
    /**
     * @brief 获取所有快捷键
     */
    QList<ShortcutInfo> allShortcuts() const;
    
    /**
     * @brief 获取指定分类的快捷键
     */
    QList<ShortcutInfo> shortcutsByCategory(const QString& category) const;
    
    /**
     * @brief 获取所有分类
     */
    QStringList categories() const;
    
    /**
     * @brief 检查快捷键是否冲突
     */
    QString checkConflict(const QString& id, const QKeySequence& key) const;
    
    /**
     * @brief 保存快捷键配置
     */
    void save();
    
    /**
     * @brief 加载快捷键配置
     */
    void load();

signals:
    /**
     * @brief 快捷键改变信号
     */
    void shortcutChanged(const QString& id, const QKeySequence& newKey);
    
    /**
     * @brief 快捷键触发信号
     */
    void shortcutTriggered(const QString& id);

protected:
    /**
     * @brief 事件过滤器
     */
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    ShortcutManager(QObject* parent = nullptr);
    ~ShortcutManager() = default;
    
    // 禁止拷贝
    ShortcutManager(const ShortcutManager&) = delete;
    ShortcutManager& operator=(const ShortcutManager&) = delete;
    
    /**
     * @brief 初始化默认快捷键
     */
    void initDefaultShortcuts();
    
    static ShortcutManager* s_instance;
    
    QMap<QString, ShortcutInfo> m_shortcuts;
    QMap<QString, ShortcutCallback> m_callbacks;
    QMap<QString, QAction*> m_actions;
    QMap<QString, QWidget*> m_widgets;
    QString m_configPath;
};

} // namespace DeviceStudio
