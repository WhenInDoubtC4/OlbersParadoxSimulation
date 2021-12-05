#pragma once

#include "Clustering.h"
#include <QObject>
#include <QThread>

#include "Global.h"


class FractalClustering : public Clustering
{
	Q_OBJECT
public:
	FractalClustering(Qt3DCore::QEntity* parentEntity, QObject* parent, int levelCount, int countPerLevel, float spacing, bool placeZeroStar);

	virtual void start() override;
	virtual void terminate() override;

	static void calculateEstimate(int levelCount, int countPerLevel, float spacing, QTime& outEstimatedTime, int& outEstimatedCount);

private:
	static const QList<QVector3D> calculateLevel(int level, const QVector3D& origin, QList<float>& volumeRadius, const float spacing);

	int _levelCount;
	int _countPerLevel;
	float _spacing;
	bool _placeZeroStar;

	QList<float> _volumeRadius;
	QList<QList<QVector3D>> _stars;
	QList<threadGroup*> _threadGroups;

	QRecursiveMutex _groupMutex;
	bool _terminatePending = false;

	int _currentLevelIndex = 0;
	int _totalStarCount = 0;
	int _starsPlacedInLevel = 0;
	int _starsPlaced = 0;


private slots:
	void construct();
};
