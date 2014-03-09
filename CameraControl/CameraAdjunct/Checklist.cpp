/*
 *  Run through a checklist for a successful multi-shot.
 */

#include <QtGui>
#include "Checklist.h"
#include "Camera.h"
#include "ShotSeq.h"
#include "Map.h"

/*
 *  Constructor for checklist.
 */
Checklist::Checklist( QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    setWindowTitle( tr("Checklist") );
    setFixedSize( 380, 320 );

    initialize();
}

/*
 *  Destructor for checklist.
 */
Checklist::~Checklist()
{
}

void
Checklist::initialize()
{
    header = new QLabel();
    for( int row = 0; row < maxConditions; row++ ) {
	item[row][0] = new QLabel( this );
	item[row][1] = new QLabel( this );
	item[row][0]->setFixedSize( 32, 32 );
	item[row][1]->setWordWrap( true );
	item[row][1]->setFixedHeight( 32 );
    }

    closeButton = new QPushButton();
    closeButton->setText( tr("Close") );
    closeButton->setDefault( true );
    QObject::connect(
	closeButton, SIGNAL(clicked()),
	this, SLOT(close()) );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget( closeButton );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( header );
    for( int row = 0; row < maxConditions; row++ ) {
	QHBoxLayout *l = new QHBoxLayout();
	l->addWidget( item[row][0] );
	l->addWidget( item[row][1] );
	l->setContentsMargins(0,0,0,0);
	layout->addLayout( l );
	layout->addSpacing( -4 );
    }
    layout->addStretch( 1 );
    layout->addLayout( buttonLayout );

    setLayout( layout );
}

