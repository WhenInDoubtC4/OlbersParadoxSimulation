#pragma once

#include <random>

#include "Clustering.h"
#include "Global.h"

class HalleyClustering : public Clustering
{
	Q_OBJECT
public:
	explicit HalleyClustering(Qt3DCore::QEntity* parentEntity, QObject* parent = nullptr, int shellCount = 1, float shellThickness = 50, float firstShellDistance = 1.29);

	virtual void start() override;
	virtual void terminate() override;

	static void calculateEstimate(int shellCount, float shellThickness, float firstShellDistance, QTime& outEstimatedTime, int& outEstimatedCount);

private:

	int _shellCount;
	float _shellThickness;
	float _firstShellDistance;

	QList<QList<QVector3D>> _stars;
	QRecursiveMutex _groupMutex;
	int _currentShellIndex = 0;
	QList<threadGroup*> _threadGroups;

	bool _terminatePending = false;

	int _totalStarCount = 0;
	int _starsPlaced = 0;
	int _starsPlacedInShell	= 0;

private slots:
	void constructShell();
};
