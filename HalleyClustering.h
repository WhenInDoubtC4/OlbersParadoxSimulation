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

private:
	int _shellCount;
	float _shellThickness;
	float _firstShellDistance;

	QList<QList<QVector3D>> _stars;
	QList<QThread*> _currentShellThreads;
	QRecursiveMutex _groupMutex;
	int _currentShellIndex = 0;

	QList<QList<QVector3D>> _groups;

	bool _terminatePending = false;

	int _totalStarCount = 0;
	int _starsPlaced = 0;
	int _starsPlacedInShell	= 0;

private slots:
	void constructShell();

signals:
	void shellDone(const int shellIndex, const int starCount, const float totalApvmag);
};

