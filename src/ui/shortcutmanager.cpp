/**
 * @file shortcutmanager.cpp
 * @brief 快捷键管理器实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "shortcutmanager.h"
#include "config/configmanager.h"
#include "log/logger.h"
#include <QAction>
#include <QWidget>
#include <QKeyEvent>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QApplication>

namespace DeviceStudio {

ShortcutManager* ShortcutManager::s_instance = nullptr;

ShortcutManager* ShortcutManager::instance()
{
    if (!s_instance) {
        s_instance = new ShortcutManager();
    }
    return s_instance;
}

ShortcutManager::ShortcutManager(QObject* parent)
    : QObject(parent)
{
    // 安装事件过滤器
    qApp->installEventFilter(this);
    
    // 设置配置路径
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    m_configPath = configPath + "/shortcuts.json";
    
    // 初始化默认快捷键
    initDefaultShortcuts();
    
    // 加载用户配置
    load();
}

void ShortcutManager::registerShortcut(const QString& id,
                                       const QString& name,
                                       const QKeySequence& defaultKey,
                                       const QString& description,
                                       const QString& category)
{
    ShortcutInfo info;
    info.id = id;
    info.name = name;
    info.description = description;
    info.defaultKey = defaultKey;
    info.currentKey = defaultKey;
    info.category = category;
    info.enabled = true;
    
    m_shortcuts[id] = info;
}

void ShortcutManager::registerShortcut(const QString& id,
                                       const QString& name,
                                       const QKeySequence& defaultKey,
                                       ShortcutCallback callback,
                                       const QString& description,
                                       const QString& category)
{
    registerShortcut(id, name, defaultKey, description, category);
    bindCallback(id, callback);
}

void ShortcutManager::unregisterShortcut(const QString& id)
{
    m_shortcuts.remove(id);
    m_callbacks.remove(id);
    m_actions.remove(id);
    m_widgets.remove(id);
}

void ShortcutManager::bindCallback(const QString& id, ShortcutCallback callback)
{
    if (m_shortcuts.contains(id)) {
        m_callbacks[id] = callback;
    }
}

void ShortcutManager::bindAction(const QString& id, QAction* action)
{
    if (m_shortcuts.contains(id) && action) {
        m_actions[id] = action;
        action->setShortcut(m_shortcuts[id].currentKey);
    }
}

void ShortcutManager::bindWidget(const QString& id, QWidget* widget)
{
    if (m_shortcuts.contains(id) && widget) {
        m_widgets[id] = widget;
    }
}

bool ShortcutManager::setShortcut(const QString& id, const QKeySequence& key)
{
    if (!m_shortcuts.contains(id)) {
        return false;
    }
    
    // 检查冲突
    QString conflict = checkConflict(id, key);
    if (!conflict.isEmpty()) {
        DS_LOG_WARN("Shortcut conflict: " + id.toStdString() + " conflicts with " + conflict.toStdString());
        return false;
    }
    
    m_shortcuts[id].currentKey = key;
    
    // 更新绑定的 QAction
    if (m_actions.contains(id)) {
        m_actions[id]->setShortcut(key);
    }
    
    emit shortcutChanged(id, key);
    DS_LOG_DEBUG("Shortcut changed: " + id.toStdString() + " -> " + key.toString().toStdString());
    
    return true;
}

void ShortcutManager::resetShortcut(const QString& id)
{
    if (m_shortcuts.contains(id)) {
        setShortcut(id, m_shortcuts[id].defaultKey);
    }
}

void ShortcutManager::resetAllShortcuts()
{
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
        setShortcut(it.key(), it->defaultKey);
    }
}

ShortcutInfo ShortcutManager::shortcut(const QString& id) const
{
    return m_shortcuts.value(id);
}

QList<ShortcutInfo> ShortcutManager::allShortcuts() const
{
    return m_shortcuts.values();
}

QList<ShortcutInfo> ShortcutManager::shortcutsByCategory(const QString& category) const
{
    QList<ShortcutInfo> result;
    for (const ShortcutInfo& info : m_shortcuts) {
        if (info.category == category) {
            result.append(info);
        }
    }
    return result;
}

QStringList ShortcutManager::categories() const
{
    QStringList result;
    for (const ShortcutInfo& info : m_shortcuts) {
        if (!result.contains(info.category)) {
            result.append(info.category);
        }
    }
    return result;
}

QString ShortcutManager::checkConflict(const QString& id, const QKeySequence& key) const
{
    if (key.isEmpty()) {
        return QString();
    }
    
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
        if (it.key() != id && it->currentKey == key && it->enabled) {
            return it.key();
        }
    }
    
    return QString();
}

void ShortcutManager::save()
{
    QJsonArray array;
    for (const ShortcutInfo& info : m_shortcuts) {
        QJsonObject obj;
        obj["id"] = info.id;
        obj["key"] = info.currentKey.toString();
        obj["enabled"] = info.enabled;
        array.append(obj);
    }
    
    QJsonDocument doc(array);
    
    QFile file(m_configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        DS_LOG_INFO("Shortcuts saved to: " + m_configPath.toStdString());
    } else {
        DS_LOG_ERROR("Failed to save shortcuts: " + m_configPath.toStdString());
    }
}

void ShortcutManager::load()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        DS_LOG_DEBUG("No saved shortcuts found, using defaults");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isArray()) {
        return;
    }
    
    for (const QJsonValue& value : doc.array()) {
        QJsonObject obj = value.toObject();
        QString id = obj["id"].toString();
        QString keyStr = obj["key"].toString();
        bool enabled = obj["enabled"].toBool(true);
        
        if (m_shortcuts.contains(id)) {
            QKeySequence key(keyStr);
            if (!key.isEmpty()) {
                m_shortcuts[id].currentKey = key;
            }
            m_shortcuts[id].enabled = enabled;
        }
    }
    
    DS_LOG_INFO("Shortcuts loaded from: " + m_configPath.toStdString());
}

bool ShortcutManager::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        QKeySequence key(keyEvent->key() | keyEvent->modifiers());
        
        // 检查是否匹配任何快捷键
        for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
            if (it->enabled && it->currentKey == key) {
                // 调用回调
                if (m_callbacks.contains(it.key())) {
                    m_callbacks[it.key()]();
                }
                
                // 触发 QAction
                if (m_actions.contains(it.key())) {
                    m_actions[it.key()]->trigger();
                }
                
                emit shortcutTriggered(it.key());
                return true;
            }
        }
    }
    
    return QObject::eventFilter(watched, event);
}

void ShortcutManager::initDefaultShortcuts()
{
    // 文件操作
    registerShortcut("file.new", tr("新建"), QKeySequence::New, 
                     tr("创建新项目"), "File");
    registerShortcut("file.open", tr("打开"), QKeySequence::Open,
                     tr("打开项目"), "File");
    registerShortcut("file.save", tr("保存"), QKeySequence::Save,
                     tr("保存当前文件"), "File");
    registerShortcut("file.saveAs", tr("另存为"), QKeySequence::SaveAs,
                     tr("另存为"), "File");
    registerShortcut("file.close", tr("关闭"), QKeySequence::Close,
                     tr("关闭当前文件"), "File");
    
    // 编辑操作
    registerShortcut("edit.undo", tr("撤销"), QKeySequence::Undo,
                     tr("撤销上一步操作"), "Edit");
    registerShortcut("edit.redo", tr("重做"), QKeySequence::Redo,
                     tr("重做上一步操作"), "Edit");
    registerShortcut("edit.cut", tr("剪切"), QKeySequence::Cut,
                     tr("剪切选中内容"), "Edit");
    registerShortcut("edit.copy", tr("复制"), QKeySequence::Copy,
                     tr("复制选中内容"), "Edit");
    registerShortcut("edit.paste", tr("粘贴"), QKeySequence::Paste,
                     tr("粘贴内容"), "Edit");
    registerShortcut("edit.selectAll", tr("全选"), QKeySequence::SelectAll,
                     tr("选择全部"), "Edit");
    registerShortcut("edit.find", tr("查找"), QKeySequence::Find,
                     tr("查找内容"), "Edit");
    registerShortcut("edit.replace", tr("替换"), QKeySequence::Replace,
                     tr("替换内容"), "Edit");
    
    // 设备操作
    registerShortcut("device.connect", tr("连接设备"), QKeySequence(Qt::CTRL | Qt::Key_D),
                     tr("连接当前选中设备"), "Device");
    registerShortcut("device.disconnect", tr("断开设备"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D),
                     tr("断开当前设备连接"), "Device");
    registerShortcut("device.refresh", tr("刷新设备"), QKeySequence(Qt::Key_F5),
                     tr("刷新设备列表"), "Device");
    
    // 终端操作
    registerShortcut("terminal.clear", tr("清空终端"), QKeySequence(Qt::CTRL | Qt::Key_L),
                     tr("清空终端内容"), "Terminal");
    registerShortcut("terminal.send", tr("发送数据"), QKeySequence(Qt::CTRL | Qt::Key_Return),
                     tr("发送终端数据"), "Terminal");
    
    // 视图操作
    registerShortcut("view.fullScreen", tr("全屏"), QKeySequence::FullScreen,
                     tr("切换全屏模式"), "View");
    registerShortcut("view.settings", tr("设置"), QKeySequence(Qt::Key_F12),
                     tr("打开设置对话框"), "View");
    
    // 帮助操作
    registerShortcut("help.about", tr("关于"), QKeySequence::HelpContents,
                     tr("显示关于对话框"), "Help");
}

} // namespace DeviceStudio
