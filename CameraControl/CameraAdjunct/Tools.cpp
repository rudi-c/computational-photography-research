/*
 *  Tools: depth of field and field of view calculators.
 */

#include <QtGui>
#include <math.h>
#include "Tools.h"
#include "Camera.h"
#include "LiveImageData.h"
#include "Widgets/DepthOfField.h"


const float Tools::Full  = 0.030;
const float Tools::APS_H = 0.023;
const float Tools::APS_C = 0.019;

/*
 *  Canon EOS cameras, circle of confusion and sensor size (width, height).
 */
const Tools::SensorDimensionType Tools::SensorMap[] = {
    { "1D X",		Full,  36.0, 24.0 },
    { "1Ds Mark III",	Full,  36.0, 24.0 },
    { "1Ds Mark II",	Full,  36.0, 24.0 },
    { "1Ds",		Full,  36.0, 24.0 },
    { "5D Mark III",	Full,  36.0, 24.0 },
    { "5D Mark II",	Full,  36.0, 24.0 },
    { "5D",		Full,  35.8, 23.9 },
    { "1D Mark IV",	APS_H, 27.9, 18.6 },
    { "1D Mark III",	APS_H, 28.1, 18.7 },
    { "1D Mark II N",	APS_H, 28.7, 19.1 },
    { "1D Mark II",	APS_H, 28.7, 19.1 },
    { "1D",		APS_H, 28.7, 19.1 },
    { "7D",		APS_C, 22.3, 14.9 },
    { "60D",		APS_C, 22.3, 14.9 },
    { "50D",		APS_C, 22.3, 14.9 },
    { "40D",		APS_C, 22.2, 14.8 },
    { "30D",		APS_C, 22.5, 15.0 },
    { "20D",		APS_C, 22.5, 15.0 },
    { "10D",		APS_C, 22.7, 15.1 },
    { "600D/Rebel T3i", APS_C, 22.3, 14.9 },	// Kiss X5
    { "550D/Rebel T2i", APS_C, 22.3, 14.9 },	// Kiss X4
    { "500D/Rebel T1i", APS_C, 22.3, 14.9 },	// Kiss X3
    { "450D/Rebel XSi", APS_C, 22.2, 14.8 },	// Kiss X2
    { "400D/Rebel XTi", APS_C, 22.2, 14.8 },	// Kiss X
    { "350D/Rebel XT",  APS_C, 22.2, 14.8 },	// Kiss N
    { "300D/Rebel",	APS_C, 22.7, 15.1 },	// Kiss
    { "1100D/Rebel T3", APS_C, 22.3, 14.7 },	// Kiss X50
    { "1000D/Rebel XS", APS_C, 22.2, 14.8 },	// Kiss F
    { 0, 0 }
};

/*
 *  Possible aperture values and their associated exponents.
 *  These are determined using the following code:
 *
 *      for( j = 0; j <= 40; j++ ) {
 *          exponent = ((double)j)/2.0;		//  Full stop scale.
 *          aperture = pow( 2.0, exponent );
 *          printf( "%4.1f (%12.8f)\n", aperture, exponent);
 *          exponent = (((double)j)/2.0)/2.0;	//  1/2 stop scale.
 *          aperture = pow( 2.0, exponent );
 *          printf( "%4.1f (%12.8f)\n", aperture, exponent);
 *          exponent = (((double)j)/3.0)/2.0;	//  1/3 stop scale.
 *          aperture = pow( 2.0, exponent );
 *          printf( "%4.1f (%12.8f)\n", aperture, exponent);
 *      }
 */
const Tools::PairType Tools::ApertureMap[] = {
    { "1.2", 0.25000000 }, { "1.4", 0.50000000 }, { "1.6", 0.66666667 },
    { "1.8", 0.83333333 }, { "2",   1.00000000 }, { "2.2", 1.16666667 },
    { "2.5", 1.33333333 }, { "2.8", 1.50000000 }, { "3.2", 1.66666667 },
    { "3.5", 1.75000000 }, { "4",   2.00000000 }, { "4.5", 2.16666667 },
    { "5",   2.33333333 }, { "5.6", 2.50000000 }, { "6.3", 2.66666667 },
    { "6.7", 2.75000000 }, { "7.1", 2.83333333 }, { "8",   3.00000000 },
    { "9",   3.16666667 }, { "9.5", 3.25000000 }, { "10",  3.33333333 },
    { "11",  3.50000000 }, { "12",  3.66666667 }, { "13",  3.75000000 },
    { "14",  3.83333333 }, { "16",  4.00000000 }, { "18",  4.16666667 },
    { "19",  4.25000000 }, { "20",  4.33333333 }, { "22",  4.50000000 },
    { "25",  4.66666667 }, { "27",  4.75000000 }, { "29",  4.83333333 },
    { "32",  5.00000000 }, { 0, 0 }
};

