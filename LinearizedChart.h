#pragma once

#include <QObject>
#include <QWidget>
#include <QtCharts>

constexpr int LABEL_X_MARGIN = 30;

namespace Ui
{
	class LinearizedChart;
}

class LinearizedChart : public QWidget
{
	Q_OBJECT
public:
	LinearizedChart(QWidget* parent = nullptr);
	~LinearizedChart();

	void addLinearPoint(float x, float y);
	void clear();

protected:
	virtual void resizeEvent(QResizeEvent* event);

private:
	Ui::LinearizedChart* _ui = nullptr;

	QChart* _chart = nullptr;
	QChartView* _chartView = nullptr;
	QScatterSeries* _series = nullptr;
	QValueAxis* _horAxis = nullptr;
	QValueAxis* _vertAxis = nullptr;

	QLabel* _chartLabel = nullptr;
	QLineSeries* _linearFitSeries = nullptr;

private slots:
	void linearFit();
	void exportImage();
};

