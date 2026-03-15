/**
 * @file ledindicator.h
 * @brief LED状态指示组件
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>

namespace DeviceStudio {

/**
 * @brief LED状态
 */
enum class LedState {
    Off,            // 关闭
    On,             // 开启
    Blinking,       // 闪烁
    Flashing        // 快闪
};

/**
 * @brief LED配置
 */
struct LedConfig {
    QString title = "LED";
    LedState state = LedState::Off;
    QColor onColor = QColor(0, 255, 0);     // 开启颜色（绿色）
    QColor offColor = QColor(60, 60, 60);    // 关闭颜色
    QColor blinkColor = QColor(255, 200, 0); // 闪烁颜色（黄色）
    QColor backgroundColor = QColor(40, 40, 40);
    QColor textColor = Qt::white;
    int size = 24;                           // LED大小
    bool showLabel = true;
    int blinkInterval = 500;                 // 闪烁间隔（毫秒）
    int flashInterval = 100;                 // 快闪间隔（毫秒）
};

/**
 * @brief LED状态指示组件
 */
class LedIndicator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(LedState state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QColor onColor READ onColor WRITE setOnColor)

public:
    explicit LedIndicator(QWidget* parent = nullptr);
    explicit LedIndicator(const LedConfig& config, QWidget* parent = nullptr);
    ~LedIndicator() override;
    
    // ========== 配置 ==========
    
    void setConfig(const LedConfig& config);
    LedConfig getConfig() const { return config_; }
    
    void setTitle(const QString& title);
    
    // ========== 状态操作 ==========
    
    void setState(LedState state);
    LedState state() const { return config_.state; }
    
    void setOn(bool on);
    bool isOn() const { return config_.state == LedState::On; }
    
    void turnOn();
    void turnOff();
    void startBlinking();
    void startFlashing();
    
    // ========== 颜色操作 ==========
    
    void setOnColor(const QColor& color);
    QColor onColor() const { return config_.onColor; }
    
    void setOffColor(const QColor& color);
    QColor offColor() const { return config_.offColor; }
    
    void setBlinkColor(const QColor& color);
    
signals:
    void stateChanged(LedState state);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void drawLed(QPainter& painter, const QRectF& rect);
    void updateBlinkState();
    
    LedConfig config_;
    bool blinkToggle_ = true;
    QTimer* blinkTimer_ = nullptr;
};

} // namespace DeviceStudio
