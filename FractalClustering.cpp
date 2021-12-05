#include "FractalClustering.h"

FractalClustering::FractalClustering(Qt3DCore::QEntity* parentEntity, QObject* parent, int levelCount, int countPerLevel, float spacing, bool placeZeroStar) : Clustering(parentEntity, parent)
{
	_levelCount = levelCount;
	_countPerLevel = countPerLevel;
	_spacing = spacing;
	_placeZeroStar = placeZeroStar;
}

void FractalClustering::calculateEstimate(int levelCount, int countPerLevel, float spacing,QTime& outEstimatedTime, int& outEstimatedCount)
{
	QList<float> volumeRadius;
	for (int level = 0; level < levelCount; level++)
	{
		const float prevRadius = level == 0 ? STELLAR_RADIUS : volumeRadius.last();
		const float radius = (countPerLevel - 1.f) * (spacing + 2.f * prevRadius) + prevRadius;
		volumeRadius << radius;
	}

	constexpr float shift = CAMERA_VFOV / CAMERA_ASPECT_RATIO;

	QList<int> count{1};
	outEstimatedCount = 1;
	int totalTime = 0;
	for (int levelIndex = 1; levelIndex < levelCount; levelIndex++)
	{
		const int levelFactor = calculateLevel(levelIndex, QVector3D(0, 0, -shift), volumeRadius, spacing).size();

		//Very sloppy calculations
		const float projectionRadius = shift + volumeRadius[levelIndex];
		const float projectionVolume = (4.f/3.f) * pow(projectionRadius, 3.f) * M_PI * (CAMERA_HFOV / 360.f) * (CAMERA_VFOV / 360.f);
		const float clusterVolume = (4.f/3.f) * pow(volumeRadius[levelIndex], 3.f) * M_PI;
		const float cullingFraction = projectionVolume / clusterVolume;

		const int levelCount = count[levelIndex - 1] * levelFactor;
		const int levelVisibleCount = floor(levelCount * cullingFraction * 1.1f);
		count << levelCount;
		outEstimatedCount += levelVisibleCount;

		const int threadGroupSize = fmax(floor(levelVisibleCount / QThread::idealThreadCount()), STARS_PER_THREAD);
		const int threadGroupCount = fmax(ceil(levelVisibleCount / threadGroupSize), 1);
		totalTime += floor(float(levelVisibleCount * (THREAD_SLEEP_TIME + 1)) / float(threadGroupCount));
	}

	outEstimatedTime = QTime(0, 0).addMSecs(totalTime);
}

const QList<QVector3D> FractalClustering::calculateLevel(int level, const QVector3D& origin, QList<float>& volumeRadius, const float spacing)
{
	assert(level > 0 && level < volumeRadius.size());
	QList<QVector3D> subLevelPositions;
	const float minExtent = -volumeRadius[level] + volumeRadius[level - 1];
	const float increment = (2 * volumeRadius[level - 1]) + spacing;
	const float maxExtent = volumeRadius[level] - volumeRadius[level - 1];

	//*Ignore wanings and use floats as loop counters*
	for (float x = minExtent; x <= maxExtent; x += increment)
	{
		for (float y = minExtent; y <= maxExtent; y += increment)
		{
			for (float z = minExtent; z <= maxExtent; z+= increment)
			{
				const QVector3D o(x, y, z);
				if (o.length() >= float(volumeRadius[level]) + (1.f/2.f) * float(volumeRadius[level - 1])) continue;
				subLevelPositions << QVector3D(o + origin);
			}
		}
	}

	return subLevelPositions;
}

