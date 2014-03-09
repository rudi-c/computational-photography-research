/*
 *  Multi-shot dialog: multiple shots for exposure bracketing,
 *  focus bracketing, super-resolution, noise reduction, interval
 *  shooting, and panoramas.
 */

#include <QtGui>
#include "MultiShot.h"
#include "Camera.h"
#include "Event.h"
#include "ShotSeq.h"
#include "Overlay.h"
#include "Checklist.h"
#include "CustomFunction.h"
#include "Widgets/ExposureComp.h"
#include "Widgets/ImageTable.h"
#include "Map.h"


/*
 *  Constructor for multi-shot.
 */
MultiShot::MultiShot( Camera *camera, QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    this->camera = camera;
    shotSeq = new ShotSeq();
    inProgress = false;
    checklist = 0;
    step = camera->getExposureLevelIncrements();

    setWindowTitle( tr("Multi-shot[*]") );
    setAttribute( Qt::WA_QuitOnClose, false ); // close window if app closes

    initialize();

    readSettings();
}

/*
 *  Destructor for multi-shot.
 */
MultiShot::~MultiShot()
{
    delete shotSeq;
}

/*
 *  Initialize dialog window.
 */
void
MultiShot::initialize()
{
    /*
     *  Pages.
     */
    exposureBracketingPage = getExposureBracketingPage();
    focusBracketingPage = getFocusBracketingPage();
    intervalsPage = getIntervalsPage();
    stitchPage = getStitchPage();

    tabWidget = new QTabWidget();
    tabWidget->addTab( exposureBracketingPage, tr("Exposure bracketing") );
    tabWidget->addTab( focusBracketingPage, tr("Focus bracketing") );
    tabWidget->addTab( intervalsPage, tr("Intervals") );
    tabWidget->addTab( stitchPage, tr("Panorama") );
    tabWidget->setCurrentIndex( 0 );
    QObject::connect(
	tabWidget, SIGNAL(currentChanged(int)),
	this, SLOT(updateChecklist()) );

    /*
     *  Buttons.
     */
    QPushButton *checklistButton = new QPushButton( tr("Checklist") );
    checklistButton->setIcon(
	QIcon(":/Resources/Misc/checklist.png") );
    QObject::connect(
	checklistButton, SIGNAL(clicked()),
	this, SLOT(checklistButtonClicked()) );

    runButton = new QPushButton( tr("Run") );
    QObject::connect(
	runButton, SIGNAL(clicked()),
	this, SLOT(runButtonClicked()) );

    stopButton = new QPushButton( tr("Stop") );
    stopButton->setEnabled( false );
    QObject::connect(
	stopButton, SIGNAL(clicked()),
	this, SLOT(stopButtonClicked()) );

    QPushButton *closeButton = new QPushButton( tr("Close") );
    closeButton->setDefault( true );
    QObject::connect(
	closeButton, SIGNAL(clicked()),
	this, SLOT(close()) );

    /*
     *  Final layout.
     */
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget( checklistButton );
    buttonLayout->addStretch();
    buttonLayout->addWidget( runButton );
    buttonLayout->addWidget( stopButton );
    buttonLayout->addWidget( closeButton );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( tabWidget );
    layout->addLayout( buttonLayout );
    setLayout( layout );
}

/*
 *  Exposure bracketing page.
 *
 *  Purpose:
 *  1. selecting best shot, and
 *  2. increase dynamic range (HDR).
 *
 *  For both of these purposes, it is best to maintain a fixed aperture,
 *  focal distance, focal length, ISO setting, subject distance, and white
 *  balance. That means Manual mode or Av mode, and no auto ISO, no auto
 *  white balance, no auto focus (or turn off auto focus once focus has
 *  been achieved when setting up the shot) and use a tripod if available
 *  (turn off image stabilization).
 *
 *  For purpose 2, want to take the frames as quickly as the exposure can
 *  be adjusted between images (to minimize movement of the subject).
 *  For static subjects, the exposure is usually adjusted by altering the
 *  shutter speed. For dynamic subjects, the exposure can be adjusted by
 *  altering the ISO (not yet implemented, but if shooting RAW this can
 *  be done in post-processing).
 */
