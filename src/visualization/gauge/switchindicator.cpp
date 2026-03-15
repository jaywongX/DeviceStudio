/**
 * @file switchindicator.cpp
 * @brief 开关状态显示组件实现
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#include "switchindicator.h"
#include <QLinearGradient>

namespace DeviceStudio {

SwitchIndicator::SwitchIndicator(QWidget* parent)
    : QWidget(parent)
{
}

SwitchIndicator::SwitchIndicator(const SwitchConfig& config, QWidget* parent)
    : QWidget(parent)
    , config_(config)
{
}

SwitchIndicator::~SwitchIndicator()
{
}

void SwitchIndicator::setConfig(const SwitchConfig& config)
{
    config_ = config;
    update();
}

void SwitchIndicator::setTitle(const QString& title)
{
    config_.title = title;
    update();
}

void SwitchIndicator::setState(SwitchState state)
{
    if (config_.state != state) {
        config_.state = state;
        update();
        emit stateChanged(state);
    }
}

void SwitchIndicator::setOpen()
{
    setState(SwitchState::Open);
}

void SwitchIndicator::setClosed()
{
    setState(SwitchState::Closed);
}

void SwitchIndicator::setFault()
{
    setState(SwitchState::Fault);
}

void SwitchIndicator::setOpenColor(const QColor& color)
{
    config_.openColor = color;
    update();
}

void SwitchIndicator::setClosedColor(const QColor& color)
{
    config_.closedColor = color;
    update();
}

void SwitchIndicator::setFaultColor(const QColor& color)
{
    config_.faultColor = color;
    update();
}

void SwitchIndicator::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), config_.backgroundColor);
    
    // 计算开关指示器位置
    int indicatorSize = config_.indicatorSize;
    int indicatorX = (width() - indicatorSize) / 2;
    int indicatorY = config_.showLabel ? 15 : (height() - indicatorSize) / 2;
    
    // 绘制开关
    drawSwitch(painter, QRectF(indicatorX, indicatorY, indicatorSize, indicatorSize));
    
    // 绘制标签
    if (config_.showLabel) {
        painter.setPen(config_.textColor);
        painter.setFont(QFont("sans", 10, QFont::Bold));
        
        int titleY = indicatorY + indicatorSize + 10;
        QRect titleRect(0, titleY, width(), 20);
        painter.drawText(titleRect, Qt::AlignCenter, config_.title);
        
        // 绘制状态文本
        int stateY = titleY + 22;
        QRect stateRect(0, stateY, width(), 18);
        drawStateText(painter, stateRect);
    }
}

void SwitchIndicator::drawSwitch(QPainter& painter, const QRectF& rect)
{
    QColor currentColor = getCurrentColor();
    
    // 绘制外框
    painter.setPen(QPen(config_.borderColor, 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect, 8, 8);
    
    // 绘制开关符号
    int margin = 8;
    QRectF innerRect = rect.adjusted(margin, margin, -margin, -margin);
    
    painter.setPen(QPen(currentColor, 4, Qt::SolidLine, Qt::RoundCap));
    
    switch (config_.state) {
        case SwitchState::Open: {
            // 断开状态：绘制断开的线条
            qreal centerX = innerRect.center().x();
            qreal centerY = innerRect.center().y();
            
            // 左半部分
            painter.drawLine(QPointF(innerRect.left(), centerY),
                           QPointF(centerX - 5, centerY - 10));
            
            // 右半部分（向下倾斜）
            painter.drawLine(QPointF(centerX + 5, centerY),
                           QPointF(innerRect.right(), centerY));
            break;
        }
        case SwitchState::Closed: {
            // 闭合状态：绘制连通的线条
            qreal centerY = innerRect.center().y();
            painter.drawLine(QPointF(innerRect.left(), centerY),
                           QPointF(innerRect.right(), centerY));
            
            // 绘制闭合指示点
            painter.setBrush(currentColor);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(innerRect.center(), 5, 5);
            break;
        }
        case SwitchState::Fault: {
            // 故障状态：绘制X符号
            painter.drawLine(QPointF(innerRect.left(), innerRect.top()),
                           QPointF(innerRect.right(), innerRect.bottom()));
            painter.drawLine(QPointF(innerRect.left(), innerRect.bottom()),
                           QPointF(innerRect.right(), innerRect.top()));
            break;
        }
    }
    
    // 绘制发光效果（仅对闭合和故障状态）
    if (config_.state != SwitchState::Open) {
        // 光晕效果
        QColor glowColor(currentColor.red(), currentColor.green(), currentColor.blue(), 50);
        painter.setPen(Qt::NoPen);
        painter.setBrush(glowColor);
        painter.drawRoundedRect(rect.adjusted(-3, -3, 3, 3), 10, 10);
    }
}

void SwitchIndicator::drawStateText(QPainter& painter, const QRectF& rect)
{
    QColor currentColor = getCurrentColor();
    QString stateText = getStateText();
    
    painter.setPen(currentColor);
    painter.setFont(QFont("sans", 9, QFont::Bold));
    painter.drawText(rect, Qt::AlignCenter, stateText);
}

QColor SwitchIndicator::getCurrentColor() const
{
    switch (config_.state) {
        case SwitchState::Open:
            return config_.openColor;
        case SwitchState::Closed:
            return config_.closedColor;
        case SwitchState::Fault:
            return config_.faultColor;
        default:
            return config_.openColor;
    }
}

QString SwitchIndicator::getStateText() const
{
    switch (config_.state) {
        case SwitchState::Open:
            return tr("OFF");
        case SwitchState::Closed:
            return tr("ON");
        case SwitchState::Fault:
            return tr("FAULT");
        default:
            return QString();
    }
}

QSize SwitchIndicator::sizeHint() const
{
    if (config_.showLabel) {
        return QSize(qMax(80, config_.indicatorSize + 20), config_.indicatorSize + 70);
    }
    return QSize(config_.indicatorSize + 10, config_.indicatorSize + 10);
}

QSize SwitchIndicator::minimumSizeHint() const
{
    return QSize(config_.indicatorSize + 5, config_.indicatorSize + 5);
}

} // namespace DeviceStudio
