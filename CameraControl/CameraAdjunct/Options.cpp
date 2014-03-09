/*
 *  Settings dialog: some less frequently changed image, exposure,
 *  EXIF, and other settings.
 */

#include <QtGui>
#include "Camera.h"
#include "Options.h"
#include "Widgets/Move.h"
#include "CustomFunction.h"
#include "Map.h"


/*
 *  Constructor for options dialog.
 */
Options::Options( Camera *camera, QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    this->camera = camera;

    setWindowTitle( tr("Settings[*]") );
    setAttribute( Qt::WA_QuitOnClose, false ); // close window if app closes
    setMinimumWidth( 375 );

    initialize();

    readSettings();
}

/*
 *  Destructor for options dialog.
 */
Options::~Options()
{
}

void
Options::initialize()
{
    /*
     *  Pages.
     */
    imagePage = getImagePage();
    exposurePage = getExposurePage();
    exifPage = getExifPage();
    interfacePage = getInterfacePage();

    tabWidget = new QTabWidget();
    tabWidget->addTab( imagePage, tr("Image") );
    tabWidget->addTab( exposurePage, tr("Exposure") );
    tabWidget->addTab( exifPage, tr("EXIF") );
    tabWidget->addTab( interfacePage, tr("Interface") );
    tabWidget->setCurrentIndex( 0 );

    /*
     *  Button.
     */
    QPushButton *closeButton = new QPushButton( tr("Close") );
    closeButton->setDefault( true );
    QObject::connect(
	closeButton, SIGNAL(clicked()),
	this, SLOT(close()) );

    /*
     *  Final layout.
     */
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( tabWidget );
    layout->addWidget( closeButton, /* stretch */0, Qt::AlignRight );
    setLayout( layout );
}

/*
 *  Image page: picture style, image quality, long exposure noise reduction,
 *  high ISO noise reduction, highlight tone priority, color space.
 */
QWidget *
Options::getImagePage()
{
    /*
     *  Picture style.
     */
    QLabel *pictureStyleLabel = new QLabel( tr("Picture style:") );
    pictureStyleLabel->setFixedWidth( 140 );
    pictureStyleLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    pictureStyleComboBox = new QComboBox();
    updatePictureStyleList();
    QObject::connect(
	pictureStyleComboBox, SIGNAL(activated(int)),
	this, SLOT(setPictureStyle(int)) );

    /*
     *  Image quality.
     */
    QLabel *imageQualityLabel = new QLabel( tr("Image quality:") );
    imageQualityLabel->setFixedWidth( 140 );
    imageQualityLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    imageQualityComboBox = new QComboBox();
    imageQualityComboBox->setMinimumWidth( 100 );
    updateImageQualityList();
    QObject::connect(
	imageQualityComboBox, SIGNAL(activated(int)),
	this, SLOT(setImageQuality(int)) );

    /*
     *  Color space (for JPEG conversions, including live view).
     */
    QLabel *colorSpaceLabel = new QLabel( tr("Color space:") );
    colorSpaceLabel->setFixedWidth( 140 );
    colorSpaceLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    colorSpaceComboBox = new QComboBox();
    colorSpaceComboBox->addItem( "sRGB" );
    colorSpaceComboBox->addItem( "Adobe RGB" );
    QObject::connect(
	colorSpaceComboBox, SIGNAL(activated(int)),
	this, SLOT(setColorSpace(int)) );

    /*
     *  Noise reduction (LENR).
     */
/*
    QLabel *longExposureNoiseLabel =
	new QLabel( tr("Long exposure noise reduction:") );
    longExposureNoiseLabel->setFixedWidth( 160 );
    longExposureNoiseLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    longExposureNoiseComboBox = new QComboBox();
    longExposureNoiseComboBox->addItem( tr("Off") );
    longExposureNoiseComboBox->addItem( tr("Auto") );
    longExposureNoiseComboBox->addItem( tr("On") );
    QObject::connect(
	longExposureNoiseComboBox, SIGNAL(activated(int)),
	this, SLOT(setLongExposureNoiseReduction(int)) );
*/

    /*
     *  High ISO noise reduction.
     */
/*
    QLabel *isoNoiseLabel = new QLabel( tr("High ISO noise reduction:") );
    isoNoiseLabel->setFixedWidth( 160 );
    isoNoiseLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    isoNoiseComboBox = new QComboBox();
    isoNoiseComboBox->addItem( tr("Standard") );
    isoNoiseComboBox->addItem( tr("Low") );
    isoNoiseComboBox->addItem( tr("Strong") );
    isoNoiseComboBox->addItem( tr("Disabled") );
    QObject::connect(
	isoNoiseComboBox, SIGNAL(activated(int)),
	this, SLOT(setISONoiseReduction(int)) );
*/

    /*
     *  Final layout.
     */
    QGridLayout *imageLayout = new QGridLayout();
    imageLayout->addWidget( pictureStyleLabel, 1, 0 );
    imageLayout->addWidget( pictureStyleComboBox, 1, 1 );
    imageLayout->addWidget( imageQualityLabel, 2, 0 );
    imageLayout->addWidget( imageQualityComboBox, 2, 1 );
    imageLayout->addWidget( colorSpaceLabel, 3, 0 );
    imageLayout->addWidget( colorSpaceComboBox, 3, 1 );
    //imageLayout->addWidget( longExposureNoiseLabel, 4, 0 );
    //imageLayout->addWidget( longExposureNoiseComboBox, 4, 1 );
    //imageLayout->addWidget( isoNoiseLabel, 5, 0 );
    //imageLayout->addWidget( isoNoiseComboBox, 5, 1 );
    imageLayout->setRowStretch( /*row*/4, /*stretch*/1 );
    imageLayout->setColumnStretch( /*column*/2, /*stretch*/1 );

    QWidget *imageWidget = new QWidget( this );
    imageWidget->setLayout( imageLayout );

    return( imageWidget );
}

