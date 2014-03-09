/*
 *  View of camera in model-view-controller design.
 */

#include <QtGui>
#include "EDSDKTypes.h"
#include "Camera.h"
#include "Import.h"
#include "View.h"
#include "LiveImageData.h"
#include "LogBrowser.h"
#include "Options.h"
#include "MultiShot.h"
#include "Widgets/LiveImage.h"
#include "Widgets/Histogram.h"
#include "Widgets/Navigation.h"
#include "Widgets/ShutterSpeed.h"
#include "Widgets/Aperture.h"
#include "Widgets/ISOSpeed.h"
#include "Widgets/WhiteBalance.h"
#include "Widgets/DriveMode.h"
#include "Widgets/MeteringMode.h"
#include "Widgets/ExposureComp.h"
#include "Widgets/FocusMode.h"
#include "Widgets/ShootingMode.h"
#include "Panels/Panel.h"
#include "Event.h"


/*
 *  Constructor for interface.
 */
View::View( Camera *camera, Import *import, QWidget *parent, Qt::WFlags flags )
    : QMainWindow( parent, flags )
{
    nAF = 0;
    vAF = 0;
    inLiveViewMode = false;
    scalingFactor = 5;
    hMax = 0;
    vMax = 0;
    hMoved = false;
    vMoved = false;

    this->camera = camera;
    this->import = import;
    QObject::connect(
	import, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)) );

    createMainWindow();
    createContextMenu();
    createStatusBar();

    setWindowTitle( "Camera Adjunct" );
    setMinimumSize( QSize(200, 300) );

    readSettings();
}

/*
 *  Destructor for interface.
 */
View::~View()
{
    /*
     *  All heap objects will be deleted automatically.
     *  See: QObject::dumpObjectTree();
     *       QObject::dumpObjectInfo();
     */
}

void
View::createMainWindow()
{
    /*
     *  Background, including panels.
     */
    QPalette palette;
    QBrush lightGray( QColor( 214, 214, 214 ) );
    lightGray.setStyle(Qt::SolidPattern);
    palette.setBrush( QPalette::Active, QPalette::Window, lightGray );
    palette.setBrush( QPalette::Disabled, QPalette::Window, lightGray );
    palette.setBrush( QPalette::Inactive, QPalette::Window, lightGray );
    setPalette( palette );
    setAutoFillBackground( true );

    /*
     *  Toolbar widgets.
     */
    tools = 0;
    logBrowser = new LogBrowser( 0, Qt::Window );

    options = new Options( camera, 0, Qt::Window );
    QObject::connect(
	options, SIGNAL(customPropertyChanged(int,int,int)),
	this, SIGNAL(customPropertyChanged(int,int,int)) );
    QObject::connect(
	options, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)) );
    QObject::connect(
	options, SIGNAL(propertyChanged(int,QString)),
	this, SIGNAL(propertyChanged(int,QString)) );
    QObject::connect(
	options, SIGNAL(propertyChanged(int,QDateTime)),
	this, SIGNAL(propertyChanged(int,QDateTime)) );
    QObject::connect(
	options, SIGNAL(propertyChanged(int,int *)),
	this, SIGNAL(propertyChanged(int,int *)) );

    multiShot = new MultiShot( camera, 0, Qt::Window );
    QObject::connect(
	multiShot, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)));
    QObject::connect(
	multiShot, SIGNAL(startSequence(ShotSeq *)),
	this, SIGNAL(startSequence(ShotSeq *)) );
    QObject::connect(
	multiShot, SIGNAL(cancelSequence()),
	this, SIGNAL(cancelSequence()) );
    QObject::connect(
	multiShot, SIGNAL(highISOtest(int)),
	this, SIGNAL(highISOtest(int)) );
    QObject::connect(
	multiShot, SIGNAL(appendLog(QString)),
	this, SLOT(appendLog(QString)) );

    /*
     *  Live view.
     */
    liveImage = new LiveImage( camera, this );
    QObject::connect(
	liveImage, SIGNAL(zoomRectChanged(int,int)),
	this, SIGNAL(zoomRectChanged(int,int)) );
    QObject::connect(
	liveImage, SIGNAL(focusPropertyChanged(int,int)),
	this, SIGNAL(focusPropertyChanged(int,int)) );

    liveScrollArea = new QScrollArea( this );
    liveScrollArea->setWidget( liveImage );
    liveScrollArea->setFocusPolicy( Qt::NoFocus );
    liveScrollArea->setAlignment( Qt::AlignCenter );
    liveScrollArea->setContextMenuPolicy( Qt::CustomContextMenu );
    /*
     *  Background for live view.
     *  Set background for displaying image to 18% gray.
     *  For a gamma of L*, this gives 127 for R,G,B.
     */
    QBrush middleGray( QColor( 127, 127, 127 ) );
    middleGray.setStyle(Qt::SolidPattern);
    palette.setBrush( QPalette::Active, QPalette::Window, middleGray );
    palette.setBrush( QPalette::Disabled, QPalette::Window, middleGray );
    palette.setBrush( QPalette::Inactive, QPalette::Window, middleGray );
    liveScrollArea->setPalette( palette );
    liveScrollArea->setAutoFillBackground( true );
    QObject::connect(
	liveScrollArea->horizontalScrollBar(), SIGNAL(rangeChanged(int,int)),
	this, SLOT(updateHorizontalSlider(int,int)) );
    QObject::connect(
	liveScrollArea->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	this, SLOT(horizontalSliderMoved(int)) );
    QObject::connect(
	liveScrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
	this, SLOT(updateVerticalSlider(int,int)) );
    QObject::connect(
	liveScrollArea->verticalScrollBar(), SIGNAL(sliderMoved(int)),
	this, SLOT(verticalSliderMoved(int)) );
    QObject::connect(
	liveScrollArea, SIGNAL(customContextMenuRequested(const QPoint&)),
	this, SLOT(showContextMenu()) );

    /*
     *  Shooting panels.
     */
    panelArea = new QScrollArea();
    panelArea->setFocusPolicy( Qt::NoFocus );
    panelArea->setFixedWidth( 293 );
    panelArea->setWidget( shootingPanelWidget() );
    panelArea->setAlignment( Qt::AlignLeft );
    panelArea->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    panelArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    panelArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    /*
     *  Intercept wheel events and scale image accordingly.
     */
    liveScrollArea->installEventFilter( this );
    liveImage->installEventFilter( this );

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget( liveScrollArea );
    layout->addWidget( panelArea );

    centralWidget = new QWidget( this );
    centralWidget->setLayout( layout );
    setCentralWidget( centralWidget );
}