/*
 *  Canon lenses.
 *
 *  EF-S lenses
 *
 *    10-22   15-85   17-55   17-85   18-55   18-135   18-200  55-250
 *   
 *    60
 *   
 *  EF lenses
 *   
 *     8-15   16-35   17-35   17-40   20-35   22-55   24-105  24-70
 *    24-85  28-105  28-135  28-200  28-300   28-70   28-80   28-90
 *   35-105  35-135  35-350  35-70    35-80   38-76   50-200  55-200
 *   70-200  70-210  70-300  75-300  80-200  90-300  100-200 100-300
 *  100-400 200-400
 *   
 *    14  15  20  24  28  35  50  65 85 100
 *   135 180 200 300 400 500 600 800
 */

/*
 *  Constructor for tools.
 */
Tools::Tools( Camera *camera, QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    this->camera = camera;

    circleOfConfusion = 0.0;
    sensorWidth = 0.0;
    sensorHeight = 0.0;
    aperture = 0.0;
    focalLength = 50.0;
    subjectDistance = 5.0;
    landscapeOrientation = true;
    rows = 1;
    cols = 1;
    coverage = 0.80;

    setWindowTitle( tr("Tools[*]") );
    setAttribute( Qt::WA_QuitOnClose, false ); // close window if app closes

    initialize();
    setDefaults();

    readSettings();
}

/*
 *  Destructor for tools.
 */
Tools::~Tools()
{
    //QObject::dumpObjectInfo();
    //QObject::dumpObjectTree();
}

/*
 *  Initialize dialog window.
 */
void
Tools::initialize()
{
    tabWidget = new QTabWidget( this );
    tabWidget->addTab( getDepthOfFieldPage(), tr("Depth of field") );
    tabWidget->addTab( getFieldOfViewPage(), tr("Field of view") );

    QPushButton *closeButton = new QPushButton( tr("Close") );
    QObject::connect(
	closeButton, SIGNAL(clicked()),
	this, SLOT(close()) );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( tabWidget );
    layout->addWidget( closeButton, /* stretch */0, Qt::AlignRight );
    setLayout( layout );
}

/*
 *  Depth of field page.
 */