/*
 *  Exposure page: mirror lockup, increment, white balance shift.
 */
QWidget *
Options::getExposurePage()
{
/*
    QLabel *mirrorLockupLabel = new QLabel( tr("Mirror lockup:") );
    mirrorLockupLabel->setFixedWidth( 140 );
    mirrorLockupLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    mirrorLockupComboBox = new QComboBox();
    mirrorLockupComboBox->addItem( tr("Disabled") );
    mirrorLockupComboBox->addItem( tr("Enabled") );
    QObject::connect(
	mirrorLockupComboBox, SIGNAL(activated(int)),
	this, SLOT(setMirrorLockup(int)) );
*/

    QLabel *incrementLabel = new QLabel( tr("Exposure level increment:") );
    incrementLabel->setFixedWidth( 140 );
    incrementLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    incrementComboBox = new QComboBox();
    incrementComboBox->addItem( "1/3 - Stop" );
    incrementComboBox->addItem( "1/2 - Stop" );
    QObject::connect(
	incrementComboBox, SIGNAL(activated(int)),
	this, SLOT(setExposureLevelIncrement(int)) );

    QLabel *wbShiftLabel = new QLabel( tr("White balance shift:") );
    wbShiftLabel->setFixedWidth( 140 );
    wbShiftLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    wbShift = new WBShift( this );
    wbShift->setPixmap( QPixmap(":/Resources/Misc/color-lined.png") );
    Move *move = new Move();
    QObject::connect(
	move, SIGNAL(moveDirection(int)),
	this, SLOT(scrollWhiteBalanceShift(int)) );
    QPushButton *setButton = new QPushButton( "Set" );
    setButton->setFixedWidth( 65 );
    QObject::connect(
	setButton, SIGNAL(clicked()),
	this, SLOT(setWhiteBalanceShift()) );

    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->addStretch( 1 );
    rowLayout->addWidget( move );
    rowLayout->addWidget( setButton, Qt::AlignHCenter );

    /*
     *  Final layout.
     */
    QGridLayout *exposureLayout = new QGridLayout();
    //exposureLayout->addWidget( mirrorLockupLabel, 0, 0 );
    //exposureLayout->addWidget( mirrorLockupComboBox, 0, 1 );
    exposureLayout->addWidget( incrementLabel, 1, 0 );
    exposureLayout->addWidget( incrementComboBox, 1, 1 );
    exposureLayout->addWidget( wbShiftLabel, 2, 0, Qt::AlignTop );
    exposureLayout->addWidget( wbShift, 2, 1, 1, 2, Qt::AlignTop );
    exposureLayout->addLayout( rowLayout, 3, 1, 1, 2, Qt::AlignRight );
    exposureLayout->setRowStretch( /*row*/4, /*stretch*/1 );
    exposureLayout->setColumnStretch( /*column*/3, /*stretch*/1 );

    QWidget *exposureWidget = new QWidget( this );
    exposureWidget->setLayout( exposureLayout );

    return( exposureWidget );
}