/*
 *  Initialize and layout shooting panels.
 */
QWidget *
View::shootingPanelWidget()
{
    histogramPanel = new Panel( tr("Histogram") );
    histogramPanel->setPanelLayout( histogramPanelLayout() );

    exposurePanel = new Panel( tr("Exposure") );
    exposurePanel->setPanelLayout( exposurePanelLayout() );

    focusPanel = new Panel( tr("Focus") );
    focusPanel->setPanelLayout( focusPanelLayout() );

    navigationPanel = new Panel( tr("View/Navigation") );
    navigationPanel->setPanelLayout( navigationPanelLayout() );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSizeConstraint( QLayout::SetFixedSize );
    layout->addWidget( histogramPanel );
    layout->addSpacing( -10 );
    layout->addLayout( toolbarLayout() );
    layout->addSpacing( -10 );
    layout->addWidget( exposurePanel );
    layout->addSpacing( -20 );
    layout->addWidget( focusPanel );
    layout->addSpacing( -20 );
    layout->addWidget( navigationPanel );
    layout->setContentsMargins( 0, 0, 0, 0 );

    QWidget *panel = new QWidget( this );
    panel->setLayout( layout );
    return( panel );
}

void
View::createStatusBar()
{
    statusBar = new QStatusBar( this );
    statusBar->setSizeGripEnabled( true );
    setStatusBar( statusBar );

    /*
     *  Progress bar for downloading images.
     */
    progressBar = new QProgressBar( this );
    progressBar->setTextVisible( false );
    progressBar->setRange( 0, 100 );
    progressBar->setValue( 0 );
    progressBar->setFixedHeight( 16 );
    progressBar->setFixedWidth( 220 );
    progressBar->hide();
    statusBar->addPermanentWidget( progressBar );

    /*
     *  Connection status with camera.
     */
    connectionStatus = new QLabel();
    statusBar->addPermanentWidget( connectionStatus );
    updateConnectionStatus();

    /*
     *  Battery level.
     */
    battery = new QLabel();
    battery->setFixedWidth( 34 );
    alert = new QLabel();
    alert->setPixmap( QPixmap(":/Resources/Misc/alert.png") );
    alert->hide();
    statusBar->addPermanentWidget( alert );
    statusBar->addPermanentWidget( battery );
    updateBatteryLevel();
}

