/**
 * @file terminalwidget.h
 * @brief 数据终端组件类
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QByteArray>
#include <QTimer>
#include <QFile>

namespace DeviceStudio {

/**
 * @brief 数据终端组件类
 * 
 * 类似串口助手的数据收发界面，支持HEX/ASCII格式
 */
class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget* parent = nullptr);
    ~TerminalWidget() override;
    
    /**
     * @brief 添加接收数据
     * @param data 接收到的数据
     * @param isHex 是否以HEX格式显示
     */
    void appendReceivedData(const QByteArray& data, bool isHex = true);
    
    /**
     * @brief 添加发送数据
     * @param data 发送的数据
     * @param isHex 是否以HEX格式显示
     */
    void appendSentData(const QByteArray& data, bool isHex = true);
    
    /**
     * @brief 清空接收区
     */
    void clearReceivedData();
    
    /**
     * @brief 清空发送区
     */
    void clearSendData();

signals:
    /**
     * @brief 发送数据信号
     */
    void dataSendRequested(const QByteArray& data);
    
    /**
     * @brief 数据接收信号
     */
    void dataReceived(const QByteArray& data);
    
    /**
     * @brief 文件发送进度信号
     */
    void fileSendProgress(qint64 sent, qint64 total);
    
    /**
     * @brief 文件发送完成信号
     */
    void fileSendCompleted();

private slots:
    /**
     * @brief 发送数据
     */
    void onSend();
    
    /**
     * @brief 清空接收区
     */
    void onClearReceived();
    
    /**
     * @brief 清空发送区
     */
    void onClearSend();
    
    /**
     * @brief 导出接收数据
     */
    void onExportData();
    
    /**
     * @brief 自动发送状态改变
     */
    void onAutoSendChanged(bool enabled);
    
    /**
     * @brief 自动发送定时器触发
     */
    void onAutoSendTimeout();
    
    /**
     * @brief 自动发送间隔改变
     */
    void onAutoSendIntervalChanged(int interval);
    
    /**
     * @brief 高亮设置改变
     */
    void onHighlightChanged(bool enabled);
    
    /**
     * @brief 选择发送文件
     */
    void onSelectFile();
    
    /**
     * @brief 发送文件
     */
    void onSendFile();
    
    /**
     * @brief 文件发送定时器触发
     */
    void onFileSendTimeout();
    
    /**
     * @brief 取消文件发送
     */
    void onCancelFileSend();

private:
    /**
     * @brief 设置UI
     */
    void setupUi();
    
    /**
     * @brief 格式化数据为HEX字符串
     */
    QString formatHex(const QByteArray& data) const;
    
    /**
     * @brief 解析HEX字符串为字节数组
     */
    QByteArray parseHex(const QString& hexString) const;
    
    /**
     * @brief 更新统计信息
     */
    void updateStatistics();
    
    /**
     * @brief 应用高亮
     */
    QString applyHighlight(const QString& text) const;
    
    /**
     * @brief 更新文件发送进度显示
     */
    void updateFileSendProgress();

private:
    // 接收区
    QTextEdit* receiveTextEdit_ = nullptr;
    QLabel* receiveCountLabel_ = nullptr;
    
    // 发送区
    QLineEdit* sendLineEdit_ = nullptr;
    QCheckBox* hexSendCheckBox_ = nullptr;
    QCheckBox* hexReceiveCheckBox_ = nullptr;
    QCheckBox* autoSendCheckBox_ = nullptr;
    QSpinBox* autoSendIntervalSpinBox_ = nullptr;
    
    // 统计
    qint64 totalReceivedBytes_ = 0;
    qint64 totalSentBytes_ = 0;
    
    // 自动发送
    QTimer* autoSendTimer_ = nullptr;
    
    // 高亮设置
    QCheckBox* highlightCheckBox_ = nullptr;
    QLineEdit* highlightPatternEdit_ = nullptr;
    QStringList highlightPatterns_;
    
    // 文件发送
    QPushButton* selectFileBtn_ = nullptr;
    QPushButton* sendFileBtn_ = nullptr;
    QPushButton* cancelFileBtn_ = nullptr;
    QLineEdit* filePathEdit_ = nullptr;
    QSpinBox* fileChunkSizeSpinBox_ = nullptr;
    QLabel* fileProgressLabel_ = nullptr;
    QTimer* fileSendTimer_ = nullptr;
    
    QFile* currentFile_ = nullptr;
    QString currentFilePath_;
    qint64 fileTotalSize_ = 0;
    qint64 fileSentSize_ = 0;
    bool fileSending_ = false;
};

} // namespace DeviceStudio