/*
 *  EXIF page: camera information, copyright information, data & time.
 */
QWidget *
Options::getExifPage()
{
    /*
     *  Camera information.
     */
    QLabel *modelNameLabel = new QLabel( tr("Model name:") );
    modelNameLabel->setFixedWidth( 120 );
    modelNameLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    modelNameDisplay = new QLabel();

    QLabel *bodySerialNumberLabel = new QLabel( tr("Body serial number:") );
    bodySerialNumberLabel->setFixedWidth( 120 );
    bodySerialNumberLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    bodySerialNumberDisplay = new QLabel();

    QLabel *firmwareVersionLabel = new QLabel( tr("Firmware version:") );
    firmwareVersionLabel->setFixedWidth( 120 );
    firmwareVersionLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    firmwareVersionDisplay = new QLabel();

    QGridLayout *cameraLayout = new QGridLayout();
    cameraLayout->addWidget( modelNameLabel, 0, 0 );
    cameraLayout->addWidget( modelNameDisplay, 0, 1 );
    cameraLayout->addWidget( bodySerialNumberLabel, 1, 0 );
    cameraLayout->addWidget( bodySerialNumberDisplay, 1, 1 );
    cameraLayout->addWidget( firmwareVersionLabel, 2, 0 );
    cameraLayout->addWidget( firmwareVersionDisplay, 2, 1 );
    cameraLayout->setColumnStretch( /*column*/1, /*stretch*/1 );
    cameraLayout->setRowStretch( /*row*/3, /*stretch*/1 );

    QGroupBox *cameraGroupBox = new QGroupBox( tr("Camera") );
    cameraGroupBox->setLayout( cameraLayout );

    /*
     *  Copyright information.
     */
    QLabel *authorNameLabel = new QLabel( tr("Author's name:") );
    authorNameLabel->setFixedWidth( 120 );
    authorNameLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    authorNameLineEdit = new QLineEdit();
    authorNameLineEdit->setMaxLength( 63 );
    QObject::connect(
	authorNameLineEdit, SIGNAL(editingFinished()),
	this, SLOT(updateAuthorName()) );

    QLabel *copyrightLabel = new QLabel( tr("Copyright statement:") );
    copyrightLabel->setFixedWidth( 120 );
    copyrightLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    copyrightLineEdit = new QLineEdit();
    copyrightLineEdit->setMaxLength( 63 );
    QObject::connect(
	copyrightLineEdit, SIGNAL(editingFinished()),
	this, SLOT(updateCopyright()) );

    QGridLayout *copyrightLayout = new QGridLayout();
    copyrightLayout->addWidget( authorNameLabel, 0, 0 );
    copyrightLayout->addWidget( authorNameLineEdit, 0, 1 );
    copyrightLayout->addWidget( copyrightLabel, 1, 0 );
    copyrightLayout->addWidget( copyrightLineEdit, 1, 1 );
    copyrightLayout->setColumnStretch( /*column*/1, /*stretch*/1 );
    copyrightLayout->setRowStretch( /*row*/2, /*stretch*/1 );

    QGroupBox *copyrightGroupBox = new QGroupBox( tr("Copyright") );
    copyrightGroupBox->setLayout( copyrightLayout );

    /*
     *  Date/time information.
     */
    QLabel *cameraDateLabel = new QLabel( tr("Camera date/time:") );
    cameraDateLabel->setFixedWidth( 120 );
    cameraDateLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    cameraDateEdit = new QDateTimeEdit();
    cameraDateEdit->setDisplayFormat( "MMMM d, yyyy  h:mm ap" );
    QObject::connect(
	cameraDateEdit, SIGNAL(dateTimeChanged(const QDateTime &)),
	this, SLOT(updateDateTime(const QDateTime &)) );
    QPushButton *synchronizeButton = new QPushButton();
    synchronizeButton->setText( tr("Synchronize camera with computer") );
    QObject::connect(
	synchronizeButton, SIGNAL(clicked()),
	this, SLOT(synchronizeDateTime()) );

    QGridLayout *dateLayout = new QGridLayout();
    dateLayout->addWidget( cameraDateLabel, 0, 0 );
    dateLayout->addWidget( cameraDateEdit, 0, 1 );
    dateLayout->addWidget( synchronizeButton, 1, 1 );
    dateLayout->setColumnStretch( /*column*/2, /*stretch*/1 );
    dateLayout->setRowStretch( /*row*/2, /*stretch*/1 );

    QGroupBox *dateGroupBox = new QGroupBox( tr("Date/time") );
    dateGroupBox->setLayout( dateLayout );

    /*
     *  Final layout.
     */
    QVBoxLayout *exifLayout = new QVBoxLayout();
    exifLayout->addWidget( cameraGroupBox );
    exifLayout->addWidget( copyrightGroupBox );
    exifLayout->addWidget( dateGroupBox );

    QWidget *exifWidget = new QWidget( this );
    exifWidget->setLayout( exifLayout );

    return( exifWidget );
}

