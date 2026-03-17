/**
 * @file scripteditorwidget.h
 * @brief 脚本编辑器组件
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QToolBar>
#include <QLabel>
#include <QTextCharFormat>
#include <memory>

namespace DeviceStudio {

class LuaScriptEngine;

/**
 * @brief Lua语法高亮器
 */
class LuaSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit LuaSyntaxHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> rules_;
    
    QTextCharFormat keywordFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat commentFormat_;
    QTextCharFormat numberFormat_;
    QTextCharFormat functionFormat_;
};

/**
 * @brief 脚本编辑器组件
 */
class ScriptEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptEditorWidget(QWidget* parent = nullptr);
    explicit ScriptEditorWidget(std::shared_ptr<LuaScriptEngine> engine, QWidget* parent = nullptr);
    ~ScriptEditorWidget() override;
    
    /**
     * @brief 设置脚本引擎
     */
    void setScriptEngine(std::shared_ptr<LuaScriptEngine> engine);
    
    /**
     * @brief 加载脚本文件
     */
    bool loadScript(const QString& filePath);
    
    /**
     * @brief 保存脚本文件
     */
    bool saveScript(const QString& filePath);
    
    /**
     * @brief 获取脚本内容
     */
    QString scriptContent() const;
    
    /**
     * @brief 设置脚本内容
     */
    void setScriptContent(const QString& content);
    
    /**
     * @brief 运行脚本
     */
    void runScript();
    
    /**
     * @brief 停止脚本
     */
    void stopScript();
    
    /**
     * @brief 清空输出
     */
    void clearOutput();

signals:
    /**
     * @brief 脚本执行完成信号
     */
    void scriptExecuted(bool success);
    
    /**
     * @brief 脚本修改信号
     */
    void scriptModified();
    
    /**
     * @brief 脚本停止信号
     */
    void scriptStopped();

private slots:
    void onRunClicked();
    void onStopClicked();
    void onClearClicked();
    void onScriptModified();
    void onPrintOutput(const QString& message);
    void onScriptExecuted(const QString& script, bool success);

private:
    void setupUI();
    void setupConnections();
    
    std::shared_ptr<LuaScriptEngine> scriptEngine_;
    
    QToolBar* toolBar_ = nullptr;
    QPlainTextEdit* codeEditor_ = nullptr;
    QPlainTextEdit* outputWidget_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    
    LuaSyntaxHighlighter* highlighter_ = nullptr;
    
    bool isRunning_ = false;
    bool isModified_ = false;
};

} // namespace DeviceStudio
