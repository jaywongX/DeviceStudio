/**
 * @file scripteditorwidget.cpp
 * @brief 脚本编辑器组件实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "scripteditorwidget.h"
#include "core/scriptengine/luascriptengine.h"
#include "utils/log/logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

namespace DeviceStudio {

// ========== LuaSyntaxHighlighter 实现 ==========

LuaSyntaxHighlighter::LuaSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    // 关键字格式
    keywordFormat_.setForeground(QColor(56, 152, 38));  // 绿色
    keywordFormat_.setFontWeight(QFont::Bold);
    
    QStringList keywords = {
        "and", "break", "do", "else", "elseif", "end",
        "false", "for", "function", "if", "in", "local",
        "nil", "not", "or", "repeat", "return", "then",
        "true", "until", "while"
    };
    
    for (const QString& keyword : keywords) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(QString("\\b%1\\b").arg(keyword));
        rule.format = keywordFormat_;
        rules_.append(rule);
    }
    
    // 字符串格式
    stringFormat_.setForeground(QColor(163, 21, 21));  // 深红色
    HighlightingRule stringRule;
    stringRule.pattern = QRegularExpression("\".*?\"|'.*?'|\\[\\[.*?\\]\\]");
    stringRule.format = stringFormat_;
    rules_.append(stringRule);
    
    // 注释格式
    commentFormat_.setForeground(QColor(128, 128, 128));  // 灰色
    commentFormat_.setFontItalic(true);
    HighlightingRule commentRule;
    commentRule.pattern = QRegularExpression("--[^\n]*");
    commentRule.format = commentFormat_;
    rules_.append(commentRule);
    
    // 数字格式
    numberFormat_.setForeground(QColor(9, 115, 207));  // 蓝色
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    numberRule.format = numberFormat_;
    rules_.append(numberRule);
    
    // 函数格式
    functionFormat_.setForeground(QColor(111, 66, 193));  // 紫色
    HighlightingRule functionRule;
    functionRule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    functionRule.format = functionFormat_;
    rules_.append(functionRule);
}

void LuaSyntaxHighlighter::highlightBlock(const QString& text)
{
    for (const HighlightingRule& rule : rules_) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

// ========== ScriptEditorWidget 实现 ==========

ScriptEditorWidget::ScriptEditorWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
}

ScriptEditorWidget::ScriptEditorWidget(std::shared_ptr<LuaScriptEngine> engine, QWidget* parent)
    : QWidget(parent)
    , scriptEngine_(engine)
{
    setupUI();
    setupConnections();
}

ScriptEditorWidget::~ScriptEditorWidget()
{
}

void ScriptEditorWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 工具栏
    toolBar_ = new QToolBar(tr("脚本工具栏"), this);
    toolBar_->setIconSize(QSize(16, 16));
    
    QAction* runAction = toolBar_->addAction(QIcon(":/icons/run.png"), tr("运行"));
    runAction->setShortcut(QKeySequence(Qt::Key_F5));
    connect(runAction, &QAction::triggered, this, &ScriptEditorWidget::onRunClicked);
    
    QAction* stopAction = toolBar_->addAction(QIcon(":/icons/stop.png"), tr("停止"));
    stopAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));
    connect(stopAction, &QAction::triggered, this, &ScriptEditorWidget::onStopClicked);
    
    toolBar_->addSeparator();
    
    QAction* clearAction = toolBar_->addAction(QIcon(":/icons/clear.png"), tr("清空输出"));
    connect(clearAction, &QAction::triggered, this, &ScriptEditorWidget::onClearClicked);
    
    mainLayout->addWidget(toolBar_);
    
    // 分割器
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    
    // 代码编辑器
    codeEditor_ = new QPlainTextEdit(this);
    codeEditor_->setFont(QFont("Courier New", 10));
    codeEditor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    codeEditor_->setTabStopDistance(40);
    
    // 设置语法高亮
    highlighter_ = new LuaSyntaxHighlighter(codeEditor_->document());
    
    splitter->addWidget(codeEditor_);
    
    // 输出窗口
    QWidget* outputWidget = new QWidget(this);
    QVBoxLayout* outputLayout = new QVBoxLayout(outputWidget);
    outputLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* outputLabel = new QLabel(tr("输出:"), this);
    outputLayout->addWidget(outputLabel);
    
    outputWidget_ = new QPlainTextEdit(this);
    outputWidget_->setFont(QFont("Courier New", 9));
    outputWidget_->setReadOnly(true);
    outputWidget_->setMaximumHeight(150);
    outputLayout->addWidget(outputWidget_);
    
    splitter->addWidget(outputWidget);
    splitter->setStretchFactor(0, 7);
    splitter->setStretchFactor(1, 3);
    
    mainLayout->addWidget(splitter);
    
    // 状态栏
    statusLabel_ = new QLabel(tr("就绪"), this);
    statusLabel_->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 2px; }");
    mainLayout->addWidget(statusLabel_);
    
    // 默认脚本
    codeEditor_->setPlainText(
        "-- DeviceStudio Lua Script\n"
        "-- 设备操作示例\n\n"
        "-- 连接设备\n"
        "Device.connect(\"SerialPort1\")\n\n"
        "-- 发送数据\n"
        "Device.send(\"SerialPort1\", \"Hello World\")\n\n"
        "-- 延时\n"
        "Util.sleep(1000)\n\n"
        "-- 接收数据\n"
        "local data = Device.receive(\"SerialPort1\")\n"
        "print(\"Received:\", data)\n\n"
        "-- 断开设备\n"
        "Device.disconnect(\"SerialPort1\")\n"
    );
}

void ScriptEditorWidget::setupConnections()
{
    connect(codeEditor_, &QPlainTextEdit::textChanged, this, &ScriptEditorWidget::onScriptModified);
    
    if (scriptEngine_) {
        connect(scriptEngine_.get(), &LuaScriptEngine::printOutput, this, &ScriptEditorWidget::onPrintOutput);
        connect(scriptEngine_.get(), &LuaScriptEngine::scriptExecuted, this, &ScriptEditorWidget::onScriptExecuted);
    }
}

void ScriptEditorWidget::setScriptEngine(std::shared_ptr<LuaScriptEngine> engine)
{
    if (scriptEngine_) {
        disconnect(scriptEngine_.get(), nullptr, this, nullptr);
    }
    
    scriptEngine_ = engine;
    setupConnections();
}

bool ScriptEditorWidget::loadScript(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        DS_LOG_ERROR("Failed to open script file: {}", filePath.toStdString());
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    setScriptContent(content);
    isModified_ = false;
    
    DS_LOG_INFO("Script loaded: {}", filePath.toStdString());
    return true;
}

bool ScriptEditorWidget::saveScript(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DS_LOG_ERROR("Failed to save script file: {}", filePath.toStdString());
        return false;
    }
    
    QTextStream out(&file);
    out << scriptContent();
    file.close();
    
    isModified_ = false;
    DS_LOG_INFO("Script saved: {}", filePath.toStdString());
    return true;
}

QString ScriptEditorWidget::scriptContent() const
{
    return codeEditor_->toPlainText();
}

void ScriptEditorWidget::setScriptContent(const QString& content)
{
    codeEditor_->setPlainText(content);
}

void ScriptEditorWidget::runScript()
{
    if (!scriptEngine_) {
        outputWidget_->appendPlainText(tr("错误: 脚本引擎未初始化"));
        return;
    }
    
    if (isRunning_) {
        outputWidget_->appendPlainText(tr("警告: 脚本正在运行"));
        return;
    }
    
    isRunning_ = true;
    statusLabel_->setText(tr("运行中..."));
    statusLabel_->setStyleSheet("QLabel { background-color: #ffeb3b; padding: 2px; }");
    
    outputWidget_->appendPlainText(tr("\n--- 脚本开始执行 ---\n"));
    
    QString script = scriptContent();
    bool success = scriptEngine_->executeScript(script);
    
    if (!success) {
        outputWidget_->appendPlainText(tr("\n错误: %1").arg(scriptEngine_->lastError()));
    }
    
    outputWidget_->appendPlainText(tr("\n--- 脚本执行完成 ---\n"));
    
    isRunning_ = false;
    statusLabel_->setText(success ? tr("执行成功") : tr("执行失败"));
    statusLabel_->setStyleSheet(success ?
        "QLabel { background-color: #4caf50; color: white; padding: 2px; }" :
        "QLabel { background-color: #f44336; color: white; padding: 2px; }");
    
    emit scriptExecuted(success);
}

void ScriptEditorWidget::stopScript()
{
    if (!isRunning_) {
        return;
    }
    
    // 调用脚本引擎停止
    if (scriptEngine_) {
        scriptEngine_->stop();
    }
    
    outputWidget_->appendPlainText(tr("\n--- 正在停止脚本 ---\n"));
    
    isRunning_ = false;
    statusLabel_->setText(tr("已停止"));
    statusLabel_->setStyleSheet("QLabel { background-color: #ff9800; padding: 2px; }");
    
    emit scriptStopped();
}

void ScriptEditorWidget::clearOutput()
{
    outputWidget_->clear();
}

void ScriptEditorWidget::onRunClicked()
{
    runScript();
}

void ScriptEditorWidget::onStopClicked()
{
    stopScript();
}

void ScriptEditorWidget::onClearClicked()
{
    clearOutput();
}

void ScriptEditorWidget::onScriptModified()
{
    isModified_ = true;
    emit scriptModified();
}

void ScriptEditorWidget::onPrintOutput(const QString& message)
{
    outputWidget_->appendPlainText(message);
}

void ScriptEditorWidget::onScriptExecuted(const QString& script, bool success)
{
    Q_UNUSED(script);
    isRunning_ = false;
    statusLabel_->setText(success ? tr("执行成功") : tr("执行失败"));
}

} // namespace DeviceStudio
