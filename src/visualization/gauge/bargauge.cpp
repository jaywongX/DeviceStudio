/**
 * @file bargauge.cpp
 * @brief 条形图仪表组件实现
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#include "bargauge.h"
#include <QPropertyAnimation>

namespace DeviceStudio {

BarGauge::BarGauge(QWidget* parent)
    : QWidget(parent)
{
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, [this]() {
        if (qAbs(animatedValue_ - config_.value) < 0.1) {
            animatedValue_ = config_.value;
            animationTimer_->stop();
        } else {
            animatedValue_ += (config_.value - animatedValue_) * 0.15;
        }
        update();
    });
}

BarGauge::BarGauge(const BarGaugeConfig& config, QWidget* parent)
    : QWidget(parent)
    , config_(config)
{
    animatedValue_ = config.value;
}

BarGauge::~BarGauge()
{
}

void BarGauge::setConfig(const BarGaugeConfig& config)
{
    config_ = config;
    animatedValue_ = config.value;
    update();
}

void BarGauge::setTitle(const QString& title)
{
    config_.title = title;
    update();
}

void BarGauge::setRange(double min, double max)
{
    config_.minValue = min;
    config_.maxValue = max;
    update();
}

void BarGauge::setUnit(const QString& unit)
{
    config_.unit = unit;
    update();
}

void BarGauge::setOrientation(BarOrientation orientation)
{
    config_.orientation = orientation;
    update();
}

void BarGauge::setValue(double value)
{
    value = qBound(config_.minValue, value, config_.maxValue);
    if (!qFuzzyCompare(config_.value, value)) {
        config_.value = value;
        
        // 启动动画
        if (!animationTimer_->isActive()) {
            animationTimer_->start(16);
        }
        
        emit valueChanged(value);
    }
}

void BarGauge::setBarColor(const QColor& color)
{
    config_.barColor = color;
    update();
}

void BarGauge::setThresholdColors(const QColor& warning, const QColor& critical)
{
    config_.barColorWarning = warning;
    config_.barColorCritical = critical;
    update();
}

void BarGauge::setThresholds(double warning, double critical)
{
    config_.warningThreshold = warning;
    config_.criticalThreshold = critical;
    update();
}

void BarGauge::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), config_.backgroundColor);
    
    if (config_.orientation == BarOrientation::Vertical) {
        drawVerticalBar(painter);
    } else {
        drawHorizontalBar(painter);
    }
}

void BarGauge::drawVerticalBar(QPainter& painter)
{
    int margin = 10;
    int barWidth = width() - 2 * margin;
    int barHeight = height() - 50;
    
    // 标题
    painter.setPen(config_.textColor);
    painter.setFont(QFont("sans", 10, QFont::Bold));
    painter.drawText(QRect(0, 5, width(), 20), Qt::AlignCenter, config_.title);
    
    // 条形背景区域
    QRectF barRect(margin, 30, barWidth, barHeight);
    painter.setBrush(config_.barBackgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(barRect, 5, 5);
    
    // 计算填充比例
    double ratio = (animatedValue_ - config_.minValue) / (config_.maxValue - config_.minValue);
    ratio = qBound(0.0, ratio, 1.0);
    
    // 绘制填充条
    int fillHeight = static_cast<int>(barHeight * ratio);
    QRectF fillRect(margin, 30 + barHeight - fillHeight, barWidth, fillHeight);
    
    painter.setBrush(getBarColorForValue(animatedValue_));
    painter.drawRoundedRect(fillRect, 5, 5);
    
    // 绘制数值
    drawValueText(painter, QRectF(0, height() - 20, width(), 20));
    
    // 绘制刻度
    painter.setPen(QPen(config_.textColor.darker(150), 1));
    painter.setFont(QFont("sans", 8));
    for (int i = 0; i <= 4; ++i) {
        int y = 30 + barHeight - (barHeight * i / 4);
        double val = config_.minValue + (config_.maxValue - config_.minValue) * i / 4;
        painter.drawLine(margin + barWidth + 2, y, margin + barWidth + 8, y);
        painter.drawText(margin + barWidth + 10, y + 4, QString::number(val, 'f', 0));
    }
}

void BarGauge::drawHorizontalBar(QPainter& painter)
{
    int margin = 10;
    int barHeight = 30;
    int barWidth = width() - 80;
    
    // 标题
    painter.setPen(config_.textColor);
    painter.setFont(QFont("sans", 10, QFont::Bold));
    painter.drawText(QRect(0, 5, width(), 20), Qt::AlignCenter, config_.title);
    
    // 条形背景区域
    QRectF barRect(margin, 30, barWidth, barHeight);
    painter.setBrush(config_.barBackgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(barRect, 5, 5);
    
    // 计算填充比例
    double ratio = (animatedValue_ - config_.minValue) / (config_.maxValue - config_.minValue);
    ratio = qBound(0.0, ratio, 1.0);
    
    // 绘制填充条
    int fillWidth = static_cast<int>(barWidth * ratio);
    QRectF fillRect(margin, 30, fillWidth, barHeight);
    
    painter.setBrush(getBarColorForValue(animatedValue_));
    painter.drawRoundedRect(fillRect, 5, 5);
    
    // 绘制数值
    drawValueText(painter, QRectF(barWidth + 20, 30, 50, barHeight));
    
    // 绘制刻度
    painter.setPen(QPen(config_.textColor.darker(150), 1));
    painter.setFont(QFont("sans", 8));
    for (int i = 0; i <= 4; ++i) {
        int x = margin + (barWidth * i / 4);
        double val = config_.minValue + (config_.maxValue - config_.minValue) * i / 4;
        painter.drawLine(x, 30 + barHeight + 2, x, 30 + barHeight + 8);
        painter.drawText(x - 10, 30 + barHeight + 22, QString::number(val, 'f', 0));
    }
}

void BarGauge::drawValueText(QPainter& painter, const QRectF& rect)
{
    painter.setPen(config_.textColor);
    painter.setFont(QFont("sans", 12, QFont::Bold));
    
    QString text = QString::number(animatedValue_, 'f', config_.decimalPlaces) + config_.unit;
    painter.drawText(rect, Qt::AlignCenter, text);
}

QColor BarGauge::getBarColorForValue(double value) const
{
    if (!config_.enableThresholdColors) {
        return config_.barColor;
    }
    
    double percentage = (value - config_.minValue) / (config_.maxValue - config_.minValue) * 100.0;
    
    if (percentage >= config_.criticalThreshold) {
        return config_.barColorCritical;
    } else if (percentage >= config_.warningThreshold) {
        return config_.barColorWarning;
    }
    
    return config_.barColor;
}

QSize BarGauge::sizeHint() const
{
    if (config_.orientation == BarOrientation::Vertical) {
        return QSize(80, 200);
    } else {
        return QSize(200, 80);
    }
}

QSize BarGauge::minimumSizeHint() const
{
    if (config_.orientation == BarOrientation::Vertical) {
        return QSize(60, 150);
    } else {
        return QSize(150, 60);
    }
}

} // namespace DeviceStudio
