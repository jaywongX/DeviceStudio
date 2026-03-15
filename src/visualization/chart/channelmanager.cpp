/**
 * @file channelmanager.cpp
 * @brief 通道管理器实现
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#include "channelmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QtMath>
#include <mutex>

namespace DeviceStudio {

ChannelManager::ChannelManager(QObject* parent)
    : QObject(parent)
{
}

ChannelManager::~ChannelManager()
{
}

int ChannelManager::addChannel(const QString& name, const QColor& color)
{
    std::lock_guard<QMutex> lock(mutex_);
    
    if (channels_.size() >= MAX_CHANNELS) {
        return -1;
    }
    
    int id = nextChannelId_++;
    
    ChannelConfig config;
    config.id = id;
    config.name = name.isEmpty() ? QString("Channel %1").arg(id) : name;
    config.color = color.isValid() ? color : generateDefaultColor(channels_.size());
    
    channels_[id] = config;
    channelData_[id] = QVector<DataPoint>();
    
    emit channelAdded(id);
    return id;
}

void ChannelManager::removeChannel(int id)
{
    std::lock_guard<QMutex> lock(mutex_);
    
    if (channels_.contains(id)) {
        channels_.remove(id);
        channelData_.remove(id);
        emit channelRemoved(id);
    }
}

void ChannelManager::clearChannels()
{
    std::lock_guard<QMutex> lock(mutex_);
    
    channels_.clear();
    channelData_.clear();
    nextChannelId_ = 1;
}

int ChannelManager::channelCount() const
{
    std::lock_guard<QMutex> lock(mutex_);
    return channels_.size();
}

QVector<int> ChannelManager::channelIds() const
{
    std::lock_guard<QMutex> lock(mutex_);
    return channels_.keys().toVector();
}

ChannelConfig ChannelManager::getChannelConfig(int id) const
{
    std::lock_guard<QMutex> lock(mutex_);
    return channels_.value(id);
}

void ChannelManager::setChannelConfig(int id, const ChannelConfig& config)
{
    std::lock_guard<QMutex> lock(mutex_);
    
    if (channels_.contains(id)) {
        channels_[id] = config;
        emit channelConfigChanged(id);
    }
}

void ChannelManager::setChannelColor(int id, const QColor& color)
{
    std::lock_guard<QMutex> lock(mutex_);
    
    if (channels_.contains(id)) {
        channels_[id].color = color;
        emit channelConfigChanged(id);
    }
}

void ChannelManager::setChannelVisible(int id, bool visible)
{
    std::lock_guard<QMutex> lock(mutex_);
    
    if (channels_.contains(id)) {
        channels_[id].visible = visible;
        emit channelConfigChanged(id);
    }
}

QColor ChannelManager::getChannelColor(int id) const
{
    std::lock_guard<QMutex> lock(mutex_);
    return channels_.value(id).color;
}

bool ChannelManager::isChannelVisible(int id) const
{
    std::lock_guard<QMutex> lock(mutex_);
    return channels_.value(id).visible;
}

void ChannelManager::addDataPoint(int id, double x, double y)
{
    std::lock_guard<QMutex> lock(mutex_);
    
    if (!channels_.contains(id)) {
        return;
    }
    
    DataPoint point;
    point.x = x;
    point.y = y;
    point.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    QVector<DataPoint>& data = channelData_[id];
    data.append(point);
    
    // 限制缓冲区大小
    while (data.size() > bufferSize_) {
        data.removeFirst();
    }
    
    emit dataUpdated(id);
}

void ChannelManager::addDataPoint(int id, double y)
{
    static qint64 baseTime = 0;
    if (baseTime == 0) {
        baseTime = QDateTime::currentMSecsSinceEpoch();
    }
    
    double x = (QDateTime::currentMSecsSinceEpoch() - baseTime) / 1000.0;
    addDataPoint(id, x, y);
}

QVector<DataPoint> ChannelManager::getChannelData(int id) const
{
    std::lock_guard<QMutex> lock(mutex_);
    return channelData_.value(id);
}

QVector<double> ChannelManager::getChannelXData(int id) const
{
    std::lock_guard<QMutex> lock(mutex_);
    
    QVector<double> xData;
    if (channelData_.contains(id)) {
        const auto& data = channelData_[id];
        xData.reserve(data.size());
        for (const auto& point : data) {
            xData.append(point.x);
        }
    }
    return xData;
}

QVector<double> ChannelManager::getChannelYData(int id) const
{
    std::lock_guard<QMutex> lock(mutex_);
    
    QVector<double> yData;
    if (channelData_.contains(id)) {
        const auto& data = channelData_[id];
        yData.reserve(data.size());
        for (const auto& point : data) {
            yData.append(point.y);
        }
    }
    return yData;
}

void ChannelManager::clearChannelData(int id)
{
    std::lock_guard<QMutex> lock(mutex_);
    
    if (channelData_.contains(id)) {
        channelData_[id].clear();
        emit dataUpdated(id);
    }
}

void ChannelManager::clearAllData()
{
    std::lock_guard<QMutex> lock(mutex_);
    
    for (auto& data : channelData_) {
        data.clear();
    }
    emit dataUpdatedBatch();
}

void ChannelManager::setBufferSize(int size)
{
    std::lock_guard<QMutex> lock(mutex_);
    bufferSize_ = qMax(100, size);
}

QString ChannelManager::exportToCSV(const QVector<int>& ids) const
{
    std::lock_guard<QMutex> lock(mutex_);
    
    QVector<int> exportIds = ids;
    if (exportIds.isEmpty()) {
        // 导出所有可见通道
        for (auto it = channels_.constBegin(); it != channels_.constEnd(); ++it) {
            if (it.value().visible) {
                exportIds.append(it.key());
            }
        }
    }
    
    if (exportIds.isEmpty()) {
        return QString();
    }
    
    QString csv;
    QTextStream stream(&csv);
    
    // 表头
    stream << "Timestamp";
    for (int id : exportIds) {
        if (channels_.contains(id)) {
            stream << "," << channels_[id].name;
        }
    }
    stream << "\n";
    
    // 获取最大数据点数
    int maxPoints = 0;
    for (int id : exportIds) {
        if (channelData_.contains(id)) {
            maxPoints = qMax(maxPoints, channelData_[id].size());
        }
    }
    
    // 数据行
    for (int i = 0; i < maxPoints; ++i) {
        // 时间戳（使用第一个有效通道的时间戳）
        bool found = false;
        for (int id : exportIds) {
            if (channelData_.contains(id) && i < channelData_[id].size()) {
                stream << QDateTime::fromMSecsSinceEpoch(channelData_[id][i].timestamp)
                          .toString("yyyy-MM-dd hh:mm:ss.zzz");
                found = true;
                break;
            }
        }
        if (!found) {
            stream << "";
        }
        
        // 各通道数据
        for (int id : exportIds) {
            stream << ",";
            if (channelData_.contains(id) && i < channelData_[id].size()) {
                stream << channelData_[id][i].y;
            }
        }
        stream << "\n";
    }
    
    return csv;
}

bool ChannelManager::exportToFile(const QString& filePath, const QVector<int>& ids) const
{
    QString csv = exportToCSV(ids);
    if (csv.isEmpty()) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << csv;
    file.close();
    
    return true;
}

ChannelManager::ChannelStats ChannelManager::getChannelStats(int id) const
{
    std::lock_guard<QMutex> lock(mutex_);
    
    ChannelStats stats = {0, 0, 0, 0, 0};
    
    if (!channelData_.contains(id) || channelData_[id].isEmpty()) {
        return stats;
    }
    
    const auto& data = channelData_[id];
    stats.count = data.size();
    stats.min = data[0].y;
    stats.max = data[0].y;
    double sum = 0;
    
    for (const auto& point : data) {
        stats.min = qMin(stats.min, point.y);
        stats.max = qMax(stats.max, point.y);
        sum += point.y;
    }
    
    stats.avg = sum / stats.count;
    
    // 计算标准差
    double variance = 0;
    for (const auto& point : data) {
        variance += qPow(point.y - stats.avg, 2);
    }
    stats.stdDev = qSqrt(variance / stats.count);
    
    return stats;
}

QColor ChannelManager::generateDefaultColor(int index)
{
    // 预定义的16种颜色
    static const QColor colors[] = {
        QColor(31, 119, 180),   // 蓝色
        QColor(255, 127, 14),   // 橙色
        QColor(44, 160, 44),    // 绿色
        QColor(214, 39, 40),    // 红色
        QColor(148, 103, 189),  // 紫色
        QColor(140, 86, 75),    // 棕色
        QColor(227, 119, 194),  // 粉色
        QColor(127, 127, 127),  // 灰色
        QColor(188, 189, 34),   // 黄绿
        QColor(23, 190, 207),   // 青色
        QColor(174, 199, 232),  // 浅蓝
        QColor(255, 187, 120),  // 浅橙
        QColor(152, 223, 138),  // 浅绿
        QColor(255, 152, 150),  // 浅红
        QColor(197, 176, 213),  // 浅紫
        QColor(196, 156, 148)   // 浅棕
    };
    
    return colors[index % MAX_CHANNELS];
}

bool ChannelManager::isValidChannel(int id) const
{
    return channels_.contains(id);
}

} // namespace DeviceStudio