void
View::readSettings()
{
    QSettings settings;

    settings.beginGroup( "MainWindow" );
    resize( settings.value("size", QSize(400, 300)).toSize() );
    move( settings.value("pos", QPoint(200, 200)).toPoint() );
    settings.endGroup();
}

void
View::writeSettings()
{
    QSettings settings;

    settings.beginGroup( "MainWindow" );
    settings.setValue( "size", size() );
    settings.setValue( "pos", pos() );
    settings.endGroup();
}

bool
View::scrollBarUsesWheel( QScrollBar *bar )
{
    return( bar->underMouse() && bar->maximum() != 0 );
}

/*
 *  Intercept wheel events from live view scroll area.
 */
bool
View::eventFilter( QObject *target, QEvent *event )
{
    if( event->type() == QEvent::Wheel ) {
	if( target == liveImage ) {
	    processZoomEvent( static_cast<QWheelEvent *>(event)->delta() );
	    event->accept();
	    return( true );
	}
	else
	if( (target == liveScrollArea) &&
	    !scrollBarUsesWheel( liveScrollArea->horizontalScrollBar() ) &&
	    !scrollBarUsesWheel( liveScrollArea->verticalScrollBar() ) ) {
	    processZoomEvent( static_cast<QWheelEvent *>(event)->delta() );
	    event->accept();
	    return( true );
	}
    }

    return( false );
}

/*
 *  Reimplementation of closeEvent to ask the user whether to
 *  save the log in the case where it has changed.
 */
void
View::closeEvent( QCloseEvent *event )
{
    if( logBrowser->isModified() ) {
	QMessageBox::StandardButton answer;
	answer = QMessageBox::warning( this, tr("Camera Adjunct"),
		    tr("The log of shots and settings has been modified.\n"
		       "Do you want to save the log?"),
		    QMessageBox::Save |
		    QMessageBox::Discard |
		    QMessageBox::Cancel );
	if( (answer == QMessageBox::Cancel) ||
	   ((answer == QMessageBox::Save) && !logBrowser->saveLog()) ) {
	    event->ignore();
	    return;
	}
    }

    if( multiShot->sequenceInProgress() ) {
	QMessageBox::StandardButton answer;
	answer = QMessageBox::warning( this, tr("Camera Adjunct"),
		    tr("A sequence of shots is still in progress.\n"
		       "Do you want to wait for the remaining shots to complete?"),
		    QMessageBox::Yes |
		    QMessageBox::No );
	if( answer == QMessageBox::Yes ) {
	    event->ignore();
	    return;
	}
	if( answer == QMessageBox::No ) {
	    emit cancelSequence();
	}
    }

    if( camera->objectsWaiting() ) {
	QMessageBox::StandardButton answer;
	answer = QMessageBox::warning( this, tr("Camera Adjunct"),
		    tr("Downloading of images is still in progress.\n"
		       "Do you want to wait for downloading to complete?"),
		    QMessageBox::Yes |
		    QMessageBox::No );
	if( answer == QMessageBox::Yes ) {
	    event->ignore();
	    return;
	}
    }

    emit stopLiveView();
    writeSettings();
    qApp->quit();
    event->accept();
}

