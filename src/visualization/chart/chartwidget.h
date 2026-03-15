/**
 * @file chartwidget.h
 * @brief 图表组件 - 支持多通道显示、缩放平移、数据导出
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#pragma once

#include <QWidget>
#include <QVector>
#include <QMouseEvent>
#include "qcustomplot.h"
#include "channelmanager.h"

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
    bool enableZoom = true;         // 启用缩放
    bool enablePan = true;          // 启用平移
};

/**
 * @brief 图表组件类
 * 
 * 支持功能：
 * - 多通道曲线显示（最多16通道）
 * - 通道颜色自定义
 * - 鼠标滚轮缩放
 * - 拖拽平移
 * - 双击复位
 * - 数据导出CSV
 * - 截图保存
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
    
    // ========== 通道管理 ==========
    
    /**
     * @brief 获取通道管理器
     */
    ChannelManager* channelManager() { return channelManager_; }
    const ChannelManager* channelManager() const { return channelManager_; }
    
    /**
     * @brief 添加通道
     */
    int addChannel(const QString& name, const QColor& color = QColor());
    
    /**
     * @brief 移除通道
     */
    void removeChannel(int id);
    
    /**
     * @brief 设置通道颜色
     */
    void setChannelColor(int id, const QColor& color);
    
    /**
     * @brief 设置通道可见性
     */
    void setChannelVisible(int id, bool visible);
    
    /**
     * @brief 添加通道数据点
     */
    void addChannelData(int id, double x, double y);
    void addChannelData(int id, double y);
    
    // ========== 曲线操作（兼容旧接口）==========
    
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
    
    // ========== 缩放/平移控制 ==========
    
    /**
     * @brief 复位视图（双击效果）
     */
    void resetView();
    
    /**
     * @brief 启用/禁用缩放
     */
    void setZoomEnabled(bool enabled);
    bool isZoomEnabled() const { return config_.enableZoom; }
    
    /**
     * @brief 启用/禁用平移
     */
    void setPanEnabled(bool enabled);
    bool isPanEnabled() const { return config_.enablePan; }
    
    // ========== 导出功能 ==========
    
    /**
     * @brief 导出为图片
     */
    bool exportToImage(const QString& filePath, int width = 800, int height = 600);
    
    /**
     * @brief 导出数据为CSV
     */
    bool exportToCSV(const QString& filePath);
    
    /**
     * @brief 获取QCustomPlot对象
     */
    QCustomPlot* plot() const { return plot_; }

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void onChannelAdded(int id);
    void onChannelRemoved(int id);
    void onChannelConfigChanged(int id);
    void onChannelDataUpdated(int id);

private:
    void setupUI();
    void setupPlot();
    void connectChannelManager();
    void updateGraphFromChannel(int id);
    
    QCustomPlot* plot_ = nullptr;
    ChannelManager* channelManager_ = nullptr;
    ChartConfig config_;
    QVector<double> xData_;     // 用于实时图
    int currentXIndex_ = 0;     // 当前X索引
    QMap<int, int> channelToGraphMap_;  // 通道ID到图表索引的映射
    
    // 保存原始范围用于复位
    double originalXMin_ = 0, originalXMax_ = 100;
    double originalYMin_ = 0, originalYMax_ = 100;
};

} // namespace DeviceStudio
