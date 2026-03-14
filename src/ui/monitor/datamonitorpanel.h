/**
 * @file datamonitorpanel.h
 * @brief 数据监控面板
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QToolBar>
#include <QLabel>
#include <QTimer>
#include <QVector>

namespace DeviceStudio {

/**
 * @brief 数据项结构
 */
struct DataItem {
    QString name;           // 数据项名称
    QString value;          // 当前值
    QString unit;           // 单位
    qint64 timestamp;       // 时间戳
    QString source;         // 数据来源
    int updateCount = 0;    // 更新次数
};

/**
 * @brief 数据监控面板
 */
class DataMonitorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DataMonitorPanel(QWidget* parent = nullptr);
    ~DataMonitorPanel() override;
    
    /**
     * @brief 添加数据项
     */
    void addDataItem(const QString& name, const QString& unit = "", const QString& source = "");
    
    /**
     * @brief 更新数据项
     */
    void updateDataItem(const QString& name, const QVariant& value);
    
    /**
     * @brief 移除数据项
     */
    void removeDataItem(const QString& name);
    
    /**
     * @brief 清空所有数据项
     */
    void clearDataItems();
    
    /**
     * @brief 获取数据项
     */
    DataItem getDataItem(const QString& name) const;
    
    /**
     * @brief 导出数据为CSV
     */
    bool exportToCsv(const QString& filePath);
    
    /**
     * @brief 设置自动刷新
     */
    void setAutoRefresh(bool enabled, int intervalMs = 1000);
    
    /**
     * @brief 设置显示历史记录
     */
    void setShowHistory(bool show);

signals:
    void dataItemUpdated(const QString& name, const QVariant& value);
    void dataItemAdded(const QString& name);

private slots:
    void onClearClicked();
    void onExportClicked();
    void onRefreshTimer();
    void onItemDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupConnections();
    void updateTable();
    int findDataRow(const QString& name) const;
    
    QToolBar* toolBar_ = nullptr;
    QTableWidget* dataTable_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QTimer* refreshTimer_ = nullptr;
    
    QVector<DataItem> dataItems_;
    bool autoRefresh_ = false;
    bool showHistory_ = false;
    int maxHistoryCount_ = 100;
};

} // namespace DeviceStudio