QWidget *
Tools::getDepthOfFieldPage()
{
    /*
     *  Settings.
     */
    QLabel *cameraLabel = new QLabel( tr("Camera:") );
    cameraLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    dofCameraComboBox = new QComboBox();
    dofCameraComboBox->setMaxVisibleItems( 16 );
    for( int i = 0; SensorMap[i].description != 0; i++ ) {
	dofCameraComboBox->addItem(
		QString("Canon EOS ") +
		QString(SensorMap[i].description) );
    }
    QObject::connect(
	dofCameraComboBox, SIGNAL(currentIndexChanged(int)),
	this, SLOT(setCamera(int)) );

    QLabel *apertureLabel = new QLabel( tr("Aperture:") );
    apertureLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    apertureStackedWidget = new QStackedWidget();
    apertureStackedWidget->setFont( QFont("Calibri", 12) );
    int i;
    for( i = 0; ApertureMap[i].description != 0; i++ ) {
	QLabel *label = new QLabel( apertureStackedWidget );
	QString text;
	text.sprintf( "F%s", ApertureMap[i].description );
	label->setText( text );
	label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
	apertureStackedWidget->insertWidget( i, label );
    }

    apertureSlider = new QSlider();
    apertureSlider->setCursor( QCursor(Qt::SizeHorCursor) );
    apertureSlider->setMinimum( 0 );
    apertureSlider->setMaximum( apertureStackedWidget->count() - 1 );
    apertureSlider->setPageStep( 1 );
    apertureSlider->setOrientation( Qt::Horizontal );
    apertureSlider->setTickPosition( QSlider::TicksBelow );
    apertureSlider->setTickInterval( 1 );
    QObject::connect(
	apertureSlider, SIGNAL(valueChanged(int)),
	apertureStackedWidget, SLOT(setCurrentIndex(int)));
    QObject::connect(
	apertureSlider, SIGNAL(valueChanged(int)),
	this, SLOT(setAperture(int)));

    QLabel *focalLengthLabel = new QLabel( tr("Focal length:") );
    focalLengthLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    dofFocalLengthLineEdit = new QLineEdit();
    dofFocalLengthLineEdit->setValidator(
	new QRegExpValidator(
	    QRegExp("^[1-9]\\d{0,2}$|^[1-9]\\d{0,2}\\.\\d{0,2}$"),
	    dofFocalLengthLineEdit ) );
    QObject::connect(
	dofFocalLengthLineEdit, SIGNAL(textChanged(const QString &)),
	this, SLOT(setFocalLength(const QString &)) );
    QLabel *focalLengthUnitsLabel = new QLabel();
    focalLengthUnitsLabel->setText( "mm" );
    focalLengthUnitsLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    QLabel *subjectDistanceLabel = new QLabel( tr( "Subject distance:" ) );
    subjectDistanceLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    subjectDistanceLineEdit = new QLineEdit();
    subjectDistanceLineEdit->setValidator(
	new QRegExpValidator(
	    QRegExp("^[1-9]\\d{0,3}$|^[1-9]\\d{0,3}\\.\\d{0,2}$|^0\\.[1-9]\\d{0,1}"),
	    subjectDistanceLineEdit ) );
    QObject::connect(
	subjectDistanceLineEdit, SIGNAL(textChanged(const QString &)),
	this, SLOT(setSubjectDistance(const QString &)) );
    unitsComboBox = new QComboBox();
    unitsComboBox->addItem( "feet" , DepthOfField::UNITS_Feet );
    unitsComboBox->addItem( "metres", DepthOfField::UNITS_Metres );
    QObject::connect(
	unitsComboBox, SIGNAL(currentIndexChanged(int)),
	this, SLOT(setUnits(int)) );

    QLabel *subjectDistanceNote = new QLabel();
    QFont font;
    font.setItalic( true );
    subjectDistanceNote->setFont( font );
    subjectDistanceNote->setText(
	tr("Subject distance is not needed for hyperfocal distance calculation.") );
    subjectDistanceNote->setAlignment( Qt::AlignRight );

    /*
     *  Results.
     */
    QLabel *subjectCamera = new QLabel();
    subjectCamera->setPixmap( QPixmap(":/Resources/Misc/t2i.png") );
    subjectDisplay = new DepthOfField( tr("Subject"), this );

    QLabel *hyperfocalCamera = new QLabel();
    hyperfocalCamera->setPixmap( QPixmap(":/Resources/Misc/t2i.png") );
    hyperfocalDisplay = new DepthOfField( tr("Hyperfocal"), this );

    /*
     *  Settings layout.
     */
    QGridLayout *settingsLayout = new QGridLayout();
    settingsLayout->addWidget( cameraLabel,	        0, 0 );
    settingsLayout->addWidget( dofCameraComboBox,       0, 1 );
    settingsLayout->addWidget( apertureLabel,	        1, 0 );
    settingsLayout->addWidget( apertureSlider,	        1, 1 );
    settingsLayout->addWidget( apertureStackedWidget,   1, 2 );
    settingsLayout->addWidget( focalLengthLabel,        2, 0 );
    settingsLayout->addWidget( dofFocalLengthLineEdit,  2, 1 );
    settingsLayout->addWidget( focalLengthUnitsLabel,   2, 2 );
    settingsLayout->addWidget( subjectDistanceLabel,    3, 0 );
    settingsLayout->addWidget( subjectDistanceLineEdit, 3, 1 );
    settingsLayout->addWidget( unitsComboBox,	        3, 2 );
    settingsLayout->addWidget( subjectDistanceNote,     4, 0, 1, 3 );
    settingsLayout->setColumnMinimumWidth( 0, 100 );
    settingsLayout->setRowStretch( /*row*/5, /*stretch*/1 );
    settingsLayout->setColumnStretch( /*column*/3, /*stretch*/1 );

    QGroupBox *settingsGroupBox = new QGroupBox( tr("Settings") );
    settingsGroupBox->setLayout( settingsLayout );

    /*
     *  Subject distance layout.
     */
    QHBoxLayout *subjectLayout = new QHBoxLayout();
    subjectLayout->addWidget( subjectCamera );
    subjectLayout->addWidget( subjectDisplay );

    QGroupBox *subjectGroupBox =
	new QGroupBox( tr("Focus at subject distance") );
    subjectGroupBox->setLayout( subjectLayout );

    /*
     *  Hyperfocal distance layout.
     */
    QHBoxLayout *hyperfocalLayout = new QHBoxLayout();
    hyperfocalLayout->addWidget( hyperfocalCamera );
    hyperfocalLayout->addWidget( hyperfocalDisplay );

    QGroupBox *hyperfocalGroupBox =
	new QGroupBox( tr("Focus at hyperfocal distance") );
    hyperfocalGroupBox->setLayout( hyperfocalLayout );

    /*
     *  Final layout.
     */
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( settingsGroupBox );
    layout->addWidget( subjectGroupBox );
    layout->addWidget( hyperfocalGroupBox );

    QWidget *widget = new QWidget( this );
    widget->setLayout( layout );

    return( widget );
}