void
Options::updateExifPage()
{
    modelNameDisplay->setText( camera->getModelName() );
    bodySerialNumberDisplay->setText( camera->getSerialNumber() );
    firmwareVersionDisplay->setText( camera->getFirmware() );

    QString author = camera->getAuthorName();
    if( author.isEmpty() ) {
	author = "%Name%";
    }

    QString copyright = camera->getCopyright();
    if( copyright.isEmpty() ) {
	QDate date = QDate::currentDate();
	QString year = date.toString( "yyyy" );
	copyright = QString( "Copyright (C) %2 %3. All rights reserved.")
		    .arg(year)
		    .arg(author);
    }

    authorNameLineEdit->setText( author );
    copyrightLineEdit->setText( copyright );

    cameraDateEdit->setDateTime( camera->getDateTime() );
}

/*
 *  Interface page: text, background
 */
QWidget *
Options::getInterfacePage()
{
    /*
     *  Text settings.
     */
    QLabel *fontSizeLabel = new QLabel( tr("Font size:") );
    fontSizeLabel->setFixedWidth( 120 );
    fontSizeLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    fontSizeComboBox = new QComboBox();
    fontSizeComboBox->addItem( tr("Small") );		 // 0:  7
    fontSizeComboBox->addItem( tr("Medium (default)") ); // 1:  8
    fontSizeComboBox->addItem( tr("Large") );		 // 2:  9
    fontSizeComboBox->addItem( tr("Largest") );		 // 3: 10
    QSettings settings;
    int fontSize = settings.value( "Interface/fontSize", 1 ).toInt();
    fontSizeComboBox->setCurrentIndex( fontSize );
    if( fontSize != 1 ) {
	setFontSize( fontSize );
    }
    QObject::connect(
	fontSizeComboBox, SIGNAL(currentIndexChanged(int)),
	this, SLOT(setFontSize(int)) );

    QLabel *languageLabel = new QLabel( tr("Language:") );
    languageLabel->setFixedWidth( 120 );
    languageLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    QLabel *languageDisplay = new QLabel( "English" );

    /*
     *  About message.
     */
    QLabel *cameraLabel = new QLabel();
    cameraLabel->setPixmap( QPixmap(":/Resources/Misc/camera.png") );
    cameraLabel->setFixedSize( 72, 72 );
    cameraLabel->setScaledContents( true );
    QLabel *nameLabel = new QLabel();
    nameLabel->setText( "<html><head></head><body style=\" font-family:'Calibri'; font-size:14pt; font-weight:400; font-style:normal;\"><p>Camera Adjunct</p></body></html>" );
    QLabel *versionLabel = new QLabel();
    versionLabel->setText( "<html><head></head><body style=\" font-family:'Calibri'; font-size:8pt; font-weight:400; font-style:normal;\"><p>Version 1.0</p></body></html>" );
    QLabel *aboutLabel = new QLabel();
    aboutLabel->setWordWrap( true );
    aboutLabel->setText(
	QString("© 2012-2013 Peter van Beek. All rights reserved. Many of the icons are from the oxygen icon set: http://www.oxygen-icons.org/") );

    /*
     *  Final layout.
     */
    QGridLayout *textLayout = new QGridLayout();
    textLayout->addWidget( fontSizeLabel, 0, 0 );
    textLayout->addWidget( fontSizeComboBox, 0, 1 );
    textLayout->addWidget( languageLabel, 1, 0 );
    textLayout->addWidget( languageDisplay, 1, 1 );
    textLayout->setColumnStretch( /*column*/2, /*stretch*/1 );

    QGroupBox *textGroupBox = new QGroupBox( tr("Text") );
    textGroupBox->setLayout( textLayout );

    QGridLayout *aboutLayout = new QGridLayout();
    aboutLayout->addWidget( cameraLabel, 0, 0, 3, 1 );
    aboutLayout->addWidget( nameLabel, 0, 1 );
    aboutLayout->addWidget( versionLabel, 1, 1, Qt::AlignTop );
    aboutLayout->addWidget( aboutLabel, 2, 1 );
    aboutLayout->setHorizontalSpacing( 12 );
    aboutLayout->setVerticalSpacing( 1 );

    QGroupBox *aboutGroupBox = new QGroupBox( tr("About") );
    aboutGroupBox->setLayout( aboutLayout );

    QVBoxLayout *interfaceLayout = new QVBoxLayout();
    interfaceLayout->addWidget( textGroupBox );
    interfaceLayout->addWidget( aboutGroupBox );
    interfaceLayout->addStretch();

    QWidget *interfaceWidget = new QWidget( this );
    interfaceWidget->setLayout( interfaceLayout );

    return( interfaceWidget );
}

