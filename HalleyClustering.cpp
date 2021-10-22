#include "HalleyClustering.h"

#include <QDebug>

HalleyClustering::HalleyClustering(Qt3DCore::QEntity* parentEntity, QObject* parent, int shellCount, float shellThickness, float firstShellDistance) : Clustering(parentEntity, parent)
{
	_shellCount = shellCount;
	_shellThickness = shellThickness;
	_firstShellDistance = firstShellDistance;
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
		qDebug() << "SHELL#" << n << "R_outer (pc):" << outerRadius << "R_inner (pc):" << innerRadius << "Volume (pc^3):" << volume << "Stars:" << shell.size();
		_stars << shell;
		_totalStarCount += shell.size();
	}

	constructShell();
}

void HalleyClustering::terminate()
{
	_terminatePending = true;
}

void HalleyClustering::constructShell()
{
	//Exit if current shell is still constructing
	if (!_currentShellThreads.isEmpty()) return;

	if (_currentShellIndex > 0)
	{
		QList<QVector3D> starsInCurrentAndPreviousShells;
		for (int i = 0; i < _currentShellIndex; i++) starsInCurrentAndPreviousShells << _stars[i];
		float apvmagSum = 0.f;
		for (const QVector3D& star : starsInCurrentAndPreviousShells)
		{
			const float distance = star.length();
			const float apvmag = ABSOLUTE_VISUAL_MAGNITUDE + 5.f * log10(distance / 10.f);
			apvmagSum += pow(10.f, (-0.4f * apvmag));
		}
		apvmagSum = -2.5f * log10(apvmagSum);

		emit shellDone(_currentShellIndex - 1, starsInCurrentAndPreviousShells.size(), apvmagSum);
	}

	if (_currentShellIndex == _shellCount)
	{
		 emit finished();
		 return;
	}

	_starsPlacedInShell = 0;
	const QList<QVector3D> starsInShell = _stars[_currentShellIndex];

	_groups.clear();

	//Full groups
	const int fullGroupCount = floor(starsInShell.size() / STARS_PER_THREAD);
	for (int groupIndex = 0; groupIndex < fullGroupCount; groupIndex++)
	{
		_groups << starsInShell.mid(groupIndex * STARS_PER_THREAD, STARS_PER_THREAD);
	}
	//Last group
	_groups << starsInShell.mid(fullGroupCount * STARS_PER_THREAD, starsInShell.size() % STARS_PER_THREAD);

	for (int groupIndex = 0; groupIndex < _groups.size(); groupIndex++)
	{
		QThread* thread = QThread::create([=]
		{
			const QList<QVector3D>& stars = _groups[groupIndex];
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

				emit updateProgress(qRound((float(++_starsPlaced) / float(_totalStarCount)) * 100.f));
				emit updateClusterProgress(qRound(float(++_starsPlacedInShell) / float(starsInShell.size()) * 100.f));

				_groupMutex.unlock();
				qApp->processEvents();
			}
		});
		_currentShellThreads << thread;
		QObject::connect(thread, &QThread::finished, [=]
		{
			_currentShellThreads.removeAll(thread);
			thread->deleteLater();
			constructShell();
		});
		thread->start();
	}

	_currentShellIndex++;
}
