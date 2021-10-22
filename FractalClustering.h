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

private:
	int _levelCount;
	int _countPerLevel;
	float _spacing;
	bool _placeZeroStar;

	QList<float> _volumeRadius;
	QList<QList<QVector3D>> _groups;

	int _runningThreads = 0;
	int _threadCount = 0;
	QRecursiveMutex _groupMutex;

	int _totalStarsPlaced = 0;
	int _totalStarCount = 0;
	float _apvmagSum = 0.f;

	bool _terminatePending = false;

	const QList<QVector3D> calculateLevel(int level, const QVector3D origin);

private slots:
	void construct();

signals:
	void groupDone(const int totalStarCount, const float totalApvmag);
};