/*
 *  Field of view page.
 */
QWidget *
Tools::getFieldOfViewPage()
{
    /*
     *  Settings.
     */
    QLabel *cameraLabel = new QLabel( tr("Camera:") );
    cameraLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    fovCameraComboBox = new QComboBox();
    fovCameraComboBox->setMaxVisibleItems( 16 );
    for( int i = 0; SensorMap[i].description != 0; i++ ) {
	fovCameraComboBox->addItem(
		QString("Canon EOS ") +
		QString(SensorMap[i].description) );
    }
    QObject::connect(
	fovCameraComboBox, SIGNAL(currentIndexChanged(int)),
	this, SLOT(setCamera(int)) );

    QLabel *orientationLabel = new QLabel( tr("Orientation:") );
    orientationLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    QRadioButton *landscape =
	new QRadioButton( tr("Camera in landscape orientation") );
    QRadioButton *portrait =
	new QRadioButton( tr("Camera in portrait orientation") );
    landscape->setChecked( landscapeOrientation );
    portrait->setChecked( !landscapeOrientation );
    QObject::connect(
	landscape, SIGNAL(toggled(bool)),
	this, SLOT(setOrientation(bool)) );

    QLabel *focalLengthLabel = new QLabel( tr("Focal length:") );
    focalLengthLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    fovFocalLengthLineEdit = new QLineEdit();
    fovFocalLengthLineEdit->setValidator(
	new QRegExpValidator(
	    QRegExp("^[1-9]\\d{0,2}$|^[1-9]\\d{0,2}\\.\\d{0,2}$"),
	    fovFocalLengthLineEdit ) );
    QObject::connect(
	fovFocalLengthLineEdit, SIGNAL(textChanged(const QString &)),
	this, SLOT(setFocalLength(const QString &)) );
    QLabel *focalLengthUnitsLabel = new QLabel();
    focalLengthUnitsLabel->setText( "mm" );
    focalLengthUnitsLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    QLabel *columnsLabel = new QLabel( tr("Horizontal size:") );
    columnsLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    QSpinBox *columnsSpinBox = new QSpinBox();
    columnsSpinBox->setRange( 1, 7 );
    QObject::connect(
	columnsSpinBox, SIGNAL(valueChanged(int)),
	this, SLOT(setCols(int)) );
    colText = new QLabel( tr("image") );

    QLabel *rowsLabel = new QLabel( tr("Vertical size:") );
    rowsLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    QSpinBox *rowsSpinBox = new QSpinBox();
    rowsSpinBox->setRange( 1, 7 );
    QObject::connect(
	rowsSpinBox, SIGNAL(valueChanged(int)),
	this, SLOT(setRows(int)) );
    rowText = new QLabel( tr("image") );

    QLabel *overlapLabel = new QLabel( tr("Overlap:") );
    overlapLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    QSpinBox *overlapSpinBox = new QSpinBox();
    overlapSpinBox->setRange( 10, 30 );
    overlapSpinBox->setSuffix( "%" );
    overlapSpinBox->setValue( 100 - 100*coverage );
    QObject::connect(
	overlapSpinBox, SIGNAL(valueChanged(int)),
	this, SLOT(setCoverage(int)) );

    QLabel *overlapNote = new QLabel();
    QFont font;
    font.setItalic( true );
    overlapNote->setFont( font );
    overlapNote->setText(
	tr("       Overlap is only needed for panoramas of multiple images.") );

    /*
     *  Results.
     */
    QLabel *header1 = new QLabel( tr("One image") );
    QLabel *headerP = new QLabel( tr("Panorama of images") );
    header1->setAlignment( Qt::AlignHCenter );
    headerP->setAlignment( Qt::AlignHCenter );

    QLabel *hAxis1 = new QLabel( tr("Horizontal") );
    QLabel *vAxis1 = new QLabel( tr("Vertical") );
    hAxis1->setAlignment( Qt::AlignHCenter );
    vAxis1->setAlignment( Qt::AlignHCenter );

    QLabel *hAxisP = new QLabel( tr("Horizontal") );
    QLabel *vAxisP = new QLabel( tr("Vertical") );
    hAxisP->setAlignment( Qt::AlignHCenter );
    vAxisP->setAlignment( Qt::AlignHCenter );

    hAngle1 = new QLabel();
    vAngle1 = new QLabel();
    hAngle1->setAlignment( Qt::AlignHCenter );
    vAngle1->setAlignment( Qt::AlignHCenter );

    hAngleP = new QLabel();
    vAngleP = new QLabel();
    hAngleP->setAlignment( Qt::AlignHCenter );
    vAngleP->setAlignment( Qt::AlignHCenter );

    /*
     *  Settings layout.
     */
    QGridLayout *settingsLayout = new QGridLayout();
    settingsLayout->addWidget( cameraLabel,	       0, 0 );
    settingsLayout->addWidget( fovCameraComboBox,      0, 1 );
    settingsLayout->addWidget( orientationLabel,       1, 0 );
    settingsLayout->addWidget( landscape,	       1, 1 );
    settingsLayout->addWidget( portrait,	       2, 1 );
    settingsLayout->addWidget( focalLengthLabel,       3, 0 );
    settingsLayout->addWidget( fovFocalLengthLineEdit, 3, 1 );
    settingsLayout->addWidget( focalLengthUnitsLabel,  3, 2 );
    settingsLayout->addWidget( columnsLabel,	       4, 0 );
    settingsLayout->addWidget( columnsSpinBox,	       4, 1 );
    settingsLayout->addWidget( colText,		       4, 2 );
    settingsLayout->addWidget( rowsLabel,	       5, 0 );
    settingsLayout->addWidget( rowsSpinBox,	       5, 1 );
    settingsLayout->addWidget( rowText,		       5, 2 );
    settingsLayout->addWidget( overlapLabel,	       6, 0 );
    settingsLayout->addWidget( overlapSpinBox,	       6, 1 );
    settingsLayout->addWidget( overlapNote,	       7, 0, 1, 3 );
    settingsLayout->setColumnMinimumWidth( 0, 100 );
    settingsLayout->setColumnStretch( /*column*/3, /*stretch*/1 );

    QGroupBox *settingsGroupBox = new QGroupBox( tr("Settings") );
    settingsGroupBox->setLayout( settingsLayout );

    /*
     *  Results layout.
     */
    QGridLayout *resultsLayout = new QGridLayout();
    resultsLayout->addWidget( header1, 0, 1, 1, 2/*, Qt::AlignHCenter*/ );
    resultsLayout->addWidget( headerP, 0, 4, 1, 2/*, Qt::AlignHCenter*/ );
    resultsLayout->addWidget( hAxis1,  1, 1 );
    resultsLayout->addWidget( vAxis1,  1, 2 );
    resultsLayout->addWidget( hAxisP,  1, 4 );
    resultsLayout->addWidget( vAxisP,  1, 5 );
    resultsLayout->addWidget( hAngle1, 4, 1 );
    resultsLayout->addWidget( vAngle1, 4, 2 );
    resultsLayout->addWidget( hAngleP, 4, 4 );
    resultsLayout->addWidget( vAngleP, 4, 5 );
    resultsLayout->setColumnMinimumWidth( 0, 100 );
    resultsLayout->setColumnMinimumWidth( 1, 70 );
    resultsLayout->setColumnMinimumWidth( 2, 70 );
    resultsLayout->setColumnMinimumWidth( 3, 20 );
    resultsLayout->setColumnMinimumWidth( 4, 70 );
    resultsLayout->setColumnMinimumWidth( 5, 70 );
    resultsLayout->setColumnStretch( /*column*/6, /*stretch*/1 );

    QGroupBox *resultsGroupBox = new QGroupBox( tr("Angular field of view") );
    resultsGroupBox->setLayout( resultsLayout );

    /*
     *  Final layout.
     */
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( settingsGroupBox );
    layout->addWidget( resultsGroupBox );
    layout->addStretch();

    QWidget *widget = new QWidget( this );
    widget->setLayout( layout );

    return( widget );
}

