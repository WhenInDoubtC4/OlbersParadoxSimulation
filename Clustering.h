#pragma once

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QVector4D>
#include <QMatrix4x4>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DCore/QTransform>


class Clustering : public QObject
{
	Q_OBJECT
public:
	explicit Clustering(Qt3DCore::QEntity* parentEntity, QObject* parent = nullptr);

	enum method
	{
		HALLEY,
		FRACTAL
	};

	virtual void start() = 0;
	virtual void terminate() = 0;

	void setCameraProjectionMatrix(const QMatrix4x4& projectionMatrix, const QMatrix4x4& viewMatrix, const QRect& rect);

	void setStarProperties(const float size, const float powerFactor);

protected:
	Qt3DCore::QEntity* _parentEntity = nullptr;

	bool isPointVisible(const QVector3D& point);

	Qt3DCore::QEntity* createStar(const QVector3D& location);

private:
	QMatrix4x4 _projectionMatrix;
	QMatrix4x4 _viewMatrix;
	QRect _rect;

	float _starSize = 0.05f;
	float _starPowerFactor = 0.3f;

signals:
	void finished();
	void updateProgress(const int precentage);
	void updateClusterProgress(const int percentage);
};
