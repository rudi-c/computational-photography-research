/*
 *  Commands for shooting.
 */

#include <QtGui>
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "Controller.h"
#include "Camera.h"
#include "ShotSeq.h"
#include "Event.h"
#include "Map.h"


void
Controller::startSequence( ShotSeq *shotSeq )
{
    if( camera->handle() == 0 ) {
	doSeq = false;
	emit eventReport( new Event( Event::SequenceCancelledNoConnection ) );
	return;
    }

    if( (camera->getMirrorLockup() == 1) && !doLive ) {
	doSeq = false;
	emit eventReport( new Event( Event::SequenceCancelledMirrorLockup ) );
	return;
    }

    if( (shotSeq->type == ShotSeq::FocusBracketing) &&
        (camera->getEvfOutputDevice() == 0) ) {
	doSeq = false;
	emit eventReport( new Event( Event::SequenceCancelledNotEvfMode ) );
	return;
    }

    this->shotSeq = shotSeq;
    doSeq = true;

    emit eventReport( new Event( Event::SequenceInitiated, shotSeq->type ) );

    if( shotSeq->type == ShotSeq::ExposureBracketingManual ) {
	/*
	 *  Exposure bracketing in Manual shooting mode.
	 *  Keep aperture constant, adjust shutter speed.
	 */
	for( int i = 0; (i < shotSeq->frames) && doSeq; i++ ) {
	    setPropertyCommand( kEdsPropID_Tv, shotSeq->bracket[i] );
	    takePictureCommand();
	    QCoreApplication::processEvents();
	}
	// restore shutter speed to its original setting
	setPropertyCommand( kEdsPropID_Tv, shotSeq->bracket[0] );
    }
    else
    if( shotSeq->type == ShotSeq::ExposureBracketingAv ) {
	/*
	 *  Exposure bracketing in Av shooting mode.
	 *  Keep aperture constant, adjust shutter speed
	 *  by changing exposure compensation.
	 */
	for( int i = 0; (i < shotSeq->frames) && doSeq; i++ ) {
	    setPropertyCommand( kEdsPropID_ExposureCompensation,
		shotSeq->bracket[i] );
	    takePictureCommand();
	    QCoreApplication::processEvents();
	}
	// restore exposure compensation to its original setting
	setPropertyCommand( kEdsPropID_ExposureCompensation,
	    shotSeq->bracket[0] );
    }
    else
    if( shotSeq->type == ShotSeq::FocusBracketing ) {
	/*
	 *  Focus bracketing.
	 */
	//const int latency = 333;	// 100 is sufficient for USM lens
	const int latency = 3333;	// 100 is sufficient for USM lens
	for( int i = 0; (i < shotSeq->frames) && doSeq; i++ ) {
	    if( shotSeq->bracket[i] != 0 ) {
		focusAdjustment( shotSeq->bracket[i] );
	    }
	    Sleep( latency );
	    takePictureCommand();
	    /*
	     *  The camera becomes unstable if focus adjustments
	     *  are attempted while the shutter is open. This is
	     *  an inelegant but simple fix.
	     */
	    QApplication::setOverrideCursor( Qt::BusyCursor );
	    int l = Map::toShutterSpeed( camera->getTv() );
	    // break into 100 millisecond chunks
	    l = l/100;
	    for( int i = 0; i <= l; i++ ) {
		QCoreApplication::processEvents();
		Sleep( 100 );
	    }
	    QCoreApplication::processEvents();
	    QApplication::restoreOverrideCursor();
	}
    }
    else
    if( shotSeq->type == ShotSeq::Interval ) {
	/*
	 *  Interval shooting. Delay til first shot.
	 */
	QTimer::singleShot( shotSeq->delay, this, SLOT(firstShot()) );
    }
    else
    if( shotSeq->type == ShotSeq::Stitch ) {
	/*
	 *  Panorama shooting.
	 */
	doSeq = true;
    }
    else {
	doSeq = false;
	emit eventReport( new Event( Event::SequenceCancelled ) );
    }
}

void
Controller::cancelSequence()
{
    if( camera->handle() == 0 ) {
	return;
    }

    if( bulbLock ) {
	bulbCloseShutter();
    }
    intervalTimer.stop();
    doSeq = false;
    for( int i = 0; (i < 3) && camera->objectsWaiting(); i++ ) {
	Sleep( 500 );	// milliseconds
	QCoreApplication::processEvents();
    }
    emit eventReport( new Event( Event::SequenceCancelled ) );
}