void
View::eventReport( Event *event )
{
    switch( event->type ) {
	/*
	 *  Progress/status for downloading or registering
	 *  image and movie files.
	 */
	case Event::DownloadInitiated:
	    QApplication::setOverrideCursor( Qt::WaitCursor );
	    break;
	case Event::DownloadProgress:
	    updateProgressBar( event->data );
	    break;
	case Event::DownloadComplete:
	    appendLog( event->description );
	    QApplication::restoreOverrideCursor();
	    break;
	case Event::RegisterComplete:
	    appendLog( event->description );
	    break;
	/*
	 *  Progress/status for bulb shooting.
	 */
	case Event::BulbExposureTime:
	    statusBar->showMessage( QString("Exposure time: %1 seconds")
					.arg( event->data ), 3000 );
	    break;
	/*
	 *  Progress/status for multi-shot sequence.
	 */
	case Event::SequenceInitiated:
	    QApplication::setOverrideCursor( Qt::BusyCursor );
	    break;
	case Event::SequenceProgress:
	    updateProgressBar( event->data );
	    break;
	case Event::SequenceCancelledNoConnection:
	    appendLog( "sequence cancelled, no connection to camera" );
	    cleanupMultiShot();
	    break;
	case Event::SequenceCancelledMirrorLockup:
	    QMessageBox::information( this, tr("Camera Adjunct"),
		tr("Mirror lockup is not available with multi-shot.\n"
		   "As an alternative, turn on live view mode to\n"
		   "effectively reduce vibration.") );
	    appendLog( "sequence cancelled, mirror lockup enabled" );
	    cleanupMultiShot();
	    break;
	case Event::SequenceCancelledNotEvfMode:
	    QMessageBox::information( this, tr("Camera Adjunct"),
		tr("Live view mode must be turned on for focus bracketing.\n") );
	    appendLog( "sequence cancelled, not in live view mode" );
	    cleanupMultiShot();
	    break;
	case Event::SequenceCancelled:
	case Event::SequenceComplete:
	    cleanupMultiShot();
	    break;
	/*
	 *  Warnings.
	 */
	case Event::CameraBusy:
	    statusBar->showMessage( tr("Camera busy"), 3000 );
	    break;
	case Event::ConnectionLost:
	    statusBar->showMessage( tr("Connection to camera lost"), 3000 );
	    break;
	case Event::NotEvfMode:
	    statusBar->showMessage( tr("Camera not in live view mode"), 3000 );
	    break;
	case Event::LensInManualFocus:
	    QMessageBox::information( this, tr("Camera Adjunct"),
		tr("The lens is set to manual focus mode.\n") );
	    break;
	case Event::NotVideoMode:
	    QMessageBox::information( this, tr("Camera Adjunct"),
		tr("The camera is not in video shooting mode.\n") );
	    break;
	case Event::NotZoomFaceMode:
	    QMessageBox::information( this, tr("Camera Adjunct"),
		tr("Zoom is not available in face recognition focus mode.") );
	    break;
	case Event::ISOAuto:
	    QMessageBox::information( this, tr("Camera Adjunct"),
		tr("High ISO test not available as ISO is set to auto.") );
	    break;
	/*
	 *  Failures.
	 */
	case Event::GetPropertyFailure:
	    qDebug( "getPropertyCommand: failure: %d", event->data );
	    break;
	case Event::SetPropertyFailure:
	    qDebug( "setPropertyCommand: failure: %d", event->data );
	    break;
	case Event::DownloadFailure:
	    qDebug( "download failure: %d", event->data );
	    break;
	case Event::RegisterFailure:
	    qDebug( "register failure: %d", event->data );
	    break;
	case Event::LockFailure:
	    qDebug( "lock failure: %d", event->data );
	    break;
	case Event::ShutterFailure:
	    qDebug( "shutter failure: %d", event->data );
	    break;
	case Event::LiveviewFailure:
	    qDebug( "live view failure: %d", event->data );
	    break;
	case Event::CaptureFailure:
	    qDebug( "capture failure: %d", event->data );
	    break;
	/*
	 *  Internal.
	 */
	case Event::GetPropertyUnhandled:
	    //qDebug( "getProperty: Unhandled property: %d", event->data );
	    break;
	case Event::GetPropertyListUnhandled:
	    //qDebug( "getPropertyList: Unhandled property: %d", event->data );
	    break;
	case Event::SetPropertyUnhandled:
	    //qDebug( "setProperty: Unhandled property: %d", event->data );
	    break;
    }

    delete event;
}

void
View::appendLog( const QString text )
{
    logBrowser->appendLog( text );
}

void
View::cleanupMultiShot()
{
    updateProgressBar( 100 );
    multiShot->finishSeq();
    liveImage->setImageOverlay( NULL );
    QApplication::restoreOverrideCursor();
    updateProgressBar( -1 );
}

void
View::updatePanorama( QImage image, int shot )
{
    multiShot->setImage( image, camera->getRotationSetting(), shot );
    liveImage->setImageOverlay( multiShot->getOverlay( shot ) );
}

void
View::updateProgressBar( int percent )
{
    if( percent < 0 ) {
	progressBar->reset();
	progressBar->setVisible( false );
    }
    else {
	progressBar->setValue( percent );
	progressBar->setVisible( true );
    }
}

