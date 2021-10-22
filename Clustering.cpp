#include "Clustering.h"

Clustering::Clustering(Qt3DCore::QEntity* parentEntity, QObject* parent) : QObject(parent)
{
	_parentEntity = parentEntity;
}

void Clustering::setCameraProjectionMatrix(const QMatrix4x4& projectionMatrix, const QMatrix4x4& viewMatrix, const QRect& rect)
{
	_projectionMatrix = projectionMatrix;
	_viewMatrix = viewMatrix;
	_rect = rect;
}

void Clustering::setStarProperties(const float size, const float powerFactor)
{
	_starSize = size;
	_starPowerFactor = powerFactor;
}

bool Clustering::isPointVisible(const QVector3D& point)
{
	const QVector3D projected = point.project(_viewMatrix, _projectionMatrix, _rect);
	return projected.z() < 1 && projected.x() > 0 && projected.x() < _rect.x() + _rect.width() && projected.y() > 0 && projected.y() < _rect.y() + _rect.height();
}

Qt3DCore::QEntity* Clustering::createStar(const QVector3D& location)
{
	auto starEntity = new Qt3DCore::QEntity();
	auto star = new Qt3DExtras::QSphereMesh();
	auto starTransform = new Qt3DCore::QTransform();
	auto starMaterial = new Qt3DExtras::QGoochMaterial();
	star->setRadius(_starSize *	pow(location.length(), _starPowerFactor));
	star->setRings(12);
	star->setSlices(12);
	starTransform->setScale(1.f);
	starTransform->setTranslation(location);
	starMaterial->setDiffuse(0xffffff);
	starEntity->addComponent(star);
	starEntity->addComponent(starTransform);
	starEntity->addComponent(starMaterial);

	return starEntity;
}
