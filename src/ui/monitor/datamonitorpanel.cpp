/**
 * @file datamonitorpanel.cpp
 * @brief 数据监控面板实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#include "datamonitorpanel.h"
#include "utils/log/logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>

namespace DeviceStudio {

DataMonitorPanel::DataMonitorPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
}

DataMonitorPanel::~DataMonitorPanel()
{
}

void DataMonitorPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 工具栏
    toolBar_ = new QToolBar(tr("数据监控工具栏"), this);
    toolBar_->setIconSize(QSize(16, 16));
    
    QAction* clearAction = toolBar_->addAction(QIcon(":/icons/clear.png"), tr("清空数据"));
    connect(clearAction, &QAction::triggered, this, &DataMonitorPanel::onClearClicked);
    
    QAction* exportAction = toolBar_->addAction(QIcon(":/icons/export.png"), tr("导出数据"));
    connect(exportAction, &QAction::triggered, this, &DataMonitorPanel::onExportClicked);
    
    mainLayout->addWidget(toolBar_);
    
    // 数据表格
    dataTable_ = new QTableWidget(this);
    dataTable_->setColumnCount(6);
    dataTable_->setHorizontalHeaderLabels({
        tr("数据项"), tr("当前值"), tr("单位"),
        tr("更新时间"), tr("来源"), tr("更新次数")
    });
    
    // 设置表格属性
    dataTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    dataTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    dataTable_->setAlternatingRowColors(true);
    dataTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dataTable_->verticalHeader()->setVisible(false);
    
    // 设置列宽
    dataTable_->horizontalHeader()->setStretchLastSection(true);
    dataTable_->setColumnWidth(0, 150);
    dataTable_->setColumnWidth(1, 100);
    dataTable_->setColumnWidth(2, 60);
    dataTable_->setColumnWidth(3, 150);
    dataTable_->setColumnWidth(4, 100);
    
    mainLayout->addWidget(dataTable_);
    
    // 状态栏
    statusLabel_ = new QLabel(tr("数据项: 0"), this);
    statusLabel_->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 2px; }");
    mainLayout->addWidget(statusLabel_);
    
    // 刷新定时器
    refreshTimer_ = new QTimer(this);
}

void DataMonitorPanel::setupConnections()
{
    connect(refreshTimer_, &QTimer::timeout, this, &DataMonitorPanel::onRefreshTimer);
    connect(dataTable_, &QTableWidget::cellDoubleClicked, this, &DataMonitorPanel::onItemDoubleClicked);
}

void DataMonitorPanel::addDataItem(const QString& name, const QString& unit, const QString& source)
{
    // 检查是否已存在
    if (findDataRow(name) >= 0) {
        DS_LOG_WARN("Data item already exists: {}", name.toStdString());
        return;
    }
    
    DataItem item;
    item.name = name;
    item.unit = unit;
    item.source = source;
    item.timestamp = QDateTime::currentMSecsSinceEpoch();
    item.updateCount = 0;
    
    dataItems_.append(item);
    
    // 添加表格行
    int row = dataTable_->rowCount();
    dataTable_->insertRow(row);
    
    dataTable_->setItem(row, 0, new QTableWidgetItem(name));
    dataTable_->setItem(row, 1, new QTableWidgetItem("--"));
    dataTable_->setItem(row, 2, new QTableWidgetItem(unit));
    dataTable_->setItem(row, 3, new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(item.timestamp).toString("hh:mm:ss.zzz")));
    dataTable_->setItem(row, 4, new QTableWidgetItem(source));
    dataTable_->setItem(row, 5, new QTableWidgetItem("0"));
    
    // 更新状态栏
    statusLabel_->setText(tr("数据项: %1").arg(dataItems_.size()));
    
    DS_LOG_DEBUG("Data item added: {}", name.toStdString());
    emit dataItemAdded(name);
}

void DataMonitorPanel::updateDataItem(const QString& name, const QVariant& value)
{
    int row = findDataRow(name);
    if (row < 0) {
        DS_LOG_WARN("Data item not found: {}", name.toStdString());
        return;
    }
    
    // 更新数据
    dataItems_[row].value = value.toString();
    dataItems_[row].timestamp = QDateTime::currentMSecsSinceEpoch();
    dataItems_[row].updateCount++;
    
    // 更新表格
    dataTable_->item(row, 1)->setText(value.toString());
    dataTable_->item(row, 3)->setText(
        QDateTime::fromMSecsSinceEpoch(dataItems_[row].timestamp).toString("hh:mm:ss.zzz")
    );
    dataTable_->item(row, 5)->setText(QString::number(dataItems_[row].updateCount));
    
    DS_LOG_TRACE("Data item updated: {} = {}", name.toStdString(), value.toString().toStdString());
    emit dataItemUpdated(name, value);
}

void DataMonitorPanel::removeDataItem(const QString& name)
{
    int row = findDataRow(name);
    if (row < 0) {
        return;
    }
    
    dataItems_.remove(row);
    dataTable_->removeRow(row);
    
    statusLabel_->setText(tr("数据项: %1").arg(dataItems_.size()));
    DS_LOG_DEBUG("Data item removed: {}", name.toStdString());
}

void DataMonitorPanel::clearDataItems()
{
    dataItems_.clear();
    dataTable_->setRowCount(0);
    statusLabel_->setText(tr("数据项: 0"));
    DS_LOG_DEBUG("All data items cleared");
}

DataItem DataMonitorPanel::getDataItem(const QString& name) const
{
    int row = findDataRow(name);
    if (row >= 0) {
        return dataItems_[row];
    }
    return DataItem();
}

bool DataMonitorPanel::exportToCsv(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        DS_LOG_ERROR("Failed to open file for export: {}", filePath.toStdString());
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // 写入标题
    out << "数据项,当前值,单位,更新时间,来源,更新次数\n";
    
    // 写入数据
    for (const DataItem& item : dataItems_) {
        out << QString("%1,%2,%3,%4,%5,%6\n")
            .arg(item.name)
            .arg(item.value)
            .arg(item.unit)
            .arg(QDateTime::fromMSecsSinceEpoch(item.timestamp).toString("yyyy-MM-dd hh:mm:ss.zzz"))
            .arg(item.source)
            .arg(item.updateCount);
    }
    
    file.close();
    DS_LOG_INFO("Data exported to: {}", filePath.toStdString());
    return true;
}

void DataMonitorPanel::setAutoRefresh(bool enabled, int intervalMs)
{
    autoRefresh_ = enabled;
    
    if (enabled) {
        refreshTimer_->start(intervalMs);
    } else {
        refreshTimer_->stop();
    }
}

void DataMonitorPanel::setShowHistory(bool show)
{
    showHistory_ = show;
}

void DataMonitorPanel::onClearClicked()
{
    clearDataItems();
}

void DataMonitorPanel::onExportClicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("导出数据"),
        QString(),
        tr("CSV文件 (*.csv);;所有文件 (*.*)")
    );
    
    if (!filePath.isEmpty()) {
        if (exportToCsv(filePath)) {
            QMessageBox::information(this, tr("导出成功"), tr("数据已导出到:\n%1").arg(filePath));
        } else {
            QMessageBox::warning(this, tr("导出失败"), tr("无法导出数据到文件"));
        }
    }
}

void DataMonitorPanel::onRefreshTimer()
{
    updateTable();
}

void DataMonitorPanel::onItemDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0 && row < dataItems_.size()) {
        const DataItem& item = dataItems_[row];
        QString info = tr("数据项: %1\n当前值: %2 %3\n更新次数: %4\n更新时间: %5")
            .arg(item.name)
            .arg(item.value)
            .arg(item.unit)
            .arg(item.updateCount)
            .arg(QDateTime::fromMSecsSinceEpoch(item.timestamp).toString("yyyy-MM-dd hh:mm:ss.zzz"));
        
        QMessageBox::information(this, tr("数据项详情"), info);
    }
}

void DataMonitorPanel::updateTable()
{
    dataTable_->update();
}

int DataMonitorPanel::findDataRow(const QString& name) const
{
    for (int i = 0; i < dataItems_.size(); ++i) {
        if (dataItems_[i].name == name) {
            return i;
        }
    }
    return -1;
}

} // namespace DeviceStudio
