/**
 * @file settingsdialog.cpp
 * @brief 设置对话框实现
 * @author DeviceStudio Team
 * @date 2026-03-16
 */

#include "settingsdialog.h"
#include "config/configmanager.h"
#include "log/logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFileDialog>
#include <QSettings>
#include <QDialogButtonBox>

namespace DeviceStudio {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    setupConnections();
    loadSettings();
    
    setWindowTitle(tr("设置"));
    setMinimumSize(500, 400);
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建选项卡
    QTabWidget* tabWidget = new QTabWidget(this);
    
    // 设置各选项卡页面
    setupGeneralPage();
    setupLogPage();
    setupCommunicationPage();
    
    tabWidget->addTab(generalPage_, tr("常规"));
    tabWidget->addTab(logPage_, tr("日志"));
    tabWidget->addTab(commPage_, tr("通信"));
    
    mainLayout->addWidget(tabWidget);
    
    // 创建按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    okButton_ = new QPushButton(tr("确定"), this);
    cancelButton_ = new QPushButton(tr("取消"), this);
    applyButton_ = new QPushButton(tr("应用"), this);
    resetButton_ = new QPushButton(tr("重置"), this);
    
    buttonLayout->addWidget(okButton_);
    buttonLayout->addWidget(cancelButton_);
    buttonLayout->addWidget(applyButton_);
    buttonLayout->addWidget(resetButton_);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsDialog::setupGeneralPage()
{
    generalPage_ = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(generalPage_);
    
    // 外观设置
    QGroupBox* appearanceGroup = new QGroupBox(tr("外观"), generalPage_);
    QFormLayout* appearanceLayout = new QFormLayout(appearanceGroup);
    
    themeCombo_ = new QComboBox(generalPage_);
    themeCombo_->addItem(tr("亮色"), "light");
    themeCombo_->addItem(tr("暗色"), "dark");
    themeCombo_->addItem(tr("跟随系统"), "auto");
    appearanceLayout->addRow(tr("主题:"), themeCombo_);
    
    languageCombo_ = new QComboBox(generalPage_);
    languageCombo_->addItem(tr("简体中文"), "zh_CN");
    languageCombo_->addItem(tr("English"), "en_US");
    appearanceLayout->addRow(tr("语言:"), languageCombo_);
    
    fontSizeSpin_ = new QSpinBox(generalPage_);
    fontSizeSpin_->setRange(8, 24);
    fontSizeSpin_->setValue(12);
    appearanceLayout->addRow(tr("字体大小:"), fontSizeSpin_);
    
    layout->addWidget(appearanceGroup);
    
    // 自动保存设置
    QGroupBox* autoSaveGroup = new QGroupBox(tr("自动保存"), generalPage_);
    QFormLayout* autoSaveLayout = new QFormLayout(autoSaveGroup);
    
    autoSaveCheck_ = new QCheckBox(tr("启用自动保存"), autoSaveGroup);
    autoSaveLayout->addRow(autoSaveCheck_);
    
    autoSaveIntervalSpin_ = new QSpinBox(autoSaveGroup);
    autoSaveIntervalSpin_->setRange(10, 600);
    autoSaveIntervalSpin_->setValue(60);
    autoSaveIntervalSpin_->setSuffix(tr(" 秒"));
    autoSaveLayout->addRow(tr("保存间隔:"), autoSaveIntervalSpin_);
    
    layout->addWidget(autoSaveGroup);
    
    // 会话设置
    QGroupBox* sessionGroup = new QGroupBox(tr("会话"), generalPage_);
    QFormLayout* sessionLayout = new QFormLayout(sessionGroup);
    
    restoreSessionCheck_ = new QCheckBox(tr("启动时恢复上次会话"), sessionGroup);
    sessionLayout->addRow(restoreSessionCheck_);
    
    layout->addWidget(sessionGroup);
    layout->addStretch();
}

void SettingsDialog::setupLogPage()
{
    logPage_ = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(logPage_);
    
    // 日志级别
    QGroupBox* levelGroup = new QGroupBox(tr("日志级别"), logPage_);
    QFormLayout* levelLayout = new QFormLayout(levelGroup);
    
    logLevelCombo_ = new QComboBox(logPage_);
    logLevelCombo_->addItem(tr("调试"), "debug");
    logLevelCombo_->addItem(tr("信息"), "info");
    logLevelCombo_->addItem(tr("警告"), "warning");
    logLevelCombo_->addItem(tr("错误"), "error");
    levelLayout->addRow(tr("级别:"), logLevelCombo_);
    
    layout->addWidget(levelGroup);
    
    // 日志文件
    QGroupBox* fileGroup = new QGroupBox(tr("日志文件"), logPage_);
    QFormLayout* fileLayout = new QFormLayout(fileGroup);
    
    QHBoxLayout* pathLayout = new QHBoxLayout();
    logPathEdit_ = new QLineEdit(logPage_);
    logPathButton_ = new QPushButton(tr("浏览..."), logPage_);
    pathLayout->addWidget(logPathEdit_);
    pathLayout->addWidget(logPathButton_);
    fileLayout->addRow(tr("路径:"), pathLayout);
    
    maxLogSizeSpin_ = new QSpinBox(logPage_);
    maxLogSizeSpin_->setRange(1, 1000);
    maxLogSizeSpin_->setValue(100);
    maxLogSizeSpin_->setSuffix(tr(" MB"));
    fileLayout->addRow(tr("最大大小:"), maxLogSizeSpin_);
    
    maxLogFilesSpin_ = new QSpinBox(logPage_);
    maxLogFilesSpin_->setRange(1, 100);
    maxLogFilesSpin_->setValue(10);
    maxLogFilesSpin_->setSuffix(tr(" 个"));
    fileLayout->addRow(tr("最大文件数:"), maxLogFilesSpin_);
    
    layout->addWidget(fileGroup);
    
    // 控制台输出
    QGroupBox* consoleGroup = new QGroupBox(tr("控制台"), logPage_);
    QFormLayout* consoleLayout = new QFormLayout(consoleGroup);
    
    logToConsoleCheck_ = new QCheckBox(tr("输出到控制台"), consoleGroup);
    consoleLayout->addRow(logToConsoleCheck_);
    
    layout->addWidget(consoleGroup);
    layout->addStretch();
}

void SettingsDialog::setupCommunicationPage()
{
    commPage_ = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(commPage_);
    
    // 默认超时设置
    QGroupBox* timeoutGroup = new QGroupBox(tr("超时设置"), commPage_);
    QFormLayout* timeoutLayout = new QFormLayout(timeoutGroup);
    
    defaultTimeoutSpin_ = new QSpinBox(commPage_);
    defaultTimeoutSpin_->setRange(100, 60000);
    defaultTimeoutSpin_->setValue(5000);
    defaultTimeoutSpin_->setSuffix(tr(" 毫秒"));
    timeoutLayout->addRow(tr("默认超时:"), defaultTimeoutSpin_);
    
    defaultRetrySpin_ = new QSpinBox(commPage_);
    defaultRetrySpin_->setRange(0, 10);
    defaultRetrySpin_->setValue(3);
    defaultRetrySpin_->setSuffix(tr(" 次"));
    timeoutLayout->addRow(tr("重试次数:"), defaultRetrySpin_);
    
    layout->addWidget(timeoutGroup);
    
    // 默认串口设置
    QGroupBox* serialGroup = new QGroupBox(tr("默认串口设置"), commPage_);
    QFormLayout* serialLayout = new QFormLayout(serialGroup);
    
    defaultBaudRateSpin_ = new QSpinBox(commPage_);
    defaultBaudRateSpin_->setRange(300, 921600);
    defaultBaudRateSpin_->setValue(115200);
    serialLayout->addRow(tr("波特率:"), defaultBaudRateSpin_);
    
    defaultDataBitsSpin_ = new QSpinBox(commPage_);
    defaultDataBitsSpin_->setRange(5, 8);
    defaultDataBitsSpin_->setValue(8);
    serialLayout->addRow(tr("数据位:"), defaultDataBitsSpin_);
    
    defaultParityCombo_ = new QComboBox(commPage_);
    defaultParityCombo_->addItem(tr("无"), "none");
    defaultParityCombo_->addItem(tr("奇校验"), "odd");
    defaultParityCombo_->addItem(tr("偶校验"), "even");
    serialLayout->addRow(tr("校验位:"), defaultParityCombo_);
    
    defaultStopBitsCombo_ = new QComboBox(commPage_);
    defaultStopBitsCombo_->addItem("1", 1);
    defaultStopBitsCombo_->addItem("1.5", 1.5);
    defaultStopBitsCombo_->addItem("2", 2);
    serialLayout->addRow(tr("停止位:"), defaultStopBitsCombo_);
    
    layout->addWidget(serialGroup);
    layout->addStretch();
}

void SettingsDialog::setupConnections()
{
    connect(okButton_, &QPushButton::clicked, this, &SettingsDialog::onAccept);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    connect(applyButton_, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(resetButton_, &QPushButton::clicked, this, &SettingsDialog::onReset);
    connect(logPathButton_, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, tr("选择日志路径"));
        if (!path.isEmpty()) {
            logPathEdit_->setText(path);
        }
    });
    