void
Options::showConnected( bool status )
{
    exposurePage->setEnabled( status );
    imagePage->setEnabled( status );
    exifPage->setEnabled( status );
    interfacePage->setEnabled( true );
}

/*
 *  Reimplementation of showEvent to enable/disable tabs
 *  based on camera connection status.
 */
void
Options::showEvent( QShowEvent *event )
{
    showConnected( camera->handle() != 0 );
    event->accept();
}

/*
 *  Set whether the mirror is locked up before exposure.
 */
void
Options::setMirrorLockup( int index )
{
    emit customPropertyChanged(
	kEdsPropID_CFn,
	index,
	CFn_MirrorLockup );
}

/*
 *  Set the increment size for the exposure level.
 */
void
Options::setExposureLevelIncrement( int index )
{
    emit customPropertyChanged(
	kEdsPropID_CFn,
	index,
	CFn_ExposureLevelIncrements );
}

/*
 *  Set picture style.
 */
void
Options::setPictureStyle( int index )
{
    if( nPS > 0 ) {
	emit propertyChanged( kEdsPropID_PictureStyle, vPS[index] );
    }
}

/*
 *  Set image quality.
 */
void
Options::setImageQuality( int index )
{
    if( nIQ > 0 ) {
	emit propertyChanged( kEdsPropID_ImageQuality, vIQ[index] );
    }
}

