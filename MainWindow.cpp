#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
  , _ui(new Ui::MainWindow)
  , _progressBar(new QProgressBar())
  , _clusterProgressBar(new QProgressBar())
{
	_ui->setupUi(this);

	_progressBar->setMaximumWidth(200);
	_progressBar->setAlignment(Qt::AlignRight);
	_progressBar->setTextVisible(true);
	_progressBar->setFormat("Total: %p%");
	_clusterProgressBar->setMaximumWidth(200);
	_clusterProgressBar->setTextVisible(true);
	_clusterProgressBar->setFormat("Cluster: %p%");
	_ui->statusbar->addPermanentWidget(_progressBar);
	_ui->statusbar->addPermanentWidget(_clusterProgressBar);

	_viewport = new Qt3DExtras::Qt3DWindow(nullptr, Qt3DRender::API::OpenGL);
	_viewport->renderSettings()->setRenderPolicy(Qt3DRender::QRenderSettings::RenderPolicy::OnDemand);
	_viewportContainer = QWidget::createWindowContainer(_viewport);

	_renderCapture = new Qt3DRender::QRenderCapture();
	_viewport->activeFrameGraph()->setParent(_renderCapture);
	_viewport->setActiveFrameGraph(_renderCapture);

	//Set backgound color to black
	_viewport->defaultFrameGraph()->setClearColor(QRgb(0));

	_ui->viewportPlaceholder->addWidget(_viewportContainer);

	_rootEntity = new Qt3DCore::QEntity();
	_viewport->setRootEntity(_rootEntity);

	//Set up camera
	_viewport->camera()->setPosition(QVector3D(0.f, 0.f, 0.f));
	_viewport->camera()->setUpVector(QVector3D(0, 1, 0));
	_viewport->camera()->setViewCenter(QVector3D(0, 0, 0));
	_viewport->camera()->lens()->setPerspectiveProjection(CAMERA_VFOV, CAMERA_ASPECT_RATIO, CAMERA_NEAR_CLIP_PLANE, CAMERA_FAR_CLIP_PLANE);
	_viewport->camera()->setPosition(QVector3D(0.f, 0.f, 0.f));

	auto directionalLightEntity = new Qt3DCore::QEntity(_rootEntity);
	auto directionalLight = new Qt3DRender::QDirectionalLight();
	auto directionalLightTransform = new Qt3DCore::QTransform();
	directionalLight->setColor(0xffffff);
	directionalLight->setIntensity(1.f);
	directionalLight->setWorldDirection(QVector3D(0.f, 0.f, 1.f));
	directionalLightTransform->setTranslation(_viewport->camera()->position());
	directionalLightEntity->addComponent(directionalLight);
	directionalLightEntity->addComponent(directionalLightTransform);

	_starRootEntity = new Qt3DCore::QEntity(_rootEntity);

	_ui->dataTable->setCameraData(CAMERA_HFOV, CAMERA_VFOV, CAMERA_ANGULAR_AREA_SQ_DEG, CAMERA_ANGULAR_AREA_SQ_ARCSEC);

	QObject::connect(_ui->clusteringComboBox, qOverload<int>(&QComboBox::currentIndexChanged), _ui->stackedWidget, &QStackedWidget::setCurrentIndex);
	QObject::connect(_ui->runButton, &QPushButton::pressed, this, &MainWindow::onRunPressed);
	QObject::connect(_ui->terminateButton, &QPushButton::pressed, this, &MainWindow::onTerminatePressed);
	QObject::connect(_ui->clearButton, &QPushButton::pressed, this, &MainWindow::onClearPressed);

	QObject::connect(_ui->renderSaveLocationButton, &QPushButton::clicked, this, &MainWindow::selectRenderSaveLocation);
}

MainWindow::~MainWindow()
{
	delete _ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	_viewport->camera()->setAspectRatio(CAMERA_ASPECT_RATIO);
	_viewport->camera()->setFieldOfView(CAMERA_VFOV);
}

void MainWindow::updateUI(bool running)
{
	if (running)
	{
		setFixedSize(size());
	}
	else
	{
		setMinimumSize(0, 0);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
	}

	_ui->runButton->setEnabled(!running);
	_ui->terminateButton->setEnabled(running);
	_ui->clearButton->setEnabled(!running);

	_ui->clusteringComboBox->setEnabled(!running);

	_ui->shellCountSpinBox->setEnabled(!running);
	_ui->shellThicknessSpinBox->setEnabled(!running);
	_ui->firstShellDistanceSpinBox->setEnabled(!running);

	_ui->statusbar->clearMessage();
	if (!running)
	{
		_ui->statusbar->showMessage("Done", 30*1000);
		_progressBar->setValue(0);
		_clusterProgressBar->setValue(0);
	}

	_ui->shellCountSpinBox->setEnabled(!running);
	_ui->shellThicknessSpinBox->setEnabled(!running);
	_ui->firstShellDistanceSpinBox->setEnabled(!running);

	_ui->levelCountSpinBox->setEnabled(!running);
	_ui->countPerLevelSpinBox->setEnabled(!running);
	_ui->spacingSpinBox->setEnabled(!running);
	_ui->centralClusterCheckBox->setEnabled(!running);

	_ui->sizeSpinBox->setEnabled(!running);
	_ui->distanceScalePowerSpinBox->setEnabled(!running);

	_ui->saveRenderCheckBox->setEnabled(!running);
	_ui->renderSaveLocationButton->setEnabled(!running);
}