QWidget *
MultiShot::getExposureBracketingPage()
{
    /*
     *  Settings.
     */
    const int MAX_Frames = 7; // range is 1 .. 2*MAX + 1

    QLabel *eFramesLabel = new QLabel( tr("Images:" ) );
    eFramesLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    QStackedWidget *eFramesDisplay = new QStackedWidget();
    eFramesDisplay->setFont( QFont("Calibri", 12) );
    QLabel *eFramePage[MAX_Frames+1];
    for( int i = 0; i <= MAX_Frames; i++ ) {
	eFramePage[i] = new QLabel( eFramesDisplay );
	eFramePage[i]->setText( QString::number((2*i)+1) );
	eFramePage[i]->setAlignment( Qt::AlignLeft );
	eFramesDisplay->insertWidget( i, eFramePage[i] );
    }
    eFramesDisplay->setCurrentIndex( 0 );

    eFrames = new QSlider();
    eFrames->setCursor( QCursor(Qt::SizeHorCursor) );
    eFrames->setValue( 0 );
    eFrames->setMinimum( 0 );
    eFrames->setMaximum( MAX_Frames );
    eFrames->setPageStep( 1 );
    eFrames->setOrientation( Qt::Horizontal );
    eFrames->setTickPosition( QSlider::TicksBelow );
    eFrames->setTickInterval( 1 );
    QObject::connect(
	eFrames, SIGNAL(valueChanged(int)),
	eFramesDisplay, SLOT(setCurrentIndex(int)));

    QLabel *eIncrementLabel = new QLabel( tr("Increment:" ) );
    eIncrementLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    QStackedWidget *eIncrementDisplay = new QStackedWidget();
    eIncrementDisplay->setFont( QFont("Calibri", 12) );
    for( int i = 0; i < 10; i++ ) {
	eIncrementPage[i] = new QLabel( eIncrementDisplay );
	eIncrementPage[i]->setAlignment( Qt::AlignLeft );
	eIncrementDisplay->insertWidget( i, eIncrementPage[i] );
    }
    eIncrementDisplay->setCurrentIndex( 0 );

    eIncrement = new QSlider();
    eIncrement->setCursor( QCursor(Qt::SizeHorCursor) );
    eIncrement->setValue( 0 );
    eIncrement->setMinimum( 0 );
    eIncrement->setPageStep( 1 );
    eIncrement->setOrientation( Qt::Horizontal );
    eIncrement->setTickPosition( QSlider::TicksBelow );
    eIncrement->setTickInterval( 1 );

    setIncrementPages();

    QObject::connect(
	eIncrement, SIGNAL(valueChanged(int)),
	eIncrementDisplay, SLOT(setCurrentIndex(int)));

    //QPushButton *autoButton = new QPushButton( tr("Auto") );

    eCompensation = new ExposureComp( camera, 5 );
    QObject::connect(
	eCompensation, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)));
    QObject::connect(
	eFrames, SIGNAL(valueChanged(int)),
	eCompensation, SLOT(updateExposures(int)));
    QObject::connect(
	eIncrement, SIGNAL(valueChanged(int)),
	eCompensation, SLOT(updateIncrement(int)));

    /*
     *  Final layout.
     */
    QGridLayout *settingsLayout = new QGridLayout();
    settingsLayout->addWidget( eFramesLabel,	  0, 0 );
    settingsLayout->addWidget( eFrames,		  0, 1 );
    settingsLayout->addWidget( eFramesDisplay,	  0, 2 );
    settingsLayout->addWidget( eIncrementLabel,   1, 0 );
    settingsLayout->addWidget( eIncrement,	  1, 1 );
    settingsLayout->addWidget( eIncrementDisplay, 1, 2 );
    //settingsLayout->addWidget( autoButton,	  2, 1 );
    settingsLayout->setColumnStretch( 3, 1 );
    settingsLayout->setColumnMinimumWidth( 0, 100 );

    QGroupBox *settingsGroupBox = new QGroupBox( tr("Settings") );
    settingsGroupBox->setLayout( settingsLayout );

    QHBoxLayout *bracketingLayout = new QHBoxLayout();
    bracketingLayout->addSpacing( 35 );
    bracketingLayout->addWidget( eCompensation, 0, Qt::AlignLeft );

    QGroupBox *bracketingGroupBox =
	new QGroupBox( tr("Exposure compensation and bracketing") );
    bracketingGroupBox->setLayout( bracketingLayout );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget( settingsGroupBox );
    layout->addWidget( bracketingGroupBox );
    layout->addStretch( 1 );

    QWidget *widget = new QWidget( this );
    widget->setLayout( layout );

    return( widget );
}

/*
 *  Focus bracketing page.
 *
 *  Purpose:
 *  1. selecting best shot, and
 *  2. increase depth of field (focus stacking).
 *
 *  For both of these purposes, it is best to maintain a fixed aperture,
 *  focal length, ISO setting, shutter speed, subject distance, and white
 *  balance. That means Manual mode, and no auto ISO, no auto white balance,
 *  and a tripod would be mandatory (turn off image stabilization).
 *
 *  For purpose 2, want to take the frames as quickly as the lens can
 *  be adjusted between images (to minimize movement of the subject).
 */