void
View::updateConnectionStatus()
{
    if( camera->handle() == 0 ) {
	statusBar->showMessage( tr("No camera connected"), 0 );
	connectionStatus->setPixmap(
		QPixmap(":/Resources/Misc/disconnected.png") );
    }
    else {
	QString text = camera->getModelName();
	statusBar->showMessage( text + tr(" connected"), 0 );
	connectionStatus->setPixmap(
		QPixmap(":/Resources/Misc/connected.png") );
    }
}

/*
 *  Update the battery icon in the status bar.
 *  On T2i, the values are: 80, 50, 30, 1
 */
void
View::updateBatteryLevel()
{
    if( camera->handle() == 0 ) {
	battery->setPixmap( QPixmap() );
	return;
    }

    int l = camera->getBatteryLevel();

    if(  0 <= l && l < 30 ) {
	alert->show();
	battery->setPixmap( QPixmap(":/Resources/Battery/battery-020.png") );
    }
    else
    if( 30 <= l && l < 50 ) {
	alert->show();
	battery->setPixmap( QPixmap(":/Resources/Battery/battery-040.png") );
    }
    else
    if( 50 <= l && l < 80 ) {
	alert->hide();
	battery->setPixmap( QPixmap(":/Resources/Battery/battery-060.png") );
    }
    else
    if( 80 <= l && l < 90 ) {
	alert->hide();
	battery->setPixmap( QPixmap(":/Resources/Battery/battery-080.png") );
    }
    else
    if( 90 <= l && l <= 100 ) {
	alert->hide();
	battery->setPixmap( QPixmap(":/Resources/Battery/battery-100.png") );
    }
}

void
View::updateProperty( int property, int parameter )
{
    multiShot->updateProperty( property, parameter );

    switch( property ) {
	case kEdsPropID_ProductName:
	    updateConnectionStatus();
	    options->updateProperty( property, parameter );
	    rotationComboBox->setCurrentIndex( 0 );
	    setRotation( 0 );
	    // fall through intentional
	case kEdsPropID_BatteryLevel:
	    updateBatteryLevel();		
	    break;
	case kEdsPropID_AEMode:
	case kEdsPropID_AEModeSelect:
	    shootingMode->updateValue( camera );
	    break;
	case kEdsPropID_Av:
	    aperture->updateValue( camera );
	    break;
	case kEdsPropID_Tv:
	    shutterSpeed->updateValue( camera );
	    break;
	case kEdsPropID_ISOSpeed:
	    isoSpeed->updateValue( camera );
	    break;
	case kEdsPropID_MeteringMode:
	    meteringMode->updateValue( camera );
	    break;
	case kEdsPropID_WhiteBalance:
	    whiteBalance->updateValue( camera );
	    break;
	case kEdsPropID_ExposureCompensation:
	    exposureComp->updateValue( camera );
	    break;
	case kEdsPropID_DriveMode:
	    driveMode->updateValue( camera );
	    break;
	case kEdsPropID_AFMode:
	    focusMode->updateValue( camera );
	    break;
	case kEdsPropID_Evf_AFMode:
	    updateEvfAFValue( camera );	
	    break;
	/*
	 *  Options pages.
	 */
	case kEdsPropID_Artist:
	case kEdsPropID_BodyIDEx:
	case kEdsPropID_CFn:
	case kEdsPropID_ColorSpace:
	case kEdsPropID_Copyright:
	case kEdsPropID_DateTime:
	case kEdsPropID_FirmwareVersion:
	case kEdsPropID_ImageQuality:
	case kEdsPropID_PictureStyle:
	case kEdsPropID_WhiteBalanceShift:
	    options->updateProperty( property, parameter );
	    break;
	/*
	 *  Ignored.
	 */
	case kEdsPropID_AvailableShots:
	case kEdsPropID_DepthOfField:
	case kEdsPropID_Evf_DepthOfFieldPreview:
	case kEdsPropID_Evf_Mode:
	case kEdsPropID_Evf_OutputDevice:
	case kEdsPropID_FocusInfo:
	case kEdsPropID_LensStatus:
	case kEdsPropID_Record:
	case kEdsPropID_SaveTo:
	    break;
	default:
	    break;
    }
}

