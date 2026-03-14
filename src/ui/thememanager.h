/**
 * @file thememanager.h
 * @brief 主题管理器
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QObject>
#include <QMap>
#include <QColor>
#include <QString>
#include <QJsonObject>

namespace DeviceStudio {

/**
 * @brief 主题颜色配置
 */
struct ThemeColors
{
    QColor background;
    QColor foreground;
    QColor primary;
    QColor secondary;
    QColor accent;
    QColor error;
    QColor warning;
    QColor success;
    QColor border;
    QColor disabled;
    
    // 终端专用颜色
    QColor terminalBackground;
    QColor terminalForeground;
    QColor terminalRxColor;
    QColor terminalTxColor;
    QColor terminalHighlightColor;
    
    // 编辑器专用颜色
    QColor editorBackground;
    QColor editorForeground;
    QColor editorLineNumber;
    QColor editorSelection;
    QColor editorKeyword;
    QColor editorString;
    QColor editorComment;
    QColor editorNumber;
};

/**
 * @brief 主题管理器
 * 
 * 管理应用程序的主题配置，支持自定义主题
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static ThemeManager* instance();
    
    /**
     * @brief 获取当前主题名称
     */
    QString currentTheme() const { return m_currentTheme; }
    
    /**
     * @brief 获取可用主题列表
     */
    QStringList availableThemes() const;
    
    /**
     * @brief 切换主题
     */
    bool setTheme(const QString& themeName);
    
    /**
     * @brief 获取当前主题颜色
     */
    ThemeColors colors() const { return m_colors; }
    
    /**
     * @brief 获取特定颜色
     */
    QColor color(const QString& name) const;
    
    /**
     * @brief 应用主题到应用程序
     */
    void applyTheme();
    
    /**
     * @brief 保存自定义主题
     */
    bool saveCustomTheme(const QString& name, const ThemeColors& colors);
    
    /**
     * @brief 删除自定义主题
     */
    bool deleteCustomTheme(const QString& name);
    
    /**
     * @brief 获取主题样式表
     */
    QString styleSheet() const;

signals:
    /**
     * @brief 主题改变信号
     */
    void themeChanged(const QString& themeName);
    
    /**
     * @brief 颜色改变信号
     */
    void colorsChanged(const ThemeColors& colors);

private:
    ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() = default;
    
    // 禁止拷贝
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    /**
     * @brief 加载内置主题
     */
    void loadBuiltinThemes();
    
    /**
     * @brief 加载自定义主题
     */
    void loadCustomThemes();
    
    /**
     * @brief 从JSON加载主题
     */
    bool loadThemeFromJson(const QString& filePath);
    
    /**
     * @brief 生成样式表
     */
    void generateStyleSheet();
    
    static ThemeManager* s_instance;
    
    QString m_currentTheme;
    ThemeColors m_colors;
    QMap<QString, ThemeColors> m_themes;
    QString m_styleSheet;
    
    // 内置主题路径
    QString m_builtinThemesPath;
    QString m_customThemesPath;
};

} // namespace DeviceStudio