QWidget *
MultiShot::getFocusBracketingPage()
{
    QLabel *fFramesLabel = new QLabel( tr("Images:") );
    fFramesLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    fFrames = new QSpinBox();
    fFrames->setRange( 1, ShotSeq::MAX_BRACKET-1 );

    QLabel *fStepLabel = new QLabel( tr("Increment:" ) );
    fStepLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    fStep = new QComboBox();
    fStep->setIconSize( QSize(22, 16) );
    fStep->addItem( QIcon(":/Resources/Focus/arrow-left3.png"),
		     "Near 3", kEdsEvfDriveLens_Near3 );
    fStep->addItem( QIcon(":/Resources/Focus/arrow-left2.png"),
		     "Near 2", kEdsEvfDriveLens_Near2 );
    fStep->addItem( QIcon(":/Resources/Focus/arrow-left1.png"),
		     "Near 1", kEdsEvfDriveLens_Near1 );
    fStep->addItem( QIcon(":/Resources/Focus/arrow-right1.png"),
		     "Far 1", kEdsEvfDriveLens_Far1 );
    fStep->addItem( QIcon(":/Resources/Focus/arrow-right2.png"),
		     "Far 2", kEdsEvfDriveLens_Far2 );
    fStep->addItem( QIcon(":/Resources/Focus/arrow-right3.png"),
		     "Far 3", kEdsEvfDriveLens_Far3 );
    fStep->setCurrentIndex( 0 );

    /*
//TODO: focus bracketing display widget
     *  Could use manual setting of lens to start lens in a known
     *  position, but the number of focus steps is dependent on
     *  the focal length and the lens.
     *
     *  User input: focal length? (see tools.cpp)
     *  Show manual focus switch?
     */

    /*
     *  Final layout.
     */
    QGridLayout *settingsLayout = new QGridLayout();
    settingsLayout->addWidget( fFramesLabel,	 0, 0 );
    settingsLayout->addWidget( fFrames,		 0, 1 );
    settingsLayout->addWidget( fStepLabel,	 1, 0 );
    settingsLayout->addWidget( fStep,		 1, 1 );
    settingsLayout->setColumnStretch( 2, 1 );
    settingsLayout->setColumnMinimumWidth( 0, 100 );

    QGroupBox *settingsGroupBox = new QGroupBox( tr("Settings") );
    settingsGroupBox->setLayout( settingsLayout );

    QHBoxLayout *bracketingLayout = new QHBoxLayout();
    bracketingLayout->addSpacing( 35 );

    QGroupBox *bracketingGroupBox =
	new QGroupBox( tr("Focus depth and bracketing") );
    bracketingGroupBox->setLayout( bracketingLayout );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget( settingsGroupBox );
    layout->addWidget( bracketingGroupBox );
    layout->addStretch( 1 );

    QWidget *widget = new QWidget( this );
    widget->setLayout( layout );

    return( widget );
}

/*
 *  Intervals page (fixed exposure and focus).
 *
 *  Purpose:
 *  1. selecting best shot in a dynamically changing scene (e.g., cresting
 *     wave, sports action shot),
 *  2. combine a fast burst of images into one image to show a sequence
 *     of actions (e.g., a skateboarder doing a trick jump),
 *  3. increasing resolution and object details (i.e., super-resolution),
 *  4. noise reduction,
 *  5. remove transient objects from a scene,
 *  6. time-lapse photography, and
 *  7. longer exposure times than are possible in a single exposure in low
 *     light imaging (e.g., night photography, star trails).
 *
 *  For each of these purposes, it is best to maintain a fixed aperture,
 *  focal distance, focal length, ISO setting, shutter speed, subject
 *  distance, and white balance. That means Manual mode works best, and
 *  no auto ISO, no auto white balance, no auto focus (or turn off auto
 *  focus once focus has been achieved when setting up the shot) and use
 *  a tripod if available (turn off image stabilization).
 *
 *  For purposes 1-4, generally want burst mode with fast shutter speeds
 *  and fewer images. The limiting factor is max images in burst mode.
 *  For purposes 6-7, generally want many more images at regular intervals
 *  with both bulb and faster shutter speeds. The limiting factor is the
 *  speed of the camera, SD card, and download link for storing the images.
 *  May need to increase the interval length or decrease the resolution.
 *  For bulb shooting, turn off live view, dim the display, and pause a
 *  few seconds before beginning taking shots.
 */