void
Checklist::setChecklist( Camera *camera, ShotSeq *shotSeq )
{
    QFont font;
    font.setPointSize( 10 );
    header->setFont( font );
    if( (shotSeq->type == ShotSeq::ExposureBracketingManual) ||
	(shotSeq->type == ShotSeq::ExposureBracketingAv) ) {
	header->setText( tr("Exposure bracketing checklist") );
    }
    else
    if( shotSeq->type == ShotSeq::FocusBracketing ) {
	 header->setText( tr("Focus bracketing checklist") );
    }
    else
    if( shotSeq->type == ShotSeq::Interval ) {
	 header->setText( tr("Interval checklist") );
    }
    else
    if( shotSeq->type == ShotSeq::Stitch ) {
	 header->setText( tr("Panorama checklist") );
    }

    /*
     *  Conditions so far: 9 (current max: 10).
     */
    int condition = 0;
    int mode = camera->getShootingMode();

    for( int i = 0; i < maxConditions; i++ ) {
	item[i][0]->show();
	item[i][1]->show();
    }

    /*
     *  Condition: Manual or Av shooting mode.
     */
    if( (shotSeq->type == ShotSeq::ExposureBracketingManual) ||
	(shotSeq->type == ShotSeq::ExposureBracketingAv) ) {
	if( (mode == kEdsAEMode_Manual) || (mode == kEdsAEMode_Av) ) {
	    setPixmap( condition, ICON_checkmark );
	    item[condition][1]->setText( tr("Manual or Av shooting mode.")  );
	}
	else {
	    setPixmap( condition, ICON_warning );
	    item[condition][1]->setText(
			tr("The shooting mode is not Manual or Av.")  );
	}
    }
    else {
	if( mode == kEdsAEMode_Manual ) {
	    setPixmap( condition, ICON_checkmark );
	    item[condition][1]->setText( tr("Manual shooting mode.")  );
	}
	else {
	    setPixmap( condition, ICON_warning );
	    item[condition][1]->setText( tr("The shooting mode is not Manual.")  );
	}
    }
    condition++;

    /*
     *  Condition: Not in ISO auto mode.
     */
    if( camera->getISO() == 0x00 ) {
	setPixmap( condition, ICON_warning );
	item[condition][1]->setText( tr("The ISO setting is on auto.")  );
    }
    else {
	setPixmap( condition, ICON_checkmark );
	item[condition][1]->setText( tr("Fixed ISO setting.")  );
    }
    condition++;

    /*
     *  Condition: No auto white balance.
     */
    if( camera->getWhiteBalance() == kEdsWhiteBalance_Auto ) {
	setPixmap( condition, ICON_warning );
	item[condition][1]->setText( tr("The white balance setting is on auto.")  );
    }
    else {
	setPixmap( condition, ICON_checkmark );
	item[condition][1]->setText( tr("Fixed white balance setting.")  );
    }
    condition++;

    /*
     *  Condition: Mirror lockup.
     */
    if( (camera->getMirrorLockup() == 1) &&
        (camera->getEvfOutputDevice() == 0) ) {
	setPixmap( condition, ICON_warning );
	item[condition][1]->setText( tr("Mirror lockup is not available with multi-shot. As an alternative, enable live view mode to effectively reduce vibration.") );
    }
    else {
	item[condition][0]->hide();
	item[condition][1]->hide();
    }
    condition++;

    /*
     *  Condition: Auto focus.
     */
    if( shotSeq->type == ShotSeq::FocusBracketing ) {
	if( camera->getAFMode() == Map::AFMODE_Manual ) {
	    setPixmap( condition, ICON_warning );
	    item[condition][1]->setText( tr("The camera is in manual focus mode.")  );
	}
	else {
	    setPixmap( condition, ICON_checkmark );
	    item[condition][1]->setText( tr("Auto focus mode setting.")  );
	}
    }
    else {
	if( camera->getAFMode() == Map::AFMODE_Manual ) {
	    item[condition][0]->hide();
	    item[condition][1]->hide();
	}
	else {
	    setPixmap( condition, ICON_tip );
	    item[condition][1]->setText( tr("Use manual focus (or turn off auto focus once focus has been achieved when setting up the shot).")  );
	}
    }
    condition++;

    /*
     *  Condition: Specific to type of multi-shot.
     */
    if( (shotSeq->type == ShotSeq::ExposureBracketingManual) ||
	(shotSeq->type == ShotSeq::ExposureBracketingAv) ) {
	item[condition][0]->hide();
	item[condition][1]->hide();
    }
    else
    if( shotSeq->type == ShotSeq::FocusBracketing ) {
	/*
	 *  Warn about enabling live view mode.
	 */
	if( camera->getEvfOutputDevice() == 0 ) {
	    setPixmap( condition, ICON_warning );
	    item[condition][1]->setText( tr("Live view mode is not enabled (it must be turned on for focus bracketing).") );
	}
	else {
	    item[condition][0]->hide();
	    item[condition][1]->hide();
	}
    }
    else
    if( shotSeq->type == ShotSeq::Interval ) {
	/*
	 *  Warn about battery level.
	 */
    	int l = camera->getBatteryLevel();
	if( (0 <= l) && (l < 50) && (shotSeq->frames > 15) ) {
	    setPixmap( condition, ICON_warning );
	    item[condition][1]->setText( tr("Battery level may be too low for a long sequence of shots.") );
	}
	else {
	    item[condition][0]->hide();
	    item[condition][1]->hide();
	}
    }
    else
    if( shotSeq->type == ShotSeq::Stitch ) {
	/*
	 *  Better if images are downloaded to camera.
	 */
	if( camera->getSaveTo() == kEdsSaveTo_Camera ) {
	    setPixmap( condition, ICON_tip );
	    item[condition][1]->setText( tr("Change import destination to computer so that live view can be used to help align next image in panorama.")  );
	}
	else {
	    item[condition][0]->hide();
	    item[condition][1]->hide();
	}
    }
    condition++;

    /*
     *  Conditions: Specific to interval.
     */
    if( shotSeq->type == ShotSeq::Interval ) {
	/*
	 *  Warning if interval to short for sustained shooting.
	 */
	int downloadTime = shotSeq->interval - shotSeq->shutter;
	if( (shotSeq->frames > 10) &&
	    (downloadTime < 1000) ) {	// milliseconds
	    setPixmap( condition, ICON_warning );
	    item[condition][1]->setText( tr("The interval between shots is not long enough to allow for storing or downloading the image.")  );
	    condition++;
	}
	/*
	 *  Tip for night photography.
	 */
	if( camera->shutterInBulbMode() ) {
	    setPixmap( condition, ICON_tip );
	    item[condition][1]->setText( tr("For night photography, dim the display, turn off live view, and delay one minute before beginning to take images.")  );
	    condition++;
	}
    }

    /*
     *  Condition: Tripod
     */
    setPixmap( condition, ICON_tip );
    item[condition][1]->setText( tr("Use a tripod if available and turn off image stabilization.")  );
    condition++;

    for( int i = condition; i < maxConditions; i++ ) {
	item[i][0]->hide();
	item[i][1]->hide();
    }
}


void
Checklist::setPixmap( int row, int value )
{
    switch( value ) {
	case ICON_tip:
	    item[row][0]->setPixmap( QPixmap(":/Resources/Misc/tip.png") );
	    break;
	case ICON_checkmark:
	    item[row][0]->setPixmap( QPixmap(":/Resources/Misc/ok.png") );
	    break;
	case ICON_warning:
	    item[row][0]->setPixmap( QPixmap(":/Resources/Misc/warning.png") );
	    break;
    }
}