void MainWindow::onRunPressed()
{
	updateUI(true);

	const auto selectedClusteringMethod = static_cast<Clustering::method>(_ui->clusteringComboBox->currentIndex());

	if (_activeClustering) delete _activeClustering;

	switch (selectedClusteringMethod)
	{
		case Clustering::method::HALLEY:
		{
			auto halleyClustering = new HalleyClustering(_starRootEntity, this, _ui->shellCountSpinBox->value(), _ui->shellThicknessSpinBox->value(), _ui->firstShellDistanceSpinBox->value());
			QObject::connect(halleyClustering, &HalleyClustering::updateClusterProgress, _clusterProgressBar, &QProgressBar::setValue);
			QObject::connect(halleyClustering, &HalleyClustering::shellDone, [=](const int shellIndex, const int starCount, const float totalApvmag)
			{
				const float surfaceBrightness = totalApvmag + 2.5f * log10(CAMERA_ANGULAR_AREA_SQ_ARCSEC);
				const float linearSurfaceBrightness = pow(M_E, -surfaceBrightness);
				_ui->dataTable->addRow<Clustering::method::HALLEY>(shellIndex, starCount, totalApvmag, surfaceBrightness, linearSurfaceBrightness);
				_ui->dataChart->addDataPoint(starCount, surfaceBrightness);
				_ui->linearizedChart->addLinearPoint(starCount, linearSurfaceBrightness);
			});
			QObject::connect(halleyClustering, &HalleyClustering::shellDone, this, &MainWindow::saveRender);
			_activeClustering = halleyClustering;
			break;
		}
		case Clustering::method::FRACTAL:
		{
			auto fractalClustering = new FractalClustering(_starRootEntity, this, _ui->levelCountSpinBox->value(), _ui->countPerLevelSpinBox->value(), _ui->spacingSpinBox->value(), _ui->centralClusterCheckBox->isChecked());
			QObject::connect(fractalClustering, &FractalClustering::groupDone, [=](const int totalStarCount, const float totalApvmag)
			{
				const float surfaceBrightness = totalApvmag + 2.5f * log10(CAMERA_ANGULAR_AREA_SQ_ARCSEC);
				const float linearSurfaceBrightness = pow(M_E, -surfaceBrightness);
				_ui->dataTable->addRow<Clustering::method::FRACTAL>(totalStarCount, totalApvmag, surfaceBrightness, linearSurfaceBrightness);
				_ui->dataChart->addDataPoint(totalStarCount, surfaceBrightness);
				_ui->linearizedChart->addLinearPoint(totalStarCount, linearSurfaceBrightness);
			});
			QObject::connect(fractalClustering, &FractalClustering::groupDone, this, &MainWindow::saveRender);
			_activeClustering = fractalClustering;
			break;
		}
	}

	QObject::connect(_activeClustering, &Clustering::finished, this, &MainWindow::onFinished);
	QObject::connect(_activeClustering, &Clustering::updateProgress, _progressBar, &QProgressBar::setValue);

	_activeClustering->setCameraProjectionMatrix(_viewport->camera()->projectionMatrix(), _viewport->camera()->viewMatrix(), _viewportContainer->rect());
	_activeClustering->setStarProperties(_ui->sizeSpinBox->value(), _ui->distanceScalePowerSpinBox->value());
	_activeClustering->start();

	_ui->dataTable->setHeader(selectedClusteringMethod);
}

void MainWindow::onTerminatePressed()
{
	_activeClustering->terminate();

	updateUI(false);
}

void MainWindow::onClearPressed()
{
	_ui->dataTable->clear();
	_ui->dataChart->clear();
	_ui->linearizedChart->clear();

	QThread* clearThread = QThread::create([=]
	{
		_clearThreadMutex.lock();
		for (Qt3DCore::QNode* node : _starRootEntity->childNodes())
		{
			delete node;
			//Prevent application from freezing
			qApp->processEvents();
		}
		_clearThreadMutex.unlock();
	});
	clearThread->start();
	QObject::connect(clearThread, &QThread::finished, clearThread, &QThread::deleteLater);
}

void MainWindow::onFinished()
{
	delete _activeClustering;
	_activeClustering = nullptr;

	updateUI(false);
}

void MainWindow::selectRenderSaveLocation()
{
	const QString saveLocation = QFileDialog::getExistingDirectory(nullptr, "Select render image folder", QDir::homePath());
	_ui->renderSaveLocationLineEdit->setText(saveLocation);
}

void MainWindow::saveRender()
{
	if (!_ui->saveRenderCheckBox->isChecked() || !QDir(_ui->renderSaveLocationLineEdit->text()).exists()) return;

	const QString timeSignature = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
	_reply = _renderCapture->requestCapture();
	QObject::connect(_reply, &Qt3DRender::QRenderCaptureReply::completed, [=]
	{
		_reply->saveImage(_ui->renderSaveLocationLineEdit->text() + "/render_" + timeSignature + ".png");
		_reply->deleteLater();
		_reply = nullptr;
	});
}