QWidget *
MultiShot::getIntervalsPage()
{
    /*
     *  Settings.
     */
    QLabel *iDelayLabel = new QLabel( tr("Delay (mm:ss):") );
    iDelayLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    iDelay = new QTimeEdit();
    iDelay->setDisplayFormat( "m':'ss" );
    iDelay->setCurrentSection( QDateTimeEdit::MinuteSection );
    iDelay->setTime( QTime(0,0,0) );
    iDelay->setWrapping( true );
    QObject::connect(
	iDelay, SIGNAL(timeChanged(const QTime &)),
	this, SLOT(updateIntervalTiming()) );

    QLabel *iIntervalLabel = new QLabel( tr("Interval (mm:ss):") );
    iIntervalLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    iInterval = new QTimeEdit();
    iInterval->setDisplayFormat( "m':'ss" );
    iInterval->setCurrentSection( QDateTimeEdit::MinuteSection );
    iInterval->setTime( QTime(0,0,0) );
    iInterval->setWrapping( true );
    //iInterval->setSpecialValueText( "Continuous" );
    QObject::connect(
	iInterval, SIGNAL(timeChanged(const QTime &)),
	this, SLOT(updateIntervalTiming()) );

    QLabel *iFramesLabel = new QLabel( tr("Images:") );
    iFramesLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    iFrames = new QSpinBox();
    iFrames->setRange( 1, 1000 );
    QObject::connect(
	iFrames, SIGNAL(valueChanged(int)),
	this, SLOT(updateIntervalTiming()) );

    QLabel *iBulbLabel = new QLabel( tr("Bulb (mm:ss):") );
    iBulbLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    iBulb = new QTimeEdit();
    iBulb->setDisplayFormat( "m':'ss" );
    iBulb->setCurrentSection( QDateTimeEdit::MinuteSection );
    iBulb->setMinimumTime( QTime(0,0,30) );
    iBulb->setWrapping( true );
    iBulb->setEnabled( camera->shutterInBulbMode() );
    QObject::connect(
	iBulb, SIGNAL(timeChanged(const QTime &)),
	this, SLOT(updateIntervalTiming()) );

    iTest = new QPushButton( tr("High ISO test") );
    QObject::connect(
	iTest, SIGNAL(clicked()),
	this, SLOT(highISOtestButtonClicked()) );

    /*
     *  Display timing of shots.
     */
    QLabel *timingLabel = new QLabel( tr("Timing:") );
    timingDisplay = new TimingDisplay( this );

    /*
     *  Final layout.
     */
    QGridLayout *settingsLayout = new QGridLayout();
    settingsLayout->addWidget( iDelayLabel,	 0, 0 );
    settingsLayout->addWidget( iDelay,		 0, 1 );
    settingsLayout->addWidget( iFramesLabel,	 0, 2 );
    settingsLayout->addWidget( iFrames,		 0, 3 );
    settingsLayout->addWidget( iIntervalLabel,	 1, 0 );
    settingsLayout->addWidget( iInterval,	 1, 1 );
    settingsLayout->addWidget( iBulbLabel,	 1, 2 );
    settingsLayout->addWidget( iBulb,		 1, 3 );
    settingsLayout->addWidget( iTest,		 2, 3 );
    settingsLayout->setColumnStretch( 4, 1 );
    settingsLayout->setColumnMinimumWidth( 0, 100 );
    settingsLayout->setColumnMinimumWidth( 1, 80 );
    settingsLayout->setColumnMinimumWidth( 2, 100 );
    settingsLayout->setColumnMinimumWidth( 3, 80 );

    QGroupBox *settingsGroupBox = new QGroupBox( tr("Settings") );
    settingsGroupBox->setLayout( settingsLayout );

    QHBoxLayout *timingLayout = new QHBoxLayout();
    timingLayout->addWidget( timingDisplay );

    QGroupBox *timingGroupBox = new QGroupBox( tr("Timing") );
    timingGroupBox->setLayout( timingLayout );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget( settingsGroupBox );
    layout->addWidget( timingGroupBox );
    layout->addStretch( 1 );

    QWidget *widget = new QWidget( this );
    widget->setLayout( layout );

    return( widget );
}

/*
 *  Stitching page (multiple shots for panoramas).
 *
 *  Purpose:
 *  1. select best shot,
 *  2. increase resolution, and
 *  3. increase coverage (e.g., panoramas, collages).
 *
 *  For all of these purposes, it is best to maintain a fixed aperture,
 *  focal distance, focal length, ISO setting, subject distance, and white
 *  balance. That means Manual mode, and no auto ISO, no auto white balance,
 *  no auto focus (or turn off auto focus once focus has been achieved when
 *  setting up the shot) and use a tripod if available (turn off image
 *  stabilization).
 */