    connect(autoSaveCheck_, &QCheckBox::toggled, autoSaveIntervalSpin_, &QSpinBox::setEnabled);
}

void SettingsDialog::loadSettings()
{
    QSettings settings;
    
    // 常规设置
    settings.beginGroup("General");
    QString theme = settings.value("theme", "light").toString();
    int themeIndex = themeCombo_->findData(theme);
    if (themeIndex >= 0) themeCombo_->setCurrentIndex(themeIndex);
    
    QString language = settings.value("language", "zh_CN").toString();
    int langIndex = languageCombo_->findData(language);
    if (langIndex >= 0) languageCombo_->setCurrentIndex(langIndex);
    
    fontSizeSpin_->setValue(settings.value("fontSize", 12).toInt());
    autoSaveCheck_->setChecked(settings.value("autoSave", true).toBool());
    autoSaveIntervalSpin_->setValue(settings.value("autoSaveInterval", 60).toInt());
    restoreSessionCheck_->setChecked(settings.value("restoreSession", true).toBool());
    settings.endGroup();
    
    // 日志设置
    settings.beginGroup("Log");
    QString logLevel = settings.value("level", "info").toString();
    int levelIndex = logLevelCombo_->findData(logLevel);
    if (levelIndex >= 0) logLevelCombo_->setCurrentIndex(levelIndex);
    
    logPathEdit_->setText(settings.value("path", "logs/").toString());
    maxLogSizeSpin_->setValue(settings.value("maxSizeMB", 100).toInt());
    maxLogFilesSpin_->setValue(settings.value("maxFiles", 10).toInt());
    logToConsoleCheck_->setChecked(settings.value("toConsole", true).toBool());
    settings.endGroup();
    
    // 通信设置
    settings.beginGroup("Communication");
    defaultTimeoutSpin_->setValue(settings.value("timeout", 5000).toInt());
    defaultRetrySpin_->setValue(settings.value("retryCount", 3).toInt());
    defaultBaudRateSpin_->setValue(settings.value("defaultBaudRate", 115200).toInt());
    defaultDataBitsSpin_->setValue(settings.value("defaultDataBits", 8).toInt());
    
    QString parity = settings.value("defaultParity", "none").toString();
    int parityIndex = defaultParityCombo_->findData(parity);
    if (parityIndex >= 0) defaultParityCombo_->setCurrentIndex(parityIndex);
    
    double stopBits = settings.value("defaultStopBits", 1).toDouble();
    for (int i = 0; i < defaultStopBitsCombo_->count(); ++i) {
        if (qFuzzyCompare(defaultStopBitsCombo_->itemData(i).toDouble(), stopBits)) {
            defaultStopBitsCombo_->setCurrentIndex(i);
            break;
        }
    }
    settings.endGroup();
    
    autoSaveIntervalSpin_->setEnabled(autoSaveCheck_->isChecked());
}

