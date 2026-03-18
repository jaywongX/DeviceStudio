/**
 * @file terminalwidget.cpp
 * @brief 数据终端组件实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "terminalwidget.h"
#include "log/logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDateTime>
#include <QTimer>
#include <QProgressBar>
#include <QMessageBox>

namespace DeviceStudio {

TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

TerminalWidget::~TerminalWidget()
{
    if (currentFile_) {
        currentFile_->close();
        delete currentFile_;
        currentFile_ = nullptr;
    }
    if (fileSendTimer_) {
        fileSendTimer_->stop();
    }
}

void TerminalWidget::appendReceivedData(const QByteArray& data, bool isHex)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString displayData = isHex ? formatHex(data) : QString::fromUtf8(data);
    
    // 应用高亮
    if (highlightCheckBox_ && highlightCheckBox_->isChecked()) {
        displayData = applyHighlight(displayData);
    }
    
    receiveTextEdit_->append(QString("[%1] RX: %2").arg(timestamp, displayData));
    
    totalReceivedBytes_ += data.size();
    updateStatistics();
    
    DS_LOG_DEBUG("Received " + std::to_string(data.size()) + " bytes");
}

void TerminalWidget::appendSentData(const QByteArray& data, bool isHex)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString displayData = isHex ? formatHex(data) : QString::fromUtf8(data);
    
    // 应用高亮
    if (highlightCheckBox_ && highlightCheckBox_->isChecked()) {
        displayData = applyHighlight(displayData);
    }
    
    receiveTextEdit_->append(QString("[%1] TX: %2").arg(timestamp, displayData));
    
    totalSentBytes_ += data.size();
    updateStatistics();
    
    DS_LOG_DEBUG("Sent " + std::to_string(data.size()) + " bytes");
}

void TerminalWidget::clearReceivedData()
{
    receiveTextEdit_->clear();
    totalReceivedBytes_ = 0;
    totalSentBytes_ = 0;
    updateStatistics();
}

void TerminalWidget::clearSendData()
{
    sendLineEdit_->clear();
}

void TerminalWidget::onSend()
{
    QString text = sendLineEdit_->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    
    QByteArray data;
    if (hexSendCheckBox_->isChecked()) {
        data = parseHex(text);
    } else {
        data = text.toUtf8();
    }
    
    if (data.isEmpty()) {
        DS_LOG_WARNING("Empty data to send");
        return;
    }
    
    emit dataSendRequested(data);
    
    // 显示发送的数据
    appendSentData(data, hexSendCheckBox_->isChecked());
    
    // 清空发送区（可选）
    // sendLineEdit_->clear();
}

void TerminalWidget::onClearReceived()
{
    clearReceivedData();
    DS_LOG_INFO("Received data cleared");
}

void TerminalWidget::onClearSend()
{
    clearSendData();
}

void TerminalWidget::onExportData()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("导出数据"),
        QString(), tr("文本文件 (*.txt);;所有文件 (*.*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << receiveTextEdit_->toPlainText();
        file.close();
        
        DS_LOG_INFO("Data exported to: " + fileName.toStdString());
    }
}

void TerminalWidget::onAutoSendChanged(bool enabled)
{
    autoSendIntervalSpinBox_->setEnabled(enabled);
    
    if (enabled) {
        // 启动自动发送定时器
        if (!autoSendTimer_) {
            autoSendTimer_ = new QTimer(this);
            connect(autoSendTimer_, &QTimer::timeout, this, &TerminalWidget::onAutoSendTimeout);
        }
        autoSendTimer_->start(autoSendIntervalSpinBox_->value());
        DS_LOG_INFO("Auto send enabled with interval " + std::to_string(autoSendIntervalSpinBox_->value()) + "ms");
    } else {
        // 停止自动发送定时器
        if (autoSendTimer_) {
            autoSendTimer_->stop();
        }
        DS_LOG_INFO("Auto send disabled");
    }
}

void TerminalWidget::onAutoSendTimeout()
{
    // 检查是否有数据要发送
    QString text = sendLineEdit_->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    
    // 调用发送函数
    onSend();
}

void TerminalWidget::onAutoSendIntervalChanged(int interval)
{
    // 更新定时器间隔
    if (autoSendTimer_ && autoSendTimer_->isActive()) {
        autoSendTimer_->setInterval(interval);
        DS_LOG_DEBUG("Auto send interval changed to " + std::to_string(interval) + "ms");
    }
}

void TerminalWidget::onHighlightChanged(bool enabled)
{
    highlightPatternEdit_->setEnabled(enabled);
    if (enabled) {
        QString patterns = highlightPatternEdit_->text();
        highlightPatterns_ = patterns.split(',', Qt::SkipEmptyParts);
        for (QString& p : highlightPatterns_) {
            p = p.trimmed();
        }
    } else {
        highlightPatterns_.clear();
    }
    DS_LOG_DEBUG("Highlight " + std::string(enabled ? "enabled" : "disabled"));
}

QString TerminalWidget::applyHighlight(const QString& text) const
{
    QString result = text;
    
    // 使用HTML格式高亮
    for (const QString& pattern : highlightPatterns_) {
        if (!pattern.isEmpty()) {
            result.replace(pattern, QString("<span style='color: red; font-weight: bold;'>%1</span>").arg(pattern),
                          Qt::CaseInsensitive);
        }
    }
    
    return result;
}

void TerminalWidget::onSelectFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择要发送的文件"),
        QString(), tr("所有文件 (*.*);;二进制文件 (*.bin *.dat);;文本文件 (*.txt)"));
    
    if (!fileName.isEmpty()) {
        currentFilePath_ = fileName;
        filePathEdit_->setText(fileName);
        sendFileBtn_->setEnabled(true);
        DS_LOG_INFO("Selected file: " + fileName.toStdString());
    }
}

void TerminalWidget::onSendFile()
{
    if (currentFilePath_.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择要发送的文件"));
        return;
    }
    
    if (fileSending_) {
        return;  // 已经在发送中
    }
    
    // 打开文件
    if (currentFile_) {
        currentFile_->close();
        delete currentFile_;
    }
    
    currentFile_ = new QFile(currentFilePath_);
    if (!currentFile_->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件: %1").arg(currentFilePath_));
        delete currentFile_;
        currentFile_ = nullptr;
        return;
    }
    
    fileTotalSize_ = currentFile_->size();
    fileSentSize_ = 0;
    fileSending_ = true;
    
    // 更新UI
    sendFileBtn_->setEnabled(false);
    selectFileBtn_->setEnabled(false);
    cancelFileBtn_->setEnabled(true);
    fileProgressLabel_->setText(tr("发送中: 0 / %1 字节").arg(fileTotalSize_));
    
    // 创建定时器
    if (!fileSendTimer_) {
        fileSendTimer_ = new QTimer(this);
        connect(fileSendTimer_, &QTimer::timeout, this, &TerminalWidget::onFileSendTimeout);
    }
    
    fileSendTimer_->start(10);  // 10ms间隔发送数据块
    DS_LOG_INFO("Starting file send: " + currentFilePath_.toStdString());
}

void TerminalWidget::onFileSendTimeout()
{
    if (!currentFile_ || !fileSending_) {
        fileSendTimer_->stop();
        return;
    }
    
    // 读取一块数据
    int chunkSize = fileChunkSizeSpinBox_->value();
    QByteArray chunk = currentFile_->read(chunkSize);
    
    if (chunk.isEmpty()) {
        // 文件发送完成
        onCancelFileSend();
        emit fileSendCompleted();
        DS_LOG_INFO("File send completed: " + std::to_string(fileSentSize_) + " bytes");
        return;
    }
    
    // 发送数据
    emit dataSendRequested(chunk);
    fileSentSize_ += chunk.size();
    
    // 显示进度
    updateFileSendProgress();
    emit fileSendProgress(fileSentSize_, fileTotalSize_);
}

void TerminalWidget::onCancelFileSend()
{
    if (fileSendTimer_) {
        fileSendTimer_->stop();
    }
    
    if (currentFile_) {
        currentFile_->close();
        delete currentFile_;
        currentFile_ = nullptr;
    }
    
    fileSending_ = false;
    
    // 恢复UI
    sendFileBtn_->setEnabled(true);
    selectFileBtn_->setEnabled(true);
    cancelFileBtn_->setEnabled(false);
    
    if (fileSentSize_ >= fileTotalSize_) {
        fileProgressLabel_->setText(tr("发送完成: %1 字节").arg(fileTotalSize_));
    } else {
        fileProgressLabel_->setText(tr("已取消: %1 / %2 字节").arg(fileSentSize_).arg(fileTotalSize_));
    }
    
    DS_LOG_INFO("File send stopped");
}

void TerminalWidget::updateFileSendProgress()
{
    double percent = fileTotalSize_ > 0 ? (fileSentSize_ * 100.0 / fileTotalSize_) : 0;
    fileProgressLabel_->setText(tr("发送中: %1 / %2 字节 (%3%)")
        .arg(fileSentSize_).arg(fileTotalSize_).arg(percent, 0, 'f', 1));
}

void TerminalWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // ========== 接收区 ==========
    QGroupBox* receiveGroup = new QGroupBox(tr("接收区"), this);
    QVBoxLayout* receiveLayout = new QVBoxLayout(receiveGroup);
    
    receiveTextEdit_ = new QTextEdit(this);
    receiveTextEdit_->setReadOnly(true);
    receiveTextEdit_->setFont(QFont("Consolas", 9));
    receiveLayout->addWidget(receiveTextEdit_);
    
    // 接收区按钮
    QHBoxLayout* receiveButtonLayout = new QHBoxLayout();
    
    hexReceiveCheckBox_ = new QCheckBox(tr("HEX显示"), this);
    hexReceiveCheckBox_->setChecked(true);
    receiveButtonLayout->addWidget(hexReceiveCheckBox_);
    
    highlightCheckBox_ = new QCheckBox(tr("高亮"), this);
    highlightCheckBox_->setChecked(false);
    connect(highlightCheckBox_, &QCheckBox::toggled, this, &TerminalWidget::onHighlightChanged);
    receiveButtonLayout->addWidget(highlightCheckBox_);
    
    highlightPatternEdit_ = new QLineEdit(this);
    highlightPatternEdit_->setPlaceholderText(tr("高亮关键字(逗号分隔)"));
    highlightPatternEdit_->setEnabled(false);
    highlightPatternEdit_->setMaximumWidth(150);
    receiveButtonLayout->addWidget(highlightPatternEdit_);
    
    receiveButtonLayout->addStretch();
    
    QPushButton* clearReceivedButton = new QPushButton(tr("清空"), this);
    connect(clearReceivedButton, &QPushButton::clicked, this, &TerminalWidget::onClearReceived);
    receiveButtonLayout->addWidget(clearReceivedButton);
    
    QPushButton* exportButton = new QPushButton(tr("导出"), this);
    connect(exportButton, &QPushButton::clicked, this, &TerminalWidget::onExportData);
    receiveButtonLayout->addWidget(exportButton);
    
    receiveCountLabel_ = new QLabel(tr("接收: 0 字节  发送: 0 字节"), this);
    receiveButtonLayout->addWidget(receiveCountLabel_);
    
    receiveLayout->addLayout(receiveButtonLayout);
    mainLayout->addWidget(receiveGroup);
    
    // ========== 发送区 ==========
    QGroupBox* sendGroup = new QGroupBox(tr("发送区"), this);
    QVBoxLayout* sendLayout = new QVBoxLayout(sendGroup);
    
    sendLineEdit_ = new QLineEdit(this);
    sendLineEdit_->setPlaceholderText(tr("输入要发送的数据"));
    sendLineEdit_->setFont(QFont("Consolas", 9));
    connect(sendLineEdit_, &QLineEdit::returnPressed, this, &TerminalWidget::onSend);
    sendLayout->addWidget(sendLineEdit_);
    
    // 发送区选项
    QHBoxLayout* sendOptionLayout = new QHBoxLayout();
    
    hexSendCheckBox_ = new QCheckBox(tr("HEX发送"), this);
    hexSendCheckBox_->setChecked(true);
    sendOptionLayout->addWidget(hexSendCheckBox_);
    
    autoSendCheckBox_ = new QCheckBox(tr("自动发送"), this);
    connect(autoSendCheckBox_, &QCheckBox::toggled, this, &TerminalWidget::onAutoSendChanged);
    sendOptionLayout->addWidget(autoSendCheckBox_);
    
    sendOptionLayout->addWidget(new QLabel(tr("间隔:"), this));
    
    autoSendIntervalSpinBox_ = new QSpinBox(this);
    autoSendIntervalSpinBox_->setRange(10, 10000);
    autoSendIntervalSpinBox_->setValue(1000);
    autoSendIntervalSpinBox_->setSuffix(tr(" ms"));
    autoSendIntervalSpinBox_->setEnabled(false);
    connect(autoSendIntervalSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TerminalWidget::onAutoSendIntervalChanged);
    sendOptionLayout->addWidget(autoSendIntervalSpinBox_);
    
    sendOptionLayout->addStretch();
    
    QPushButton* sendButton = new QPushButton(tr("发送"), this);
    sendButton->setDefault(true);
    connect(sendButton, &QPushButton::clicked, this, &TerminalWidget::onSend);
    sendOptionLayout->addWidget(sendButton);
    
    QPushButton* clearSendButton = new QPushButton(tr("清空"), this);
    connect(clearSendButton, &QPushButton::clicked, this, &TerminalWidget::onClearSend);
    sendOptionLayout->addWidget(clearSendButton);
    
    sendLayout->addLayout(sendOptionLayout);
    mainLayout->addWidget(sendGroup);
    
    // ========== 文件发送区 ==========
    QGroupBox* fileSendGroup = new QGroupBox(tr("文件发送"), this);
    QVBoxLayout* fileSendLayout = new QVBoxLayout(fileSendGroup);
    
    // 文件选择行
    QHBoxLayout* fileSelectLayout = new QHBoxLayout();
    
    selectFileBtn_ = new QPushButton(tr("选择文件"), this);
    connect(selectFileBtn_, &QPushButton::clicked, this, &TerminalWidget::onSelectFile);
    fileSelectLayout->addWidget(selectFileBtn_);
    
    filePathEdit_ = new QLineEdit(this);
    filePathEdit_->setReadOnly(true);
    filePathEdit_->setPlaceholderText(tr("未选择文件"));
    fileSelectLayout->addWidget(filePathEdit_);
    
    fileSendLayout->addLayout(fileSelectLayout);
    
    // 文件发送选项行
    QHBoxLayout* fileOptionLayout = new QHBoxLayout();
    
    fileOptionLayout->addWidget(new QLabel(tr("块大小:"), this));
    fileChunkSizeSpinBox_ = new QSpinBox(this);
    fileChunkSizeSpinBox_->setRange(1, 4096);
    fileChunkSizeSpinBox_->setValue(512);
    fileChunkSizeSpinBox_->setSuffix(tr(" 字节"));
    fileOptionLayout->addWidget(fileChunkSizeSpinBox_);
    
    fileOptionLayout->addStretch();
    
    sendFileBtn_ = new QPushButton(tr("发送文件"), this);
    sendFileBtn_->setEnabled(false);
    connect(sendFileBtn_, &QPushButton::clicked, this, &TerminalWidget::onSendFile);
    fileOptionLayout->addWidget(sendFileBtn_);
    
    cancelFileBtn_ = new QPushButton(tr("取消"), this);
    cancelFileBtn_->setEnabled(false);
    connect(cancelFileBtn_, &QPushButton::clicked, this, &TerminalWidget::onCancelFileSend);
    fileOptionLayout->addWidget(cancelFileBtn_);
    
    fileSendLayout->addLayout(fileOptionLayout);
    
    // 进度显示
    fileProgressLabel_ = new QLabel(tr("就绪"), this);
    fileSendLayout->addWidget(fileProgressLabel_);
    
    mainLayout->addWidget(fileSendGroup);
}

QString TerminalWidget::formatHex(const QByteArray& data) const
{
    QString result;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) {
            result += " ";
        }
        result += QString("%1").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return result;
}

QByteArray TerminalWidget::parseHex(const QString& hexString) const
{
    QString cleanString = hexString;
    cleanString.remove(' ').remove('\t').remove('\n').remove('\r');
    
    // 确保长度为偶数
    if (cleanString.length() % 2 != 0) {
        cleanString.prepend('0');
    }
    
    QByteArray result;
    for (int i = 0; i < cleanString.length(); i += 2) {
        bool ok;
        unsigned char byte = static_cast<unsigned char>(cleanString.mid(i, 2).toUInt(&ok, 16));
        if (ok) {
            result.append(static_cast<char>(byte));
        }
    }
    
    return result;
}

void TerminalWidget::updateStatistics()
{
    QString text = tr("接收: %1 字节  发送: %2 字节")
        .arg(totalReceivedBytes_)
        .arg(totalSentBytes_);
    
    receiveCountLabel_->setText(text);
}

} // namespace DeviceStudio
