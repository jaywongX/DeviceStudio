/**
 * @file ledindicator.cpp
 * @brief LED状态指示组件实现
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#include "ledindicator.h"
#include <QRadialGradient>

namespace DeviceStudio {

LedIndicator::LedIndicator(QWidget* parent)
    : QWidget(parent)
{
    blinkTimer_ = new QTimer(this);
    connect(blinkTimer_, &QTimer::timeout, this, &LedIndicator::updateBlinkState);
}

LedIndicator::LedIndicator(const LedConfig& config, QWidget* parent)
    : QWidget(parent)
    , config_(config)
{
    blinkTimer_ = new QTimer(this);
    connect(blinkTimer_, &QTimer::timeout, this, &LedIndicator::updateBlinkState);
}

LedIndicator::~LedIndicator()
{
}

void LedIndicator::setConfig(const LedConfig& config)
{
    config_ = config;
    
    // 更新定时器
    if (config.state == LedState::Blinking) {
        blinkTimer_->start(config.blinkInterval);
    } else if (config.state == LedState::Flashing) {
        blinkTimer_->start(config.flashInterval);
    } else {
        blinkTimer_->stop();
    }
    
    update();
}

void LedIndicator::setTitle(const QString& title)
{
    config_.title = title;
    update();
}

void LedIndicator::setState(LedState state)
{
    if (config_.state != state) {
        config_.state = state;
        
        // 根据状态启动/停止闪烁
        if (state == LedState::Blinking) {
            blinkToggle_ = true;
            blinkTimer_->start(config_.blinkInterval);
        } else if (state == LedState::Flashing) {
            blinkToggle_ = true;
            blinkTimer_->start(config_.flashInterval);
        } else {
            blinkTimer_->stop();
        }
        
        update();
        emit stateChanged(state);
    }
}

void LedIndicator::setOn(bool on)
{
    setState(on ? LedState::On : LedState::Off);
}

void LedIndicator::turnOn()
{
    setOn(true);
}

void LedIndicator::turnOff()
{
    setOn(false);
}

void LedIndicator::startBlinking()
{
    setState(LedState::Blinking);
}

void LedIndicator::startFlashing()
{
    setState(LedState::Flashing);
}

void LedIndicator::setOnColor(const QColor& color)
{
    config_.onColor = color;
    update();
}

void LedIndicator::setOffColor(const QColor& color)
{
    config_.offColor = color;
    update();
}

void LedIndicator::setBlinkColor(const QColor& color)
{
    config_.blinkColor = color;
    update();
}

void LedIndicator::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), config_.backgroundColor);
    
    // 计算LED位置
    int ledSize = config_.size;
    int ledX = (width() - ledSize) / 2;
    int ledY = config_.showLabel ? 10 : (height() - ledSize) / 2;
    
    // 绘制LED
    drawLed(painter, QRectF(ledX, ledY, ledSize, ledSize));
    
    // 绘制标签
    if (config_.showLabel) {
        painter.setPen(config_.textColor);
        painter.setFont(QFont("sans", 9));
        
        int textY = ledY + ledSize + 15;
        QRect textRect(0, textY, width(), 20);
        painter.drawText(textRect, Qt::AlignCenter, config_.title);
    }
}

void LedIndicator::drawLed(QPainter& painter, const QRectF& rect)
{
    // 确定当前颜色
    QColor currentColor;
    switch (config_.state) {
        case LedState::On:
            currentColor = config_.onColor;
            break;
        case LedState::Off:
            currentColor = config_.offColor;
            break;
        case LedState::Blinking:
        case LedState::Flashing:
            currentColor = blinkToggle_ ? config_.blinkColor : config_.offColor;
            break;
    }
    
    // 绘制外圈（边框）
    painter.setPen(QPen(config_.backgroundColor.lighter(130), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect);
    
    // 绘制LED主体（带渐变效果）
    QRadialGradient gradient(rect.center(), rect.width() / 2);
    
    if (config_.state == LedState::On || 
        ((config_.state == LedState::Blinking || config_.state == LedState::Flashing) && blinkToggle_)) {
        // 开启状态 - 发光效果
        gradient.setColorAt(0, currentColor.lighter(150));
        gradient.setColorAt(0.5, currentColor);
        gradient.setColorAt(1, currentColor.darker(130));
        
        // 绘制光晕
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(currentColor.red(), currentColor.green(), currentColor.blue(), 60));
        painter.drawEllipse(rect.adjusted(-5, -5, 5, 5));
    } else {
        // 关闭状态 - 平淡效果
        gradient.setColorAt(0, currentColor.lighter(120));
        gradient.setColorAt(0.5, currentColor);
        gradient.setColorAt(1, currentColor.darker(110));
    }
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawEllipse(rect);
    
    // 绘制高光
    QRectF highlightRect(rect.x() + rect.width() * 0.2, 
                         rect.y() + rect.height() * 0.15,
                         rect.width() * 0.4, 
                         rect.height() * 0.3);
    QRadialGradient highlightGradient(highlightRect.center(), highlightRect.width());
    highlightGradient.setColorAt(0, QColor(255, 255, 255, 80));
    highlightGradient.setColorAt(1, QColor(255, 255, 255, 0));
    painter.setBrush(highlightGradient);
    painter.drawEllipse(highlightRect);
}

void LedIndicator::updateBlinkState()
{
    blinkToggle_ = !blinkToggle_;
    update();
}

QSize LedIndicator::sizeHint() const
{
    if (config_.showLabel) {
        return QSize(qMax(60, config_.size + 20), config_.size + 45);
    }
    return QSize(config_.size + 10, config_.size + 10);
}

QSize LedIndicator::minimumSizeHint() const
{
    return QSize(config_.size + 5, config_.size + 5);
}

} // namespace DeviceStudio
