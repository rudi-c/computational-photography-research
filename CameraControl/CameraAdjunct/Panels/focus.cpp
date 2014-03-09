/*
 *  Panel for setting focus.
 */

#include <QtGui>
#include "EDSDKTypes.h"
#include "Camera.h"
#include "View.h"
#include "LiveImageData.h"
#include "Widgets/FocusMode.h"
#include "Event.h"
#include "Map.h"


QBoxLayout *
View::focusPanelLayout()
{
    /*
     *  Focus mode when not in live view.
     */
    QLabel *focusModeLabel = new QLabel( tr("Focus mode:") );
    focusMode = new FocusMode();
    focusMode->setStatusTip( tr("Focus mode when not in live view") );
    QObject::connect(
	focusMode, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)));

    /*
     *  Focus mode when in live view.
     */
    QLabel *focusModeLiveLabel = new QLabel( tr("Live view:") );
    focusModeLive = new QComboBox();
    focusModeLive->setMaxCount( Map::MAX_FocusMode );
    focusModeLive->setStatusTip( tr("Focus mode when in live view") );
    focusModeLive->setFixedWidth( 80 );
    setEvfAFList( camera );
    QObject::connect(
	focusModeLive, SIGNAL(activated(int)),
	this, SLOT(toEvfAFValue(int)) );

    /*
     *  Auto focus button.
     */
    QLabel *focusLabel = new QLabel( tr("Focus:  ") );
    QToolButton *focusButton = new QToolButton();
    focusButton->setAutoRaise( true );
    focusButton->setIcon( QIcon(":/Resources/Misc/button.png") );
    focusButton->setIconSize( QSize(22, 22) );
    focusButton->setStatusTip( tr("Auto focus") );
    QObject::connect(
	focusButton, SIGNAL(pressed()),
	this, SLOT(autoFocusInitiated()) );
    QObject::connect(
	focusButton, SIGNAL(released()),
	this, SLOT(autoFocusDiscontinued()) );

    /*
     *  Focus map to show on live view.
     */
    QLabel *focusMapLabel = new QLabel( tr("Focus map:") );
    QComboBox *focusMapComboBox = new QComboBox();
    focusMapComboBox->setStatusTip( tr("Capture series of images for constructing focus map") );
    focusMapComboBox->addItem( tr("Disabled") );
    focusMapComboBox->addItem( tr("Enabled") );
    focusMapComboBox->setFixedWidth( 80 );
    focusMapComboBox->setCurrentIndex( 0 );
    camera->setFocusMapSetting( 0 );
    QObject::connect(
	focusMapComboBox, SIGNAL(activated(int)),
	this, SLOT(setFocusMap(int)) );

    /*
     *  Depth of field (DOF) preview.
     */
    QLabel *dofPreviewLabel = new QLabel( tr("DOF preview:") );
    QComboBox *dofPreviewComboBox = new QComboBox();
    dofPreviewComboBox->addItem( tr("Disabled") );
    dofPreviewComboBox->addItem( tr("Enabled") );
    dofPreviewComboBox->setFixedWidth( 80 );
    dofPreviewComboBox->setCurrentIndex( 0 );
    QObject::connect(
	dofPreviewComboBox, SIGNAL(activated(int)),
	this, SLOT(setDOFPreview(int)) );

    /*
     *  Focus adjustment buttons.
     */
    const bool flat = false;
    const int width = 22;
    const int height = 16;
    const int buttonHeight = 23;
    const int latency = 333;
    QLabel *focusAdjustmentLabel = new QLabel();
    focusAdjustmentLabel->setText( tr("Focus adjustment:") );

    /*
     *  Near focus: <<<
     */
    QPushButton *focusNear3Button = new QPushButton();
    focusNear3Button->setStatusTip( tr("Near focus: large movement") );
    focusNear3Button->setIcon( QIcon(":/Resources/Focus/arrow-left3.png") );
    focusNear3Button->setIconSize( QSize(width, height) );
    focusNear3Button->setFixedHeight( buttonHeight );
    focusNear3Button->setFlat( flat );
    focusNear3Button->setAutoRepeat( true );
    focusNear3Button->setAutoRepeatDelay( latency );
    focusNear3Button->setAutoRepeatInterval( latency );

    /*
     *  Near focus: <<
     */
    QPushButton *focusNear2Button = new QPushButton();
    focusNear2Button->setStatusTip( tr("Near focus: medium movement") );
    focusNear2Button->setIcon( QIcon(":/Resources/Focus/arrow-left2.png") );
    focusNear2Button->setIconSize( QSize(width, height) );
    focusNear2Button->setFixedHeight( buttonHeight );
    focusNear2Button->setFlat( flat );
    focusNear2Button->setAutoRepeat( true );
    focusNear2Button->setAutoRepeatDelay( latency );
    focusNear2Button->setAutoRepeatInterval( latency );

    /*
     *  Near focus: <
     */
    QPushButton *focusNear1Button = new QPushButton();
    focusNear1Button->setStatusTip( tr("Near focus: small movement") );
    focusNear1Button->setIcon( QIcon(":/Resources/Focus/arrow-left1.png") );
    focusNear1Button->setIconSize( QSize(width, height) );
    focusNear1Button->setFixedHeight( buttonHeight );
    focusNear1Button->setFlat( flat );
    focusNear1Button->setAutoRepeat( true );
    focusNear1Button->setAutoRepeatDelay( latency );
    focusNear1Button->setAutoRepeatInterval( latency );

    /*
     *  Far focus: >
     */
    QPushButton *focusFar1Button = new QPushButton();
    focusFar1Button->setStatusTip( tr("Far focus: small movement") );
    focusFar1Button->setIcon( QIcon(":/Resources/Focus/arrow-right1.png") );
    focusFar1Button->setIconSize( QSize(width, height) );
    focusFar1Button->setFlat( flat );
    focusFar1Button->setAutoRepeat( true );
    focusFar1Button->setAutoRepeatDelay( latency );
    focusFar1Button->setAutoRepeatInterval( latency );

    /*
     *  Far focus: >>
     */
    QPushButton *focusFar2Button = new QPushButton();
    focusFar2Button->setStatusTip( tr("Far focus: medium movement") );
    focusFar2Button->setIcon( QIcon(":/Resources/Focus/arrow-right2.png") );
    focusFar2Button->setIconSize( QSize(width, height) );
    focusFar2Button->setFlat( flat );
    focusFar2Button->setAutoRepeat( true );
    focusFar2Button->setAutoRepeatDelay( latency );
    focusFar2Button->setAutoRepeatInterval( latency );

    /*
     *  Far focus: >>>
     */
    QPushButton *focusFar3Button = new QPushButton();
    focusFar3Button->setStatusTip( tr("Far focus: large movement") );
    focusFar3Button->setIcon( QIcon(":/Resources/Focus/arrow-right3.png") );
    focusFar3Button->setIconSize( QSize(width, height) );
    focusFar3Button->setFlat( flat );
    focusFar3Button->setAutoRepeat( true );
    focusFar3Button->setAutoRepeatDelay( latency );
    focusFar3Button->setAutoRepeatInterval( latency );

    QButtonGroup *focusChosen = new QButtonGroup();
    focusChosen->addButton( focusNear3Button, kEdsEvfDriveLens_Near3 );
    focusChosen->addButton( focusNear2Button, kEdsEvfDriveLens_Near2 );
    focusChosen->addButton( focusNear1Button, kEdsEvfDriveLens_Near1 );
    focusChosen->addButton( focusFar1Button, kEdsEvfDriveLens_Far1 );
    focusChosen->addButton( focusFar2Button, kEdsEvfDriveLens_Far2 );
    focusChosen->addButton( focusFar3Button, kEdsEvfDriveLens_Far3 );
    focusChosen->setExclusive( false );
    QObject::connect(
	focusChosen, SIGNAL(buttonClicked(int)),
	this, SIGNAL(focusAdjustment(int)) );

    /*
     *  Final layout.
     */
    QVBoxLayout *focusButtonLayout = new QVBoxLayout();
    focusButtonLayout->addStretch( 1 );
    focusButtonLayout->addWidget( focusLabel );
    focusButtonLayout->addWidget( focusButton );
    focusButtonLayout->addStretch( 1 );

    QGridLayout *settingsLayout = new QGridLayout();
    settingsLayout->addWidget( focusModeLabel,	   0, 0 );
    settingsLayout->addWidget( focusMode,	   1, 0 );
    settingsLayout->addWidget( focusModeLiveLabel, 0, 2 );
    settingsLayout->addWidget( focusModeLive,      1, 2 );
    settingsLayout->addWidget( focusMapLabel,	   2, 0 );
    settingsLayout->addWidget( focusMapComboBox,   3, 0 );
    settingsLayout->addWidget( dofPreviewLabel,	   2, 2 );
    settingsLayout->addWidget( dofPreviewComboBox,    3, 2 );
    settingsLayout->addLayout( focusButtonLayout,  0, 4, 4, 1 );
    settingsLayout->setColumnStretch( 1, 1 );
    settingsLayout->setColumnStretch( 3, 1 );

    QHBoxLayout *focusAdjustmentLayout = new QHBoxLayout();
    focusAdjustmentLayout->addWidget( focusNear3Button );
    focusAdjustmentLayout->addWidget( focusNear2Button );
    focusAdjustmentLayout->addWidget( focusNear1Button );
    focusAdjustmentLayout->addWidget( focusFar1Button );
    focusAdjustmentLayout->addWidget( focusFar2Button );
    focusAdjustmentLayout->addWidget( focusFar3Button );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout( settingsLayout );
    layout->addWidget( focusAdjustmentLabel );
    layout->addLayout( focusAdjustmentLayout );
    layout->setContentsMargins( 0, 5, 0, 5 );

    return( layout );
}

