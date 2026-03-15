/**
 * @file switchindicator.h
 * @brief 开关状态显示组件
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>

namespace DeviceStudio {

/**
 * @brief 开关状态
 */
enum class SwitchState {
    Open,           // 断开
    Closed,         // 闭合
    Fault           // 故障
};

/**
 * @brief 开关配置
 */
struct SwitchConfig {
    QString title = "Switch";
    SwitchState state = SwitchState::Open;
    
    // 颜色配置
    QColor openColor = QColor(150, 150, 150);       // 断开颜色（灰色）
    QColor closedColor = QColor(0, 200, 0);         // 闭合颜色（绿色）
    QColor faultColor = QColor(255, 50, 50);        // 故障颜色（红色）
    QColor backgroundColor = QColor(40, 40, 40);
    QColor textColor = Qt::white;
    QColor borderColor = QColor(100, 100, 100);
    
    // 显示配置
    bool showLabel = true;
    int indicatorSize = 40;
};

/**
 * @brief 开关状态显示组件
 * 
 * 显示开关/断路器等设备的状态
 */
class SwitchIndicator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(SwitchState state READ state WRITE setState NOTIFY stateChanged)

public:
    explicit SwitchIndicator(QWidget* parent = nullptr);
    explicit SwitchIndicator(const SwitchConfig& config, QWidget* parent = nullptr);
    ~SwitchIndicator() override;
    
    // ========== 配置 ==========
    
    void setConfig(const SwitchConfig& config);
    SwitchConfig getConfig() const { return config_; }
    
    void setTitle(const QString& title);
    
    // ========== 状态操作 ==========
    
    void setState(SwitchState state);
    SwitchState state() const { return config_.state; }
    
    void setOpen();
    void setClosed();
    void setFault();
    
    bool isOpen() const { return config_.state == SwitchState::Open; }
    bool isClosed() const { return config_.state == SwitchState::Closed; }
    bool isFault() const { return config_.state == SwitchState::Fault; }
    
    // ========== 颜色操作 ==========
    
    void setOpenColor(const QColor& color);
    void setClosedColor(const QColor& color);
    void setFaultColor(const QColor& color);
    
signals:
    void stateChanged(SwitchState state);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void drawSwitch(QPainter& painter, const QRectF& rect);
    void drawStateText(QPainter& painter, const QRectF& rect);
    QColor getCurrentColor() const;
    QString getStateText() const;
    
    SwitchConfig config_;
};

} // namespace DeviceStudio
