/*
 *  Commands for automatically and manually focusing camera.
 */

#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include "Controller.h"
#include "Camera.h"
#include "Event.h"
#include "Map.h"


/*
 *  Command to drive the lens for making focus adjustments.
 *
 *  Possible values:
 *	kEdsEvfDriveLens_Near1
 *	kEdsEvfDriveLens_Near2
 *	kEdsEvfDriveLens_Near3
 *	kEdsEvfDriveLens_Far1
 *	kEdsEvfDriveLens_Far2
 *	kEdsEvfDriveLens_Far3
 *
 *  The sdk always returns EDS_ERR_OK for focus adjustments,
 *  even if they are unsuccessful or ignored. With each lens,
 *  some experimentation is needed to determine how fast a
 *  succession of focus adjustments can be made.
 */
bool
Controller::focusAdjustment( int value )
{

    if( camera->handle() == 0 ) {
	return( false );
    }

    /*
     *  Check if lens is in manual focus mode,
     *  in which case the command has no effect.
     */
    if( camera->getAFMode() == Map::AFMODE_Manual ) {
	emit eventReport( new Event( Event::LensInManualFocus ) );
	return( false );
    }

    /*
     *  Check if camera is not in live view mode, in which case
     *  issuing the command would put the camera in a bad state.
     */
    if( camera->getEvfOutputDevice() == 0 ) {
	emit eventReport( new Event( Event::NotEvfMode ) );
	return( false );
    }

    /*
     *  Always returns ok, even when not sucessful.
     */
    EdsSendCommand( camera->handle(),
		    kEdsCameraCommand_DriveLensEvf,
		    value );

    return( true );
}

/*
 *  Command to start and stop auto focusing of camera.
 *
 *  Possible values:
 *	kEdsCameraCommand_EvfAf_OFF
 *	kEdsCameraCommand_EvfAf_ON
 */
bool
Controller::autoFocusChanged( int status )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    /*
     *  Check if lens is in manual focus mode,
     *  in which case the command has no effect.
     */
    if( camera->getAFMode() == Map::AFMODE_Manual ) {
	emit eventReport( new Event( Event::LensInManualFocus ) );
	return( false );
    }

    /*
     *  Check if camera is not in live view mode, in which case
     *  issuing the command could put the camera in a bad state.
     */
    if( camera->getEvfOutputDevice() == 0 ) {
	emit eventReport( new Event( Event::NotEvfMode ) );
	return( false );
    }

    int result = EDS_ERR_OK;

    result = EdsSendCommand(
		camera->handle(),
		kEdsCameraCommand_DoEvfAf,
		status );

    return( result == EDS_ERR_OK );
}