/*
 *  Command to press the shutter button.
 *
 *  Possible values:
 *	kEdsCameraCommand_ShutterButton_OFF
 *	kEdsCameraCommand_ShutterButton_Halfway
 *	kEdsCameraCommand_ShutterButton_Completely
 *	kEdsCameraCommand_ShutterButton_Halfway_NonAF
 *	kEdsCameraCommand_ShutterButton_Completely_NonAF
 */
bool
Controller::shutterCommand( int value )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    int result = EDS_ERR_OK;

    /*
     *  Retry if camera device is busy.
     */
    do {

	result = EdsSendCommand(
			camera->handle(),
			kEdsCameraCommand_PressShutterButton,
			value );

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy, 0, "shutter" ) );
	    EdsSendCommand(
		camera->handle(),
		kEdsCameraCommand_PressShutterButton,
		kEdsCameraCommand_ShutterButton_OFF );
	    QCoreApplication::processEvents();
	    Sleep( 500 );	// milliseconds
	}

    } while( result == EDS_ERR_DEVICE_BUSY );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::ShutterFailure, result ) );
    }

    return( result == EDS_ERR_OK );
}

void
Controller::takePictureCommand()
{
    if( shutterCommand( kEdsCameraCommand_ShutterButton_Completely_NonAF ) ) {
	shutterCommand( kEdsCameraCommand_ShutterButton_OFF );
    }

    if( (shotSeq->shot+1 == shotSeq->frames) &&
        (shotSeq->type == ShotSeq::Interval) ) {
	intervalTimer.stop();
    }
}

/*
 *  Take first shot in interval shooting and start
 *  timer for taking subsequent shots.
 */
void
Controller::firstShot()
{
    if( shotSeq->interval == 0 ) {
	/*
	 *  Shoot as fast as possible.
	 */
	for( int i = 0; i < shotSeq->frames; i++ ) {
	    takePictureCommand();
	    QCoreApplication::processEvents();
	}
    }
    else {
	/*
	 *  Shots timed by intervals.
	 */
	disconnect( &intervalTimer, 0, 0, 0 );
	if( shotSeq->bulbMode ) {
	    bulbShutterTime = shotSeq->shutter;
	    QObject::connect(
		&intervalTimer, SIGNAL(timeout()),
		this, SLOT(bulbOpenShutter()) );
	    bulbOpenShutter();
	}
	else {
	    QObject::connect(
		&intervalTimer, SIGNAL(timeout()),
		this, SLOT(takePictureCommand()) );
	    takePictureCommand();
	}

	if( shotSeq->frames > 1 ) {
	    intervalTimer.start( shotSeq->interval );
	}
    }
}

/*
 *  Bulb shooting command.
 *
 *  Depending on speed of camera, speed of usb connection, and speed
 *  of SD card or equivalent, the interval must be at least 2 seconds
 *  longer than shutter speed if downloading to the computer and 1.5
 *  seconds if images stay on camera.
 *  The camera is locked during bulb shooting.
 *  The sdk has the commands:
 *	kEdsCameraCommand_BulbStart
 *	kEdsCameraCommand_BulbEnd
 *  but they do not appear to work for the T2i/550D.
 *
 *  Windows has 15 millisecond accuracy, so routine should only
 *  be used for shutter speeds of 100 ms or longer.
 */
void
Controller::bulbOpenShutter()
{
    if( camera->handle() == 0 ) {
	return;
    }

    /*
     *  Prevents changes being made directly on the camera.
     *  Also need to disable the user interface to prevent
     *  changes being sent to camera, as otherwise the
     *  camera hangs.
     */
    bulbLock = lockCommand( kEdsCameraStatusCommand_UILock );

    if( bulbLock ) {
	bool b = shutterCommand(
			kEdsCameraCommand_ShutterButton_Completely_NonAF );

	if( b ) {
	    /*
	     *  Close shutter after specified interval (milliseconds).
	     */
	    QTimer::singleShot( bulbShutterTime, this,
		SLOT(bulbCloseShutter()) );
	}
	else {
	    /*
	     *  Unlock the camera if not successful at opening shutter.
	     */
	    lockCommand( kEdsCameraStatusCommand_UIUnLock );
	    bulbLock = false;
	}
    }

}