/*
 *  Set default values for depth of field.
 */
void
Tools::setDefaults()
{
    QSettings settings;

    /*
     *  Find the currently connected camera model.
     */
    QString model = camera->getModelName();
    int index = 0;
    if( model.isEmpty() ) {
	index = settings.value( "Tools/model", 0 ).toInt();
    }
    else {
	bool found = false;
	for( int i = 0; (SensorMap[i].description != 0) && !found; i++ ) {
	    QString desc = SensorMap[i].description;
	    QStringList list = desc.split( '/' );
	    for( int j = 0; (j < list.size()) && !found; j++ ) {
		found = (model.compare( "Canon EOS " + list[j],
			    Qt::CaseInsensitive ) == 0);
	    }
	    if( found ) {
		index = i;
	    }
	}
    }
    dofCameraComboBox->setCurrentIndex( index );
    circleOfConfusion = SensorMap[index].circleOfConfusion;
    sensorWidth = SensorMap[index].sensorWidth;
    sensorHeight = SensorMap[index].sensorHeight;

    dofFocalLengthLineEdit->setText( "50" );
    subjectDistanceLineEdit->setText( "5" );

    apertureSlider->setValue( apertureStackedWidget->count() / 2 ); // F8

    units = settings.value( "Tools/units", 1 ).toInt();
    unitsComboBox->setCurrentIndex( units );
}

