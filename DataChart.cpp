#include "DataChart.h"
#include "ui_DataChart.h"

DataChart::DataChart(QWidget* parent) : QWidget(parent)
  , _ui(new Ui::DataChart)
  , _chart(new QChart())
  , _chartView(new QChartView(_chart))
  , _series(new QScatterSeries())
{
	_ui->setupUi(this);

   _series->setName("Simulation data");

   _chart->addSeries(_series);
   _chart->setTitle("Sky surface brightness vs. number of stars");
   _chart->legend()->hide();

   _horAxis = new QValueAxis();
   _horAxis->setLabelFormat("%i");
   _horAxis->setTitleText("Number of stars [1]");
   _chart->addAxis(_horAxis, Qt::AlignBottom);
   _series->attachAxis(_horAxis);

   _vertAxis = new QValueAxis();
   _vertAxis->setTitleText("Sky surface brightness [mag\u22C5arcsec\u207B\u00B2]");
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

   QObject::connect(_ui->logFitButton, &QPushButton::pressed, this, &DataChart::logFit);
   QObject::connect(_ui->exportButton, &QPushButton::clicked, this, &DataChart::exportImage);
}

DataChart::~DataChart()
{
	delete _ui;
}

void DataChart::addDataPoint(const float x, const float y)
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

void DataChart::resizeEvent([[maybe_unused]] QResizeEvent* event)
{
	if (!_chartLabel) return;

	int x = _chart->plotArea().topRight().x() - _chartLabel->width();
	int y = _chart->plotArea().topRight().y() + _chartView->pos().y();

	_chartLabel->move(x, y);
}

void DataChart::logFit()
{
	if (_series->points().isEmpty()) return;

	if (_chartLabel) delete _chartLabel;
	_chartLabel = new QLabel(this);

	//https://mathworld.wolfram.com/LeastSquaresFittingLogarithmic.html
	//y = a + blnx
	const int n = _series->points().size();

	float yLnxSum = 0.f;
	float ySum = 0.f;
	float lnxSum = 0.f;
	float lnxSquareSum = 0.f;
	for (const QPointF& point : _series->points())
	{
		yLnxSum += point.y() * log(point.x());
		ySum += point.y();
		lnxSum += log(point.x());

		lnxSquareSum += pow(log(point.x()), 2.f);
	}
	const float lnxSumSquare = pow(lnxSum, 2.f);

	const float b = (n * yLnxSum - ySum * lnxSum) / (n * lnxSquareSum - lnxSumSquare);
	const float a = (ySum - b * lnxSum) / n;

	const float yAvg = ySum / n;
	float resSquaredSum = 0.f;
	float totalSquaredSum = 0.f;

	_logFitSeries = new QSplineSeries();
	_logFitSeries->setPen(QPen(QColor(Qt::darkBlue), 1));
	_logFitSeries->setName("f(x)");
	_chart->addSeries(_logFitSeries);
	_logFitSeries->attachAxis(_horAxis);
	_logFitSeries->attachAxis(_vertAxis);
	for (const QPointF& point : _series->points())
	{
		const float val = a + b * log(point.x());
		*_logFitSeries << QPointF(point.x(), val);

		resSquaredSum += pow(point.y() - val, 2.f);
		totalSquaredSum += pow(point.y() - yAvg, 2.f);
	}

	_chart->legend()->show();
	_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

	const float rSquared = 1.f - (resSquaredSum / totalSquaredSum);
	const QString sign = b < 0 ? QString() : " + ";
	_chartLabel->setText("f(x) = " + QString::number(a) + sign + QString::number(b) + "ln(x) \nR\u00B2 = " + QString::number(rSquared));
	_chartLabel->setStyleSheet("color:#000000; background-color:#ffffff; border: 1px solid black;");
	_chartLabel->adjustSize();
	int x = _chart->plotArea().topRight().x() - _chartLabel->width();
	int y = _chart->plotArea().topRight().y() + _chartView->pos().y();

	_chartLabel->move(x, y);
	_chartLabel->show();
}

void DataChart::clear()
{
	_series->clear();

	if (_chartLabel)
	{
		delete _chartLabel;
		_chartLabel = nullptr;
	}

	if (_logFitSeries)
	{
		_chart->removeSeries(_logFitSeries);
		delete _logFitSeries;
		_logFitSeries = nullptr;
	}

	_chart->legend()->hide();
}

void DataChart::exportImage()
{
	const QString fileName = QFileDialog::getSaveFileName(nullptr, "Export chart", QDir::homePath() + "/dataChart.png", "PNG files (*.png);;All files (*.*)");

	QImage image(_chartView->size(), QImage::Format_ARGB32);
	QPainter painter(&image);
	painter.setRenderHint(QPainter::Antialiasing);

	image.fill(Qt::transparent);
	_chartView->render(&painter);
	if (_chartLabel) _chartLabel->render(&painter, _chartLabel->pos());

	image.save(fileName);
}
