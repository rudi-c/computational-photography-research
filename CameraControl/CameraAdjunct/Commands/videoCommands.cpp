/*
 *  Commands to start and stop video shooting.
 */

#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "Controller.h"
#include "Camera.h"
#include "Event.h"
#include "Map.h"


bool
Controller::startVideo()
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    /*
     *  Camera must be set to movie shooting mode.
     *  On some cameras video mode is a shooting mode;
     *  on others it is a separate switch.
     */
    if( (camera->getShootingMode() != kEdsAEMode_Movie) &&
	(camera->getEvfMode() != 2) ) {
	emit eventReport( new Event( Event::NotVideoMode ) );
	return( false );
    }

    /*
     *  Get current record mode.
     *  Nothing to do if already recording a movie.
     */
    int recordMode = camera->getRecordMode();
    if( recordMode == Map::RecordModeStart ) {
	return( true );
    }

    /*
     *  Save video on camera (as cannot transfer directly to PC).
     *  Remember the current setting so it can be restored when
     *  video recording is terminated.
     */
    saveSaveTo = camera->getSaveTo();
    if( !setPropertyCommand( kEdsPropID_SaveTo, kEdsSaveTo_Camera ) ) {
	setPropertyCommand( kEdsPropID_SaveTo, saveSaveTo );
	return( false );
    }

    int result = EDS_ERR_OK;

    /*
     *  Begin movie shooting.
     */
    uint recordStart = Map::RecordModeStart;
    result = EdsSetPropertyData( camera->handle(),
		kEdsPropID_Record,
		0,
		sizeof(recordStart),
		&recordStart );
    /*
     *  Will also be set through a callback from the camera.
     */
    if( result == EDS_ERR_OK ) {
	camera->setRecordMode( Map::RecordModeStart );
    }

    return( result == EDS_ERR_OK );
}

void
Controller::stopVideo()
{
    int result = EDS_ERR_OK;

    /*
     *  Check whether already not in movie recording mode.
     */
    if( camera->getRecordMode() == Map::RecordModeStop ) {
	return;
    }

    /*
     *  Stop movie shooting.
     */
    uint recordStop = Map::RecordModeStop;
    result = EdsSetPropertyData( camera->handle(),
		kEdsPropID_Record,
		0,
		sizeof(recordStop),
		&recordStop );

    /*
     *  Will also be set through a callback from the camera.
     */
    if( result == EDS_ERR_OK ) {
	camera->setRecordMode( Map::RecordModeStop );
    }

    /*
     *  Restore the previous setting for destination of images.
     */
    setPropertyCommand( kEdsPropID_SaveTo, saveSaveTo );
}

