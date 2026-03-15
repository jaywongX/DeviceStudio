/**
 * @file chartwidget.cpp
 * @brief 图表组件实现 - 多通道、缩放平移、数据导出
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#include "chartwidget.h"
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QFile>
#include <QTextStream>

namespace DeviceStudio {

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupPlot();
    connectChannelManager();
}

ChartWidget::ChartWidget(const ChartConfig& config, QWidget* parent)
    : QWidget(parent)
    , config_(config)
{
    setupUI();
    setupPlot();
    connectChannelManager();
}

ChartWidget::~ChartWidget()
{
    if (channelManager_) {
        delete channelManager_;
    }
}

void ChartWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    plot_ = new QCustomPlot(this);
    layout->addWidget(plot_);
    
    // 创建通道管理器
    channelManager_ = new ChannelManager(this);
}

void ChartWidget::setupPlot()
{
    if (!plot_) return;
    
    // 设置标题
    if (!config_.title.isEmpty()) {
        plot_->plotLayout()->insertRow(0);
        QCPTextElement* title = new QCPTextElement(plot_, config_.title);
        title->setFont(QFont("sans", 12, QFont::Bold));
        plot_->plotLayout()->addElement(0, 0, title);
    }
    
    // 设置坐标轴标签
    plot_->xAxis->setLabel(config_.xLabel);
    plot_->yAxis->setLabel(config_.yLabel);
    
    // 设置网格
    plot_->xAxis->grid()->setVisible(config_.showGrid);
    plot_->yAxis->grid()->setVisible(config_.showGrid);
    
    // 设置抗锯齿
    plot_->setAntialiasedElements(QCP::aeAll);
    
    // 设置图例
    if (config_.showLegend) {
        plot_->legend->setVisible(true);
        plot_->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);
    }
    
    // 设置交互（缩放和平移）
    QCP::Interactions interactions = QCP::iSelectPlottables;
    if (config_.enablePan) {
        interactions |= QCP::iRangeDrag;
    }
    if (config_.enableZoom) {
        interactions |= QCP::iRangeZoom;
    }
    plot_->setInteractions(interactions);
    
    // 保存初始范围
    originalXMin_ = plot_->xAxis->range().lower;
    originalXMax_ = plot_->xAxis->range().upper;
    originalYMin_ = plot_->yAxis->range().lower;
    originalYMax_ = plot_->yAxis->range().upper;
}

void ChartWidget::connectChannelManager()
{
    connect(channelManager_, &ChannelManager::channelAdded,
            this, &ChartWidget::onChannelAdded);
    connect(channelManager_, &ChannelManager::channelRemoved,
            this, &ChartWidget::onChannelRemoved);
    connect(channelManager_, &ChannelManager::channelConfigChanged,
            this, &ChartWidget::onChannelConfigChanged);
    connect(channelManager_, &ChannelManager::dataUpdated,
            this, &ChartWidget::onChannelDataUpdated);
}

void ChartWidget::setConfig(const ChartConfig& config)
{
    config_ = config;
    setupPlot();
    plot_->replot();
}

void ChartWidget::setTitle(const QString& title)
{
    config_.title = title;
    if (plot_->plotLayout()->rowCount() > 0) {
        QCPTextElement* titleElement = qobject_cast<QCPTextElement*>(plot_->plotLayout()->element(0, 0));
        if (titleElement) {
            titleElement->setText(title);
        }
    }
    plot_->replot();
}

void ChartWidget::setAxisLabels(const QString& xLabel, const QString& yLabel)
{
    config_.xLabel = xLabel;
    config_.yLabel = yLabel;
    plot_->xAxis->setLabel(xLabel);
    plot_->yAxis->setLabel(yLabel);
    plot_->replot();
}

// ========== 通道管理 ==========

int ChartWidget::addChannel(const QString& name, const QColor& color)
{
    return channelManager_->addChannel(name, color);
}

void ChartWidget::removeChannel(int id)
{
    channelManager_->removeChannel(id);
}

void ChartWidget::setChannelColor(int id, const QColor& color)
{
    channelManager_->setChannelColor(id, color);
}

void ChartWidget::setChannelVisible(int id, bool visible)
{
    channelManager_->setChannelVisible(id, visible);
}

void ChartWidget::addChannelData(int id, double x, double y)
{
    channelManager_->addDataPoint(id, x, y);
}

void ChartWidget::addChannelData(int id, double y)
{
    channelManager_->addDataPoint(id, y);
}

// ========== 通道事件处理 ==========

void ChartWidget::onChannelAdded(int id)
{
    ChannelConfig config = channelManager_->getChannelConfig(id);
    
    // 创建对应的图表曲线
    int graphIndex = addGraph(config.name, config.color);
    channelToGraphMap_[id] = graphIndex;
    config.graphIndex = graphIndex;
    
    // 更新通道配置
    channelManager_->setChannelConfig(id, config);
    
    plot_->replot();
}

void ChartWidget::onChannelRemoved(int id)
{
    if (channelToGraphMap_.contains(id)) {
        int graphIndex = channelToGraphMap_[id];
        removeGraph(graphIndex);
        channelToGraphMap_.remove(id);
        
        // 更新映射（因为移除后索引会变化）
        for (auto it = channelToGraphMap_.begin(); it != channelToGraphMap_.end(); ++it) {
            if (it.value() > graphIndex) {
                it.value()--;
            }
        }
    }
}

void ChartWidget::onChannelConfigChanged(int id)
{
    if (!channelToGraphMap_.contains(id)) return;
    
    int graphIndex = channelToGraphMap_[id];
    ChannelConfig config = channelManager_->getChannelConfig(id);
    
    if (graphIndex >= 0 && graphIndex < plot_->graphCount()) {
        QCPGraph* graph = plot_->graph(graphIndex);
        if (graph) {
            graph->setName(config.name);
            graph->setVisible(config.visible);
            
            // 更新颜色
            QPen pen(config.color);
            pen.setWidth(2);
            graph->setPen(pen);
            
            plot_->replot();
        }
    }
}

void ChartWidget::onChannelDataUpdated(int id)
{
    updateGraphFromChannel(id);
}

void ChartWidget::updateGraphFromChannel(int id)
{
    if (!channelToGraphMap_.contains(id)) return;
    
    int graphIndex = channelToGraphMap_[id];
    if (graphIndex < 0 || graphIndex >= plot_->graphCount()) return;
    
    QVector<double> xData = channelManager_->getChannelXData(id);
    QVector<double> yData = channelManager_->getChannelYData(id);
    
    plot_->graph(graphIndex)->setData(xData, yData);
    plot_->replot();
}

// ========== 兼容旧接口 ==========

int ChartWidget::addGraph(const QString& name, const QColor& color)
{
    QCPGraph* graph = plot_->addGraph();
    graph->setName(name);
    
    // 设置曲线样式
    QPen pen(color.isValid() ? color : Qt::blue);
    pen.setWidth(2);
    graph->setPen(pen);
    
    // 设置曲线类型
    switch (config_.type) {
        case ChartType::Line:
            graph->setLineStyle(QCPGraph::lsLine);
            break;
        case ChartType::Scatter:
            graph->setLineStyle(QCPGraph::lsNone);
            graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
            break;
        case ChartType::RealTime:
            graph->setLineStyle(QCPGraph::lsLine);
            break;
        default:
            break;
    }
    
    plot_->replot();
    return plot_->graphCount() - 1;
}

void ChartWidget::removeGraph(int index)
{
    if (index >= 0 && index < plot_->graphCount()) {
        plot_->removeGraph(index);
        plot_->replot();
    }
}

void ChartWidget::clearGraphs()
{
    plot_->clearGraphs();
    channelToGraphMap_.clear();
    plot_->replot();
}

void ChartWidget::setGraphData(int index, const QVector<double>& x, const QVector<double>& y)
{
    if (index >= 0 && index < plot_->graphCount()) {
        plot_->graph(index)->setData(x, y);
        plot_->replot();
    }
}

void ChartWidget::addDataPoint(int index, double x, double y)
{
    if (index >= 0 && index < plot_->graphCount()) {
        plot_->graph(index)->addData(x, y);
        
        // 限制数据点数量（实时图）
        if (config_.type == ChartType::RealTime) {
            QCPDataContainer<QCPGraphData>* data = plot_->graph(index)->data();
            while (data->size() > static_cast<size_t>(config_.maxPoints)) {
                data->remove(data->beginKey());
            }
        }
        
        plot_->replot();
    }
}

void ChartWidget::addDataPoint(int index, double y)
{
    addDataPoint(index, currentXIndex_++, y);
}

void ChartWidget::clearGraphData(int index)
{
    if (index >= 0 && index < plot_->graphCount()) {
        plot_->graph(index)->data()->clear();
        plot_->replot();
    }
}

// ========== 显示控制 ==========

void ChartWidget::rescaleAxes()
{
    plot_->rescaleAxes();
    plot_->replot();
}

void ChartWidget::setAxisRange(double xMin, double xMax, double yMin, double yMax)
{
    plot_->xAxis->setRange(xMin, xMax);
    plot_->yAxis->setRange(yMin, yMax);
    plot_->replot();
}

void ChartWidget::setYRange(double min, double max)
{
    plot_->yAxis->setRange(min, max);
    plot_->replot();
}

void ChartWidget::setLegendVisible(bool visible)
{
    config_.showLegend = visible;
    plot_->legend->setVisible(visible);
    plot_->replot();
}

void ChartWidget::setGridVisible(bool visible)
{
    config_.showGrid = visible;
    plot_->xAxis->grid()->setVisible(visible);
    plot_->yAxis->grid()->setVisible(visible);
    plot_->replot();
}

// ========== 缩放/平移控制 ==========

void ChartWidget::resetView()
{
    // 复位到原始范围
    plot_->xAxis->setRange(originalXMin_, originalXMax_);
    plot_->yAxis->setRange(originalYMin_, originalYMax_);
    
    // 或者自动调整
    plot_->rescaleAxes();
    
    plot_->replot();
}

void ChartWidget::setZoomEnabled(bool enabled)
{
    config_.enableZoom = enabled;
    
    QCP::Interactions interactions = plot_->interactions();
    if (enabled) {
        interactions |= QCP::iRangeZoom;
    } else {
        interactions &= ~QCP::iRangeZoom;
    }
    plot_->setInteractions(interactions);
}

void ChartWidget::setPanEnabled(bool enabled)
{
    config_.enablePan = enabled;
    
    QCP::Interactions interactions = plot_->interactions();
    if (enabled) {
        interactions |= QCP::iRangeDrag;
    } else {
        interactions &= ~QCP::iRangeDrag;
    }
    plot_->setInteractions(interactions);
}

// ========== 导出功能 ==========

bool ChartWidget::exportToImage(const QString& filePath, int width, int height)
{
    return plot_->savePng(filePath, width, height);
}

bool ChartWidget::exportToCSV(const QString& filePath)
{
    return channelManager_->exportToFile(filePath);
}

// ========== 事件处理 ==========

void ChartWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    plot_->replot();
}

void ChartWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        resetView();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void ChartWidget::wheelEvent(QWheelEvent* event)
{
    if (!config_.enableZoom) {
        QWidget::wheelEvent(event);
        return;
    }
    
    // QCustomPlot已经内置了滚轮缩放支持
    // 这里可以添加额外的逻辑，如限制缩放比例等
    QWidget::wheelEvent(event);
}

} // namespace DeviceStudio