QWidget *
MultiShot::getStitchPage()
{
    /*
     *  Settings.
     */
    QLabel *columnsLabel = new QLabel( tr("Horizontal size:") );
    columnsLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    columns = new QSpinBox();
    columns->setRange( 1, 7 );

    QLabel *rowsLabel = new QLabel( tr("Vertical size:") );
    rowsLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    rows = new QSpinBox();
    rows->setRange( 1, 2 );

    QLabel *orderLabel = new QLabel( tr("Order:") );
    orderLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    QComboBox *order = new QComboBox();
    order->addItem( "L-to-R", ORDER_LtoR );
    order->addItem( "L-to-R (alt)", ORDER_LtoR_alt );
    order->addItem( "R-to-L", ORDER_RtoL );
    order->addItem( "R-to-L (alt)", ORDER_RtoL_alt );
    order->setCurrentIndex( 0 );

    QLabel *overlapLabel = new QLabel( tr("Overlap:") );
    overlapLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    overlap = new QSpinBox();
    overlap->setRange( 20, 40 );
    overlap->setValue( 30 );
    overlap->setSuffix( "%" );

    QLabel *opacityLabel = new QLabel( tr("Opacity:") );
    opacityLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    opacity = new QSpinBox();
    opacity->setRange( 0, 100 );
    opacity->setValue( 50 );
    opacity->setSuffix( "%" );

    QPushButton *clearButton = new QPushButton( tr("Clear table") );

    QLabel *note = new QLabel();
    QFont font;
    font.setItalic( true );
    note->setFont( font );
    note->setText(
	tr("Use live view and the shutter button to align and take individual images.") );

    /*
     *  Table of images.
     */
    imageTable = new ImageTable();
    QObject::connect(
	columns, SIGNAL(valueChanged(int)),
	imageTable, SLOT(setColumns(int)) );
    QObject::connect(
	rows, SIGNAL(valueChanged(int)),
	imageTable, SLOT(setRows(int)) );
    QObject::connect(
	order, SIGNAL(activated(int)),
	imageTable, SLOT(setOrder(int)) );
    QObject::connect(
	clearButton, SIGNAL(clicked()),
	imageTable, SLOT(clearTable()) );

    scrollArea = new QScrollArea();
    scrollArea->setWidget( imageTable );
    scrollArea->setFocusPolicy( Qt::NoFocus );
    scrollArea->setAlignment( Qt::AlignCenter );

    /*
     *  Final layout.
     */
    QGridLayout *settingsLayout = new QGridLayout();
    settingsLayout->addWidget( columnsLabel, 0, 0 );
    settingsLayout->addWidget( columns,	     0, 1 );
    settingsLayout->addWidget( rowsLabel,    1, 0 );
    settingsLayout->addWidget( rows,	     1, 1 );
    settingsLayout->addWidget( orderLabel,   2, 0 );
    settingsLayout->addWidget( order,	     2, 1 );
    settingsLayout->addWidget( overlapLabel, 0, 2 );
    settingsLayout->addWidget( overlap,	     0, 3 );
    settingsLayout->addWidget( opacityLabel, 1, 2 );
    settingsLayout->addWidget( opacity,	     1, 3 );
    settingsLayout->addWidget( clearButton,  2, 3 );
    settingsLayout->addWidget( note,         3, 0, 1, 4 );
    settingsLayout->setColumnStretch( 4, 1 );
    settingsLayout->setColumnMinimumWidth( 0, 100 );
    settingsLayout->setColumnMinimumWidth( 1, 80 );
    settingsLayout->setColumnMinimumWidth( 2, 100 );
    settingsLayout->setColumnMinimumWidth( 3, 80 );

    QGroupBox *settingsGroupBox = new QGroupBox( tr("Settings") );
    settingsGroupBox->setLayout( settingsLayout );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget( settingsGroupBox );
    layout->addWidget( scrollArea );

    QWidget *widget = new QWidget( this );
    widget->setLayout( layout );

    return( widget );
}

/*
 *  Onion skinning page.
 *
 *  Purpose:
 *  1. product shots,
 *  2. replicating famous shots, and
 *  3. animation.
 *
QWidget *
MultiShot::getOnionSkinningPage()
{
    Controls: image to onion skin and a way to position
    and size it, opacity of image.
}
 */

