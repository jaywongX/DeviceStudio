/**
 * @file datastore.h
 * @brief 数据存储实现
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include "idatacenter.h"
#include <QMap>
#include <QList>

namespace DeviceStudio {

/**
 * @brief 数据存储类
 * 
 * 实现数据中心接口
 */
class DataStore : public IDataCenter
{
    Q_OBJECT

public:
    explicit DataStore(QObject* parent = nullptr);
    ~DataStore() override = default;
    
    // ========== IDataCenter 接口实现 ==========
    
    void setData(const QString& key, const QVariant& value) override;
    QVariant getData(const QString& key, const QVariant& defaultValue = QVariant()) const override;
    bool hasData(const QString& key) const override;
    void removeData(const QString& key) override;
    void clearAllData() override;
    
    void addDataItem(const DataItem& item) override;
    DataItem getDataItem(const QString& name) const override;
    QMap<QString, DataItem> getAllDataItems() const override;
    
    void addHistoryData(const QString& key, const QVariant& value, int maxCount = 1000) override;
    QList<QVariant> getHistoryData(const QString& key, int count = -1) const override;
    void clearHistoryData(const QString& key) override;
    
    int totalDataCount() const override;
    int totalHistoryCount() const override;

private:
    QMap<QString, QVariant> dataStore_;
    QMap<QString, DataItem> dataItems_;
    QMap<QString, QList<QVariant>> historyData_;
};

} // namespace DeviceStudio