/*
 *  Set long exposure noise reduction.
 */
void
Options::setLongExposureNoiseReduction( int index )
{
    emit customPropertyChanged(
	kEdsPropID_CFn,
	index,
	CFn_LongExposureNoiseReduction );
}

/*
 *  Set high ISO speed noise reduction.
 */
void
Options::setISONoiseReduction( int index )
{
    emit customPropertyChanged(
	kEdsPropID_CFn,
	index,
	CFn_HighISOSpeedNoiseReduction );
}

void
Options::setWhiteBalanceShift()
{
    /*
     *  shift[0]: blue-amber shift
     *  blue: -9, ..., 0, ..., +9 :amber
     *  
     *  shift[1]: green-magenta shift
     *  green: -9, ..., 0, ..., +9 :magenta
     */
    int shift[2];
    wbShift->getValues( &shift[0], &shift[1] );

    emit propertyChanged( kEdsPropID_WhiteBalanceShift, shift );
}

void
Options::scrollWhiteBalanceShift( int quadrant )
{
    switch( quadrant ) {
	case Qt::Key_Home:
	    wbShift->setValues( 0, 0 );
	    break;
	case Qt::Key_Left:
	    wbShift->scrollValues( -1, 0 );
	    break;
	case Qt::Key_Down:
	    wbShift->scrollValues( 0, -1 );
	    break;
	case Qt::Key_Up:
	    wbShift->scrollValues( 0, +1 );
	    break;
	case Qt::Key_Right:
	    wbShift->scrollValues( +1, 0 );
	    break;
    }
}

void
Options::updateWBShiftValue()
{
    int *shift = camera->getWhiteBalanceShift();
    wbShift->setValues( shift[0], shift[1] );
}

void
Options::WBShift::scrollValues( int dx, int dy )
{
    wbX += dx;
    wbY += dy;
    if( wbX < -9 ) wbX = -9;
    if( wbX >  9 ) wbX =  9;
    if( wbY < -9 ) wbY = -9;
    if( wbY >  9 ) wbY =  9;
    update();
}

void
Options::WBShift::setValues( int x, int y )
{
    wbX = x;
    wbY = y;
    update();
}

void
Options::WBShift::getValues( int *x, int *y )
{
    *x = wbX;
    *y = wbY;
}

void
Options::WBShift::paintEvent( QPaintEvent *event )
{
    QLabel::paintEvent( event );

    QPainter painter( this );
    painter.fillRect( 7*(wbX+9), 7*(-wbY+9), 6, 6, Qt::white );
}

void
Options::updateAuthorName()
{
    emit propertyChanged( kEdsPropID_Artist, authorNameLineEdit->text() );
}

void
Options::updateCopyright()
{
    emit propertyChanged( kEdsPropID_Copyright, copyrightLineEdit->text() );
}

void
Options::synchronizeDateTime()
{
    updateDateTime( QDateTime::currentDateTime() );
}

void
Options::updateDateTime( const QDateTime &dateTime )
{
    emit propertyChanged( kEdsPropID_DateTime, dateTime );
}

void
Options::setFontSize( int index )
{
    /*
     *  index = 0: font size  7
     *  index = 1: font size  8 (default)
     *  index = 2: font size  9
     *  index = 3: font size 10
     */
    QFont font;
    font.setPointSize( index + 7 );
    QApplication::setFont( font );

    QSettings settings;
    settings.setValue( "Interface/fontSize", index );
}

void
Options::setColorSpace( int index )
{
    emit propertyChanged( kEdsPropID_ColorSpace, index+1 );
}

void
Options::updatePictureStyleValue()
{
    /*
     *  Get the current picture style.
     */
    int value = camera->getPictureStyle();

    /*
     *  Find the corresponding index of user setting of picture style.
     */
    int index;
    for( index = 0; index < nPS; index++ ) {
	if( value == vPS[index] ) break;
    }
    if( index >= nPS ) {
	index = 0;
    }
    pictureStyleComboBox->setCurrentIndex( index );
    update();
}