const char *MultiShot::textHalves[] = {
    "0 stops",
    "1/2 stop",
    "1 stop",
    "<html><head></head><body style=\" font-family:'Calibri'; font-size:12pt; font-weight:400; font-style:normal;\"><p>1<span style=\" vertical-align:super;\">1/2</span> stops</p></body></html>",
    "2 stops",
    NULL
};

const char *MultiShot::textThirds[] = {
    "0 stops",
    "1/3 stop",
    "2/3 stop",
    "1 stop",
    "<html><head></head><body style=\" font-family:'Calibri'; font-size:12pt; font-weight:400; font-style:normal;\"><p>1<span style=\" vertical-align:super;\">1/3</span> stops</p></body></html>",
    "<html><head></head><body style=\" font-family:'Calibri'; font-size:12pt; font-weight:400; font-style:normal;\"><p>1<span style=\" vertical-align:super;\">2/3</span> stops</p></body></html>",
    "2 stops",
    NULL
};

void
MultiShot::setIncrementPages()
{
    int i = 0;
    if( step == 0 ) {
	for( i = 0; textThirds[i]; i++ ) {
	    eIncrementPage[i]->setText( textThirds[i] );
	}
    }
    else
    if( step == 1 ) {
	for( i = 0; textHalves[i]; i++ ) {
	    eIncrementPage[i]->setText( textHalves[i] );
	}
    }
    eIncrement->setMaximum( i-1 );
}

void
MultiShot::updateIntervalTiming()
{
    timingDisplay->updateTiming(
			camera->getTv(),
			iBulb->time(),
			iDelay->time(),
			iInterval->time(),
			iFrames->value() );
}

MultiShot::TimingDisplay::TimingDisplay( QWidget *parent )
    : QLabel( parent )
{
    QPixmap pixmap( ":/Resources/Misc/timing.png" );
    setPixmap( pixmap );
    setFrameStyle( QFrame::Box | QFrame::Plain );
    setLineWidth( 1 );

    setFixedSize( pixmap.width(), pixmap.height() );
}

void
MultiShot::TimingDisplay::updateTiming(
			int speed,
			QTime &bulbTime,
			QTime &delayTime,
			QTime &intervalTime,
			int frames )
{
    if( speed == 0x0c ) { // bulb
	shutter = bulbTime.toString( "m''ss'\" (bulb)'" );
    }
    else {
	shutter = Map::toString_Tv( speed );
    }
    delay = delayTime.toString( "m''ss'\"'" );
    interval = intervalTime.toString( "m''ss'\"'" );

    int s, m, h;
    if( (speed == 0x0c) && (bulbTime > intervalTime) ) {
	s = 60 * (delayTime.minute() + frames * bulbTime.minute()) +
		  delayTime.second() + frames * bulbTime.second();
    }
    else {
	s = 60 * (delayTime.minute() + frames * intervalTime.minute()) +
		  delayTime.second() + frames * intervalTime.second();
    }
    double d = double(s) / double(3600 * 24);
    if( d > 1 ) {
	estTotalTime = tr( "Approximate total time (days): %1" )
			.arg(d, 0, 'f', 1);
    }
    else {
	h = s / 3600;
	s = s - (h * 3600);
	m = s / 60;
	s = s - (m * 60);
	QTime time( h, m, s );
	if( h > 0 ) {
	    estTotalTime = tr( "Approximate total time (hh:mm): " ) +
			time.toString( "h':'mm" );
	}
	else {
	    estTotalTime = tr( "Approximate total time (mm:ss): " ) +
			time.toString( "m':'ss" );
	}
    }
    update();
}

void
MultiShot::TimingDisplay::paintEvent( QPaintEvent *event )
{
    QLabel::paintEvent( event );

    QPainter painter( this );
    painter.setFont( QApplication::font() );
    painter.drawText(  28, 64, tr("delay") );
    painter.drawText(  28, 81, delay );
    painter.drawText( 120, 64, tr("interval") );
    painter.drawText( 120, 81, interval );
    painter.drawText( 288, 64, tr("interval") );
    painter.drawText( 288, 81, interval );
    painter.drawText( 120, 21, tr("shutter") );
    painter.drawText( 120, 38, shutter );
    painter.drawText( 288, 21, tr("shutter") );
    painter.drawText( 288, 38, shutter );
    painter.drawText(  28,108, estTotalTime );
}

void
MultiShot::highISOtestButtonClicked()
{
    emit highISOtest( 60 * iBulb->time().minute() +
			   iBulb->time().second() );
}

int
MultiShot::msec( const QTime &time )
{
    return(
	60000 * time.minute() +
	 1000 * time.second() +
	        time.msec()
    );
}