void
Tools::readSettings()
{
    QSettings settings;

    settings.beginGroup( "Tools" );
    resize( settings.value("size", QSize(300, 320)).toSize() );
    move( settings.value("pos", QPoint(100, 100)).toPoint() );
    settings.endGroup();
}

void
Tools::writeSettings()
{
    QSettings settings;

    settings.beginGroup( "Tools" );
    settings.setValue( "size", size() );
    settings.setValue( "pos", pos() );
    settings.setValue( "model", dofCameraComboBox->currentIndex() );
    settings.setValue( "units", units );
    settings.endGroup();
}

/*
 *  Reimplementation of closeEvent to save settings.
 */
void
Tools::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}


void
Tools::setCamera( int index )
{
    dofCameraComboBox->blockSignals( true );
    dofCameraComboBox->setCurrentIndex( index );
    dofCameraComboBox->blockSignals( false );

    fovCameraComboBox->blockSignals( true );
    fovCameraComboBox->setCurrentIndex( index );
    fovCameraComboBox->blockSignals( false );

    circleOfConfusion = SensorMap[index].circleOfConfusion;
    sensorWidth = SensorMap[index].sensorWidth;
    sensorHeight = SensorMap[index].sensorHeight;
    updateCalculations();
}

void
Tools::setFocalLength( const QString &text )
{
    dofFocalLengthLineEdit->blockSignals( true );
    dofFocalLengthLineEdit->setText( text );
    dofFocalLengthLineEdit->blockSignals( false );

    fovFocalLengthLineEdit->blockSignals( true );
    fovFocalLengthLineEdit->setText( text );
    fovFocalLengthLineEdit->blockSignals( false );

    focalLength = text.toDouble();
    updateCalculations();
}

void
Tools::setAperture( int index )
{
    aperture = pow( 2.0, ApertureMap[index].exponent );
    updateCalculations();
}

void
Tools::setSubjectDistance( const QString &text )
{
    subjectDistance = subjectDistanceLineEdit->text().toDouble();
    updateCalculations();
}

