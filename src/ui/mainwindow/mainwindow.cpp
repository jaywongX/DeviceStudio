/**
 * @file mainwindow.cpp
 * @brief 主窗口实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "mainwindow.h"
#include "devicepanel/devicepanel.h"
#include "terminal/terminalwidget.h"
#include "log/logger.h"
#include "config/configmanager.h"
#include "visualization/chart/chartwidget.h"
#include "visualization/gauge/gaugewidget.h"
#include "scripteditor/scripteditorwidget.h"
#include "monitor/datamonitorpanel.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QDockWidget>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>

namespace DeviceStudio {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    init();
}

MainWindow::~MainWindow()
{
    saveSettings();
    DS_LOG_INFO("MainWindow destroyed");
}

void MainWindow::init()
{
    setupUi();
    loadSettings();
    
    DS_LOG_INFO("MainWindow initialized");
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // TODO: 检查是否有未保存的数据
    event->accept();
}

void MainWindow::setupUi()
{
    // 设置窗口属性
    setWindowTitle("DeviceStudio");
    setWindowIcon(QIcon(":/icons/app_icon.png"));
    setMinimumSize(800, 600);
    
    // 创建菜单栏
    setupMenuBar();
    
    // 创建工具栏
    setupToolBar();
    
    // 创建状态栏
    setupStatusBar();
    
    // 创建停靠窗口
    setupDockWidgets();
    
    // 创建中心部件
    setupCentralWidget();
}

void MainWindow::setupMenuBar()
{
    // ========== 文件菜单 ==========
    QMenu* fileMenu = menuBar()->addMenu(tr("&文件"));
    
    newAction_ = new QAction(QIcon(":/icons/new.png"), tr("新建项目(&N)"), this);
    newAction_->setShortcut(QKeySequence::New);
    newAction_->setStatusTip(tr("创建新项目"));
    connect(newAction_, &QAction::triggered, this, &MainWindow::onNewProject);
    fileMenu->addAction(newAction_);
    
    openAction_ = new QAction(QIcon(":/icons/open.png"), tr("打开项目(&O)"), this);
    openAction_->setShortcut(QKeySequence::Open);
    openAction_->setStatusTip(tr("打开现有项目"));
    connect(openAction_, &QAction::triggered, this, &MainWindow::onOpenProject);
    fileMenu->addAction(openAction_);
    
    saveAction_ = new QAction(QIcon(":/icons/save.png"), tr("保存项目(&S)"), this);
    saveAction_->setShortcut(QKeySequence::Save);
    saveAction_->setStatusTip(tr("保存当前项目"));
    connect(saveAction_, &QAction::triggered, this, &MainWindow::onSaveProject);
    fileMenu->addAction(saveAction_);
    
    fileMenu->addSeparator();
    
    exitAction_ = new QAction(tr("退出(&X)"), this);
    exitAction_->setShortcut(QKeySequence::Quit);
    exitAction_->setStatusTip(tr("退出应用程序"));
    connect(exitAction_, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction_);
    
    // ========== 编辑菜单 ==========
    QMenu* editMenu = menuBar()->addMenu(tr("&编辑"));
    editMenu->addAction(tr("剪切(&X)"), this, [](){}, QKeySequence::Cut);
    editMenu->addAction(tr("复制(&C)"), this, [](){}, QKeySequence::Copy);
    editMenu->addAction(tr("粘贴(&V)"), this, [](){}, QKeySequence::Paste);
    
    // ========== 设备菜单 ==========
    QMenu* deviceMenu = menuBar()->addMenu(tr("&设备"));
    
    addDeviceAction_ = new QAction(QIcon(":/icons/add_device.png"), tr("添加设备(&A)"), this);
    addDeviceAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    addDeviceAction_->setStatusTip(tr("添加新设备"));
    connect(addDeviceAction_, &QAction::triggered, this, &MainWindow::onAddDevice);
    deviceMenu->addAction(addDeviceAction_);
    
    // ========== 脚本菜单 ==========
    QMenu* scriptMenu = menuBar()->addMenu(tr("&脚本"));
    
    runScriptAction_ = new QAction(QIcon(":/icons/run.png"), tr("运行脚本(&R)"), this);
    runScriptAction_->setShortcut(QKeySequence(Qt::Key_F5));
    runScriptAction_->setStatusTip(tr("运行当前脚本"));
    connect(runScriptAction_, &QAction::triggered, this, &MainWindow::onRunScript);
    scriptMenu->addAction(runScriptAction_);
    
    stopScriptAction_ = new QAction(QIcon(":/icons/stop.png"), tr("停止脚本(&S)"), this);
    stopScriptAction_->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));
    stopScriptAction_->setStatusTip(tr("停止当前运行的脚本"));
    connect(stopScriptAction_, &QAction::triggered, this, &MainWindow::onStopScript);
    scriptMenu->addAction(stopScriptAction_);
    
    // ========== 视图菜单 ==========
    QMenu* viewMenu = menuBar()->addMenu(tr("&视图"));
    // 视图菜单会自动填充停靠窗口的切换动作
    
    // ========== 工具菜单 ==========
    QMenu* toolsMenu = menuBar()->addMenu(tr("&工具"));
    
    settingsAction_ = new QAction(QIcon(":/icons/settings.png"), tr("设置(&S)"), this);
    settingsAction_->setShortcut(QKeySequence::Preferences);
    settingsAction_->setStatusTip(tr("打开应用程序设置"));
    connect(settingsAction_, &QAction::triggered, this, &MainWindow::onOpenSettings);
    toolsMenu->addAction(settingsAction_);
    
    // ========== 帮助菜单 ==========
    QMenu* helpMenu = menuBar()->addMenu(tr("&帮助"));
    
    aboutAction_ = new QAction(tr("关于 DeviceStudio(&A)"), this);
    aboutAction_->setStatusTip(tr("显示应用程序信息"));
    connect(aboutAction_, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction_);
    
    aboutQtAction_ = new QAction(tr("关于 Qt(&Q)"), this);
    aboutQtAction_->setStatusTip(tr("显示 Qt 信息"));
    connect(aboutQtAction_, &QAction::triggered, qApp, &QApplication::aboutQt);
    helpMenu->addAction(aboutQtAction_);
}

void MainWindow::setupToolBar()
{
    QToolBar* mainToolBar = addToolBar(tr("主工具栏"));
    mainToolBar->setMovable(false);
    
    mainToolBar->addAction(newAction_);
    mainToolBar->addAction(openAction_);
    mainToolBar->addAction(saveAction_);
    mainToolBar->addSeparator();
    mainToolBar->addAction(settingsAction_);
}

void MainWindow::setupStatusBar()
{
    QStatusBar* statusBar = this->statusBar();
    statusBar->showMessage(tr("就绪"));
}

void MainWindow::setupDockWidgets()
{
    // 设备列表面板
    devicePanel_ = new DevicePanel(this);
    QDockWidget* deviceDock = new QDockWidget(tr("设备列表"), this);
    deviceDock->setWidget(devicePanel_);
    deviceDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, deviceDock);
    
    // 连接设备连接状态信号
    connect(devicePanel_, &DevicePanel::deviceConnectionChanged, 
            this, &MainWindow::onDeviceConnectionChanged);
}

void MainWindow::setupCentralWidget()
{
    // 创建中心 Tab 组件
    centralTabWidget_ = new QTabWidget(this);
    centralTabWidget_->setTabsClosable(false);
    centralTabWidget_->setDocumentMode(true);
    
    // 数据终端
    terminalWidget_ = new TerminalWidget(this);
    centralTabWidget_->addTab(terminalWidget_, QIcon(":/icons/terminal.png"), tr("数据终端"));
    
    // 图表组件
    chartWidget_ = new ChartWidget(this);
    centralTabWidget_->addTab(chartWidget_, QIcon(":/icons/chart.png"), tr("曲线图表"));
    
    // 仪表盘组件
    gaugeWidget_ = new GaugeWidget(this);
    centralTabWidget_->addTab(gaugeWidget_, QIcon(":/icons/gauge.png"), tr("仪表盘"));
    
    // 脚本编辑器
    scriptEditor_ = new ScriptEditorWidget(this);
    centralTabWidget_->addTab(scriptEditor_, QIcon(":/icons/script.png"), tr("脚本编辑"));
    
    // 数据监控面板
    dataMonitorPanel_ = new DataMonitorPanel(this);
    centralTabWidget_->addTab(dataMonitorPanel_, QIcon(":/icons/monitor.png"), tr("数据监控"));
    
    setCentralWidget(centralTabWidget_);
}

void MainWindow::createActions()
{
    // 动作已在 setupMenuBar 中创建
}

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    
    // 恢复窗口几何
    restoreGeometry(settings.value("geometry").toByteArray());
    
    // 恢复窗口状态
    restoreState(settings.value("state").toByteArray());
    
    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    
    // 保存窗口几何
    settings.setValue("geometry", saveGeometry());
    
    // 保存窗口状态
    settings.setValue("state", saveState());
    
    settings.endGroup();
    
    settings.sync();
}

void MainWindow::onNewProject()
{
    DS_LOG_INFO("New project requested");
    // TODO: 实现新建项目逻辑
    statusBar()->showMessage(tr("新建项目"), 2000);
}

void MainWindow::onOpenProject()
{
    DS_LOG_INFO("Open project requested");
    // TODO: 实现打开项目逻辑
    statusBar()->showMessage(tr("打开项目"), 2000);
}

void MainWindow::onSaveProject()
{
    DS_LOG_INFO("Save project requested");
    // TODO: 实现保存项目逻辑
    statusBar()->showMessage(tr("项目已保存"), 2000);
}

void MainWindow::onOpenSettings()
{
    DS_LOG_INFO("Settings dialog requested");
    // TODO: 实现设置对话框
    QMessageBox::information(this, tr("设置"), tr("设置对话框待实现"));
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("关于 DeviceStudio"),
        tr("<h3>DeviceStudio v1.0.0</h3>"
           "<p>通用设备调试与数据分析平台</p>"
           "<p>面向嵌入式开发、物联网开发、硬件工程师的专业工具</p>"
           "<p>© 2026 DeviceStudio Team. All rights reserved.</p>"));
}

void MainWindow::onDeviceConnectionChanged(bool connected)
{
    QString status = connected ? tr("设备已连接") : tr("设备已断开");
    statusBar()->showMessage(status, 3000);
    DS_LOG_INFO("Device connection changed: " + status.toStdString());
}

void MainWindow::onAddDevice()
{
    DS_LOG_INFO("Add device requested");
    statusBar()->showMessage(tr("添加设备"), 2000);
    // TODO: 打开设备配置对话框
}

void MainWindow::onRunScript()
{
    DS_LOG_INFO("Run script requested");
    statusBar()->showMessage(tr("运行脚本"), 2000);
    // TODO: 运行脚本
}

void MainWindow::onStopScript()
{
    DS_LOG_INFO("Stop script requested");
    statusBar()->showMessage(tr("停止脚本"), 2000);
    // TODO: 停止脚本
}

void MainWindow::onDataReceived(const QByteArray& data)
{
    // 转发数据到各个组件
    if (chartWidget_) {
        // TODO: 更新图表数据
    }
    
    if (dataMonitorPanel_) {
        // TODO: 更新数据监控面板
    }
}

void MainWindow::updateStatusBar()
{
    // TODO: 更新状态栏信息
}

} // namespace DeviceStudio
