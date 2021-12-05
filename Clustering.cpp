#include "Clustering.h"

Clustering::Clustering(Qt3DCore::QEntity* parentEntity, QObject* parent) : QObject(parent)
{
	_parentEntity = parentEntity;

	//Signals and slots within the same class, function calls don't work (across threads)
	QObject::connect(this, &Clustering::requestReserveGroups, this, &Clustering::reserveGroups);
	QObject::connect(this, &Clustering::requestAddStarInGroup, this, &Clustering::addStarInGroup);
}

void Clustering::setCameraProjectionMatrix(const QMatrix4x4& projectionMatrix, const QMatrix4x4& viewMatrix, const QRect& rect)
{
	_projectionMatrix = projectionMatrix;
	_viewMatrix = viewMatrix;
	_viewportRect = rect;
}

void Clustering::setStarProperties(const float size, const float powerFactor)
{
	_starSize = size;
	_starPowerFactor = powerFactor;
}

bool Clustering::isPointVisible(const QVector3D& point)
{
	const QVector3D projected = point.project(_viewMatrix, _projectionMatrix, _viewportRect);
	return projected.z() > 0 && projected.z() < 1 && projected.x() > 0 && projected.x() < _viewportRect.x() + _viewportRect.width() && projected.y() > 0 && projected.y() < _viewportRect.y() + _viewportRect.height();
}

QList<Clustering::threadGroup*> Clustering::distributeStarsInThreads(const QList<QVector3D>& stars)
{
	QList<Clustering::threadGroup*> groups;
	const int groupSize = fmax(floor(stars.size() / QThread::idealThreadCount()), STARS_PER_THREAD);
	const int fullGroupCount = floor(stars.size() / groupSize);
	for (int groupIndex = 0; groupIndex < fullGroupCount; groupIndex++)
	{
		auto currentGroup = new threadGroup;
		currentGroup->stars << stars.mid(groupIndex * groupSize, groupSize);
		groups << currentGroup;
	}
	const int overflowAmount = stars.size() % groupSize;
	if (overflowAmount != 0)
	{
		auto overflow = new threadGroup;
		overflow->stars << stars.mid(fullGroupCount * groupSize, overflowAmount);
		groups << overflow;
	}
	return groups;
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

void Clustering::reserveGroups(const int count)
{
	QList<instancedStarGroup*> newGroups;
	for (int i = 0; i < count; i++)
	{
		auto group = new instancedStarGroup;
		group->entity = new Qt3DCore::QEntity(_parentEntity);
		group->instancedStar = new InstancedStar();
		group->instancedStar->setRadius(_starSize);
		group->instancedStar->setSlices(12);
		group->instancedStar->setRings(12);
		group->instancedStarMaterial = new InstancedStarMaterial();
		group->geometryRenderer = new Qt3DRender::QGeometryRenderer();
		group->geometryRenderer->setGeometry(group->instancedStar);
		group->entity->addComponent(group->instancedStarMaterial);
		group->entity->addComponent(group->geometryRenderer);
		newGroups << group;
	}
	_groups << newGroups;
}

void Clustering::addStarInGroup(const int& index, const QVector3D& location)
{
	instancedStarGroup* activeGroup = _groups.last()[index];
	const float scaleFactor = 1.f / pow(location.length(), _starPowerFactor);
	activeGroup->instancedStar->addPoint(location, scaleFactor);
	activeGroup->geometryRenderer->setInstanceCount(activeGroup->instancedStar->getCount());
}