void
View::updatePropertyList( int property )
{
    multiShot->updatePropertyList( property );

    switch( property ) {
	case kEdsPropID_AEMode:
	case kEdsPropID_AEModeSelect:
	    shootingMode->updateList( camera );
	    break;
	case kEdsPropID_Av:
	    aperture->updateList( camera );
	    break;
	case kEdsPropID_Tv:
	    shutterSpeed->updateList( camera );
	    break;
	case kEdsPropID_ISOSpeed:
	    isoSpeed->updateList( camera );
	    break;
	case kEdsPropID_MeteringMode:
	    meteringMode->updateList( camera );
	    break;
	case kEdsPropID_WhiteBalance:
	    whiteBalance->updateList( camera );
	    break;
	case kEdsPropID_ExposureCompensation:
	    exposureComp->updateList( camera );
	    break;
	case kEdsPropID_PictureStyle:
	    options->updatePictureStyleList();
	    break;
	case kEdsPropID_ImageQuality:
	    options->updateImageQualityList();
	    break;
	case kEdsPropID_DriveMode:
	    driveMode->updateList( camera );
	    break;
	case kEdsPropID_AFMode:
	    focusMode->updateList( camera );
	    break;
	case kEdsPropID_Evf_AFMode:
	    setEvfAFList( camera );
	    break;
	default:
	    break;
    }
}

void
View::updateHorizontalSlider( int newMin, int newMax )
{
    if( newMax == 0 ) {
	/*
	 *  Scroll bar was or has become inactive.
	 */
	hMoved = false;
	liveScrollArea->horizontalScrollBar()->setSliderDown( false );
    }
    else
    if( !hMoved ) {
	/*
	 *  Active scroll bar and user hasn't moved the slider.
	 *  Set the value of the scroll bar to correspond to the
	 *  center of the zoom rectangle.
	 */
	QPoint zoomPosition = camera->getEvfZoomPosition();
	QSize zoomSize = camera->getEvfZoomSize();
	QRect worldRect = camera->getEvfCoordinateSystem();
	int centerX = zoomPosition.x() + zoomSize.width() / 2;
	double scale = double(centerX) /
			double(worldRect.width());
	int value = int(scale * newMax);
	liveScrollArea->horizontalScrollBar()->setSliderDown( false );
	liveScrollArea->horizontalScrollBar()->setValue( value );
	liveScrollArea->horizontalScrollBar()->setSliderDown( true );
    }
    else
    if( hMoved ) {
	/*
	 *  Active scroll bar and user has moved the slider.
	 *  Set the new value of the scroll bar to correspond to
	 *  the old value scaled to the new range.
	 */
	double scale = double(newMax) / double(hMax);
	int value = int(scale * liveScrollArea->horizontalScrollBar()->value());
	liveScrollArea->horizontalScrollBar()->setSliderDown( false );
	liveScrollArea->horizontalScrollBar()->setValue( value );
	liveScrollArea->horizontalScrollBar()->setSliderDown( true );
    }
    hMax = newMax;
}

void
View::updateVerticalSlider( int newMin, int newMax )
{
    if( newMax == 0 ) {
	/*
	 *  Scroll bar was or has become inactive.
	 */
	vMoved = false;
	liveScrollArea->verticalScrollBar()->setSliderDown( false );
    }
    else
    if( !vMoved ) {
	/*
	 *  Active scroll bar and user hasn't moved the slider.
	 *  Set the value of the scroll bar to correspond to the
	 *  center of the zoom rectangle.
	 */
	QPoint zoomPosition = camera->getEvfZoomPosition();
	QSize zoomSize = camera->getEvfZoomSize();
	QRect worldRect = camera->getEvfCoordinateSystem();
	int centerY = zoomPosition.y() + zoomSize.height() / 2;
	double scale = double(centerY) /
			double(worldRect.height());
	int value = int(scale * newMax);
	liveScrollArea->verticalScrollBar()->setSliderDown( false );
	liveScrollArea->verticalScrollBar()->setValue( value );
	liveScrollArea->verticalScrollBar()->setSliderDown( true );
    }
    else
    if( vMoved ) {
	/*
	 *  Active scroll bar and user has moved the slider.
	 *  Set the new value of the scroll bar to correspond to
	 *  the old value scaled to the new range.
	 */
	double scale = double(newMax) / double(vMax);
	int value = int(scale * liveScrollArea->verticalScrollBar()->value());
	liveScrollArea->verticalScrollBar()->setSliderDown( false );
	liveScrollArea->verticalScrollBar()->setValue( value );
	liveScrollArea->verticalScrollBar()->setSliderDown( true );
    }
    vMax = newMax;
}