void SettingsDialog::saveSettings()
{
    QSettings settings;
    
    // 常规设置
    settings.beginGroup("General");
    settings.setValue("theme", themeCombo_->currentData());
    settings.setValue("language", languageCombo_->currentData());
    settings.setValue("fontSize", fontSizeSpin_->value());
    settings.setValue("autoSave", autoSaveCheck_->isChecked());
    settings.setValue("autoSaveInterval", autoSaveIntervalSpin_->value());
    settings.setValue("restoreSession", restoreSessionCheck_->isChecked());
    settings.endGroup();
    
    // 日志设置
    settings.beginGroup("Log");
    settings.setValue("level", logLevelCombo_->currentData());
    settings.setValue("path", logPathEdit_->text());
    settings.setValue("maxSizeMB", maxLogSizeSpin_->value());
    settings.setValue("maxFiles", maxLogFilesSpin_->value());
    settings.setValue("toConsole", logToConsoleCheck_->isChecked());
    settings.endGroup();
    
    // 通信设置
    settings.beginGroup("Communication");
    settings.setValue("timeout", defaultTimeoutSpin_->value());
    settings.setValue("retryCount", defaultRetrySpin_->value());
    settings.setValue("defaultBaudRate", defaultBaudRateSpin_->value());
    settings.setValue("defaultDataBits", defaultDataBitsSpin_->value());
    settings.setValue("defaultParity", defaultParityCombo_->currentData());
    settings.setValue("defaultStopBits", defaultStopBitsCombo_->currentData());
    settings.endGroup();
    
    settings.sync();
    DS_LOG_INFO("Settings saved");
}

void SettingsDialog::onAccept()
{
    saveSettings();
    accept();
}

void SettingsDialog::onApply()
{
    saveSettings();
}

void SettingsDialog::onReset()
{
    // 重置为默认值
    themeCombo_->setCurrentIndex(0);
    languageCombo_->setCurrentIndex(0);
    fontSizeSpin_->setValue(12);
    autoSaveCheck_->setChecked(true);
    autoSaveIntervalSpin_->setValue(60);
    restoreSessionCheck_->setChecked(true);
    
    logLevelCombo_->setCurrentIndex(1); // info
    logPathEdit_->setText("logs/");
    maxLogSizeSpin_->setValue(100);
    maxLogFilesSpin_->setValue(10);
    logToConsoleCheck_->setChecked(true);
    
    defaultTimeoutSpin_->setValue(5000);
    defaultRetrySpin_->setValue(3);
    defaultBaudRateSpin_->setValue(115200);
    defaultDataBitsSpin_->setValue(8);
    defaultParityCombo_->setCurrentIndex(0);
    defaultStopBitsCombo_->setCurrentIndex(0);
}

} // namespace DeviceStudio
