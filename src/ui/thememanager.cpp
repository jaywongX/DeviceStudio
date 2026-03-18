/**
 * @file thememanager.cpp
 * @brief 主题管理器实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "thememanager.h"
#include "config/configmanager.h"
#include "log/logger.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace DeviceStudio {

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    // 设置主题路径
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    m_customThemesPath = configPath + "/themes";
    m_builtinThemesPath = ":/themes";  // 资源文件路径
    
    // 创建自定义主题目录
    QDir().mkpath(m_customThemesPath);
    
    // 加载主题
    loadBuiltinThemes();
    loadCustomThemes();
    
    // 应用默认主题
    QString savedTheme = QString::fromStdString(ConfigManager::instance().get<std::string>("ui.theme", "Light"));
    setTheme(savedTheme);
}

QStringList ThemeManager::availableThemes() const
{
    return m_themes.keys();
}

bool ThemeManager::setTheme(const QString& themeName)
{
    if (!m_themes.contains(themeName)) {
        DS_LOG_WARN("Theme not found: " + themeName.toStdString());
        return false;
    }
    
    m_currentTheme = themeName;
    m_colors = m_themes[themeName];
    
    // 保存配置
    ConfigManager::instance().set<std::string>("ui.theme", themeName.toStdString());
    
    // 生成样式表
    generateStyleSheet();
    
    // 应用主题
    applyTheme();
    
    DS_LOG_INFO("Theme changed to: " + themeName.toStdString());
    emit themeChanged(themeName);
    emit colorsChanged(m_colors);
    
    return true;
}

QColor ThemeManager::color(const QString& name) const
{
    if (name == "background") return m_colors.background;
    if (name == "foreground") return m_colors.foreground;
    if (name == "primary") return m_colors.primary;
    if (name == "secondary") return m_colors.secondary;
    if (name == "accent") return m_colors.accent;
    if (name == "error") return m_colors.error;
    if (name == "warning") return m_colors.warning;
    if (name == "success") return m_colors.success;
    if (name == "border") return m_colors.border;
    if (name == "disabled") return m_colors.disabled;
    if (name == "terminalBackground") return m_colors.terminalBackground;
    if (name == "terminalForeground") return m_colors.terminalForeground;
    if (name == "terminalRxColor") return m_colors.terminalRxColor;
    if (name == "terminalTxColor") return m_colors.terminalTxColor;
    if (name == "terminalHighlightColor") return m_colors.terminalHighlightColor;
    if (name == "editorBackground") return m_colors.editorBackground;
    if (name == "editorForeground") return m_colors.editorForeground;
    if (name == "editorLineNumber") return m_colors.editorLineNumber;
    if (name == "editorSelection") return m_colors.editorSelection;
    if (name == "editorKeyword") return m_colors.editorKeyword;
    if (name == "editorString") return m_colors.editorString;
    if (name == "editorComment") return m_colors.editorComment;
    if (name == "editorNumber") return m_colors.editorNumber;
    
    return QColor();
}

void ThemeManager::applyTheme()
{
    if (qApp) {
        qApp->setStyleSheet(m_styleSheet);
    }
}

bool ThemeManager::saveCustomTheme(const QString& name, const ThemeColors& colors)
{
    QJsonObject json;
    json["name"] = name;
    
    // 基础颜色
    QJsonObject basicColors;
    basicColors["background"] = colors.background.name();
    basicColors["foreground"] = colors.foreground.name();
    basicColors["primary"] = colors.primary.name();
    basicColors["secondary"] = colors.secondary.name();
    basicColors["accent"] = colors.accent.name();
    basicColors["error"] = colors.error.name();
    basicColors["warning"] = colors.warning.name();
    basicColors["success"] = colors.success.name();
    basicColors["border"] = colors.border.name();
    basicColors["disabled"] = colors.disabled.name();
    json["basic"] = basicColors;
    
    // 终端颜色
    QJsonObject terminalColors;
    terminalColors["background"] = colors.terminalBackground.name();
    terminalColors["foreground"] = colors.terminalForeground.name();
    terminalColors["rx"] = colors.terminalRxColor.name();
    terminalColors["tx"] = colors.terminalTxColor.name();
    terminalColors["highlight"] = colors.terminalHighlightColor.name();
    json["terminal"] = terminalColors;
    
    // 编辑器颜色
    QJsonObject editorColors;
    editorColors["background"] = colors.editorBackground.name();
    editorColors["foreground"] = colors.editorForeground.name();
    editorColors["lineNumber"] = colors.editorLineNumber.name();
    editorColors["selection"] = colors.editorSelection.name();
    editorColors["keyword"] = colors.editorKeyword.name();
    editorColors["string"] = colors.editorString.name();
    editorColors["comment"] = colors.editorComment.name();
    editorColors["number"] = colors.editorNumber.name();
    json["editor"] = editorColors;
    
    // 保存文件
    QString filePath = m_customThemesPath + "/" + name + ".json";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        DS_LOG_ERROR("Failed to save theme: " + filePath.toStdString());
        return false;
    }
    
    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
    
    // 添加到内存
    m_themes[name] = colors;
    
    DS_LOG_INFO("Custom theme saved: " + name.toStdString());
    return true;
}

bool ThemeManager::deleteCustomTheme(const QString& name)
{
    if (name == "Light" || name == "Dark") {
        DS_LOG_WARN("Cannot delete built-in theme: " + name.toStdString());
        return false;
    }
    
    QString filePath = m_customThemesPath + "/" + name + ".json";
    QFile file(filePath);
    if (!file.remove()) {
        DS_LOG_ERROR("Failed to delete theme: " + filePath.toStdString());
        return false;
    }
    
    m_themes.remove(name);
    DS_LOG_INFO("Custom theme deleted: " + name.toStdString());
    return true;
}

QString ThemeManager::styleSheet() const
{
    return m_styleSheet;
}

void ThemeManager::loadBuiltinThemes()
{
    // Light 主题
    ThemeColors light;
    light.background = "#FFFFFF";
    light.foreground = "#1A1A1A";
    light.primary = "#0078D4";
    light.secondary = "#6B7280";
    light.accent = "#9333EA";
    light.error = "#EF4444";
    light.warning = "#F59E0B";
    light.success = "#10B981";
    light.border = "#E5E7EB";
    light.disabled = "#9CA3AF";
    
    light.terminalBackground = "#1E1E1E";
    light.terminalForeground = "#D4D4D4";
    light.terminalRxColor = "#4EC9B0";
    light.terminalTxColor = "#CE9178";
    light.terminalHighlightColor = "#FF0000";
    
    light.editorBackground = "#FFFFFF";
    light.editorForeground = "#1A1A1A";
    light.editorLineNumber = "#999999";
    light.editorSelection = "#ADD6FF";
    light.editorKeyword = "#0000FF";
    light.editorString = "#A31515";
    light.editorComment = "#008000";
    light.editorNumber = "#098658";
    
    m_themes["Light"] = light;
    
    // Dark 主题
    ThemeColors dark;
    dark.background = "#1E1E1E";
    dark.foreground = "#D4D4D4";
    dark.primary = "#0078D4";
    dark.secondary = "#6B7280";
    dark.accent = "#A78BFA";
    dark.error = "#F87171";
    dark.warning = "#FBBF24";
    dark.success = "#34D399";
    dark.border = "#3F3F46";
    dark.disabled = "#6B7280";
    
    dark.terminalBackground = "#0D0D0D";
    dark.terminalForeground = "#D4D4D4";
    dark.terminalRxColor = "#4EC9B0";
    dark.terminalTxColor = "#CE9178";
    dark.terminalHighlightColor = "#FF0000";
    
    dark.editorBackground = "#1E1E1E";
    dark.editorForeground = "#D4D4D4";
    dark.editorLineNumber = "#858585";
    dark.editorSelection = "#264F78";
    dark.editorKeyword = "#569CD6";
    dark.editorString = "#CE9178";
    dark.editorComment = "#6A9955";
    dark.editorNumber = "#B5CEA8";
    
    m_themes["Dark"] = dark;
}

void ThemeManager::loadCustomThemes()
{
    QDir dir(m_customThemesPath);
    QStringList filters;
    filters << "*.json";
    
    for (const QFileInfo& fileInfo : dir.entryInfoList(filters)) {
        loadThemeFromJson(fileInfo.absoluteFilePath());
    }
}

bool ThemeManager::loadThemeFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return false;
    }
    
    QJsonObject json = doc.object();
    QString name = json["name"].toString();
    
    if (name.isEmpty()) {
        return false;
    }
    
    ThemeColors colors;
    
    // 基础颜色
    QJsonObject basic = json["basic"].toObject();
    colors.background = QColor(basic["background"].toString("#FFFFFF"));
    colors.foreground = QColor(basic["foreground"].toString("#1A1A1A"));
    colors.primary = QColor(basic["primary"].toString("#0078D4"));
    colors.secondary = QColor(basic["secondary"].toString("#6B7280"));
    colors.accent = QColor(basic["accent"].toString("#9333EA"));
    colors.error = QColor(basic["error"].toString("#EF4444"));
    colors.warning = QColor(basic["warning"].toString("#F59E0B"));
    colors.success = QColor(basic["success"].toString("#10B981"));
    colors.border = QColor(basic["border"].toString("#E5E7EB"));
    colors.disabled = QColor(basic["disabled"].toString("#9CA3AF"));
    
    // 终端颜色
    QJsonObject terminal = json["terminal"].toObject();
    colors.terminalBackground = QColor(terminal["background"].toString("#1E1E1E"));
    colors.terminalForeground = QColor(terminal["foreground"].toString("#D4D4D4"));
    colors.terminalRxColor = QColor(terminal["rx"].toString("#4EC9B0"));
    colors.terminalTxColor = QColor(terminal["tx"].toString("#CE9178"));
    colors.terminalHighlightColor = QColor(terminal["highlight"].toString("#FF0000"));
    
    // 编辑器颜色
    QJsonObject editor = json["editor"].toObject();
    colors.editorBackground = QColor(editor["background"].toString("#FFFFFF"));
    colors.editorForeground = QColor(editor["foreground"].toString("#1A1A1A"));
    colors.editorLineNumber = QColor(editor["lineNumber"].toString("#999999"));
    colors.editorSelection = QColor(editor["selection"].toString("#ADD6FF"));
    colors.editorKeyword = QColor(editor["keyword"].toString("#0000FF"));
    colors.editorString = QColor(editor["string"].toString("#A31515"));
    colors.editorComment = QColor(editor["comment"].toString("#008000"));
    colors.editorNumber = QColor(editor["number"].toString("#098658"));
    
    m_themes[name] = colors;
    DS_LOG_INFO("Loaded custom theme: " + name.toStdString());
    return true;
}

void ThemeManager::generateStyleSheet()
{
    QString style;
    
    // 主窗口样式
    style += QString(R"(
QMainWindow, QWidget {
    background-color: %1;
    color: %2;
}
)").arg(m_colors.background.name(), m_colors.foreground.name());

    // 按钮样式
    style += QString(R"(
QPushButton {
    background-color: %1;
    color: white;
    border: none;
    padding: 6px 16px;
    border-radius: 4px;
    min-width: 80px;
}
QPushButton:hover {
    background-color: %2;
}
QPushButton:pressed {
    background-color: %3;
}
QPushButton:disabled {
    background-color: %4;
    color: %5;
}
)").arg(m_colors.primary.name(),
        m_colors.primary.darker(110).name(),
        m_colors.primary.darker(120).name(),
        m_colors.disabled.name(),
        m_colors.background.name());

    // 输入框样式
    style += QString(R"(
QLineEdit, QTextEdit, QPlainTextEdit {
    background-color: %1;
    color: %2;
    border: 1px solid %3;
    border-radius: 4px;
    padding: 4px 8px;
}
QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
    border-color: %4;
}
)").arg(m_colors.background.name(),
        m_colors.foreground.name(),
        m_colors.border.name(),
        m_colors.primary.name());

    // 分组框样式
    style += QString(R"(
QGroupBox {
    border: 1px solid %1;
    border-radius: 4px;
    margin-top: 8px;
    padding-top: 8px;
}
QGroupBox::title {
    color: %2;
    subcontrol-origin: margin;
    left: 8px;
}
)").arg(m_colors.border.name(), m_colors.foreground.name());

    // 标签样式
    style += QString(R"(
QLabel {
    color: %1;
}
)").arg(m_colors.foreground.name());

    // 复选框样式
    style += QString(R"(
QCheckBox {
    color: %1;
}
QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border: 2px solid %2;
    border-radius: 3px;
}
QCheckBox::indicator:checked {
    background-color: %3;
    border-color: %3;
}
)").arg(m_colors.foreground.name(),
        m_colors.border.name(),
        m_colors.primary.name());

    // 下拉框样式
    style += QString(R"(
QComboBox {
    background-color: %1;
    color: %2;
    border: 1px solid %3;
    border-radius: 4px;
    padding: 4px 8px;
    min-width: 100px;
}
QComboBox:hover {
    border-color: %4;
}
QComboBox::drop-down {
    border: none;
    width: 20px;
}
)").arg(m_colors.background.name(),
        m_colors.foreground.name(),
        m_colors.border.name(),
        m_colors.primary.name());

    // 表格样式
    style += QString(R"(
QTableWidget, QTableView {
    background-color: %1;
    color: %2;
    gridline-color: %3;
    border: 1px solid %3;
}
QTableWidget::item:selected, QTableView::item:selected {
    background-color: %4;
}
QHeaderView::section {
    background-color: %5;
    color: %2;
    padding: 4px;
    border: 1px solid %3;
}
)").arg(m_colors.background.name(),
        m_colors.foreground.name(),
        m_colors.border.name(),
        m_colors.primary.lighter(150).name(),
        m_colors.secondary.lighter(130).name());

    // 菜单样式
    style += QString(R"(
QMenuBar {
    background-color: %1;
    color: %2;
}
QMenuBar::item:selected {
    background-color: %3;
}
QMenu {
    background-color: %1;
    color: %2;
    border: 1px solid %4;
}
QMenu::item:selected {
    background-color: %3;
}
)").arg(m_colors.background.name(),
        m_colors.foreground.name(),
        m_colors.primary.lighter(150).name(),
        m_colors.border.name());

    // 滚动条样式
    style += QString(R"(
QScrollBar:vertical {
    background-color: %1;
    width: 10px;
    border-radius: 5px;
}
QScrollBar::handle:vertical {
    background-color: %2;
    min-height: 30px;
    border-radius: 5px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}
QScrollBar:horizontal {
    background-color: %1;
    height: 10px;
    border-radius: 5px;
}
QScrollBar::handle:horizontal {
    background-color: %2;
    min-width: 30px;
    border-radius: 5px;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0px;
}
)").arg(m_colors.background.name(), m_colors.border.name());

    // 工具栏样式
    style += QString(R"(
QToolBar {
    background-color: %1;
    border: none;
    spacing: 4px;
    padding: 4px;
}
QToolButton {
    background-color: transparent;
    color: %2;
    border: none;
    padding: 4px;
    border-radius: 4px;
}
QToolButton:hover {
    background-color: %3;
}
QToolButton:pressed {
    background-color: %4;
}
)").arg(m_colors.background.name(),
        m_colors.foreground.name(),
        m_colors.primary.lighter(150).name(),
        m_colors.primary.lighter(130).name());

    m_styleSheet = style;
}

} // namespace DeviceStudio
