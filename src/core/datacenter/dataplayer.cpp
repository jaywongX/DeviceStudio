/**
 * @file dataplayer.cpp
 * @brief 数据回放器实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "dataplayer.h"
#include "log/logger.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QDataStream>
#include <QTextStream>

namespace DeviceStudio {

DataPlayer::DataPlayer(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &DataPlayer::onTimerTimeout);
    m_timer->setInterval(1);
}

DataPlayer::~DataPlayer()
{
    stop();
}

bool DataPlayer::loadFile(const QString& filePath)
{
    unloadFile();
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();
    
    if (suffix == "json") {
        m_format = DataRecorder::Format::Json;
        if (!loadJsonFile(filePath)) return false;
    } else if (suffix == "bin" || suffix == "dat") {
        m_format = DataRecorder::Format::Binary;
        if (!loadBinaryFile(filePath)) return false;
    } else if (suffix == "csv") {
        m_format = DataRecorder::Format::Csv;
        if (!loadCsvFile(filePath)) return false;
    } else {
        emit errorOccurred(tr("不支持的文件格式: %1").arg(suffix));
        return false;
    }
    m_loaded = true;
    DS_LOG_INFO("Loaded recording file: " + filePath.toStdString());
    return true;
}

void DataPlayer::unloadFile()
{
    stop();
    m_items.clear();
    m_metadata.clear();
    m_totalDuration = 0;
    m_loaded = false;
}

bool DataPlayer::play()
{
    if (!m_loaded || m_items.isEmpty()) {
        emit errorOccurred(tr("未加载文件"));
        return false;
    }
    if (m_state == State::Playing) return true;
    if (m_state == State::Paused) { resume(); return true; }
    
    m_currentIndex = 0;
    m_playStartTime = QDateTime::currentMSecsSinceEpoch();
    m_state = State::Playing;
    m_timer->start();
    emit stateChanged(m_state);
    return true;
}

void DataPlayer::pause()
{
    if (m_state != State::Playing) return;
    m_timer->stop();
    m_pausedPosition = currentPosition();
    m_state = State::Paused;
    emit stateChanged(m_state);
}

void DataPlayer::resume()
{
    if (m_state != State::Paused) return;
    m_playStartTime = QDateTime::currentMSecsSinceEpoch() - m_pausedPosition;
    m_state = State::Playing;
    m_timer->start();
    emit stateChanged(m_state);
}

void DataPlayer::stop()
{
    if (m_state == State::Stopped) return;
    m_timer->stop();
    m_currentIndex = 0;
    m_state = State::Stopped;
    emit stateChanged(m_state);
}

void DataPlayer::seek(qint64 position)
{
    if (!m_loaded || position < 0) return;
    position = qMin(position, m_totalDuration);
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].timestamp >= position) { m_currentIndex = i; break; }
    }
    m_pausedPosition = position;
    emit progressChanged(m_currentIndex, position);
}

void DataPlayer::seekToItem(int index)
{
    if (!m_loaded || index < 0 || index >= m_items.size()) return;
    m_currentIndex = index;
    qint64 position = m_items[index].timestamp;
    m_pausedPosition = position;
    emit progressChanged(m_currentIndex, position);
}

void DataPlayer::setSpeed(double speed)
{
    m_speed = qBound(0.1, speed, 10.0);
}

qint64 DataPlayer::currentPosition() const
{
    if (m_state == State::Stopped) return 0;
    if (m_state == State::Paused) return m_pausedPosition;
    qint64 elapsed = (QDateTime::currentMSecsSinceEpoch() - m_playStartTime) * m_speed;
    return qMin(elapsed, m_totalDuration);
}

RecordedDataItem DataPlayer::currentItem() const
{
    if (!m_loaded || m_currentIndex < 0 || m_currentIndex >= m_items.size())
        return RecordedDataItem();
    return m_items[m_currentIndex];
}

void DataPlayer::onTimerTimeout()
{
    if (m_currentIndex >= m_items.size()) {
        if (m_loop) {
            m_currentIndex = 0;
            m_playStartTime = QDateTime::currentMSecsSinceEpoch();
        } else {
            stop();
            emit playbackFinished();
            return;
        }
    }
    
    qint64 currentPos = currentPosition();
    while (m_currentIndex < m_items.size() && m_items[m_currentIndex].timestamp <= currentPos) {
        emit dataPlayed(m_items[m_currentIndex]);
        m_currentIndex++;
        emit progressChanged(m_currentIndex, currentPos);
    }
}

bool DataPlayer::loadJsonFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) { emit errorOccurred(tr("无法打开文件")); return false; }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) { emit errorOccurred(tr("无效的JSON格式")); return false; }
    
    QJsonObject root = doc.object();
    m_startTime = root["startTime"].toVariant().toLongLong();
    m_metadata = root["metadata"].toObject().toVariantMap();
    
    QJsonArray dataArray = root["data"].toArray();
    for (const auto& itemVal : dataArray) {
        QJsonObject obj = itemVal.toObject();
        RecordedDataItem item;
        item.timestamp = obj["timestamp"].toVariant().toLongLong();
        item.data = QByteArray::fromHex(obj["data"].toString().toLatin1());
        item.source = obj["source"].toString();
        item.isSent = obj["direction"].toString() == "TX";
        item.format = obj["format"].toString("hex");
        m_items.append(item);
    }
    if (!m_items.isEmpty()) m_totalDuration = m_items.last().timestamp;
    return true;
}

bool DataPlayer::loadBinaryFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) { emit errorOccurred(tr("无法打开文件")); return false; }
    
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_6_0);
    quint32 magic, version;
    stream >> magic >> version;
    if (magic != 0x44535243) { emit errorOccurred(tr("无效的二进制格式")); return false; }
    
    stream >> m_startTime;
    while (!stream.atEnd()) {
        RecordedDataItem item;
        stream >> item.timestamp >> item.data >> item.source >> item.isSent >> item.format;
        if (stream.status() == QDataStream::Ok) m_items.append(item);
    }
    if (!m_items.isEmpty()) m_totalDuration = m_items.last().timestamp;
    return true;
}

bool DataPlayer::loadCsvFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { emit errorOccurred(tr("无法打开文件")); return false; }
    
    QTextStream stream(&file);
    stream.readLine(); // skip header
    while (!stream.atEnd()) {
        QStringList parts = stream.readLine().split(',');
        if (parts.size() >= 5) {
            RecordedDataItem item;
            item.timestamp = parts[0].toLongLong();
            item.source = parts[1];
            item.isSent = parts[2] == "TX";
            item.format = parts[3];
            item.data = QByteArray::fromHex(parts[4].toLatin1());
            m_items.append(item);
        }
    }
    if (!m_items.isEmpty()) m_totalDuration = m_items.last().timestamp;
    return true;
}

} // namespace DeviceStudio
