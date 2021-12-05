#pragma once

#include <QMainWindow>
#include <QProgressBar>
#include <QStackedLayout>
#include <QDateTime>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QRenderCapture>

#include "Global.h"
#include "HalleyClustering.h"
#include "FractalClustering.h"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

protected:
	virtual void resizeEvent(QResizeEvent* event) override;

private:
	Ui::MainWindow* _ui = nullptr;

	Qt3DExtras::Qt3DWindow* _viewport = nullptr;
	QWidget* _viewportContainer = nullptr;

	Qt3DCore::QEntity* _rootEntity = nullptr;
	Qt3DCore::QEntity* _starRootEntity = nullptr;

	Clustering* _activeClustering = nullptr;

	QMutex _clearThreadMutex;

	QWidget* _progressStackPlaceholder = nullptr;
	QWidget* _clusterProgressStackPlaceholder = nullptr;
	QStackedLayout* _progressStack = nullptr;
	QStackedLayout* _clusterProgressStack = nullptr;
	QProgressBar* _progressBar = nullptr;
	QProgressBar* _clusterProgressBar = nullptr;
	QLabel* _progressLabel = nullptr;
	QLabel* _clusterProgressLabel = nullptr;

	Qt3DRender::QRenderCapture* _renderCapture = nullptr;
	Qt3DRender::QRenderCaptureReply* _reply = nullptr;

	void updateUI(bool running);

private slots:
	void onRunPressed();
	void onTerminatePressed();
	void onClearPressed();
	void onFinished();

	void updateProgress(const int placed, const int total, bool cluster = false);

	void selectRenderSaveLocation();
	void saveRender();

	void updateEstimate();
};
