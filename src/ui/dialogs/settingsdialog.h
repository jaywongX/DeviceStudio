/**
 * @file settingsdialog.h
 * @brief 设置对话框
 * @author DeviceStudio Team
 * @date 2026-03-16
 */

#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

namespace DeviceStudio {

/**
 * @brief 设置对话框
 * 
 * 包含常规设置、日志设置、通信设置等选项卡
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;
    
    /**
     * @brief 加载设置
     */
    void loadSettings();
    
    /**
     * @brief 保存设置
     */
    void saveSettings();

private slots:
    void onAccept();
    void onApply();
    void onReset();

private:
    void setupUI();
    void setupGeneralPage();
    void setupLogPage();
    void setupCommunicationPage();
    void setupConnections();
    
    // ========== 常规设置 ==========
    QWidget* generalPage_ = nullptr;
    QComboBox* themeCombo_ = nullptr;
    QComboBox* languageCombo_ = nullptr;
    QSpinBox* fontSizeSpin_ = nullptr;
    QCheckBox* autoSaveCheck_ = nullptr;
    QSpinBox* autoSaveIntervalSpin_ = nullptr;
    QCheckBox* restoreSessionCheck_ = nullptr;
    
    // ========== 日志设置 ==========
    QWidget* logPage_ = nullptr;
    QComboBox* logLevelCombo_ = nullptr;
    QLineEdit* logPathEdit_ = nullptr;
    QPushButton* logPathButton_ = nullptr;
    QSpinBox* maxLogSizeSpin_ = nullptr;
    QSpinBox* maxLogFilesSpin_ = nullptr;
    QCheckBox* logToConsoleCheck_ = nullptr;
    
    // ========== 通信设置 ==========
    QWidget* commPage_ = nullptr;
    QSpinBox* defaultTimeoutSpin_ = nullptr;
    QSpinBox* defaultRetrySpin_ = nullptr;
    QSpinBox* defaultBaudRateSpin_ = nullptr;
    QSpinBox* defaultDataBitsSpin_ = nullptr;
    QComboBox* defaultParityCombo_ = nullptr;
    QComboBox* defaultStopBitsCombo_ = nullptr;
    
    // 按钮
    QPushButton* okButton_ = nullptr;
    QPushButton* cancelButton_ = nullptr;
    QPushButton* applyButton_ = nullptr;
    QPushButton* resetButton_ = nullptr;
};

} // namespace DeviceStudio
