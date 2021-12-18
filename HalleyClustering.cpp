#include "HalleyClustering.h"

#include <QDebug>

HalleyClustering::HalleyClustering(Qt3DCore::QEntity* parentEntity, QObject* parent, int shellCount, float shellThickness, float firstShellDistance) : Clustering(parentEntity, parent)
{
	_shellCount = shellCount;
	_shellThickness = shellThickness;
	_firstShellDistance = firstShellDistance;
}

void HalleyClustering::calculateEstimate(int shellCount, float shellThickness, float firstShellDistance, QTime& outEstimatedTime, int& outEstimatedCount)
{
	outEstimatedCount = 0;
	int totalTime = 0;

	for (int n = 0; n < shellCount; n++)
	{
		const float outerRadius = firstShellDistance + n * shellThickness;
		const float innerRadius = firstShellDistance + (n + 1) * shellThickness;
		const float volume = (4.f / 3.f) * M_PI * (pow(innerRadius, 3) - pow(outerRadius, 3));
		const int starCount = qRound(floor(volume / STELLAR_DENSITY) * CULLING_FRACTION);
		outEstimatedCount += starCount;

		const int threadGroupSize = fmax(floor(starCount / QThread::idealThreadCount()), STARS_PER_THREAD);
		const int threadGroupCount = fmax(ceil(starCount / threadGroupSize), 1);
		totalTime += floor(float(starCount * (THREAD_SLEEP_TIME + 1)) / float(threadGroupCount));
	}

	outEstimatedTime = QTime(0, 0).addMSecs(totalTime);
}

void HalleyClustering::start()
{
	_terminatePending = false;
	_totalStarCount = 0;
	_starsPlaced = 0;

	std::random_device randomDevice;
	std::mt19937 gen(randomDevice());
	for (int n = 0; n < _shellCount; n++)
	{
		const float outerRadius = _firstShellDistance + n * _shellThickness;
		const float innerRadius = _firstShellDistance + (n + 1) * _shellThickness;
		const float volume = (4.f / 3.f) * M_PI * (pow(innerRadius, 3) - pow(outerRadius, 3));
		const int starCount = floor(volume / STELLAR_DENSITY);

		std::uniform_real_distribution<float> zDist(-1.f, 1.f);
		std::uniform_real_distribution<float> thetaDist(0.f, 2 * M_PI);
		std::uniform_real_distribution<float> radBiasDist(outerRadius, innerRadius);

		QList<QVector3D> shell;
		for (int star = 0; star < starCount; star++)
		{
			const float radBias = radBiasDist(gen);
			const float phi = zDist(gen);
			const float theta = thetaDist(gen);

			const float r = sqrt(1.f - pow(phi, 2));

			const float x = r * cos(theta) * radBias;
			const float y = r * sin(theta) * radBias;
			const float z = phi * radBias;

			const QVector3D locaton = QVector3D(x, y, z);

			//Occlusion culling
			if (isPointVisible(locaton)) shell << locaton;
		}
		_stars << shell;
		_totalStarCount += shell.size();
	}

	constructShell();
	_isNextClusterReady = true;
}

void HalleyClustering::terminate()
{
	_terminatePending = true;
}

void HalleyClustering::constructShell()
{
	//Exit if current shell is still constructing
	if (!_threadGroups.isEmpty()) return;

	if (_currentShellIndex > 0)
	{
		QList<QVector3D> starsInCurrentAndPreviousShells;
		for (int i = 0; i < _currentShellIndex; i++) starsInCurrentAndPreviousShells << _stars[i];
		double apvmagSum = 0.;
		for (const QVector3D& star : starsInCurrentAndPreviousShells)
		{
			const double distance = star.length();
			const double apvmag = ABSOLUTE_VISUAL_MAGNITUDE + 5. * log10(distance / 10.);
			apvmagSum += pow(10., (-0.4 * apvmag));
		}
		apvmagSum = -2.5 * log10(apvmagSum);

		const int starCount = starsInCurrentAndPreviousShells.size();
		const double surfaceBrightness = apvmagSum + 2.5 * log10(CAMERA_ANGULAR_AREA_SQ_ARCSEC);
		const double linearSurfaceBrightness = pow(M_E, -surfaceBrightness);

		if (_dataTable) _dataTable->addRow<clusteringMethod::HALLEY>(_currentShellIndex - 1, starCount, apvmagSum, surfaceBrightness, linearSurfaceBrightness);
		if (_dataChart) _dataChart->addDataPoint(starCount, surfaceBrightness);
		if (_linearizedChart) _linearizedChart->addLinearPoint(starCount, linearSurfaceBrightness);
		_isNextClusterReady = false;
		emit clusterDone();
	}

	//Was last shell to construct
	if (_currentShellIndex == _shellCount)
	{
		emit finished();
		return;
	}

	_starsPlacedInShell = 0;
	const QList<QVector3D>& starsInShell = _stars[_currentShellIndex];

	_threadGroups.clear();
	_threadGroups = distributeStarsInThreads(starsInShell);

	emit requestReserveGroups(_threadGroups.size());

	for (int groupIndex = 0; groupIndex < _threadGroups.size(); groupIndex++)
	{
		threadGroup* currentGroup = _threadGroups[groupIndex];
		currentGroup->thread = QThread::create([=]
		{
			_groupMutex.lock();
			while (!_isNextClusterReady)
			{
				qApp->processEvents();
				currentGroup->thread->msleep(THREAD_SLEEP_TIME);
			}
			_groupMutex.unlock();

			for (const QVector3D& starLocation : qAsConst(currentGroup->stars))
			{
				currentGroup->thread->msleep(THREAD_SLEEP_TIME);

				_groupMutex.lock();
				emit requestAddStarInGroup(groupIndex, starLocation);

				emit updateProgress(++_starsPlaced, _totalStarCount);
				emit updateProgress(++_starsPlacedInShell, starsInShell.size(), true);

				_groupMutex.unlock();
				if (_terminatePending) return;
			}
		});
		QObject::connect(currentGroup->thread, &QThread::finished, [=]
		{
			currentGroup->thread->deleteLater();
			_threadGroups.removeAll(currentGroup);
			constructShell();
		});
		currentGroup->thread->start();
	}

	_currentShellIndex++;
}