/*
 *  Bulb shooting command.
 */
void
Controller::bulbCloseShutter()
{
    shutterCommand( kEdsCameraCommand_ShutterButton_OFF );

    if( (shotSeq != NULL) &&
	(shotSeq->shot+1 == shotSeq->frames) &&
        (shotSeq->type == ShotSeq::Interval) ) {
	intervalTimer.stop();
    }

    if( bulbLock ) {
	lockCommand( kEdsCameraStatusCommand_UIUnLock );
	bulbLock = false;
    }
}

/*
 *  Command to un/lock the camera to prevent changes
 *  being made directly on the camera. Necessary during
 *  bulb shooting as otherwise the camera may hang.
 *
 *  Possible values:
 *	kEdsCameraStatusCommand_UILock,
 *	kEdsCameraStatusCommand_UIUnLock,
 */
bool
Controller::lockCommand( int value )
{
    int result = EDS_ERR_OK;

    /*
     *  Retry if camera device is busy.
     */
    do {

	result = EdsSendStatusCommand(
			camera->handle(),
			value,
			0 );

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy, 0, "lock" ) );
	    EdsSendStatusCommand(
		camera->handle(),
		kEdsCameraStatusCommand_UIUnLock,
		0 );
	    QCoreApplication::processEvents();
	    Sleep( 500 );	// milliseconds
	}

    } while( result == EDS_ERR_DEVICE_BUSY );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::LockFailure, result ) );
    }

    return( result == EDS_ERR_OK );
}

/*
 *  Given a bulb shutter speed and a lower ISO,
 *  take a test shot using a higher ISO and a
 *  correspondingly lower shutter speed. The
 *  shutter speed is in seconds.
 */
void
Controller::highISOtest( int shutterSpeed )
{
    if( camera->handle() == 0 ) {
	return;
    }

    int saveISO;
    int ISO;
    int maxISO;

    /*
     *  Save current ISO value.
     */
    saveISO = camera->getISO();
    if( saveISO == 0 ) {
	emit eventReport( new Event( Event::ISOAuto ) );
	return;
    }

    /*
     *  Get max ISO value.
     */
    int n, *v;
    camera->getISO_List( &n, &v );
    maxISO = Map::toISO( v[n-1] );

    /*
     *  Determine shutter speed and ISO value for test shot.
     */
    ISO = Map::toISO( saveISO );
    while( (shutterSpeed >= 4) && (ISO < maxISO) ) {
	shutterSpeed = (int)ceil(shutterSpeed/2.0);
	ISO *= 2;
    }

    /*
     *  Take picture with new ISO and shutter speed.
     */
    setPropertyCommand( kEdsPropID_ISOSpeed, Map::fromISO( ISO ) );
    bulbShutterTime = 1000 * shutterSpeed;	// milliseconds
    bulbOpenShutter();

    /*
     *  Restore ISO to its original setting.
     */
    setPropertyCommand( kEdsPropID_ISOSpeed, saveISO );
}

/*
bulbStart:
    if( result == EDS_ERR_OK ) {
	result = EdsSendCommand(
			camera->handle(),
			kEdsCameraCommand_BulbStart,
			0 );
	if( result != EDS_ERR_OK ) {
	    ERROR( "EdsSendCommand BulbStart", result );
	}
	if( (result != EDS_ERR_OK) && bulbLock ) {
	    EdsSendStatusCommand(
		camera->handle(),
		kEdsCameraStatusCommand_UIUnLock,
		0 );
	    bulbLock = false;
	    return( false );
	}
    }
*/

/*
bulbEnd:
    result = EdsSendCommand(
		camera->handle(),
		kEdsCameraCommand_BulbEnd,
		0 );
    if( result != EDS_ERR_OK ) {
	ERROR( "EdsSendCommand BulbEnd", result );
    }
*/

/*
bool
Controller::takePictureCommand()
{
    int result = EDS_ERR_OK;

    result = EdsSendCommand(
		camera->handle(),
		kEdsCameraCommand_TakePicture,
		0 );

    if( (result != EDS_ERR_OK) && (result != EDS_ERR_DEVICE_BUSY) ) {
	ERROR( "EdsSendCommand takePicture", result );
    }

    return( result == EDS_ERR_OK );
};
*/

