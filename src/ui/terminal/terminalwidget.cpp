/**
 * @file terminalwidget.cpp
 * @brief 数据终端组件实现
 * @author DeviceStudio Team
 * @date 2026-03-14
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

namespace DeviceStudio {

TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void TerminalWidget::appendReceivedData(const QByteArray& data, bool isHex)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString displayData = isHex ? formatHex(data) : QString::fromUtf8(data);
    
    receiveTextEdit_->append(QString("[%1] RX: %2").arg(timestamp, displayData));
    
    totalReceivedBytes_ += data.size();
    updateStatistics();
    
    DS_LOG_DEBUG("Received " + std::to_string(data.size()) + " bytes");
}

void TerminalWidget::appendSentData(const QByteArray& data, bool isHex)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString displayData = isHex ? formatHex(data) : QString::fromUtf8(data);
    
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
        // TODO: 启动自动发送定时器
        DS_LOG_INFO("Auto send enabled");
    } else {
        // TODO: 停止自动发送定时器
        DS_LOG_INFO("Auto send disabled");
    }
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
