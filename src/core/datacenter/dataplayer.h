/**
 * @file dataplayer.h
 * @brief 数据回放器
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include "datarecorder.h"

namespace DeviceStudio {

/**
 * @brief 数据回放器
 */
class DataPlayer : public QObject
{
    Q_OBJECT

public:
    enum class State { Stopped, Playing, Paused };

    explicit DataPlayer(QObject* parent = nullptr);
    ~DataPlayer() override;

    // 文件加载
    bool loadFile(const QString& filePath);
    void unloadFile();
    bool isLoaded() const { return m_loaded; }
    DataRecorder::Format format() const { return m_format; }

    // 回放控制
    bool play();
    void pause();
    void resume();
    void stop();
    void seek(qint64 position);
    void seekToItem(int index);
    void setSpeed(double speed);
    void setLoop(bool enabled) { m_loop = enabled; }

    State state() const { return m_state; }
    bool isPlaying() const { return m_state == State::Playing; }
    bool isPaused() const { return m_state == State::Paused; }

    // 信息获取
    int totalCount() const { return m_items.size(); }
    int currentIndex() const { return m_currentIndex; }
    qint64 totalDuration() const { return m_totalDuration; }
    qint64 currentPosition() const;
    RecordedDataItem currentItem() const;
    const QList<RecordedDataItem>& items() const { return m_items; }
    QVariantMap metadata() const { return m_metadata; }

signals:
    void stateChanged(State state);
    void dataPlayed(const RecordedDataItem& item);
    void progressChanged(int currentIndex, qint64 currentPosition);
    void playbackFinished();
    void errorOccurred(const QString& error);

private slots:
    void onTimerTimeout();

private:
    bool loadJsonFile(const QString& filePath);
    bool loadBinaryFile(const QString& filePath);
    bool loadCsvFile(const QString& filePath);

private:
    bool m_loaded = false;
    State m_state = State::Stopped;
    DataRecorder::Format m_format = DataRecorder::Format::Json;
    
    QList<RecordedDataItem> m_items;
    QVariantMap m_metadata;
    qint64 m_totalDuration = 0;
    qint64 m_startTime = 0;
    
    int m_currentIndex = 0;
    double m_speed = 1.0;
    bool m_loop = false;
    
    QTimer* m_timer = nullptr;
    qint64 m_playStartTime = 0;
    qint64 m_pausedPosition = 0;
};

} // namespace DeviceStudio
