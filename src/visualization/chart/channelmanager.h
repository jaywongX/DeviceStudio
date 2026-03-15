/**
 * @file channelmanager.h
 * @brief 通道管理器 - 支持多通道数据显示
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QColor>
#include <QPair>
#include <QMutex>

namespace DeviceStudio {

/**
 * @brief 通道配置
 */
struct ChannelConfig {
    int id;                     // 通道ID
    QString name;               // 通道名称
    QColor color;               // 通道颜色
    bool visible = true;        // 是否可见
    double minValue = 0.0;      // 最小值（用于缩放）
    double maxValue = 100.0;    // 最大值（用于缩放）
    QString unit;               // 单位
    int graphIndex = -1;        // 关联的图表索引
};

/**
 * @brief 数据点
 */
struct DataPoint {
    double x;
    double y;
    qint64 timestamp;           // 时间戳（毫秒）
};

/**
 * @brief 通道管理器类
 * 
 * 管理多个数据通道，支持：
 * - 最多16个通道同时显示
 * - 通道颜色自定义
 * - 通道显示/隐藏
 * - 数据缓冲和采样
 */
class ChannelManager : public QObject
{
    Q_OBJECT

public:
    static constexpr int MAX_CHANNELS = 16;
    static constexpr int DEFAULT_BUFFER_SIZE = 1000;

    explicit ChannelManager(QObject* parent = nullptr);
    ~ChannelManager() override;

    // ========== 通道管理 ==========

    /**
     * @brief 添加通道
     * @param name 通道名称
     * @param color 通道颜色（默认自动分配）
     * @return 通道ID，失败返回-1
     */
    int addChannel(const QString& name, const QColor& color = QColor());

    /**
     * @brief 移除通道
     */
    void removeChannel(int id);

    /**
     * @brief 清空所有通道
     */
    void clearChannels();

    /**
     * @brief 获取通道数量
     */
    int channelCount() const;

    /**
     * @brief 获取所有通道ID
     */
    QVector<int> channelIds() const;

    /**
     * @brief 获取通道配置
     */
    ChannelConfig getChannelConfig(int id) const;
    
    /**
     * @brief 设置通道配置
     */
    void setChannelConfig(int id, const ChannelConfig& config);

    /**
     * @brief 设置通道颜色
     */
    void setChannelColor(int id, const QColor& color);

    /**
     * @brief 设置通道可见性
     */
    void setChannelVisible(int id, bool visible);

    /**
     * @brief 获取通道颜色
     */
    QColor getChannelColor(int id) const;

    /**
     * @brief 检查通道是否可见
     */
    bool isChannelVisible(int id) const;

    // ========== 数据操作 ==========

    /**
     * @brief 添加数据点
     * @param id 通道ID
     * @param x X值
     * @param y Y值
     */
    void addDataPoint(int id, double x, double y);

    /**
     * @brief 添加数据点（自动生成X值）
     */
    void addDataPoint(int id, double y);

    /**
     * @brief 获取通道数据
     */
    QVector<DataPoint> getChannelData(int id) const;

    /**
     * @brief 获取通道X数据
     */
    QVector<double> getChannelXData(int id) const;

    /**
     * @brief 获取通道Y数据
     */
    QVector<double> getChannelYData(int id) const;

    /**
     * @brief 清空通道数据
     */
    void clearChannelData(int id);

    /**
     * @brief 清空所有通道数据
     */
    void clearAllData();

    /**
     * @brief 设置数据缓冲区大小
     */
    void setBufferSize(int size);

    /**
     * @brief 获取数据缓冲区大小
     */
    int bufferSize() const { return bufferSize_; }

    // ========== 导出功能 ==========

    /**
     * @brief 导出通道数据为CSV格式
     * @param ids 要导出的通道ID列表（空表示所有可见通道）
     * @return CSV格式字符串
     */
    QString exportToCSV(const QVector<int>& ids = QVector<int>()) const;

    /**
     * @brief 导出通道数据到文件
     */
    bool exportToFile(const QString& filePath, const QVector<int>& ids = QVector<int>()) const;

    // ========== 统计信息 ==========

    /**
     * @brief 获取通道统计信息
     */
    struct ChannelStats {
        double min;
        double max;
        double avg;
        double stdDev;
        int count;
    };
    ChannelStats getChannelStats(int id) const;

signals:
    /**
     * @brief 通道添加信号
     */
    void channelAdded(int id);

    /**
     * @brief 通道移除信号
     */
    void channelRemoved(int id);

    /**
     * @brief 通道配置改变信号
     */
    void channelConfigChanged(int id);

    /**
     * @brief 数据更新信号
     */
    void dataUpdated(int id);

    /**
     * @brief 所有通道数据更新信号（批量更新时使用）
     */
    void dataUpdatedBatch();

private:
    /**
     * @brief 生成默认颜色
     */
    QColor generateDefaultColor(int index);

    /**
     * @brief 检查通道ID是否有效
     */
    bool isValidChannel(int id) const;

    mutable QMutex mutex_;
    QMap<int, ChannelConfig> channels_;
    QMap<int, QVector<DataPoint>> channelData_;
    int bufferSize_ = DEFAULT_BUFFER_SIZE;
    int nextChannelId_ = 1;
};

} // namespace DeviceStudio