void
Tools::setUnits( int index )
{
    units = unitsComboBox->itemData( index ).toInt();
    updateCalculations();
}

void
Tools::setOrientation( bool checked )
{
    landscapeOrientation = checked;
    updateCalculations();
}

void
Tools::setCols( int value )
{
    cols = value;
    if( cols == 1 ) {
	colText->setText( tr("image") );
    }
    else {
	colText->setText( tr("images") );
    }
    updateCalculations();
}

void
Tools::setRows( int value )
{
    rows = value;
    if( rows == 1 ) {
	rowText->setText( tr("image") );
    }
    else {
	rowText->setText( tr("images") );
    }
    updateCalculations();
}

void
Tools::setCoverage( int value )
{
    coverage = 1.0 - double(value)/100.0;
    updateCalculations();
}

/*
 *  Determine the depth of field and field of view.
 *  Depth of field calculations are in millimetres.
 *  Field of view calculations are in degrees.
 */
void
Tools::updateCalculations()
{
    if( (circleOfConfusion == 0.0) ||
	(aperture == 0.0) ||
	(focalLength == 0.0) ) {
	return;
    }

    /*
     *  Convert subject distance to millimetres.
     */
    double distance;
    if( units == DepthOfField::UNITS_Feet ) {
	// conversion from 1 foot to millimetres: 304.8
	distance = subjectDistance * 304.8;
    }
    else
    if( units == DepthOfField::UNITS_Metres ) {
	// conversion from 1 metre to millimetres: 1000.0
	distance = subjectDistance * 1000.0;
    }

    double hyperfocalDistance =
	    ((focalLength * focalLength) /
	     (aperture * circleOfConfusion)) +
	    focalLength;

    double nearDistance =
	(distance * (hyperfocalDistance - focalLength)) /
	(hyperfocalDistance + distance - 2.0 * focalLength);

    double farDistance;
    if( (hyperfocalDistance - distance) <= 0.0 ) {
	farDistance = -1.0;	// infinity
    }
    else {
	farDistance =
	    (distance * (hyperfocalDistance - focalLength)) /
	    (hyperfocalDistance - distance);
    }

    subjectDisplay->setData(
	distance,
	nearDistance,		// near limit of acceptable sharpness
	farDistance,		// far limit of acceptable sharpness
	units );

    hyperfocalDisplay->setData(
	hyperfocalDistance,
	hyperfocalDistance/2.0, // near limit of acceptable sharpness
	-1.0 /* infinity */,	// far limit of acceptable sharpness
	units );

    double hAngle = (360.0/M_PI) * atan( sensorWidth / (2.0 * focalLength) );
    double vAngle = (360.0/M_PI) * atan( sensorHeight / (2.0 * focalLength) );

    setFOVData( hAngle, vAngle );
}

QString
Tools::angleToText( double angle )
{
    if( angle < 10.0 ) {
	return( QString::number( angle, 'f', 1 ) +
		QString( QChar(0x00B0) ) );	// degrees
	    }
    else {
	return( QString::number( angle, 'f', 0 ) +
		QString( QChar(0x00B0) ) );	// degrees
    }
}

void
Tools::setFOVData( double hAngle, double vAngle )
{
    if( landscapeOrientation ) {
	/*
	 *  Angular field of view: one image, landscape.
	 */
	hAngle1->setText( angleToText( hAngle ) );
	vAngle1->setText( angleToText( vAngle ) );
	/*
	 *  Angular field of view: panorama of images, landscape.
	 */
	hAngleP->setText( angleToText( hAngle * (1.0 + (cols-1)*coverage) ) );
	vAngleP->setText( angleToText( vAngle * (1.0 + (rows-1)*coverage) ) );
    }
    else {
	/*
	 *  Angular field of view: one image, portrait.
	 */
	hAngle1->setText( angleToText( vAngle ) );
	vAngle1->setText( angleToText( hAngle ) );
	/*
	 *  Angular field of view: panorama of images, portrait.
	 */
	hAngleP->setText( angleToText( vAngle * (1.0 + (cols-1)*coverage) ) );
	vAngleP->setText( angleToText( hAngle * (1.0 + (rows-1)*coverage) ) );
    }
}

