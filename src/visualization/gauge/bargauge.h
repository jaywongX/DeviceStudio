/**
 * @file bargauge.h
 * @brief 条形图仪表组件
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>

namespace DeviceStudio {

/**
 * @brief 条形图方向
 */
enum class BarOrientation {
    Horizontal,     // 水平条形图
    Vertical        // 垂直条形图
};

/**
 * @brief 条形图仪表配置
 */
struct BarGaugeConfig {
    QString title = "Bar Gauge";
    double minValue = 0.0;
    double maxValue = 100.0;
    double value = 0.0;
    QString unit = "";
    BarOrientation orientation = BarOrientation::Vertical;
    int decimalPlaces = 1;
    
    // 颜色配置
    QColor backgroundColor = QColor(40, 40, 40);
    QColor barBackgroundColor = QColor(60, 60, 60);
    QColor barColor = QColor(0, 200, 255);
    QColor barColorWarning = QColor(255, 200, 0);    // 警告颜色
    QColor barColorCritical = QColor(255, 50, 50);   // 危险颜色
    QColor textColor = Qt::white;
    
    // 阈值配置
    double warningThreshold = 70.0;   // 警告阈值（百分比）
    double criticalThreshold = 90.0;  // 危险阈值（百分比）
    bool enableThresholdColors = true;
};

/**
 * @brief 条形图仪表组件
 */
class BarGauge : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit BarGauge(QWidget* parent = nullptr);
    explicit BarGauge(const BarGaugeConfig& config, QWidget* parent = nullptr);
    ~BarGauge() override;
    
    // ========== 配置 ==========
    
    void setConfig(const BarGaugeConfig& config);
    BarGaugeConfig getConfig() const { return config_; }
    
    void setTitle(const QString& title);
    void setRange(double min, double max);
    void setUnit(const QString& unit);
    void setOrientation(BarOrientation orientation);
    
    // ========== 值操作 ==========
    
    void setValue(double value);
    double value() const { return config_.value; }
    
    void setBarColor(const QColor& color);
    void setThresholdColors(const QColor& warning, const QColor& critical);
    void setThresholds(double warning, double critical);
    
signals:
    void valueChanged(double value);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void drawHorizontalBar(QPainter& painter);
    void drawVerticalBar(QPainter& painter);
    void drawValueText(QPainter& painter, const QRectF& rect);
    QColor getBarColorForValue(double value) const;
    
    BarGaugeConfig config_;
    double animatedValue_ = 0.0;
    QTimer* animationTimer_ = nullptr;
};

} // namespace DeviceStudio
