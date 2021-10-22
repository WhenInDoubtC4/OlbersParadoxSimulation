#pragma once

#include <QObject>
#include <QWidget>
#include <QtCharts>

namespace Ui
{
	class DataChart;
}

class DataChart : public QWidget
{
	Q_OBJECT
public:
	DataChart(QWidget* parent = nullptr);
	~DataChart();

	void addDataPoint(const float x, const float y);
	void clear();

protected:
	virtual void resizeEvent(QResizeEvent* event);

private:
	Ui::DataChart* _ui = nullptr;
	QChart* _chart = nullptr;
	QChartView* _chartView = nullptr;
	QScatterSeries* _series = nullptr;
	QValueAxis* _horAxis = nullptr;
	QValueAxis* _vertAxis = nullptr;
	QLabel* _chartLabel = nullptr;
	QSplineSeries* _logFitSeries = nullptr;

private slots:
	void logFit();
	void exportImage();
};
