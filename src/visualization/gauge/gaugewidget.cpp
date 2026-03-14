/**
 * @file gaugewidget.cpp
 * @brief 仪表盘组件实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "gaugewidget.h"
#include <QtMath>

namespace DeviceStudio {

GaugeWidget::GaugeWidget(QWidget* parent)
    : QWidget(parent)
{
    animationTimer_ = new QTimer(this);
    connect(animationTimer_, &QTimer::timeout, this, [this]() {
        // 平滑动画
        double diff = config_.value - animatedValue_;
        if (qAbs(diff) < 0.1) {
            animatedValue_ = config_.value;
            animationTimer_->stop();
        } else {
            animatedValue_ += diff * 0.1;
        }
        update();
    });
}

GaugeWidget::GaugeWidget(const GaugeConfig& config, QWidget* parent)
    : GaugeWidget(parent)
{
    setConfig(config);
}

GaugeWidget::~GaugeWidget()
{
}

void GaugeWidget::setConfig(const GaugeConfig& config)
{
    config_ = config;
    animatedValue_ = config.value;
    update();
}

void GaugeWidget::setTitle(const QString& title)
{
    config_.title = title;
    update();
}

void GaugeWidget::setRange(double min, double max)
{
    config_.minValue = min;
    config_.maxValue = max;
    update();
}

void GaugeWidget::setUnit(const QString& unit)
{
    config_.unit = unit;
    update();
}

void GaugeWidget::setValue(double value)
{
    if (config_.value != value) {
        config_.value = qBound(config_.minValue, value, config_.maxValue);
        animationTimer_->start(16); // 60 FPS
        emit valueChanged(config_.value);
    }
}

void GaugeWidget::setColors(const QColor& background, const QColor& foreground, const QColor& value)
{
    config_.backgroundColor = background;
    config_.foregroundColor = foreground;
    config_.valueColor = value;
    update();
}

void GaugeWidget::setDecimalPlaces(int places)
{
    config_.decimalPlaces = places;
    update();
}

void GaugeWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), config_.backgroundColor);
    
    // 根据样式绘制仪表盘
    switch (config_.style) {
        case GaugeStyle::Circular:
            drawCircularGauge(painter);
            break;
        case GaugeStyle::Semicircle:
            drawSemicircleGauge(painter);
            break;
        case GaugeStyle::Digital:
            drawDigitalGauge(painter);
            break;
    }
}

QSize GaugeWidget::sizeHint() const
{
    return QSize(200, 200);
}

QSize GaugeWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

void GaugeWidget::drawCircularGauge(QPainter& painter)
{
    int side = qMin(width(), height());
    int margin = 20;
    
    QRectF rect(margin, margin, side - 2 * margin, side - 2 * margin);
    
    // 绘制外圈
    painter.setPen(QPen(config_.foregroundColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect);
    
    // 绘制前景弧
    double startAngle = 90 * 16; // Qt使用1/16度
    double spanAngle = -270 * 16 * ((animatedValue_ - config_.minValue) / (config_.maxValue - config_.minValue));
    
    painter.setPen(QPen(config_.valueColor, 8, Qt::SolidLine, Qt::RoundCap));
    QPainterPath path;
    path.arcMoveTo(rect, 225);
    path.arcTo(rect, 225, spanAngle / 16.0);
    painter.drawPath(path);
    
    // 绘制刻度
    drawTicks(painter, rect, 10);
    
    // 绘制指针
    double angle = 225 + (270 * ((animatedValue_ - config_.minValue) / (config_.maxValue - config_.minValue)));
    drawNeedle(painter, rect, angle);
    
    // 绘制值
    drawValue(painter, rect);
    
    // 绘制标题
    painter.setPen(config_.textColor);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(rect.adjusted(0, side * 0.2, 0, 0), Qt::AlignHCenter | Qt::AlignTop, config_.title);
}

void GaugeWidget::drawSemicircleGauge(QPainter& painter)
{
    int side = qMin(width(), height() * 2);
    int margin = 20;
    
    QRectF rect(margin, margin, side - 2 * margin, side - 2 * margin);
    
    // 绘制外圈（半圆）
    painter.setPen(QPen(config_.foregroundColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, 0, 180 * 16);
    
    // 绘制前景弧
    double spanAngle = -180 * 16 * ((animatedValue_ - config_.minValue) / (config_.maxValue - config_.minValue));
    
    painter.setPen(QPen(config_.valueColor, 8, Qt::SolidLine, Qt::RoundCap));
    QPainterPath path;
    path.arcMoveTo(rect, 180);
    path.arcTo(rect, 180, spanAngle / 16.0);
    painter.drawPath(path);
    
    // 绘制刻度
    drawTicks(painter, rect, 10);
    
    // 绘制指针
    double angle = 180 + (180 * ((animatedValue_ - config_.minValue) / (config_.maxValue - config_.minValue)));
    drawNeedle(painter, rect, angle);
    
    // 绘制值
    drawValue(painter, rect);
    
    // 绘制标题
    painter.setPen(config_.textColor);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(rect.adjusted(0, side * 0.1, 0, 0), Qt::AlignHCenter | Qt::AlignTop, config_.title);
}

void GaugeWidget::drawDigitalGauge(QPainter& painter)
{
    int margin = 10;
    QRectF rect = this->rect().adjusted(margin, margin, -margin, -margin);
    
    // 绘制边框
    painter.setPen(QPen(config_.foregroundColor, 2));
    painter.setBrush(config_.foregroundColor.lighter(20));
    painter.drawRoundedRect(rect, 10, 10);
    
    // 绘制标题
    painter.setPen(config_.textColor);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(rect.adjusted(0, 10, 0, 0), Qt::AlignHCenter | Qt::AlignTop, config_.title);
    
    // 绘制数值
    painter.setFont(QFont("Digital-7", 32, QFont::Bold));  // 数字字体
    painter.setPen(config_.valueColor);
    QString valueText = QString::number(animatedValue_, 'f', config_.decimalPlaces);
    painter.drawText(rect.adjusted(0, 30, 0, -20), Qt::AlignCenter, valueText);
    
    // 绘制单位
    painter.setFont(QFont("Arial", 10));
    painter.setPen(config_.textColor);
    painter.drawText(rect.adjusted(0, 0, 0, -5), Qt::AlignHCenter | Qt::AlignBottom, config_.unit);
}

void GaugeWidget::drawTicks(QPainter& painter, const QRectF& rect, int tickCount)
{
    painter.setPen(QPen(config_.tickColor, 2));
    painter.setFont(QFont("Arial", 8));
    
    double range = config_.maxValue - config_.minValue;
    double angleStep = 270.0 / (tickCount - 1);
    
    for (int i = 0; i < tickCount; ++i) {
        double angle = qDegreesToRadians(225.0 + i * angleStep);
        
        // 内外点
        QPointF innerPoint(
            rect.center().x() + (rect.width() / 2 - 15) * qCos(angle),
            rect.center().y() - (rect.height() / 2 - 15) * qSin(angle)
        );
        QPointF outerPoint(
            rect.center().x() + (rect.width() / 2 - 5) * qCos(angle),
            rect.center().y() - (rect.height() / 2 - 5) * qSin(angle)
        );
        
        painter.drawLine(innerPoint, outerPoint);
        
        // 刻度值
        double value = config_.minValue + (range * i) / (tickCount - 1);
        QPointF textPoint(
            rect.center().x() + (rect.width() / 2 - 30) * qCos(angle),
            rect.center().y() - (rect.height() / 2 - 30) * qSin(angle)
        );
        painter.drawText(QRectF(textPoint.x() - 15, textPoint.y() - 8, 30, 16),
                        Qt::AlignCenter, QString::number(value, 'f', 0));
    }
}

void GaugeWidget::drawNeedle(QPainter& painter, const QRectF& rect, double angle)
{
    painter.save();
    painter.translate(rect.center());
    painter.rotate(-angle);
    
    // 指针
    QPainterPath needlePath;
    needlePath.moveTo(0, -5);
    needlePath.lineTo(rect.width() / 2 - 20, 0);
    needlePath.lineTo(0, 5);
    needlePath.lineTo(-10, 0);
    needlePath.closeSubpath();
    
    painter.setBrush(config_.valueColor);
    painter.setPen(QPen(config_.valueColor.darker(), 2));
    painter.drawPath(needlePath);
    
    // 中心点
    painter.setBrush(config_.foregroundColor);
    painter.drawEllipse(QPointF(0, 0), 8, 8);
    
    painter.restore();
}

void GaugeWidget::drawValue(QPainter& painter, const QRectF& rect)
{
    QString valueText = QString::number(animatedValue_, 'f', config_.decimalPlaces) + " " + config_.unit;
    
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.setPen(config_.valueColor);
    
    QRectF textRect = rect.adjusted(0, rect.height() * 0.5, 0, -rect.height() * 0.1);
    painter.drawText(textRect, Qt::AlignCenter, valueText);
}

} // namespace DeviceStudio
