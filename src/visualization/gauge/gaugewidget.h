/**
 * @file gaugewidget.h
 * @brief 仪表盘组件
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>

namespace DeviceStudio {

/**
 * @brief 仪表盘样式
 */
enum class GaugeStyle {
    Circular,       // 圆形仪表盘
    Semicircle,     // 半圆形仪表盘
    Digital         // 数字显示
};

/**
 * @brief 仪表盘配置
 */
struct GaugeConfig {
    QString title = "Gauge";
    double minValue = 0.0;
    double maxValue = 100.0;
    double value = 0.0;
    QString unit = "";
    GaugeStyle style = GaugeStyle::Circular;
    int decimalPlaces = 1;
    QColor backgroundColor = QColor(40, 40, 40);
    QColor foregroundColor = QColor(100, 100, 100);
    QColor valueColor = QColor(0, 200, 255);
    QColor textColor = Qt::white;
    QColor tickColor = Qt::white;
};

/**
 * @brief 仪表盘组件类
 */
class GaugeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit GaugeWidget(QWidget* parent = nullptr);
    explicit GaugeWidget(const GaugeConfig& config, QWidget* parent = nullptr);
    ~GaugeWidget() override;
    
    // ========== 配置 ==========
    
    /**
     * @brief 设置配置
     */
    void setConfig(const GaugeConfig& config);
    GaugeConfig getConfig() const { return config_; }
    
    /**
     * @brief 设置标题
     */
    void setTitle(const QString& title);
    
    /**
     * @brief 设置范围
     */
    void setRange(double min, double max);
    
    /**
     * @brief 设置单位
     */
    void setUnit(const QString& unit);
    
    // ========== 值操作 ==========
    
    /**
     * @brief 设置当前值
     */
    void setValue(double value);
    
    /**
     * @brief 获取当前值
     */
    double value() const { return config_.value; }
    
    /**
     * @brief 设置颜色
     */
    void setColors(const QColor& background, const QColor& foreground, const QColor& value);
    
    /**
     * @brief 设置小数位数
     */
    void setDecimalPlaces(int places);

signals:
    void valueChanged(double value);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void drawCircularGauge(QPainter& painter);
    void drawSemicircleGauge(QPainter& painter);
    void drawDigitalGauge(QPainter& painter);
    void drawTicks(QPainter& painter, const QRectF& rect, int tickCount = 10);
    void drawNeedle(QPainter& painter, const QRectF& rect, double angle);
    void drawValue(QPainter& painter, const QRectF& rect);
    
    GaugeConfig config_;
    double animatedValue_ = 0.0;
    QTimer* animationTimer_ = nullptr;
};

} // namespace DeviceStudio