/*
 *  Focus panel: Signal change in depth of field preview.
 */
void
View::setDOFPreview( int index )
{
    emit propertyChanged( kEdsPropID_Evf_DepthOfFieldPreview, index );
}

/*
 *  Focus panel: Convert from a combo box index value to a value that
 *  represents a valid Canon Evf Focus Mode internal value.
 */
void
View::toEvfAFValue( int index )
{
    if( nAF > 0 ) {
	emit propertyChanged( kEdsPropID_Evf_AFMode, vAF[index] );
    }
}

/*
 *  Focus panel: Current user setting of live view focus mode.
 */
void
View::updateEvfAFValue( Camera *camera )
{
    int index, value;

    if( camera->getAFMode() == Map::AFMODE_Manual ) {
	eventReport( new Event( Event::LensInManualFocus ) );
    }

    /*
     *  Get the current live view focus mode.
     */
    value = camera->getEvfAFMode();

    for( index = 0; index < nAF; index++ ) {
	if( value == vAF[index] ) break;
    }
    if( index >= nAF ) {
	index = 0;
    }

    focusModeLive->setCurrentIndex( index );
    update();
}

/*
 *  Focus panel: Possible settings of live view focus mode.
 */
void
View::setEvfAFList( Camera *camera )
{
    camera->getEvfAFMode_List( &nAF, &vAF );

    focusModeLive->clear();

    if( nAF == 0 ) {
	/*
	 *  Happens on startup if no connection.
	 */
    }
    else {
	/*
	 *  Current list of choices for live view focus mode.
	 */
	for( int i = 0; i < nAF; i++ ) {
	    focusModeLive->insertItem( i, Map::toString_EvfAFMode( vAF[i] ) );
	}
	updateEvfAFValue( camera );
    }
}

/*
 *  Focus panel: Signal start of auto focus.
 */
void
View::autoFocusInitiated()
{
    emit autoFocusChanged( 1 );
}

/*
 *  Focus panel: Signal end of auto focus.
 */
void
View::autoFocusDiscontinued()
{
    emit autoFocusChanged( 0 );
}

/*
 *  Determine current setting for which focus map to display
 *  on live view image.
 */
void
View::setFocusMap( int index )
{
    camera->setFocusMapSetting( index );
}

