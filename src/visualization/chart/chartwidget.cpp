/**
 * @file chartwidget.cpp
 * @brief 图表组件实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "chartwidget.h"
#include <QVBoxLayout>

namespace DeviceStudio {

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupPlot();
}

ChartWidget::ChartWidget(const ChartConfig& config, QWidget* parent)
    : QWidget(parent)
    , config_(config)
{
    setupUI();
    setupPlot();
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    plot_ = new QCustomPlot(this);
    layout->addWidget(plot_);
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
    
    // 设置交互
    plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
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

int ChartWidget::addGraph(const QString& name, const QColor& color)
{
    QCPGraph* graph = plot_->addGraph();
    graph->setName(name);
    
    // 设置曲线样式
    QPen pen(color);
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
            while (data->size() > config_.maxPoints) {
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

bool ChartWidget::exportToImage(const QString& filePath, int width, int height)
{
    return plot_->savePng(filePath, width, height);
}

void ChartWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    plot_->replot();
}

} // namespace DeviceStudio
