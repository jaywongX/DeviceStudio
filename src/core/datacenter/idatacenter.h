/**
 * @file idatacenter.h
 * @brief 数据中心接口定义
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QDateTime>
#include <QMap>
#include <memory>

namespace DeviceStudio {

/**
 * @brief 数据项结构
 */
struct DataItem {
    QString name;           ///< 数据名称
    QString displayName;    ///< 显示名称
    QVariant value;         ///< 数据值
    QString unit;           ///< 单位
    QDateTime timestamp;    ///< 时间戳
    QString sourceDevice;   ///< 来源设备ID
};

/**
 * @brief 数据中心接口类
 * 
 * 负责数据的存储、缓存和管理
 */
class IDataCenter : public QObject
{
    Q_OBJECT

public:
    explicit IDataCenter(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDataCenter() = default;
    
    // ========== 数据存储 ==========
    
    /**
     * @brief 存储数据
     * @param key 数据键
     * @param value 数据值
     */
    virtual void setData(const QString& key, const QVariant& value) = 0;
    
    /**
     * @brief 获取数据
     * @param key 数据键
     * @param defaultValue 默认值
     * @return 数据值
     */
    virtual QVariant getData(const QString& key, const QVariant& defaultValue = QVariant()) const = 0;
    
    /**
     * @brief 检查数据是否存在
     */
    virtual bool hasData(const QString& key) const = 0;
    
    /**
     * @brief 删除数据
     */
    virtual void removeData(const QString& key) = 0;
    
    /**
     * @brief 清空所有数据
     */
    virtual void clearAllData() = 0;
    
    // ========== 数据项管理 ==========
    
    /**
     * @brief 添加数据项
     * @param item 数据项
     */
    virtual void addDataItem(const DataItem& item) = 0;
    
    /**
     * @brief 获取数据项
     * @param name 数据项名称
     * @return 数据项
     */
    virtual DataItem getDataItem(const QString& name) const = 0;
    
    /**
     * @brief 获取所有数据项
     */
    virtual QMap<QString, DataItem> getAllDataItems() const = 0;
    
    // ========== 历史数据 ==========
    
    /**
     * @brief 添加历史数据
     * @param key 数据键
     * @param value 数据值
     * @param maxCount 最大历史记录数
     */
    virtual void addHistoryData(const QString& key, const QVariant& value, int maxCount = 1000) = 0;
    
    /**
     * @brief 获取历史数据
     * @param key 数据键
     * @param count 获取数量（-1表示全部）
     * @return 历史数据列表
     */
    virtual QList<QVariant> getHistoryData(const QString& key, int count = -1) const = 0;
    
    /**
     * @brief 清除历史数据
     * @param key 数据键
     */
    virtual void clearHistoryData(const QString& key) = 0;
    
    // ========== 数据统计 ==========
    
    /**
     * @brief 获取数据总数
     */
    virtual int totalDataCount() const = 0;
    
    /**
     * @brief 获取历史数据总数
     */
    virtual int totalHistoryCount() const = 0;

signals:
    /**
     * @brief 数据更新信号
     */
    void dataUpdated(const QString& key, const QVariant& value);
    
    /**
     * @brief 数据项更新信号
     */
    void dataItemUpdated(const QString& name, const DataItem& item);
    
    /**
     * @brief 历史数据添加信号
     */
    void historyDataAdded(const QString& key, const QVariant& value);
};

// 数据中心智能指针类型
using IDataCenterPtr = std::shared_ptr<IDataCenter>;

} // namespace DeviceStudio

// 元类型注册
Q_DECLARE_METATYPE(DeviceStudio::DataItem)
