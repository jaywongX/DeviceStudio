/**
 * @file main.cpp
 * @brief DeviceStudio 程序入口
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "mainwindow/mainwindow.h"
#include "log/logger.h"
#include "config/configmanager.h"

/**
 * @brief 初始化日志系统
 */
void initLogger()
{
    // 获取应用目录
    QString appDir = QCoreApplication::applicationDirPath();
    QString logPath = appDir + "/logs";
    
    // 创建日志目录
    QDir dir(logPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // 初始化日志系统
    DeviceStudio::Logger::instance().init(
        logPath.toStdString(),
        DeviceStudio::LogLevel::Info
    );
    
    DS_LOG_INFO("DeviceStudio starting...");
}

/**
 * @brief 初始化配置管理
 */
void initConfig()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configPath = appDir + "/config/default.json";
    
    // 检查配置文件是否存在
    QFile configFile(configPath);
    if (configFile.exists()) {
        DeviceStudio::ConfigManager::instance().load(configPath.toStdString());
        DS_LOG_INFO("Configuration loaded from: " + configPath.toStdString());
    } else {
        DS_LOG_WARNING("Configuration file not found: " + configPath.toStdString());
        DS_LOG_INFO("Using default configuration");
    }
}

/**
 * @brief 主函数
 */
int main(int argc, char *argv[])
{
    // 创建应用
    QApplication app(argc, argv);
    
    // 设置应用信息
    app.setApplicationName("DeviceStudio");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("DeviceStudio");
    
    // 初始化日志
    initLogger();
    
    // 初始化配置
    initConfig();
    
    // 创建主窗口
    DeviceStudio::MainWindow mainWindow;
    mainWindow.show();
    
    DS_LOG_INFO("Application initialized successfully");
    
    // 运行应用
    int result = app.exec();
    
    DS_LOG_INFO("DeviceStudio exiting with code: " + std::to_string(result));
    
    return result;
}