void FractalClustering::start()
{
	_threadGroups.clear();
	_volumeRadius.clear();
	for (int level = 0; level < _levelCount; level++)
	{
		const float prevRadius = level == 0 ? STELLAR_RADIUS : _volumeRadius.last();
		const float radius = (_countPerLevel - 1.f) * (_spacing + 2.f * prevRadius) + prevRadius;
		_volumeRadius << radius;
	}

	QList<QVector3D> previousLevel{QVector3D(0, 0, -CAMERA_VFOV / CAMERA_ASPECT_RATIO)};
	_stars << previousLevel;
	for (int levelIndex = 1; levelIndex < _levelCount; levelIndex++)
	{
		QList<QVector3D> nextLevel;
		for (const QVector3D& pos : previousLevel)
		{
			nextLevel << calculateLevel(levelIndex, pos, _volumeRadius, _spacing);
		}
		previousLevel.clear();
		previousLevel = nextLevel;
		_stars << nextLevel;
	}

	//Occlusion culling and counting
	_totalStarCount = 0;
	QList<QList<QVector3D>> visibleStars;
	for (int level = 0; level < _stars.size(); level++)
	{
		QList<QVector3D> visibleStarsInLevel;
		for (const QVector3D& star : _stars[level])
		{
			if (isPointVisible(star)) visibleStarsInLevel << star;
		}
		visibleStars << visibleStarsInLevel;
		_totalStarCount += visibleStarsInLevel.size();
	}

	_stars = visibleStars;

	//Sort by distance
	for (int level = 0; level < _stars.size(); level++)
	{
		std::sort(_stars[level].begin(), _stars[level].end(), [](QVector3D& a, QVector3D& b)
		{
			return a.length() < b.length();
		});
	}

	qApp->processEvents();

	_currentLevelIndex = 0;
	_starsPlaced = 0;
	construct();
}

void FractalClustering::construct()
{
	//Exit if current cluster is still constructing
	if (!_threadGroups.isEmpty()) return;

	if (_currentLevelIndex > 0)
	{
		double apvmagSum = 0.;
		//Stars in current and previous levels
		for (int levelIndex = 0; levelIndex < _currentLevelIndex; levelIndex++)
		{
			for (const QVector3D& star : _stars[levelIndex])
			{
				const double distance = star.length();
				const double apvmag = ABSOLUTE_VISUAL_MAGNITUDE + 5. * log10(distance / 10.);
				apvmagSum += pow(10., -0.4 * apvmag);
			}
		}
		apvmagSum = -2.5 * log10(apvmagSum);

		const double surfaceBrightness = apvmagSum + 2.5 * log10(CAMERA_ANGULAR_AREA_SQ_ARCSEC);
		const double linearSurfaceBrightness = pow(M_E, -surfaceBrightness);

		if (_dataTable) _dataTable->addRow<clusteringMethod::FRACTAL>(_currentLevelIndex - 1, _starsPlaced, apvmagSum, surfaceBrightness, linearSurfaceBrightness);
		if (_dataChart) _dataChart->addDataPoint(_starsPlaced, surfaceBrightness);
		emit clusterDone();
	}

	if (_currentLevelIndex == _stars.size())
	{
		emit finished();
		return;
	}

	_starsPlacedInLevel	= 0;
	const QList<QVector3D>& starsInLevel = _stars[_currentLevelIndex];

	_threadGroups.clear();
	_threadGroups = distributeStarsInThreads(starsInLevel);

	emit requestReserveGroups(_threadGroups.size());

	for (int groupIndex = 0; groupIndex < _threadGroups.size(); groupIndex++)
	{
		threadGroup* currentGroup = _threadGroups[groupIndex];
		currentGroup->thread = QThread::create([=]
		{
			for (const QVector3D& starLocation : qAsConst(currentGroup->stars))
			{
				currentGroup->thread->msleep(THREAD_SLEEP_TIME);

				_groupMutex.lock();
				emit requestAddStarInGroup(groupIndex, starLocation);

				emit updateProgress(++_starsPlaced, _totalStarCount);
				emit updateProgress(++_starsPlacedInLevel, starsInLevel.size(), true);

				_groupMutex.unlock();
				if (_terminatePending) return;
			}
		});
		QObject::connect(currentGroup->thread, &QThread::finished, [=]
		{
			currentGroup->thread->deleteLater();
			_threadGroups.removeAll(currentGroup);
			construct();
		});
		currentGroup->thread->start();
	}

	_currentLevelIndex++;
}

void FractalClustering::terminate()
{
	_terminatePending = true;
}
