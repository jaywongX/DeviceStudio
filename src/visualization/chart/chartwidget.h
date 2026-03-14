/**
 * @file chartwidget.h
 * @brief 图表组件
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#pragma once

#include <QWidget>
#include <QVector>
#include "qcustomplot.h"

namespace DeviceStudio {

/**
 * @brief 图表类型
 */
enum class ChartType {
    Line,           // 折线图
    Scatter,        // 散点图
    Bar,            // 柱状图
    RealTime        // 实时曲线
};

/**
 * @brief 图表配置
 */
struct ChartConfig {
    QString title;
    QString xLabel = "X";
    QString yLabel = "Y";
    ChartType type = ChartType::Line;
    bool showLegend = true;
    bool showGrid = true;
    bool antialiased = true;
    int maxPoints = 1000;           // 最大数据点数（实时图）
    double refreshRate = 30.0;      // 刷新率（Hz）
};

/**
 * @brief 图表组件类
 */
class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);
    explicit ChartWidget(const ChartConfig& config, QWidget* parent = nullptr);
    ~ChartWidget() override;
    
    // ========== 配置 ==========
    
    /**
     * @brief 设置图表配置
     */
    void setConfig(const ChartConfig& config);
    ChartConfig getConfig() const { return config_; }
    
    /**
     * @brief 设置标题
     */
    void setTitle(const QString& title);
    
    /**
     * @brief 设置坐标轴标签
     */
    void setAxisLabels(const QString& xLabel, const QString& yLabel);
    
    // ========== 数据操作 ==========
    
    /**
     * @brief 添加曲线
     * @param name 曲线名称
     * @param color 曲线颜色
     * @return 曲线索引
     */
    int addGraph(const QString& name, const QColor& color = Qt::blue);
    
    /**
     * @brief 移除曲线
     */
    void removeGraph(int index);
    
    /**
     * @brief 清空所有曲线
     */
    void clearGraphs();
    
    /**
     * @brief 设置曲线数据
     */
    void setGraphData(int index, const QVector<double>& x, const QVector<double>& y);
    
    /**
     * @brief 添加数据点（实时图）
     */
    void addDataPoint(int index, double x, double y);
    
    /**
     * @brief 添加数据点（实时图，自动生成X值）
     */
    void addDataPoint(int index, double y);
    
    /**
     * @brief 清空曲线数据
     */
    void clearGraphData(int index);
    
    // ========== 显示控制 ==========
    
    /**
     * @brief 自动调整坐标轴范围
     */
    void rescaleAxes();
    
    /**
     * @brief 设置坐标轴范围
     */
    void setAxisRange(double xMin, double xMax, double yMin, double yMax);
    
    /**
     * @brief 设置Y轴范围
     */
    void setYRange(double min, double max);
    
    /**
     * @brief 显示/隐藏图例
     */
    void setLegendVisible(bool visible);
    
    /**
     * @brief 显示/隐藏网格
     */
    void setGridVisible(bool visible);
    
    /**
     * @brief 导出为图片
     */
    bool exportToImage(const QString& filePath, int width = 800, int height = 600);
    
    /**
     * @brief 获取QCustomPlot对象
     */
    QCustomPlot* plot() const { return plot_; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void setupPlot();
    
    QCustomPlot* plot_ = nullptr;
    ChartConfig config_;
    QVector<double> xData_;     // 用于实时图
    int currentXIndex_ = 0;     // 当前X索引
};

} // namespace DeviceStudio