void
View::horizontalSliderMoved( int value )
{
    hMoved = true;
}

void
View::verticalSliderMoved( int value )
{
    vMoved = true;
}

/*
 *  Scaling values for zooming live view image.
 *  These are determined using the following code:
 *
 *      #define MAX_ZOOM 2.0
 *      main()
 *      {
 *          int i, factor;
 *          double scale, f;
 *          for( factor = 16; factor < 20; factor++ ) {
 *              printf("factor %d\n",factor);
 *              f = (double)factor;
 *              scale = MAX_ZOOM * 20.0/f;
 *              for( i = 0; i < 12; i++ ) {
 *                  scale *= f/20.0;
 *                  printf("%12.8f\n",scale);
 *              }
 *              printf("\n");
 *          }
 *      }
 */
const int View::MAX_SCALING_FACTOR = 15;
const double View::scaling[] = {
    0.31381060,
    0.34867844,
    0.38742049,
    0.43046721,
    0.47829690,
    0.53144100,
    0.59049000,
    0.65610000,
    0.72900000,
    0.81000000,
    0.90000000,
    1.00000000,
    1.12500000,
    1.50000000,
    2.00000000
};

void
View::setZoomFactor( int value )
{
    scalingFactor = value;
    double zoomF = scaling[scalingFactor];
    statusBar->showMessage( QString("Zoom: %1%").arg(
	100.0 * zoomF, 4, 'f', (zoomF < 1.0)?1:0 ), 3000 );
}

void
View::processZoomEvent( int delta )
{
    if( inLiveViewMode ) {
	if( delta < 0 ) {
	    scalingFactor--;
	    if( scalingFactor < 0 ) {
		scalingFactor = 0;
	    }
	}
	else {
	    scalingFactor++;
	    if( scalingFactor > MAX_SCALING_FACTOR-1 ) {
		scalingFactor = MAX_SCALING_FACTOR-1;
	    }
	}
	zoomSlider->setValue( scalingFactor );
	double zoomF = scaling[scalingFactor];
	statusBar->showMessage( QString("Zoom: %1%").arg(
		100.0 * zoomF, 4, 'f', (zoomF < 1.0)?1:0 ), 3000 );
    }
}

void
View::updateLiveView( EvfImageData *data )
{
    if( data != NULL ) {
	data->histogramStatus |= histogramMode;
	data->clippingFlag = getClippingSetting();
	data->overlayFlag = getOverlaySetting();
	/*
	 *  Changing live view focus mode while in zoom mode hangs the
	 *  camera, so disable widget when zooming.
	 */
	focusModeLive->setEnabled( data->zoom == 1 );
	/*
	 *  Zoom events can be initiated at both the computer and the
	 *  camera. Check if the buttons need to be reset.
	 */
	if( camera->getEvfZoom() != scaleChosen->checkedId() ) {
	    scaleChosen->button( camera->getEvfZoom() )->setChecked( true );
	}
    }
    histogram->setData( data );
    liveImage->setData( data, scaling[scalingFactor] );

    /*
     *  Determine which part of the image is currently visible in the
     *  viewport. Since the image is centered in the viewport, a positive
     *  position value means all of that axis is visible and a negative
     *  position value means some of that axis is not visible. The
     *  convention used is that QRectF( 0, 0, 1, 1 ) represents that
     *  the entire image is visible.
     */
    QRectF visibleRect;
    if( data != NULL ) {
	double imageWidth = liveImage->width();
	double imageHeight = liveImage->height();
	double x = -liveImage->pos().x() / imageWidth;
	if( x < 0.0 ) x = 0.0;
	double y = -liveImage->pos().y() / imageHeight;
	if( y < 0.0 ) y = 0.0;
	double w = liveScrollArea->viewport()->width() / imageWidth;
	if( w > 1.0 ) w = 1.0;
	double h = liveScrollArea->viewport()->height() / imageHeight;
	if( h > 1.0 ) h = 1.0;
	visibleRect.setRect( x, y, w, h );
    }
    navigation->setData( data, visibleRect );

    if( data == NULL ) {
	focusModeLive->setEnabled( true );
    }

    if( data != NULL ) {
	delete data;
	data = NULL;
    }
}

