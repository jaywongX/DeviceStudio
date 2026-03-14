/**
 * @file mainwindow.h
 * @brief 主窗口类
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <memory>

namespace DeviceStudio {

// 前向声明
class DevicePanel;
class TerminalWidget;
class ChartWidget;
class GaugeWidget;
class ScriptEditorWidget;
class DataMonitorPanel;
class DeviceManager;

/**
 * @brief 主窗口类
 * 
 * 应用程序的主窗口，包含菜单栏、工具栏、设备面板和工作区
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    
    /**
     * @brief 初始化窗口
     */
    void init();

protected:
    /**
     * @brief 关闭事件
     */
    void closeEvent(QCloseEvent* event) override;

private:
    /**
     * @brief 设置UI
     */
    void setupUi();
    
    /**
     * @brief 设置菜单栏
     */
    void setupMenuBar();
    
    /**
     * @brief 设置工具栏
     */
    void setupToolBar();
    
    /**
     * @brief 设置状态栏
     */
    void setupStatusBar();
    
    /**
     * @brief 设置停靠窗口
     */
    void setupDockWidgets();
    
    /**
     * @brief 设置中心部件
     */
    void setupCentralWidget();
    
    /**
     * @brief 创建菜单动作
     */
    void createActions();
    
    /**
     * @brief 加载窗口设置
     */
    void loadSettings();
    
    /**
     * @brief 保存窗口设置
     */
    void saveSettings();
    
    /**
     * @brief 连接信号槽
     */
    void connectSignals();

private slots:
    /**
     * @brief 新建项目
     */
    void onNewProject();
    
    /**
     * @brief 打开项目
     */
    void onOpenProject();
    
    /**
     * @brief 保存项目
     */
    void onSaveProject();
    
    /**
     * @brief 打开设置
     */
    void onOpenSettings();
    
    /**
     * @brief 打开关于对话框
     */
    void onAbout();
    
    /**
     * @brief 设备连接状态改变
     */
    void onDeviceConnectionChanged(bool connected);
    
    /**
     * @brief 添加设备
     */
    void onAddDevice();
    
    /**
     * @brief 运行脚本
     */
    void onRunScript();
    
    /**
     * @brief 停止脚本
     */
    void onStopScript();
    
    /**
     * @brief 数据接收
     */
    void onDataReceived(const QByteArray& data);
    
    /**
     * @brief 更新状态栏
     */
    void updateStatusBar();

private:
    // 核心组件
    std::shared_ptr<DeviceManager> deviceManager_;
    
    // UI 组件
    QTabWidget* centralTabWidget_ = nullptr;
    DevicePanel* devicePanel_ = nullptr;
    TerminalWidget* terminalWidget_ = nullptr;
    ChartWidget* chartWidget_ = nullptr;
    GaugeWidget* gaugeWidget_ = nullptr;
    ScriptEditorWidget* scriptEditor_ = nullptr;
    DataMonitorPanel* dataMonitorPanel_ = nullptr;
    
    // 菜单动作
    QAction* newAction_ = nullptr;
    QAction* openAction_ = nullptr;
    QAction* saveAction_ = nullptr;
    QAction* exitAction_ = nullptr;
    QAction* settingsAction_ = nullptr;
    QAction* aboutAction_ = nullptr;
    QAction* aboutQtAction_ = nullptr;
    QAction* addDeviceAction_ = nullptr;
    QAction* runScriptAction_ = nullptr;
    QAction* stopScriptAction_ = nullptr;
};

} // namespace DeviceStudio