/*
 *  Set list of shutter speeds (Canon internal values) for
 *  exposure bracketing in Manual shooting mode.
 */
void
MultiShot::setShutterSpeeds()
{
    int ind, m;
    int *v;
    /*
     *  Retrieve the current list of valid shutter speeds
     *  and find the index of the current shutter speed.
     */
    camera->getTv_List( &m, &v );
    int shutterSpeed = camera->getTv();
    for( ind = 0; ind < m; ind++ ) {
	if( v[ind] == shutterSpeed ) break;
    }
    /*
     *  Determine the step size.
     */
    int step = eIncrement->value();
    /*
     *  Set the shots.
     */
    shotSeq->bracket[0] = shutterSpeed;
    int n = eFrames->value();
    for( int i = 1; i <= n; i++ ) {
	// fill in the brighter shots, in order of increasing brightness
	int brighter = ind - i*step;
	if( brighter < 0 ) brighter = 0;
	shotSeq->bracket[i] = v[brighter];
	// fill in the darker shots, in order of increasing darkness
	int darker = ind + i*step;
	if( darker >= m ) darker = m-1;
	shotSeq->bracket[n+i] = v[darker];
    }
}

/*
 *  Set list of compensations (Canon internal values) for
 *  exposure bracketing in Av shooting mode.
 */
void
MultiShot::setCompensations()
{
    int ind, m;
    int *v;
    /*
     *  Retrieve the current list of valid shutter speeds and
     *  find the index of the current exposure compensation.
     */
    camera->getExposureComp_List( &m, &v );
    int compensation = camera->getExposureComp();
    for( ind = 0; ind < m; ind++ ) {
	if( v[ind] == compensation ) break;
    }
    /*
     *  Determine the step size.
     */
    int step = eIncrement->value();
    /*
     *  Set the shots.
     */
    shotSeq->bracket[0] = compensation;
    int n = eFrames->value();
    for( int i = 1; i <= n; i++ ) {
	// fill in the brighter shots, in order of increasing brightness
	int brighter = ind + i*step;
	if( brighter >= m ) brighter = m-1;
	shotSeq->bracket[i] = v[brighter];
	// fill in the darker shots, in order of increasing darkness
	int darker = ind - i*step;
	if( darker < 0 ) darker = 0;
	shotSeq->bracket[n+i] = v[darker];
    }
}

void
MultiShot::setShotSeq()
{
    shotSeq->shot = 0;

    if( tabWidget->currentIndex() == 0 ) {
	/*
	 *  Exposure bracketing.
	 */
	int mode = camera->getShootingMode();
	if( mode == kEdsAEMode_Manual ) {
	    shotSeq->type = ShotSeq::ExposureBracketingManual;
	    setShutterSpeeds();
	}
	else {
	    shotSeq->type = ShotSeq::ExposureBracketingAv;
	    setCompensations();
	}
	shotSeq->frames = 2 * eFrames->value() + 1;
	shotSeq->delay = 0;
	shotSeq->bulbMode = false;
    }
    else
    if( tabWidget->currentIndex() == 1 ) {
	/*
	 *  Focus bracketing.
	 */
	shotSeq->type = ShotSeq::FocusBracketing;
	shotSeq->frames = fFrames->value();
	shotSeq->delay = 0;
	shotSeq->bulbMode = false;
	int index = fStep->currentIndex();
	int step = fStep->itemData( index ).toInt();
	shotSeq->bracket[0] = 0;
	for( int i = 1; i < shotSeq->frames; i++ ) {
	    shotSeq->bracket[i] = step;
	}
    }
    else
    if( tabWidget->currentIndex() == 2 ) {
	/*
	 *  Interval.
	 */
	shotSeq->type = ShotSeq::Interval;
	shotSeq->frames = iFrames->value();
	shotSeq->delay = msec( iDelay->time() );
	shotSeq->interval = msec( iInterval->time() );
	if( camera->shutterInBulbMode() ) {
	    shotSeq->bulbMode = true;
	    shotSeq->shutter = msec( iBulb->time() );
	    if( shotSeq->interval < shotSeq->shutter ) {
		shotSeq->interval = shotSeq->shutter;
	    }
	}
	else {
	    shotSeq->bulbMode = false;
	    shotSeq->shutter = Map::toShutterSpeed( camera->getTv() );
	}
	if( shotSeq->interval < shotSeq->shutter ) {
	    shotSeq->interval = 0;	// shoot as fast as possible
	}
    } // interval
    else {
	/*
	 *  Stitch.
	 */
	shotSeq->type = ShotSeq::Stitch;
	shotSeq->frames = columns->value() * rows->value();
	shotSeq->delay = 0;
	shotSeq->bulbMode = false;
    } // stitch
}

