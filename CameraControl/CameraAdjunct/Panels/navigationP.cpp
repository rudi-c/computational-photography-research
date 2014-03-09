/*
 *  Panel for navigation and zooming.
 */

#include <QtGui>
#include "EDSDKTypes.h"
#include "Camera.h"
#include "View.h"
#include "LiveImageData.h"
#include "Widgets/Navigation.h"
#include "Widgets/Move.h"
#include "Event.h"
#include "Map.h"


QBoxLayout *
View::navigationPanelLayout()
{
    /*
     *  Navigation image.
     */
    navigation = new Navigation( camera, this );
    QObject::connect(
	navigation, SIGNAL(zoomRectChanged(int,int)),
	this, SIGNAL(zoomRectChanged(int,int)) );

    /*
     *  Camera zoom: 1x, 5x, 10x.
     *
     *  Code handles 10x, but currently commented out as the 10x image
     *  adds less information than the 5x image (at least on the t2i)
     *  as it is just a sub-area of the 5x image.
     */
    QLabel *cameraZoomLabel = new QLabel( tr("Camera zoom:") );
    cameraZoomLabel->setAlignment( Qt::AlignLeft );

    const int max = 48;
    QPushButton *buttonFit = new QPushButton();
    buttonFit->setText( "1:1" );
    buttonFit->setMaximumWidth( max );
    buttonFit->setCheckable( true );
    buttonFit->setChecked( true );
    buttonFit->setStatusTip( tr("Camera zoom 1x") );
    QPushButton *button5x = new QPushButton();
    button5x->setText( "5:1" );
    button5x->setMaximumWidth( max );
    button5x->setCheckable( true );
    button5x->setStatusTip( tr("Camera zoom 5x") );
    //QPushButton *button10x = new QPushButton;
    //button10x->setText( "10:1" );
    //button10x->setMaximumWidth( max );
    //button10x->setCheckable( true );
    //button10x->setStatusTip( tr("Camera zoom 10x") );
    scaleChosen = new QButtonGroup( this );
    //scaleChosen->addButton( button10x, EvfImageData::ZOOM_10x );
    scaleChosen->addButton( button5x, EvfImageData::ZOOM_5x );
    scaleChosen->addButton( buttonFit, EvfImageData::ZOOM_Fit );
    QObject::connect(
	scaleChosen, SIGNAL(buttonClicked(int)),
	this, SLOT(zoomLevelChanged(int)) );

    /*
     *  Move navigation and focus rectangle.
     */
    Move *move = new Move( this );
    QObject::connect(
	move, SIGNAL(moveDirection(int)),
	navigation, SLOT(moveDirection(int)) );

    /*
     *  Image overlay as composition aid.
     */
    QLabel *overlayLabel = new QLabel( tr("Overlay:") );
    overlayComboBox = new QComboBox();
    overlayComboBox->setStatusTip( tr("Live view image overlay as an aid to composition") );
    overlayComboBox->setFixedWidth( 86 );
    overlayComboBox->setMaxVisibleItems( 16 );
    for( int i = EvfImageData::OverlayNone;
	     i <= EvfImageData::OverlayLast; i++ ) {
	overlayComboBox->addItem( menuText[i], i );
    }
    overlayComboBox->setCurrentIndex( 0 );

    /*
     *  Image zoom.
     */
    QLabel *imageZoomLabel = new QLabel( tr("Image zoom:") );
    imageZoomLabel->setAlignment( Qt::AlignLeft );

    QSize iconSize(24, 24);
    QToolButton *zoomInIcon = new QToolButton();
    zoomInIcon->setAutoRaise( true );
    zoomInIcon->setAutoRepeat( true );
    zoomInIcon->setAutoRepeatInterval( 250 );
    zoomInIcon->setAutoRepeatDelay( 250 );
    zoomInIcon->setIcon( QIcon(":/Resources/Misc/zoom-in.png") );
    zoomInIcon->setIconSize( iconSize );
    zoomInIcon->setStatusTip( tr("Image zoom in") );
    QObject::connect(
	zoomInIcon, SIGNAL(clicked()),
	this, SLOT(zoomIn()));

    QToolButton *zoomOutIcon = new QToolButton();
    zoomOutIcon->setAutoRaise( true );
    zoomOutIcon->setAutoRepeat( true );
    zoomOutIcon->setAutoRepeatInterval( 250 );
    zoomOutIcon->setAutoRepeatDelay( 250 );
    zoomOutIcon->setIcon( QIcon(":/Resources/Misc/zoom-out.png") );
    zoomOutIcon->setIconSize( iconSize );
    zoomOutIcon->setStatusTip( tr("Image zoom out") );
    QObject::connect(
	zoomOutIcon, SIGNAL(clicked()),
	this, SLOT(zoomOut()));

    zoomSlider = new QSlider( Qt::Horizontal );
    zoomSlider->setFixedWidth( 86 );
    zoomSlider->setFixedHeight( 24 );
    zoomSlider->setTickPosition(QSlider::TicksBelow);
    zoomSlider->setMinimum( 0 );
    zoomSlider->setMaximum( 14 );	// MAX_SCALING_FACTOR - 1
    zoomSlider->setValue( scalingFactor );
    zoomSlider->setSingleStep( 1 );
    zoomSlider->setPageStep( 1 );
    zoomSlider->setStatusTip( tr("Image zoom") );
    QObject::connect(
	zoomSlider, SIGNAL(valueChanged(int)),
	this, SLOT(setZoomFactor(int)));

    QHBoxLayout *zoomSliderLayout = new QHBoxLayout();
    zoomSliderLayout->addWidget(zoomOutIcon);
    zoomSliderLayout->addSpacing( -4 );
    zoomSliderLayout->addWidget(zoomSlider);
    zoomSliderLayout->addSpacing( -4 );
    zoomSliderLayout->addWidget(zoomInIcon);
    zoomSliderLayout->setContentsMargins( 0, 0, 0, 0 );
    QWidget *imageZoom = new QWidget();
    imageZoom->setLayout( zoomSliderLayout );

    /*
     *  Image rotation.
     */
    QLabel *rotationLabel = new QLabel( tr("Rotation:") );
    rotationLabel->setAlignment( Qt::AlignLeft );
    rotationComboBox = new QComboBox();
    rotationComboBox->setStatusTip( tr("Live view image rotation") );
    rotationComboBox->setFixedWidth( 86 );
    for( int i = EvfImageData::RotationNone;
	     i <= EvfImageData::RotationLast; i++ ) {
	rotationComboBox->addItem( menuText[i], i );
    }
    rotationComboBox->setCurrentIndex( 0 );
    camera->setRotationSetting( EvfImageData::RotationNone );
    QObject::connect( rotationComboBox, SIGNAL(activated(int)),
		this, SLOT(setRotation(int)));

    /*
     *  Navigation panel layout.
     */
    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget( cameraZoomLabel, 0, 0, 1, 3, Qt::AlignBottom );
    controlsLayout->addWidget( buttonFit, 1, 0 );
    controlsLayout->addWidget( button5x, 1, 1 );
    controlsLayout->addWidget( move, 0, 2, 2, 1 );
    controlsLayout->addWidget( overlayLabel, 0, 3, Qt:: AlignBottom );
    controlsLayout->addWidget( overlayComboBox, 1, 3 );
    controlsLayout->addWidget( imageZoomLabel, 2, 0, 1, 3 );
    controlsLayout->addWidget( imageZoom, 3, 0, 1, 3 );
    controlsLayout->addWidget( rotationLabel, 2, 3 );
    controlsLayout->addWidget( rotationComboBox, 3, 3, Qt::AlignTop );
    controlsLayout->setColumnStretch( /*column*/3, /*stretch*/1 );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget( navigation );
    layout->setAlignment( navigation, Qt::AlignHCenter );
    layout->addLayout( controlsLayout );
    layout->setContentsMargins( 0, 5, 0, 5 );

    return( layout );
}

/*
 *  Navigation panel: Signal change in zoom level.
 */
void
View::zoomLevelChanged( int zoom )
{
    if( camera->getEvfAFMode() == EvfImageData::AFMODE_Face ) {
	/*
	 *  Zoom unavailable in face recognition focus mode.
	 */
	eventReport( new Event( Event::NotZoomFaceMode ) );
    }
    else {
	emit propertyChanged( kEdsPropID_Evf_Zoom, zoom );
    }
}

/*
 *  Determine current setting for how to rotate live view image.
 */
void
View::setRotation( int index )
{
    int flag = rotationComboBox->itemData( index ).toInt();
    camera->setRotationSetting( flag );
}

void
View::zoomIn()
{
    zoomSlider->setValue( zoomSlider->value() + 1 );
}

void
View::zoomOut()
{
    zoomSlider->setValue( zoomSlider->value() - 1 );
}

/*
 *  Determine current setting for which overlay to display
 *  on live view image.
 */
int
View::getOverlaySetting()
{
    int index = overlayComboBox->currentIndex();
    int flag = overlayComboBox->itemData( index ).toInt();

    return( flag );
}

