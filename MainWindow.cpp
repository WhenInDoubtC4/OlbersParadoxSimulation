#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
  , _ui(new Ui::MainWindow)
  , _progressStackPlaceholder(new QWidget())
  , _progressBar(new QProgressBar())
  , _progressLabel(new QLabel())
  , _clusterProgressStackPlaceholder(new QWidget())
  , _clusterProgressBar(new QProgressBar())
  , _clusterProgressLabel(new QLabel())
{
	_ui->setupUi(this);

	QFont monospacedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

	_progressStack = new QStackedLayout(_progressStackPlaceholder);
	_progressStack->setStackingMode(QStackedLayout::StackAll);
	_progressStack->addWidget(_progressBar);
	_progressStack->addWidget(_progressLabel);
	_progressStack->setCurrentIndex(1);
	_progressStackPlaceholder->setMaximumWidth(200);
	_progressLabel->setAlignment(Qt::AlignCenter);
	_progressLabel->setFont(monospacedFont);
	_ui->statusbar->addPermanentWidget(_progressStackPlaceholder);

	_clusterProgressStack = new QStackedLayout(_clusterProgressStackPlaceholder);
	_clusterProgressStack->setStackingMode(QStackedLayout::StackAll);
	_clusterProgressStack->addWidget(_clusterProgressBar);
	_clusterProgressStack->addWidget(_clusterProgressLabel);
	_clusterProgressStack->setCurrentIndex(1);
	_clusterProgressStackPlaceholder->setMaximumWidth(200);
	_clusterProgressLabel->setAlignment(Qt::AlignCenter);
	_clusterProgressLabel->setFont(monospacedFont);
	_ui->statusbar->addPermanentWidget(_clusterProgressStackPlaceholder);

	_viewport = new Qt3DExtras::Qt3DWindow(nullptr, Qt3DRender::API::OpenGL);
	_viewport->renderSettings()->setRenderPolicy(Qt3DRender::QRenderSettings::RenderPolicy::Always);
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
	_viewport->camera()->setPosition(QVector3D(0, 0, 0));
	_viewport->camera()->setUpVector(QVector3D(0, 1, 0));
	_viewport->camera()->lens()->setPerspectiveProjection(CAMERA_VFOV, CAMERA_ASPECT_RATIO, CAMERA_NEAR_CLIP_PLANE, CAMERA_FAR_CLIP_PLANE);

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

	QObject::connect(_ui->clusteringComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::updateEstimate);
	QObject::connect(_ui->shellCountSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateEstimate);
	QObject::connect(_ui->shellThicknessSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateEstimate);
	QObject::connect(_ui->firstShellDistanceSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateEstimate);
	QObject::connect(_ui->levelCountSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateEstimate);
	QObject::connect(_ui->countPerLevelSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateEstimate);
	QObject::connect(_ui->spacingSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateEstimate);
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
		_progressLabel->setText(QString());
		_clusterProgressBar->setValue(0);
		_clusterProgressLabel->setText(QString());
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
	_ui->renderSaveLocationLineEdit->setEnabled(!running);
}

void MainWindow::onRunPressed()
{
	updateUI(true);

	const auto selectedClusteringMethod = static_cast<clusteringMethod>(_ui->clusteringComboBox->currentIndex());

	if (_activeClustering)
	{
		delete _activeClustering;
		_activeClustering = nullptr;
	}

	switch (selectedClusteringMethod)
	{
		case clusteringMethod::HALLEY:
		{
			auto halleyClustering = new HalleyClustering(_starRootEntity, this, _ui->shellCountSpinBox->value(), _ui->shellThicknessSpinBox->value(), _ui->firstShellDistanceSpinBox->value());
			_activeClustering = halleyClustering;
			break;
		}
		case clusteringMethod::FRACTAL:
		{
			auto fractalClustering = new FractalClustering(_starRootEntity, this, _ui->levelCountSpinBox->value(), _ui->countPerLevelSpinBox->value(), _ui->spacingSpinBox->value(), _ui->centralClusterCheckBox->isChecked());
			_activeClustering = fractalClustering;
			break;
		}
	}

	QObject::connect(_activeClustering, &Clustering::clusterDone, this, &MainWindow::saveRender);
	QObject::connect(_activeClustering, &Clustering::finished, this, &MainWindow::onFinished);
	QObject::connect(_activeClustering, &Clustering::updateProgress, this, &MainWindow::updateProgress);

	_activeClustering->setCameraProjectionMatrix(_viewport->camera()->projectionMatrix(), _viewport->camera()->viewMatrix(), _viewportContainer->rect());
	_activeClustering->setStarProperties(_ui->sizeSpinBox->value(), _ui->distanceScalePowerSpinBox->value());
	_activeClustering->setDataTable(_ui->dataTable);
	_ui->dataTable->setHeader(selectedClusteringMethod);
	_activeClustering->setDataChart(_ui->dataChart);
	_activeClustering->setLinearizedChart(_ui->linearizedChart);
	_activeClustering->start();

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
	updateUI(false);
}

void MainWindow::updateProgress(const int placed, const int total, bool cluster)
{
	QProgressBar* progressBar = _progressBar;
	QLabel* label = _progressLabel;
	if (cluster)
	{
		progressBar = _clusterProgressBar;
		label = _clusterProgressLabel;
	}
	float percentage = (float(placed) / float(total)) * 100.f;
	progressBar->setValue(percentage);
	label->setText(QString::number(placed) + "/" + QString::number(total) + " (" + QString::number(percentage, 'f', 0) + "%)");
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

void MainWindow::updateEstimate()
{
	const auto selectedClusteringMethod = static_cast<clusteringMethod>(_ui->clusteringComboBox->currentIndex());
	QTime estimatedTime;
	int estimatedCount;
	switch (selectedClusteringMethod)
	{
		case clusteringMethod::HALLEY:
			HalleyClustering::calculateEstimate(_ui->shellCountSpinBox->value(), _ui->shellThicknessSpinBox->value(), _ui->firstShellDistanceSpinBox->value(), estimatedTime, estimatedCount);
			break;
		case clusteringMethod::FRACTAL:
			FractalClustering::calculateEstimate(_ui->levelCountSpinBox->value(), _ui->countPerLevelSpinBox->value(), _ui->spacingSpinBox->value(), estimatedTime, estimatedCount);
			break;
	}
	_ui->estimatedCountLineEdit->setText(QString::number(estimatedCount));
	_ui->etaLineEdit->setText(estimatedTime.toString("hh:mm:ss"));
}