bool
MultiShot::sequenceInProgress()
{
    return( inProgress );
}

void
MultiShot::runButtonClicked()
{
    setShotSeq();
    startSeq();
}

void
MultiShot::stopButtonClicked()
{
    appendLog( "sequence cancelled" );
    emit cancelSequence();
    //finishSeq();
}

void
MultiShot::startSeq()
{
    // only show current compensation
    eCompensation->updateExposures( 0 );
    tabWidget->setEnabled( false );
    runButton->setEnabled( false );
    stopButton->setEnabled( true );
    inProgress = true;
    updateLog( true );
    emit startSequence( shotSeq );
}

void
MultiShot::finishSeq()
{
    updateLog( false );
    inProgress = false;
    stopButton->setEnabled( false );
    runButton->setEnabled( true );
    tabWidget->setEnabled( true );
    // show all compensations once again
    eCompensation->updateExposures( eFrames->value() );
}

void
MultiShot::updateLog( bool start )
{
    QString prefix;
    QString suffix;
    if( start ) {
	prefix = "start ";
	suffix = " (" + QString::number( shotSeq->frames );
	if( shotSeq->frames == 1 ) suffix += " image)";
	else		       suffix += " images)";
    }
    else {
	prefix = "end ";
	suffix = "";
    }
    switch( shotSeq->type ) {
	case ShotSeq::ExposureBracketingManual:
	case ShotSeq::ExposureBracketingAv:
	    appendLog( prefix + "exposure bracketing" + suffix );
	    break;
	case ShotSeq::FocusBracketing:
	    appendLog( prefix + "focus bracketing" + suffix );
	    break;
	case ShotSeq::Interval:
	    appendLog( prefix + "interval shooting" + suffix );
	    break;
	case ShotSeq::Stitch:
	    appendLog( prefix + "panorama shooting" + suffix );
	    break;
    }
}

void
MultiShot::setImage( QImage image, int rotationFlag, int shot )
{
    imageTable->setImage( image, rotationFlag, shot-1 );
}

Overlay *
MultiShot::getOverlay( int shot )
{
    Overlay *overlay = imageTable->getOverlay( shot );
    if( overlay != NULL ) {
	overlay->overlap = overlap->value();
	overlay->opacity = opacity->value();
    }

    return( overlay );
}

/*
 *  Run through a checklist for a successful multi-shot.
 */
void
MultiShot::checklistButtonClicked()
{
    setShotSeq();

    if( !checklist ) {
	checklist = new Checklist( this, Qt::Window |
					 Qt::CustomizeWindowHint |
					 Qt::WindowCloseButtonHint );
    }
    checklist->setChecklist( camera, shotSeq );
    checklist->show();
    checklist->raise();
    checklist->activateWindow();
}

void
MultiShot::updateChecklist()
{
    if( checklist && checklist->isVisible() ) {
	setShotSeq();
	checklist->setChecklist( camera, shotSeq );
    }
}

void
MultiShot::updateProperty( int property, int parameter )
{
    updateChecklist();

    switch( property ) {
	case kEdsPropID_Tv:
	    iBulb->setEnabled( camera->shutterInBulbMode() );
	    iTest->setEnabled( camera->shutterInBulbMode() );
	    updateIntervalTiming();
	    break;
	case kEdsPropID_ExposureCompensation:
	    eCompensation->updateValue( camera );
	    break;
	case kEdsPropID_CFn:
	    switch( parameter ) {
		case CFn_ExposureLevelIncrements:
		    step = camera->getExposureLevelIncrements();
		    setIncrementPages();
		    break;
	    } // switch( parameter )
	    break;
    } // switch( property )
}

void
MultiShot::updatePropertyList( int property )
{
    switch( property ) {
	case kEdsPropID_ExposureCompensation:
	    eCompensation->updateList( camera );
	    break;
    }
}

void
MultiShot::readSettings()
{
    QSettings settings;

    settings.beginGroup( "MultiShot" );
    resize( settings.value("size", QSize(380, 284)).toSize() );
    move( settings.value("pos", QPoint(100, 100)).toPoint() );
    settings.endGroup();
}

void
MultiShot::writeSettings()
{
    QSettings settings;

    settings.beginGroup( "MultiShot" );
    settings.setValue( "size", size() );
    settings.setValue( "pos", pos() );
    settings.endGroup();
}

/*
 *  Reimplementation of closeEvent to save settings.
 */
void
MultiShot::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}

