#include "FractalClustering.h"

FractalClustering::FractalClustering(Qt3DCore::QEntity* parentEntity, QObject* parent, int levelCount, int countPerLevel, float spacing, bool placeZeroStar) : Clustering(parentEntity, parent)
{
	_levelCount = levelCount;
	_countPerLevel = countPerLevel;
	_spacing = spacing;
	_placeZeroStar = placeZeroStar;
}

void FractalClustering::start()
{
	_groups.clear();
	_volumeRadius.clear();
	for (int level = 0; level < _levelCount; level++)
	{
		const float prevRadius = level == 0 ? STELLAR_RADIUS : _volumeRadius.last();
		const float radius = (_countPerLevel - 1.f) * (_spacing + 2.f * prevRadius) + prevRadius;
		_volumeRadius << radius;
	}

	int levelIndex = _levelCount - 1;
	QList<QVector3D> upperLevelPositions{QVector3D(0, 0, 0)};
	while (levelIndex > 0)
	{
		QList<QVector3D> lowerLevelPositions;
		for (const QVector3D& pos : upperLevelPositions)
		{
			lowerLevelPositions << calculateLevel(levelIndex, pos);
		}
		upperLevelPositions.clear();
		upperLevelPositions << lowerLevelPositions;

		levelIndex--;
	}

	qApp->processEvents();

	QList<QVector3D> visibleStars;
	for (const QVector3D& pos : qAsConst(upperLevelPositions))
	{
		if (!isPointVisible(pos)) continue;
		if (Q_UNLIKELY(!_placeZeroStar && pos.length() < 0.5f)) continue;

		visibleStars << pos;
	}
	_totalStarCount = visibleStars.size();

	qApp->processEvents();

	std::sort(visibleStars.begin(), visibleStars.end(), [](QVector3D& a, QVector3D& b)
	{
		return a.length() < b.length();
	});

	qApp->processEvents();

	const int fullGroupCount = floor(visibleStars.size() / STARS_PER_THREAD);
	for (int groupIndex = 0; groupIndex < fullGroupCount; groupIndex++)
	{
		_groups << visibleStars.mid(groupIndex * STARS_PER_THREAD, STARS_PER_THREAD);
	}
	_groups << visibleStars.mid(fullGroupCount * STARS_PER_THREAD, visibleStars.size() % STARS_PER_THREAD);

	_runningThreads = 0;
	_threadCount = QThread::idealThreadCount();
	_totalStarsPlaced = 0;
	_apvmagSum = 0.f;
	construct();
}

const QList<QVector3D> FractalClustering::calculateLevel(int level, const QVector3D origin)
{
	assert(level > 0);
	QList<QVector3D> subLevelPositions;
	const float minExtent = -_volumeRadius[level] + _volumeRadius[level - 1];
	const float increment = (2 * _volumeRadius[level - 1]) + _spacing;
	const float maxExtent = _volumeRadius[level] - _volumeRadius[level - 1];

	//*Ignore wanings and use floats as loop counters*
	for (float x = minExtent; x <= maxExtent; x += increment)
	{
		for (float y = minExtent; y <= maxExtent; y += increment)
		{
			for (float z = minExtent; z <= maxExtent; z+= increment)
			{
				const QVector3D o(x, y, z);
				if (o.length() >= float(_volumeRadius[level]) + (1.f/2.f) * float(_volumeRadius[level - 1])) continue;
				subLevelPositions << QVector3D(o + origin);
			}
		}
	}

	return subLevelPositions;
}

void FractalClustering::construct()
{
	const bool empty = _groups.length() == 0;
	if (empty && _runningThreads == 0) emit finished();
	if (empty || _runningThreads == _threadCount || _terminatePending) return;

	QThread* thread = QThread::create([=]
	{
		const QList<QVector3D> stars = _groups.length() > 0 ? _groups.takeFirst() : QList<QVector3D>{};
		construct();
		for (const QVector3D& location : stars)
		{
			if (_terminatePending) return;

			thread->msleep(THREAD_SLEEP_TIME);
			_groupMutex.lock();

			auto starEntity = createStar(location);
			if (!_parentEntity || _terminatePending)
			{
				_groupMutex.unlock();
				continue;
			}
			starEntity->moveToThread(_parentEntity->thread());
			starEntity->setParent(_parentEntity);

			emit updateProgress(qRound((float(++_totalStarsPlaced) / float(_totalStarCount)) * 100.f));

			float distance = location.length();
			if (Q_UNLIKELY(distance < 1.2f)) distance = 1.2f;
			const float apvmag = ABSOLUTE_VISUAL_MAGNITUDE + 5.f * log10(distance / 10.f);
			_apvmagSum += apvmag;
			if (_totalStarsPlaced % STARS_PER_THREAD == 0 || _totalStarsPlaced == _totalStarCount)
			{
				const float totalApvmag = -2.5f * log10(_apvmagSum);
				emit groupDone(_totalStarsPlaced, totalApvmag);
			}

			_groupMutex.unlock();
			qApp->processEvents();
		}
	});
	QObject::connect(thread, &QThread::finished, [=]
	{
		thread->deleteLater();
		_runningThreads--;
		construct();
	});
	thread->start();
	_runningThreads++;
}

void FractalClustering::terminate()
{
	_terminatePending = true;
}
