#include "LinearizedChart.h"
#include "ui_LinearizedChart.h"

LinearizedChart::LinearizedChart(QWidget* parent) : QWidget(parent)
  , _ui(new Ui::LinearizedChart)
  , _chart(new QChart())
  , _chartView(new QChartView(_chart))
  , _series(new QScatterSeries())
{
	_ui->setupUi(this);

	_series->setName("Linearized simulation data");

	_chart->addSeries(_series);
	_chart->setTitle("Inverse exponential sky surface brightness vs. number of stars");
	_chart->legend()->hide();

	_horAxis = new QValueAxis();
	_horAxis->setLabelFormat("%i");
	_horAxis->setTitleText("Number of stars [1]");
	_chart->addAxis(_horAxis, Qt::AlignBottom);
	_series->attachAxis(_horAxis);

	_vertAxis = new QValueAxis();
	_vertAxis->setTitleText("Inverse exponential sky surface brightness [idk??]");
	_chart->addAxis(_vertAxis, Qt::AlignLeft);
	_series->attachAxis(_vertAxis);

	QPolygon ln1;
	QPolygon ln2;
	ln1 << QPoint(0, 0) << QPoint(10, 10);
	ln2 << QPoint(0, 10) << QPoint(10, 0);
	QPainterPath path;
	path.addPolygon(ln1);
	path.addPolygon(ln2);

	QImage cross(10, 10, QImage::Format_ARGB32);
	cross.fill(Qt::transparent);

	QPen pen(Qt::blue, 2);

	QPainter painter(&cross);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(pen);
	painter.drawPath(path);

	_series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
	_series->setMarkerSize(10);
	_series->setBrush(cross);
	_series->setPen(QColor(Qt::transparent));

	_chartView->setRenderHint(QPainter::Antialiasing);
	_ui->gridLayout->addWidget(_chartView, 1, 0, 1, 9);

	QObject::connect(_ui->hTickCountSpinBox, qOverload<int>(&QSpinBox::valueChanged), [=](int value)
	{
		_horAxis->setTickCount(value);
	});
	QObject::connect(_ui->hMinorTickCountSpinBox, qOverload<int>(&QSpinBox::valueChanged), [=](int value)
	{
	   _horAxis->setMinorTickCount(value);
	});
	QObject::connect(_ui->vTickCountSpinBox, qOverload<int>(&QSpinBox::valueChanged), [=](int value)
	{
		_vertAxis->setTickCount(value);
	});
	QObject::connect(_ui->vMinorTickCountSpinBox, qOverload<int>(&QSpinBox::valueChanged), [=](int value)
	{
	   _vertAxis->setMinorTickCount(value);
	});

	QObject::connect(_ui->linearFitButton, &QPushButton::pressed, this, &LinearizedChart::linearFit);
	QObject::connect(_ui->exportButton, &QPushButton::clicked, this, &LinearizedChart::exportImage);
}

LinearizedChart::~LinearizedChart()
{
	delete _ui;
}

void LinearizedChart::addLinearPoint(float x, float y)
{
	_series->append(x, y);

	float xmax = -INFINITY;
	float ymin = INFINITY;
	float ymax = -INFINITY;
	for (QPointF& dataPoint : _series->points())
	{
		if (dataPoint.x() > xmax) xmax = dataPoint.x();
		if (dataPoint.y() < ymin) ymin = dataPoint.y();
		if (dataPoint.y() > ymax) ymax = dataPoint.y();
	}
	_horAxis->setMax(xmax);
	_vertAxis->setMin(ymin);
	_vertAxis->setMax(ymax);

	_horAxis->applyNiceNumbers();
	_vertAxis->applyNiceNumbers();

	_ui->hTickCountSpinBox->setValue(_horAxis->tickCount());
	_ui->hMinorTickCountSpinBox->setValue(_horAxis->minorTickCount());
	_ui->vTickCountSpinBox->setValue(_vertAxis->tickCount());
	_ui->vMinorTickCountSpinBox->setValue(_vertAxis->minorTickCount());
}


void LinearizedChart::resizeEvent([[maybe_unused]] QResizeEvent* event)
{
	if (!_chartLabel) return;

	int x = _chart->plotArea().topLeft().x() + LABEL_X_MARGIN;
	int y = _chart->plotArea().topLeft().y() + _chartView->pos().y();

	_chartLabel->move(x, y);
}

void LinearizedChart::linearFit()
{
	if (_series->points().isEmpty()) return;

	if (_chartLabel) delete _chartLabel;
	_chartLabel = new QLabel(this);

	//https://mathworld.wolfram.com/LeastSquaresFitting.html
	//y = a + bx
	const int n = _series->points().size();

	float ySum = 0.f;
	float xSquaredSum = 0.f;
	float xSum = 0.f;
	float xySum = 0.f;
	for (const QPointF& point : _series->points())
	{
		ySum += point.y();
		xSquaredSum += pow(point.x(), 2.f);
		xSum += point.x();
		xySum += point.x() * point.y();
	}
	const float xSumSquared = pow(xSum, 2.f);

	const float a = (ySum * xSquaredSum - xSum * xySum) / (n * xSquaredSum - xSumSquared);
	const float b = (n * xySum - xSum * ySum) / (n * xSquaredSum - xSumSquared);

	const float yAvg = ySum / n;
	float resSquaredSum = 0.f;
	float totalSquaredSum = 0.f;

	_linearFitSeries = new QLineSeries();
	_linearFitSeries->setPen(QColor(Qt::darkBlue));
	_linearFitSeries->setName("f(x)");
	_chart->addSeries(_linearFitSeries);
	_linearFitSeries->attachAxis(_horAxis);
	_linearFitSeries->attachAxis(_vertAxis);
	for (const QPointF& point : _series->points())
	{
		const float val = a + b * point.x();
		*_linearFitSeries << QPointF(point.x(), val);

		resSquaredSum += pow(point.y() - val, 2.f);
		totalSquaredSum += pow(point.y() - yAvg, 2.f);
	}

	_chart->legend()->show();
	_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

	const float rSquared = 1.f - (resSquaredSum / totalSquaredSum);
	const QString sign = b < 0 ? QString() : " + ";
	_chartLabel->setText("f(x) = " + QString::number(a) + sign + QString::number(b) + " x \nR\u00B2 = " + QString::number(rSquared));
	_chartLabel->setStyleSheet("color:#000000; background-color:#ffffff; border: 1px solid black;");
	_chartLabel->adjustSize();
	int x = _chart->plotArea().topLeft().x() + LABEL_X_MARGIN;
	int y = _chart->plotArea().topLeft().y() + _chartView->pos().y();

	_chartLabel->move(x, y);
	_chartLabel->show();
}

void LinearizedChart::clear()
{
	_series->clear();

	if (_chartLabel)
	{
		delete _chartLabel;
		_chartLabel = nullptr;
	}

	if (_linearFitSeries)
	{
		_chart->removeSeries(_linearFitSeries);
		delete _linearFitSeries;
		_linearFitSeries = nullptr;
	}

	_chart->legend()->hide();
}

void LinearizedChart::exportImage()
{
	const QString fileName = QFileDialog::getSaveFileName(nullptr, "Export chart", QDir::homePath() + "/linearizedChart.png", "PNG files (*.png);;All files (*.*)");

	QImage image(_chartView->size(), QImage::Format_ARGB32);
	QPainter painter(&image);
	painter.setRenderHint(QPainter::Antialiasing);

	image.fill(Qt::transparent);
	_chartView->render(&painter);
	if (_chartLabel) _chartLabel->render(&painter, _chartLabel->pos());

	image.save(fileName);
}
