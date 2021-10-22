#pragma once

#include <QObject>
#include <QWidget>
#include <QFileDialog>

#include "Clustering.h"
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

	void setHeader(const Clustering::method clusteringMethod);
	void clear();

	template<Clustering::method E, typename... Args>
	void addRow(Args... args);

	template<>
	void addRow<Clustering::method::HALLEY>(const int shellIndex, const int starCount, const float totalApvmag, const float apvmagPerSqArcsec, const float linearSurfaceBrightness);

	template<>
	void addRow<Clustering::method::FRACTAL>(const int starCount, const float totalApvmag, const float apvmagPerSqArcsec, const float linearSurfaceBrightness);

private:
	Ui::DataTable* _ui = nullptr;

	const QMap<Clustering::method, QList<QString>> HEADERS =
	{
		{Clustering::method::HALLEY, {"Shell index", "Visible star count \n n\u1D65 [1]", "Total apvmag \n m\u1D65 [mag]", "Sky brightness \n \u03BC [mag*arcsec\u207B\u00B2]", "e^(-\u03BC)"}},
		{Clustering::method::FRACTAL, {"Visible star count \n n\u1D65 [1]", "Total apvmag \n m\u1D65 [mag]", "Sky brightness \n \u03BC [mag*arcsec\u207B\u00B2]", "e^(-\u03BC)"}}
	};

private slots:
	void onExport();
};

