/**
 * @file mainwindow.cpp
 * @brief 主窗口实现
 * @author DeviceStudio Team
 * @date 2025-02-14
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
#include "dialogs/deviceconfigdialog.h"
#include "dialogs/settingsdialog.h"
#include "core/project/project.h"
#include "core/devicemanager/devicemanager.h"
#include "core/devicemanager/idevice.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QDockWidget>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDateTime>
#include <QIcon>
#include <QFileInfo>

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
    // 初始化设备管理器
    deviceManager_ = std::make_shared<DeviceManager>(this);
    
    // 初始化项目管理器
    projectManager_ = ProjectManager::instance();
    
    // 连接项目管理器信号
    connect(projectManager_, &ProjectManager::projectOpened, this, [this](const QString& path) {
        updateWindowTitle();
        statusBar()->showMessage(tr("项目已打开: %1").arg(path), 3000);
    });
    
    connect(projectManager_, &ProjectManager::projectSaved, this, [this](const QString& path) {
        updateWindowTitle();
        statusBar()->showMessage(tr("项目已保存: %1").arg(path), 3000);
    });
    
    connect(projectManager_, &ProjectManager::projectModified, this, [this](bool modified) {
        updateWindowTitle();
    });
    
    setupUi();
    loadSettings();
    updateWindowTitle();
    
    // 连接设备管理器数据信号
    connect(deviceManager_.get(), &DeviceManager::deviceDataReceived,
            this, [this](const QString& deviceId, const QByteArray& data) {
        Q_UNUSED(deviceId);
        onDataReceived(data);
    });
    
    connect(deviceManager_.get(), &DeviceManager::deviceStateChanged,
            this, [this](const QString& deviceId, DeviceState state) {
        updateStatusBar();
    });
    
    DS_LOG_INFO("MainWindow initialized");
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!checkUnsavedChanges()) {
        event->ignore();
        return;
    }
    
    // 关闭当前项目
    if (projectManager_ && projectManager_->hasOpenProject()) {
        projectManager_->closeProject();
    }
    
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
    
    QAction* cutAction = new QAction(tr("剪切(&X)"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setStatusTip(tr("剪切选中内容"));
    connect(cutAction, &QAction::triggered, this, &MainWindow::onCut);
    editMenu->addAction(cutAction);
    
    QAction* copyAction = new QAction(tr("复制(&C)"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setStatusTip(tr("复制选中内容"));
    connect(copyAction, &QAction::triggered, this, &MainWindow::onCopy);
    editMenu->addAction(copyAction);
    
    QAction* pasteAction = new QAction(tr("粘贴(&V)"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setStatusTip(tr("粘贴剪贴板内容"));
    connect(pasteAction, &QAction::triggered, this, &MainWindow::onPaste);
    editMenu->addAction(pasteAction);
    
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
    devicePanel_->setDeviceManager(deviceManager_);
    
    QDockWidget* deviceDock = new QDockWidget(tr("设备列表"), this);
    deviceDock->setWidget(devicePanel_);
    deviceDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, deviceDock);
    
    // 连接设备连接状态信号
    connect(devicePanel_, &DevicePanel::deviceConnectionChanged, 
            this, &MainWindow::onDeviceConnectionChanged);
    
    // 连接添加设备请求信号
    connect(devicePanel_, &DevicePanel::addDeviceRequested,
            this, &MainWindow::onAddDevice);
    
    // 连接设备选中信号
    connect(devicePanel_, &DevicePanel::deviceSelected,
            this, [this](const QString& deviceId) {
        // 切换到数据终端标签页
        if (terminalWidget_) {
            int tabIndex = centralTabWidget_->indexOf(terminalWidget_);
            if (tabIndex >= 0) {
                centralTabWidget_->setCurrentIndex(tabIndex);
            }
        }
        DS_LOG_INFO("Device selected: " + deviceId.toStdString());
    });
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
    
    // 检查未保存的更改
    if (!checkUnsavedChanges()) {
        return;
    }
    
    // 获取保存路径
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("新建项目"),
        QString(),
        tr("DeviceStudio 项目 (*.dsproj)")
    );
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 确保文件扩展名
    if (!filePath.endsWith(".dsproj", Qt::CaseInsensitive)) {
        filePath += ".dsproj";
    }
    
    // 创建默认配置
    ProjectConfig config;
    config.name = QFileInfo(filePath).baseName();
    config.description = tr("新建项目");
    config.author = tr("用户");
    config.version = "1.0.0";
    config.createTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    config.modifyTime = config.createTime;
    
    // 创建项目
    if (projectManager_->createProject(filePath, config)) {
        DS_LOG_INFO("Project created: " + filePath.toStdString());
        statusBar()->showMessage(tr("项目已创建: %1").arg(config.name), 3000);
    } else {
        QMessageBox::warning(this, tr("错误"), tr("无法创建项目"));
        DS_LOG_ERROR("Failed to create project: " + filePath.toStdString());
    }
}

void MainWindow::onOpenProject()
{
    DS_LOG_INFO("Open project requested");
    
    // 检查未保存的更改
    if (!checkUnsavedChanges()) {
        return;
    }
    
    // 获取文件路径
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("打开项目"),
        QString(),
        tr("DeviceStudio 项目 (*.dsproj);;所有文件 (*.*)")
    );
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 打开项目
    if (projectManager_->openProject(filePath)) {
        DS_LOG_INFO("Project opened: " + filePath.toStdString());
    } else {
        QMessageBox::warning(this, tr("错误"), tr("无法打开项目: %1").arg(filePath));
        DS_LOG_ERROR("Failed to open project: " + filePath.toStdString());
    }
}

void MainWindow::onSaveProject()
{
    DS_LOG_INFO("Save project requested");
    
    if (!projectManager_ || !projectManager_->hasOpenProject()) {
        // 没有打开的项目，提示用户
        QMessageBox::information(this, tr("提示"), tr("没有打开的项目"));
        return;
    }
    
    if (projectManager_->saveProject()) {
        DS_LOG_INFO("Project saved: " + projectManager_->projectPath().toStdString());
        statusBar()->showMessage(tr("项目已保存"), 2000);
    } else {
        QMessageBox::warning(this, tr("错误"), tr("保存项目失败"));
        DS_LOG_ERROR("Failed to save project");
    }
}

void MainWindow::onOpenSettings()
{
    DS_LOG_INFO("Settings dialog requested");
    
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 设置已保存，可能需要重新加载某些配置
        statusBar()->showMessage(tr("设置已保存"), 2000);
    }
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
    
    // 打开设备配置对话框
    DeviceConfigDialog dialog(this);
    dialog.setWindowTitle(tr("添加设备"));
    
    if (dialog.exec() == QDialog::Accepted) {
        // 获取配置
        QString name = dialog.deviceName();
        DeviceTypeConfig configType = dialog.deviceType();
        QVariantMap config = dialog.configuration();
        
        // 转换设备类型
        DeviceType deviceType = convertDeviceType(configType);
        
        // 创建设备
        auto device = deviceManager_->createDevice(deviceType, name, config);
        if (device) {
            if (deviceManager_->addDevice(device)) {
                statusBar()->showMessage(tr("设备已添加: %1").arg(name), 3000);
                DS_LOG_INFO("Device added: " + name.toStdString());
            } else {
                QMessageBox::warning(this, tr("错误"), tr("添加设备失败"));
                DS_LOG_ERROR("Failed to add device to manager");
            }
        } else {
            QMessageBox::warning(this, tr("错误"), tr("创建设备失败"));
            DS_LOG_ERROR("Failed to create device");
        }
    }
}

DeviceType MainWindow::convertDeviceType(DeviceTypeConfig configType)
{
    switch (configType) {
    case DeviceTypeConfig::Serial:
        return DeviceType::Serial;
    case DeviceTypeConfig::TcpClient:
        return DeviceType::TcpClient;
    case DeviceTypeConfig::TcpServer:
        return DeviceType::TcpServer;
    case DeviceTypeConfig::Udp:
        return DeviceType::Udp;
    case DeviceTypeConfig::ModbusRtu:
        return DeviceType::ModbusRtu;
    case DeviceTypeConfig::ModbusTcp:
        return DeviceType::ModbusTcp;
    default:
        return DeviceType::Custom;
    }
}

void MainWindow::onRunScript()
{
    DS_LOG_INFO("Run script requested");
    
    if (!scriptEditor_) {
        DS_LOG_ERROR("Script editor not initialized");
        return;
    }
    
    // 切换到脚本编辑器标签页
    int scriptTabIndex = centralTabWidget_->indexOf(scriptEditor_);
    if (scriptTabIndex >= 0) {
        centralTabWidget_->setCurrentIndex(scriptTabIndex);
    }
    
    // 运行脚本
    scriptEditor_->runScript();
    statusBar()->showMessage(tr("脚本运行中..."), 2000);
}

void MainWindow::onStopScript()
{
    DS_LOG_INFO("Stop script requested");
    
    if (!scriptEditor_) {
        return;
    }
    
    // 停止脚本
    scriptEditor_->stopScript();
    statusBar()->showMessage(tr("脚本已停止"), 2000);
}

void MainWindow::onCut()
{
    QWidget* focusWidget = QApplication::focusWidget();
    if (!focusWidget) {
        return;
    }
    
    // 尝试调用剪切方法
    if (QMetaObject::invokeMethod(focusWidget, "cut", Qt::DirectConnection)) {
        DS_LOG_DEBUG("Cut performed on focused widget");
    }
}

void MainWindow::onCopy()
{
    QWidget* focusWidget = QApplication::focusWidget();
    if (!focusWidget) {
        return;
    }
    
    // 尝试调用复制方法
    if (QMetaObject::invokeMethod(focusWidget, "copy", Qt::DirectConnection)) {
        DS_LOG_DEBUG("Copy performed on focused widget");
    }
}

void MainWindow::onPaste()
{
    QWidget* focusWidget = QApplication::focusWidget();
    if (!focusWidget) {
        return;
    }
    
    // 尝试调用粘贴方法
    if (QMetaObject::invokeMethod(focusWidget, "paste", Qt::DirectConnection)) {
        DS_LOG_DEBUG("Paste performed on focused widget");
    }
}

void MainWindow::onDataReceived(const QByteArray& data)
{
    // 记录接收统计
    static qint64 totalReceived = 0;
    totalReceived += data.size();
    
    // 更新状态栏
    statusBar()->showMessage(tr("接收: %1 字节 (总计: %2)")
        .arg(data.size()).arg(totalReceived), 2000);
    
    // 转发数据到图表组件
    if (chartWidget_) {
        // 尝试解析数据为数值（简单的实时曲线示例）
        // 实际应用中应该使用协议解析引擎
        if (data.size() >= 4) {
            // 假设数据是4字节浮点数
            float value = 0;
            memcpy(&value, data.constData(), qMin(4, data.size()));
            
            // 更新图表（使用默认通道0）
            static int graphIndex = -1;
            if (graphIndex < 0) {
                graphIndex = chartWidget_->addGraph(tr("数据"), Qt::blue);
            }
            chartWidget_->addDataPoint(graphIndex, value);
        }
    }
    
    // 转发数据到数据监控面板
    if (dataMonitorPanel_) {
        // 添加原始数据显示
        QString hexData;
        for (int i = 0; i < qMin(16, data.size()); ++i) {
            hexData += QString("%1 ").arg((unsigned char)data[i], 2, 16, QChar('0')).toUpper();
        }
        if (data.size() > 16) hexData += "...";
        
        dataMonitorPanel_->updateDataItem("last_received", hexData);
        dataMonitorPanel_->updateDataItem("bytes_received", totalReceived);
    }
}

void MainWindow::updateStatusBar()
{
    if (!deviceManager_) {
        return;
    }
    
    int connectedCount = 0;
    int totalCount = deviceManager_->deviceCount();
    
    auto devices = deviceManager_->getAllDevices();
    for (const auto& device : devices) {
        if (device && device->isConnected()) {
            connectedCount++;
        }
    }
    
    QString status = tr("设备: %1/%2 已连接").arg(connectedCount).arg(totalCount);
    statusBar()->showMessage(status);
}

bool MainWindow::checkUnsavedChanges()
{
    if (!projectManager_ || !projectManager_->hasOpenProject()) {
        return true;
    }
    
    if (!projectManager_->isModified()) {
        return true;
    }
    
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        tr("未保存的更改"),
        tr("项目有未保存的更改，是否保存？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );
    
    switch (result) {
    case QMessageBox::Save:
        return projectManager_->saveProject();
    case QMessageBox::Discard:
        return true;
    default:
        return false;
    }
}

void MainWindow::updateWindowTitle()
{
    QString title = "DeviceStudio";
    
    if (projectManager_ && projectManager_->hasOpenProject()) {
        QString projectName = projectManager_->config().name;
        QString modifiedMark = projectManager_->isModified() ? "*" : "";
        title = QString("%1%2 - DeviceStudio").arg(projectName, modifiedMark);
    }
    
    setWindowTitle(title);
}

} // namespace DeviceStudio
