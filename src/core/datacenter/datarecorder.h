/**
 * @file datarecorder.h
 * @brief 数据录制器
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include <QObject>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>

namespace DeviceStudio {

/**
 * @brief 录制数据项
 */
struct RecordedDataItem {
    qint64 timestamp;       ///< 时间戳(毫秒)
    QByteArray data;        ///< 数据
    QString source;         ///< 来源(设备ID或名称)
    bool isSent;            ///< 是否是发送的数据
    QString format;         ///< 数据格式(hex/ascii)
};

/**
 * @brief 数据录制器
 * 
 * 支持将通信数据录制到文件，支持JSON和二进制格式
 */
class DataRecorder : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 录制格式
     */
    enum class Format {
        Json,       ///< JSON格式(可读性好)
        Binary,     ///< 二进制格式(体积小)
        Csv         ///< CSV格式(易于分析)
    };

    explicit DataRecorder(QObject* parent = nullptr);
    ~DataRecorder() override;

    // ========== 录制控制 ==========

    /**
     * @brief 开始录制
     * @param filePath 文件路径
     * @param format 录制格式
     * @return 是否成功
     */
    bool startRecording(const QString& filePath, Format format = Format::Json);

    /**
     * @brief 停止录制
     */
    void stopRecording();

    /**
     * @brief 是否正在录制
     */
    bool isRecording() const { return m_recording; }

    /**
     * @brief 暂停录制
     */
    void pause();

    /**
     * @brief 继续录制
     */
    void resume();

    /**
     * @brief 是否暂停
     */
    bool isPaused() const { return m_paused; }

    // ========== 数据记录 ==========

    /**
     * @brief 记录接收的数据
     * @param data 数据
     * @param source 数据来源
     * @param format 数据格式
     */
    void recordReceived(const QByteArray& data, const QString& source = QString(),
                       const QString& format = "hex");

    /**
     * @brief 记录发送的数据
     * @param data 数据
     * @param source 数据来源
     * @param format 数据格式
     */
    void recordSent(const QByteArray& data, const QString& source = QString(),
                   const QString& format = "hex");

    /**
     * @brief 记录数据
     * @param item 数据项
     */
    void record(const RecordedDataItem& item);

    // ========== 配置 ==========

    /**
     * @brief 设置元数据
     * @param key 键
     * @param value 值
     */
    void setMetadata(const QString& key, const QVariant& value);

    /**
     * @brief 获取元数据
     */
    QVariantMap metadata() const { return m_metadata; }

    /**
     * @brief 设置最大文件大小(字节)
     * @param maxSize 最大大小,0表示无限制
     */
    void setMaxFileSize(qint64 maxSize) { m_maxFileSize = maxSize; }

    /**
     * @brief 设置自动分割
     * @param enabled 是否启用
     * @param maxSize 分割大小
     */
    void setAutoSplit(bool enabled, qint64 maxSize = 10 * 1024 * 1024);

    // ========== 统计 ==========

    /**
     * @brief 获取录制的数据项数量
     */
    qint64 itemCount() const { return m_itemCount; }

    /**
     * @brief 获取录制的字节数
     */
    qint64 totalBytes() const { return m_totalBytes; }

    /**
     * @brief 获取录制时长(毫秒)
     */
    qint64 duration() const;

    /**
     * @brief 获取当前文件路径
     */
    QString currentFilePath() const { return m_filePath; }

signals:
    /**
     * @brief 录制开始信号
     */
    void recordingStarted(const QString& filePath);

    /**
     * @brief 录制停止信号
     */
    void recordingStopped(qint64 itemCount, qint64 totalBytes);

    /**
     * @brief 数据记录信号
     */
    void dataRecorded(const RecordedDataItem& item);

    /**
     * @brief 文件分割信号
     */
    void fileSplit(const QString& oldFile, const QString& newFile);

    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& error);

private:
    // 写入数据项
    bool writeItem(const RecordedDataItem& item);

    // 写入JSON格式
    bool writeJsonItem(const RecordedDataItem& item);

    // 写入二进制格式
    bool writeBinaryItem(const RecordedDataItem& item);

    // 写入CSV格式
    bool writeCsvItem(const RecordedDataItem& item);

    // 检查文件大小
    void checkFileSize();

    // 分割文件
    bool splitFile();

private:
    bool m_recording = false;
    bool m_paused = false;
    Format m_format = Format::Json;
    QString m_filePath;
    QFile m_file;
    
    qint64 m_startTime = 0;
    qint64 m_itemCount = 0;
    qint64 m_totalBytes = 0;
    
    QVariantMap m_metadata;
    qint64 m_maxFileSize = 0;
    bool m_autoSplit = false;
    qint64 m_splitSize = 10 * 1024 * 1024;
    int m_splitIndex = 0;
    
    mutable QMutex m_mutex;
};

} // namespace DeviceStudio
