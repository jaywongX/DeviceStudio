/**
 * @file datastore.cpp
 * @brief 数据存储实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "datastore.h"

namespace DeviceStudio {

DataStore::DataStore(QObject* parent)
    : IDataCenter(parent)
{
}

void DataStore::setData(const QString& key, const QVariant& value)
{
    dataStore_[key] = value;
    emit dataUpdated(key, value);
}

QVariant DataStore::getData(const QString& key, const QVariant& defaultValue) const
{
    return dataStore_.value(key, defaultValue);
}

bool DataStore::hasData(const QString& key) const
{
    return dataStore_.contains(key);
}

void DataStore::removeData(const QString& key)
{
    dataStore_.remove(key);
}

void DataStore::clearAllData()
{
    dataStore_.clear();
    dataItems_.clear();
    historyData_.clear();
}

void DataStore::addDataItem(const DataItem& item)
{
    dataItems_[item.name] = item;
    emit dataItemUpdated(item.name, item);
}

DataItem DataStore::getDataItem(const QString& name) const
{
    return dataItems_.value(name);
}

QMap<QString, DataItem> DataStore::getAllDataItems() const
{
    return dataItems_;
}

void DataStore::addHistoryData(const QString& key, const QVariant& value, int maxCount)
{
    QList<QVariant>& history = historyData_[key];
    history.append(value);
    
    // 限制历史记录数量
    while (history.size() > maxCount) {
        history.removeFirst();
    }
    
    emit historyDataAdded(key, value);
}

QList<QVariant> DataStore::getHistoryData(const QString& key, int count) const
{
    if (!historyData_.contains(key)) {
        return QList<QVariant>();
    }
    
    const QList<QVariant>& history = historyData_[key];
    
    if (count == -1 || count >= history.size()) {
        return history;
    }
    
    // 返回最新的 count 条记录
    return history.mid(history.size() - count);
}

void DataStore::clearHistoryData(const QString& key)
{
    historyData_.remove(key);
}

int DataStore::totalDataCount() const
{
    return dataStore_.size();
}

int DataStore::totalHistoryCount() const
{
    int count = 0;
    for (const auto& history : historyData_) {
        count += history.size();
    }
    return count;
}

} // namespace DeviceStudio
