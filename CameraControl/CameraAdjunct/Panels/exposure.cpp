/*
 *  Panel for setting exposure.
 */

#include <QtGui>
#include "EDSDK.h"
#include "View.h"
#include "Widgets/ShutterSpeed.h"
#include "Widgets/Aperture.h"
#include "Widgets/ISOSpeed.h"
#include "Widgets/ShootingMode.h"
#include "Widgets/MeteringMode.h"
#include "Widgets/ExposureComp.h"
#include "Widgets/WhiteBalance.h"
#include "Widgets/DriveMode.h"


QBoxLayout *
View::exposurePanelLayout()
{
    QLabel *ssLabel = new QLabel( tr("Shutter speed:") );
    shutterSpeed = new ShutterSpeed();
    QObject::connect(
	shutterSpeed, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)) );

    QLabel *apertureLabel = new QLabel( tr("Aperture:") );
    aperture = new Aperture();
    QObject::connect(
	aperture, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)) );

    QLabel *isoLabel = new QLabel( tr("ISO speed:") );
    isoSpeed = new ISOSpeed();
    QObject::connect(
	isoSpeed, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)) );

    shootingMode = new ShootingMode();

    QLabel *ecLabel = new QLabel( tr("Exposure compensation:") );
    exposureComp = new ExposureComp( camera, 3, this );
    QObject::connect(
	exposureComp, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)));

    QLabel *expLabel = new QLabel( tr("Exposure:") );
    QToolButton *expButton = new QToolButton();
    expButton->setAutoRaise( true );
    expButton->setStatusTip( tr("Auto exposure button") );
    expButton->setIcon( QIcon(":/Resources/Misc/button.png") );
    expButton->setIconSize( QSize(22, 22) );
    QObject::connect(
	expButton, SIGNAL(pressed()),
	this, SLOT(autoMeteringInitiated()) );
    QObject::connect(
	expButton, SIGNAL(released()),
	this, SLOT(autoMeteringDiscontinued()) );

    QLabel *driveLabel = new QLabel( tr("Drive:") );
    driveMode = new DriveMode();
    QObject::connect(
	driveMode, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)) );

    QLabel *meteringLabel = new QLabel( tr("Metering:") );
    meteringMode = new MeteringMode();
    QObject::connect(
	meteringMode, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)));

    QLabel *whiteLabel = new QLabel( tr("White balance:") );
    whiteBalance = new WhiteBalance();
    QObject::connect(
	whiteBalance, SIGNAL(propertyChanged(int,int)),
	this, SIGNAL(propertyChanged(int,int)) );

    QGridLayout *layer1Layout = new QGridLayout();
    layer1Layout->addWidget( ssLabel,       0, 0 );
    layer1Layout->addWidget( shutterSpeed,  1, 0 );
    layer1Layout->addWidget( apertureLabel, 0, 1 );
    layer1Layout->addWidget( aperture,      1, 1 );
    layer1Layout->addWidget( isoLabel,      0, 2 );
    layer1Layout->addWidget( isoSpeed,      1, 2 );
    layer1Layout->setColumnStretch( 0, 1 );
    layer1Layout->setColumnStretch( 1, 1 );
    layer1Layout->setColumnStretch( 2, 1 );

    QGridLayout *layer2Layout = new QGridLayout();
    layer2Layout->addWidget( shootingMode,  0, 0, 2, 1 );
    layer2Layout->addWidget( ecLabel,       0, 2 );
    layer2Layout->addWidget( exposureComp,  1, 2 );
    layer2Layout->addWidget( expLabel,      0, 4, Qt::AlignHCenter );
    layer2Layout->addWidget( expButton,     1, 4, Qt::AlignHCenter |
					          Qt::AlignVCenter );
    layer2Layout->setColumnStretch( 1, 1 );
    layer2Layout->setColumnStretch( 3, 1 );

    QGridLayout *layer3Layout = new QGridLayout();
    layer3Layout->addWidget( driveLabel,    0, 0 );
    layer3Layout->addWidget( driveMode,     1, 0 );
    layer3Layout->addWidget( meteringLabel, 0, 1 );
    layer3Layout->addWidget( meteringMode,  1, 1 );
    layer3Layout->addWidget( whiteLabel,    0, 2 );
    layer3Layout->addWidget( whiteBalance,  1, 2 );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout( layer1Layout );
    layout->addLayout( layer2Layout );
    layout->addLayout( layer3Layout );
    layout->setContentsMargins( 0, 5, 0, 5 );

    return( layout );
}


/*
 *  Exposure panel: Signal start of auto metering.
 */
void
View::autoMeteringInitiated()
{
    emit shutterChanged( kEdsCameraCommand_ShutterButton_Halfway_NonAF );
}

/*
 *  Exposure panel: Signal end of auto metering.
 */
void
View::autoMeteringDiscontinued()
{
    emit shutterChanged( kEdsCameraCommand_ShutterButton_OFF );
}

