#pragma once

#include <QObject>
#include <QVector3D>
#include <QDataStream>
#include <QMutex>

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>

#include <Qt3DExtras/QSphereGeometry>

class InstancedStar : public Qt3DExtras::QSphereGeometry
{
	Q_OBJECT
public:
	InstancedStar(Qt3DCore::QNode* parent = nullptr);

	void setPoints(const QList<QVector3D>& points);
	void addPoint(const QVector3D& point, const float& scale = 1.f);

	int getCount();

private:
	Qt3DRender::QAttribute* _positionAttribute = nullptr;
	Qt3DRender::QBuffer* _positionBuffer = nullptr;

	Qt3DRender::QAttribute* _scaleAttribute = nullptr;
	Qt3DRender::QBuffer* _scaleBuffer = nullptr;

	QList<QVector3D> _points;
	QByteArray _positionBufferData;
	QByteArray _scaleBufferData;
};
