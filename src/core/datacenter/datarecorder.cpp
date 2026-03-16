/**
 * @file datarecorder.cpp
 * @brief 数据录制器实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "datarecorder.h"
#include "log/logger.h"

#include <QJsonDocument>
#include <QDir>
#include <QFileInfo>
#include <QDataStream>
#include <QMutexLocker>

namespace DeviceStudio {

DataRecorder::DataRecorder(QObject* parent)
    : QObject(parent)
{
}

DataRecorder::~DataRecorder()
{
    if (m_recording) {
        stopRecording();
    }
}

bool DataRecorder::startRecording(const QString& filePath, Format format)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_recording) {
        emit errorOccurred(tr("已经在录制中"));
        return false;
    }
    
    m_filePath = filePath;
    m_format = format;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_itemCount = 0;
    m_totalBytes = 0;
    m_splitIndex = 0;
    
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(tr("无法打开文件: %1").arg(filePath));
        return false;
    }
    
    if (m_format == Format::Json) {
        m_file.write("{\n");
        m_file.write("  \"version\": \"1.0\",\n");
        m_file.write("  \"startTime\": " + QByteArray::number(m_startTime) + ",\n");
        
        if (!m_metadata.isEmpty()) {
            QJsonDocument metaDoc(QJsonObject::fromVariantMap(m_metadata));
            m_file.write("  \"metadata\": " + metaDoc.toJson(QJsonDocument::Compact) + ",\n");
        }
        
        m_file.write("  \"data\": [\n");
    } else if (m_format == Format::Binary) {
        QDataStream stream(&m_file);
        stream.setVersion(QDataStream::Qt_6_0);
        stream << quint32(0x44535243);
        stream << quint32(1);
        stream << m_startTime;
    } else if (m_format == Format::Csv) {
        m_file.write("timestamp,source,direction,format,data\n");
    }
    
    m_recording = true;
    m_paused = false;
    
    DS_LOG_INFO("Recording started: " + filePath.toStdString());
    emit recordingStarted(filePath);
    
    return true;
}

void DataRecorder::stopRecording()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_recording) {
        return;
    }
    
    if (m_format == Format::Json) {
        m_file.write("\n  ]\n");
        m_file.write("  \"endTime\": " + QByteArray::number(QDateTime::currentMSecsSinceEpoch()) + ",\n");
        m_file.write("  \"totalCount\": " + QByteArray::number(m_itemCount) + "\n");
        m_file.write("}\n");
    }
    
    m_file.close();
    m_recording = false;
    m_paused = false;
    
    DS_LOG_INFO("Recording stopped: " + std::to_string(m_itemCount) + " items");
    emit recordingStopped(m_itemCount, m_totalBytes);
}

void DataRecorder::pause()
{
    QMutexLocker locker(&m_mutex);
    if (m_recording) {
        m_paused = true;
    }
}

void DataRecorder::resume()
{
    QMutexLocker locker(&m_mutex);
    if (m_recording && m_paused) {
        m_paused = false;
    }
}

void DataRecorder::recordReceived(const QByteArray& data, const QString& source, const QString& format)
{
    RecordedDataItem item;
    item.timestamp = QDateTime::currentMSecsSinceEpoch() - m_startTime;
    item.data = data;
    item.source = source;
    item.isSent = false;
    item.format = format;
    record(item);
}

void DataRecorder::recordSent(const QByteArray& data, const QString& source, const QString& format)
{
    RecordedDataItem item;
    item.timestamp = QDateTime::currentMSecsSinceEpoch() - m_startTime;
    item.data = data;
    item.source = source;
    item.isSent = true;
    item.format = format;
    record(item);
}

void DataRecorder::record(const RecordedDataItem& item)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_recording || m_paused) {
        return;
    }
    
    if (writeItem(item)) {
        m_itemCount++;
        m_totalBytes += item.data.size();
        checkFileSize();
        emit dataRecorded(item);
    }
}

void DataRecorder::setMetadata(const QString& key, const QVariant& value)
{
    QMutexLocker locker(&m_mutex);
    m_metadata[key] = value;
}

void DataRecorder::setAutoSplit(bool enabled, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);
    m_autoSplit = enabled;
    m_splitSize = maxSize;
}

qint64 DataRecorder::duration() const
{
    if (!m_recording) {
        return 0;
    }
    return QDateTime::currentMSecsSinceEpoch() - m_startTime;
}

bool DataRecorder::writeItem(const RecordedDataItem& item)
{
    switch (m_format) {
    case Format::Json:
        return writeJsonItem(item);
    case Format::Binary:
        return writeBinaryItem(item);
    case Format::Csv:
        return writeCsvItem(item);
    }
    return false;
}

bool DataRecorder::writeJsonItem(const RecordedDataItem& item)
{
    QJsonObject obj;
    obj["timestamp"] = item.timestamp;
    obj["data"] = QString(item.data.toHex());
    obj["source"] = item.source;
    obj["direction"] = item.isSent ? "TX" : "RX";
    obj["format"] = item.format;
    
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson(QJsonDocument::Compact);
    
    if (m_itemCount > 0) {
        m_file.write(",\n");
    }
    
    m_file.write("    " + json);
    m_file.flush();
    return true;
}

bool DataRecorder::writeBinaryItem(const RecordedDataItem& item)
{
    QDataStream stream(&m_file);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << item.timestamp << item.data << item.source << item.isSent << item.format;
    m_file.flush();
    return true;
}

bool DataRecorder::writeCsvItem(const RecordedDataItem& item)
{
    QString line = QString("%1,%2,%3,%4,%5\n")
        .arg(item.timestamp)
        .arg(item.source)
        .arg(item.isSent ? "TX" : "RX")
        .arg(item.format)
        .arg(QString(item.data.toHex()));
    
    m_file.write(line.toUtf8());
    m_file.flush();
    return true;
}

void DataRecorder::checkFileSize()
{
    if (!m_autoSplit || m_splitSize <= 0) {
        return;
    }
    
    if (m_file.size() >= m_splitSize) {
        splitFile();
    }
}

bool DataRecorder::splitFile()
{
    QString oldFile = m_filePath;
    
    if (m_format == Format::Json) {
        m_file.write("\n  ]\n  \"split\": true\n}\n");
    }
    m_file.close();
    
    m_splitIndex++;
    QFileInfo info(m_filePath);
    QString baseName = info.completeBaseName();
    QString newPath = info.dir().filePath(
        QString("%1_%2.%3").arg(baseName).arg(m_splitIndex, 3, 10, QChar('0')).arg(info.suffix()));
    
    m_file.setFileName(newPath);
    if (!m_file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(tr("无法创建分割文件: %1").arg(newPath));
        m_recording = false;
        return false;
    }
    
    if (m_format == Format::Json) {
        m_file.write("{\n");
        m_file.write("  \"version\": \"1.0\",\n");
        m_file.write("  \"startTime\": " + QByteArray::number(m_startTime) + ",\n");
        m_file.write("  \"splitIndex\": " + QByteArray::number(m_splitIndex) + ",\n");
        m_file.write("  \"data\": [\n");
    }
    
    m_filePath = newPath;
    emit fileSplit(oldFile, newPath);
    return true;
}

} // namespace DeviceStudio
