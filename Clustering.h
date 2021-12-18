#pragma once

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QMatrix4x4>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>

#include "DataTable.h"
#include "DataChart.h"
#include "LinearizedChart.h"
#include "InstancedStar.h"
#include "InstancedStarMaterial.h"

class Clustering : public QObject
{
	Q_OBJECT
public:
	explicit Clustering(Qt3DCore::QEntity* parentEntity, QObject* parent = nullptr);

	virtual void start() = 0;
	virtual void terminate() = 0;

	void setCameraProjectionMatrix(const QMatrix4x4& projectionMatrix, const QMatrix4x4& viewMatrix, const QRect& rect);

	void setStarProperties(const float size, const float powerFactor);

	void setDataTable(DataTable* dataTable){ _dataTable = dataTable; };
	void setDataChart(DataChart* dataChart){ _dataChart = dataChart; };
	void setLinearizedChart(LinearizedChart* linearizedChart){ _linearizedChart = linearizedChart; };
	void setNextClusterReady(){ _isNextClusterReady = true; };

public slots:
	void reserveGroups(const int count);
	void addStarInGroup(const int& index, const QVector3D& location);

protected:
	struct threadGroup
	{
		QThread* thread = nullptr;
		QList<QVector3D> stars;
	};

	Qt3DCore::QEntity* _parentEntity = nullptr;

	bool isPointVisible(const QVector3D& point);
	QList<threadGroup*> distributeStarsInThreads(const QList<QVector3D>& stars);

	Qt3DCore::QEntity* createStar(const QVector3D& location);

	DataTable* _dataTable = nullptr;
	DataChart* _dataChart = nullptr;
	LinearizedChart* _linearizedChart = nullptr;

	bool _isNextClusterReady = false;

private:
	struct instancedStarGroup
	{
		Qt3DCore::QEntity* entity = nullptr;
		InstancedStar* instancedStar = nullptr;
		InstancedStarMaterial* instancedStarMaterial = nullptr;
		Qt3DRender::QGeometryRenderer* geometryRenderer = nullptr;
	};

	QMatrix4x4 _projectionMatrix;
	QMatrix4x4 _viewMatrix;
	QRect _viewportRect;

	float _starSize = 0.05f;
	float _starPowerFactor = 0.3f;

	QList<QList<instancedStarGroup*>> _groups;

signals:
	void clusterDone();
	void finished();
	void updateProgress(const int placed, const int total, bool cluster = false);
	void requestAddStarInGroup(const int& index, const QVector3D& location);
	void requestReserveGroups(const int count);
};
