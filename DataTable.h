#pragma once

#include <QObject>
#include <QWidget>
#include <QFileDialog>
#include <QTextStream>

#include "Global.h"

namespace Ui
{
	class DataTable;
}

class DataTable : public QWidget
{
	Q_OBJECT
public:
	DataTable(QWidget* parent = nullptr);
	~DataTable();

	void setCameraData(const float hfov, const float vfov, const float areaSqDeg, const float areaSqArcsec);

	void setHeader(const clusteringMethod clusteringMethod);
	void clear();

	template<clusteringMethod E, typename... Args>
	void addRow(Args... args);

	template<>
	void addRow<clusteringMethod::HALLEY>(const int shellIndex, const int starCount, const double totalApvmag, const double apvmagPerSqArcsec, const double linearSurfaceBrightness);

	template<>
	void addRow<clusteringMethod::FRACTAL>(const int levelIndex, const int starCount, const double totalApvmag, const double apvmagPerSqArcsec, const double linearSurfaceBrightness);

private:
	void placeRow(const QList<QString>& text);

	Ui::DataTable* _ui = nullptr;

	const QMap<clusteringMethod, QList<QString>> HEADERS =
	{
		{clusteringMethod::HALLEY, {"Shell index", "Visible star count \n n\u1D65 [1]", "Total apvmag \n m\u1D65 [mag]", "Sky brightness \n \u03BC [mag*arcsec\u207B\u00B2]", "e^(-\u03BC)"}},
		{clusteringMethod::FRACTAL, {"Level index", "Visible star count \n n\u1D65 [1]", "Total apvmag \n m\u1D65 [mag]", "Sky brightness \n \u03BC [mag*arcsec\u207B\u00B2]", "e^(-\u03BC)"}}
	};

private slots:
	void onExport();
};

