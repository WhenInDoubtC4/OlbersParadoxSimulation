#include "DataTable.h"
#include "ui_DataTable.h"

#include <QDebug>

DataTable::DataTable(QWidget* parent) : QWidget(parent)
  , _ui(new Ui::DataTable)
{
	_ui->setupUi(this);

	QObject::connect(_ui->exportButton, &QPushButton::clicked, this, &DataTable::onExport);
}

DataTable::~DataTable()
{
	delete _ui;
}

void DataTable::setCameraData(const float hfov, const float vfov, const float areaSqDeg, const float areaSqArcsec)
{
	_ui->hfovLineEdit->setText(QString::number(hfov));
	_ui->vfovLineEdit->setText(QString::number(vfov));
	_ui->angularAreaSqDegLineEdit->setText(QString::number(areaSqDeg));
	_ui->angularAreaSqArcsecLineEdit->setText(QString::number(areaSqArcsec));
}

void DataTable::setHeader(const clusteringMethod clusteringMethod)
{
	clear();
	const QList<QString> header = HEADERS[clusteringMethod];
	_ui->tableWidget->setColumnCount(header.size());
	_ui->tableWidget->setHorizontalHeaderLabels(header);
}

void DataTable::clear()
{
	_ui->tableWidget->clear();
	_ui->tableWidget->setColumnCount(0);
	_ui->tableWidget->setRowCount(0);
}

template<clusteringMethod E, typename... Args>
void DataTable::addRow([[maybe_unused]] Args... args)
{
	throw std::logic_error("Unspecialized addRow function called");
}

template<>
void DataTable::addRow<clusteringMethod::HALLEY>(const int shellIndex, const int starCount, const double totalApvmag, const double apvmagPerSqArcsec, const double linearSurfaceBrightness)
{
	const QList<QString> text =
	{
		QString::number(shellIndex),
		QString::number(starCount),
		QString::number(totalApvmag),
		QString::number(apvmagPerSqArcsec),
		QString::number(linearSurfaceBrightness)
	};

	placeRow(text);
}

template<>
void DataTable::addRow<clusteringMethod::FRACTAL>(const int levelIndex, const int starCount, const double totalApvmag, const double apvmagPerSqArcsec, const double linearSurfaceBrightness)
{
	const QList<QString> text =
	{
		QString::number(levelIndex),
		QString::number(starCount),
		QString::number(totalApvmag, 'g', 14),
		QString::number(apvmagPerSqArcsec, 'g', 14),
		QString::number(linearSurfaceBrightness, 'g', 14)
	};

	placeRow(text);
}

void DataTable::placeRow(const QList<QString>& text)
{
	if (Q_UNLIKELY(text.size() != _ui->tableWidget->columnCount())) throw std::logic_error("Row element count doesn't match cosumn count");

	_ui->tableWidget->insertRow(_ui->tableWidget->rowCount());
	for (int i = 0; i < text.size(); i++)
	{
		_ui->tableWidget->setItem(_ui->tableWidget->rowCount() - 1, i, new QTableWidgetItem(text[i]));
	}
}

void DataTable::onExport()
{
	if (_ui->tableWidget->rowCount() == 0) return;

	const QString fileName = QFileDialog::getSaveFileName(nullptr, "Save data table", QDir::homePath() + "/simulation_dataTable.csv", "CSV files (*.csv);;All files (*.*)");

	QFile file(fileName);
	file.open(QIODevice::WriteOnly);
	QTextStream fileStream(&file);

	//Header
	for (int col = 0; col < _ui->tableWidget->columnCount(); col++)
	{
		fileStream << _ui->tableWidget->horizontalHeaderItem(col)->text().replace("\n", "/") << CSV_SEPARATOR;
	}
	fileStream << "HFOV [deg]" << CSV_SEPARATOR
			   << "VOFV [deg]" << CSV_SEPARATOR
			   << QStringLiteral("Angular area [arcsec\u00B2]") << "\n";

	//Data
	for (int row = 0; row < _ui->tableWidget->rowCount(); row++)
	{
		for (int col = 0; col < _ui->tableWidget->columnCount(); col++)
		{
			if (!_ui->tableWidget->item(row, col))
			{
				fileStream << CSV_SEPARATOR;
				continue;
			}
			fileStream << _ui->tableWidget->item(row, col)->text() << CSV_SEPARATOR;
		}
		if (Q_UNLIKELY(row == 0)) fileStream << _ui->hfovLineEdit->text() << CSV_SEPARATOR
											 << _ui->vfovLineEdit->text() << CSV_SEPARATOR
											 << _ui->angularAreaSqArcsecLineEdit->text();
		fileStream << "\n";
	}

	file.close();
}