void
Options::updatePictureStyleList()
{
    /*
     *  Get the list of picture styles.
     */
    camera->getPictureStyle_List( &nPS, &vPS );

    /*
     *  Fill in the comboBox.
     */
    pictureStyleComboBox->clear();
    for( int i = 0; i < nPS; i++ ) {
	pictureStyleComboBox->addItem(
		QString(Map::toString_PictureStyle( vPS[i] )) );
    }

    updatePictureStyleValue();
}

void
Options::updateImageQualityValue()
{
    /*
     *  Get the current picture style.
     */
    int value = camera->getImageQuality();

    /*
     *  Find the corresponding index of user setting of picture style.
     */
    int index;
    for( index = 0; index < nIQ; index++ ) {
	if( value == vIQ[index] ) break;
    }
    if( index >= nIQ ) {
	index = 0;
    }
    imageQualityComboBox->setCurrentIndex( index );
    update();
}

void
Options::updateImageQualityList()
{
    /*
     *  Get the list of picture styles.
     */
    camera->getImageQuality_List( &nIQ, &vIQ );

    /*
     *  Fill in the comboBox.
     */
    imageQualityComboBox->clear();
    for( int i = 0; i < nIQ; i++ ) {
	imageQualityComboBox->addItem(
		QString(Map::toString_ImageQuality( vIQ[i] )) );
    }

    updateImageQualityValue();
}

void
Options::updateProperty( int property, int parameter )
{
    switch( property ) {
	case kEdsPropID_ProductName:
	    showConnected( camera->handle() != 0 );
	    // fall through intentional
	case kEdsPropID_Artist:
	case kEdsPropID_Copyright:
	case kEdsPropID_BodyIDEx:
	case kEdsPropID_FirmwareVersion:
	case kEdsPropID_DateTime:
	    updateExifPage();
	    break;
	case kEdsPropID_ColorSpace:
	    colorSpaceComboBox->setCurrentIndex(
		camera->getColorSpace() - 1 );
	    break;
	case kEdsPropID_PictureStyle:
	    updatePictureStyleValue();
	    break;
	case kEdsPropID_ImageQuality:
	    updateImageQualityValue();
	    break;
	case kEdsPropID_WhiteBalanceShift:
	    updateWBShiftValue();
	    break;
	case kEdsPropID_CFn:
	    switch( parameter ) {
		case CFn_MirrorLockup:
			mirrorLockupComboBox->setCurrentIndex(
			    camera->getMirrorLockup() );
			break;
		case CFn_ExposureLevelIncrements:
			incrementComboBox->setCurrentIndex(
			    camera->getExposureLevelIncrements() );
			break;
		case CFn_LongExposureNoiseReduction:
			longExposureNoiseComboBox->setCurrentIndex(
			    camera->getLongExposureNoiseReduction() );
			break;
		case CFn_HighISOSpeedNoiseReduction:
			isoNoiseComboBox->setCurrentIndex(
			    camera->getHighISOSpeedNoiseReduction() );
			break;
		case CFn_FlashSyncSpeedInAvMode:
		case CFn_AutoFocusAssistBeamFiring:
		case CFn_ShutterAELockButton:
			break;
	    } // switch( parameter )
	    break;
    } // switch( property )
}

void
Options::readSettings()
{
    QSettings settings;

    settings.beginGroup( "Options" );
    resize( settings.value("size", QSize(400, 300)).toSize() );
    move( settings.value("pos", QPoint(100, 100)).toPoint() );
    settings.endGroup();
}

void
Options::writeSettings()
{
    QSettings settings;

    settings.beginGroup( "Options" );
    settings.setValue( "size", size() );
    settings.setValue( "pos", pos() );
    settings.endGroup();
}

/*
 *  Reimplementation of closeEvent to save settings.
 */
void
Options::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}

