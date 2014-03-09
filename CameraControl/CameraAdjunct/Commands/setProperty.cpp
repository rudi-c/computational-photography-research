/*
 *  Commands to set properties of the camera:
 *	setPropertyCommand( int property, int newValue, int parameter )
 *	setPropertyCommand( int property, QString string )
 *	setPropertyCommand( int property, QDateTime dateTime )
 *	setPropertyCommand( int property, int *s )
 *	setFocusPropertyCommand( int selectionMode, int fp )
 *	moveZoomRectCommand( int x, int y )
 */

#include <QtGui>
#include <QDateTime>
#include <QString>
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include "Controller.h"
#include "Camera.h"
#include "CustomFunction.h"
#include "Event.h"
#include "Map.h"


bool
Controller::setPropertyCommand( int property, int newValue, int parameter )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    /*
     *  Check camera model to ensure the new value is different
     *  from the current value.
     */
    bool done = false;
    switch( property ) {
	case kEdsPropID_AEModeSelect:
	    done = (newValue == camera->getShootingMode());
	    break;
	case kEdsPropID_Av:
	    done = (newValue == camera->getAv());
	    break;
	case kEdsPropID_Tv:
	    done = (newValue == camera->getTv());
	    break;
	case kEdsPropID_ISOSpeed:
	    done = (newValue == camera->getISO());
	    break;
	case kEdsPropID_MeteringMode:
	    done = (newValue == camera->getMeteringMode());
	    break;
	case kEdsPropID_DriveMode:
	    done = (newValue == camera->getDriveMode());
	    break;
	case kEdsPropID_AFMode:
	    done = (newValue == camera->getAFMode());
	    break;
	case kEdsPropID_WhiteBalance:
	    done = (newValue == camera->getWhiteBalance());
	    break;
	case kEdsPropID_ColorSpace:
	    done = (newValue == camera->getColorSpace());
	    break;
	case kEdsPropID_PictureStyle:
	    done = (newValue == camera->getPictureStyle());
	    break;
	case kEdsPropID_ImageQuality:
	    done = (newValue == camera->getImageQuality());
	    break;
	case kEdsPropID_Evf_Zoom:
	    done = (newValue == camera->getEvfZoom());
	    break;
	case kEdsPropID_Evf_AFMode:
	    done = (newValue == camera->getEvfAFMode());
	    break;
	case kEdsPropID_Evf_DepthOfFieldPreview:
	    done = (newValue == camera->getEvfDepthOfFieldPreview());
	    break;
	case kEdsPropID_ExposureCompensation:
	    done = (newValue == camera->getExposureComp());
	    break;
	case kEdsPropID_SaveTo:
	    done = (newValue == camera->getSaveTo());
	    break;
	case kEdsPropID_CFn:
	    switch( parameter ) {
		case CFn_ExposureLevelIncrements:
		    done = (newValue == camera->getExposureLevelIncrements());
		    break;
		case CFn_FlashSyncSpeedInAvMode:
		    done = (newValue == camera->getFlashSyncSpeedInAvMode());
		    break;
		case CFn_LongExposureNoiseReduction:
		    done = (newValue == camera->getLongExposureNoiseReduction());
		    break;
		case CFn_HighISOSpeedNoiseReduction:
		    done = (newValue == camera->getHighISOSpeedNoiseReduction());
		    break;
		case CFn_AutoFocusAssistBeamFiring:
		    done = (newValue == camera->getAutoFocusAssistBeamFiring());
		    break;
		case CFn_MirrorLockup:
		    done = (newValue == camera->getMirrorLockup());
		    break;
		case CFn_ShutterAELockButton:
		    done = (newValue == camera->getShutterAELockButton());
		    break;
	    }
	    break;
	default:
	    emit eventReport( new Event( Event::SetPropertyUnhandled, property ) );
	    return( false );
    }
    if( done ) {
	return( true );
    }

    int result = EDS_ERR_OK;

    /*
     *  Retry if camera device is busy.
     */
    do {

	/*
	 *  Send the command to the camera.
	 */
	result = EdsSetPropertyData(
			camera->handle(),
			property,
			parameter,
			sizeof(newValue),
			&newValue );

	/*
	 *  Commands are not accepted for a while when depth
	 *  of field preview has been released.
	 */
	if( (result == EDS_ERR_OK) &&
	    (property == kEdsPropID_Evf_DepthOfFieldPreview) ) {
	    Sleep( 500 );       // milliseconds
	}

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy, 0,
		QString( "setProperty" ) ) );
	    QCoreApplication::processEvents();
	    Sleep( 500 );	// milliseconds
	}

    } while( result == EDS_ERR_DEVICE_BUSY );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::SetPropertyFailure,
	    property, "setProperty" ) );
	return( false );
    }

    /*
     *  Set the capacity of the computer (false values are used).
     *
     *  typedef struct *  {
     *      EdsInt32        numberOfFreeClusters;
     *      EdsInt32        bytesPerSector;
     *      EdsBool         reset;
     *  } EdsCapacity;
     */
    if( (result == EDS_ERR_OK) &&
	(property == kEdsPropID_SaveTo) &&
       ((newValue == kEdsSaveTo_Host) || (newValue == kEdsSaveTo_Both)) ) {
	EdsCapacity capacity = {0x7FFFFFFF, 0x1000, 1};
	result = EdsSetCapacity( camera->handle(), capacity );
    }

    /*
     *  Update the camera model.
     */
    if( result == EDS_ERR_OK ) {
	switch( property ) {
	    case kEdsPropID_AEModeSelect:
		camera->setShootingMode( newValue );
		break;
	    case kEdsPropID_Av:
		camera->setAv( newValue );	
		break;
	    case kEdsPropID_Tv:
		camera->setTv( newValue );	
		break;
	    case kEdsPropID_ISOSpeed:
		camera->setISO( newValue );	
		break;
	    case kEdsPropID_MeteringMode:
		camera->setMeteringMode( newValue );
		break;
	    case kEdsPropID_DriveMode:
		camera->setDriveMode( newValue );
		break;
	    case kEdsPropID_AFMode:
		camera->setAFMode( newValue );
		break;
	    case kEdsPropID_WhiteBalance:
		camera->setWhiteBalance( newValue );
		break;
	    case kEdsPropID_ColorSpace:
		camera->setColorSpace( newValue );
		break;
	    case kEdsPropID_PictureStyle:
		camera->setPictureStyle( newValue );
		break;
	    case kEdsPropID_ImageQuality:
		camera->setImageQuality( newValue );
		break;
	    case kEdsPropID_Evf_Zoom:
		camera->setEvfZoom( newValue );
		break;
	    case kEdsPropID_Evf_AFMode:
		camera->setEvfAFMode( newValue );
		break;
	    case kEdsPropID_Evf_DepthOfFieldPreview:
		camera->setEvfDepthOfFieldPreview( newValue );
		break;
	    case kEdsPropID_ExposureCompensation:
		camera->setExposureComp( newValue );
		break;
	    case kEdsPropID_SaveTo:
		camera->setSaveTo( newValue );
		break;
	    case kEdsPropID_CFn:
		switch( parameter ) {
		    case CFn_ExposureLevelIncrements:
			camera->setExposureLevelIncrements( newValue );
			break;
		    case CFn_FlashSyncSpeedInAvMode:
			camera->setFlashSyncSpeedInAvMode( newValue );
			break;
		    case CFn_LongExposureNoiseReduction:
			camera->setLongExposureNoiseReduction( newValue );
			break;
		    case CFn_HighISOSpeedNoiseReduction:
			camera->setHighISOSpeedNoiseReduction( newValue );
			break;
		    case CFn_AutoFocusAssistBeamFiring:
			camera->setAutoFocusAssistBeamFiring( newValue );
			break;
		    case CFn_MirrorLockup:
			camera->setMirrorLockup( newValue );
			break;
		    case CFn_ShutterAELockButton:
			camera->setShutterAELockButton( newValue );
			break;
		}
		break;
	}
    }

    return( true );
}

bool
Controller::setPropertyCommand( int property, QString string )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    if( (property != kEdsPropID_Artist) &&
	(property != kEdsPropID_Copyright) ) {
	emit eventReport( new Event( Event::SetPropertyUnhandled, property ) );
	return( false );
    }

    const int MAX_SIZE = 64;

    char s[MAX_SIZE];
    for( int i = 0; (i < string.size()) && (i < MAX_SIZE); i++ ) {
	s[i] = string.at(i).toAscii();
    }
    s[string.size()] = '\0';

    /*
     *  Check camera model to ensure the new value is different
     *  from the current value.
     */
    if( (property == kEdsPropID_Artist) &&
	(strncmp(s, camera->getAuthorName(), MAX_SIZE) == 0) ) {
	return( true );
    }
    else
    if( (property == kEdsPropID_Copyright) &&
	(strncmp(s, camera->getCopyright(), MAX_SIZE) == 0) ) {
	return( true );
    }

    int result = EDS_ERR_OK;

    /*
     *  Retry if camera device is busy.
     */
    do {

	/*
	 *  Send the command to the camera.
	 */
	result = EdsSetPropertyData(
			camera->handle(),
			property,
			0,
			MAX_SIZE,	// size in bytes
			&s );

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy, 0,
		QString( "setProperty" ) ) );
	    QCoreApplication::processEvents();
	    Sleep( 500 );	// milliseconds
	}

    } while( result == EDS_ERR_DEVICE_BUSY );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::SetPropertyFailure,
	    property, "setProperty" ) );
	return( false );
    }

    /*
     *  Update the camera model.
     */
    if( result == EDS_ERR_OK ) {
	switch( property ) {
	    case kEdsPropID_Artist:    camera->setAuthorName( s ); break;
	    case kEdsPropID_Copyright: camera->setCopyright( s );  break;
	}
    }

    return( true );
}

bool
Controller::setPropertyCommand( int property, QDateTime dateTime )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    if( property != kEdsPropID_DateTime ) {
	emit eventReport( new Event( Event::SetPropertyUnhandled, property ) );
	return( false );
    }

    /*
     *  Check camera model to ensure the new value is different
     *  from the current value.
     */
    if( dateTime == camera->getDateTime() ) {
	return( true );
    }

    /*
     *  Send the command to the camera.
     */
    EdsTime time;

    QDate d = dateTime.date();
    time.year = d.year();
    time.month = d.month();
    time.day = d.day();

    QTime t = dateTime.time();
    time.hour = t.hour();
    time.minute = t.minute();
    time.second = t.second();

    int result = EdsSetPropertyData(
			camera->handle(),
			property,
			0,
			28,
			&time );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::SetPropertyFailure,
	    property, "setProperty" ) );
	return( false );
    }

    /*
     *  Update the camera model.
     */
    camera->setDateTime( dateTime );

    return( true );
}

bool
Controller::setPropertyCommand( int property, int *s )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    if( property != kEdsPropID_WhiteBalanceShift ) {
	emit eventReport( new Event( Event::SetPropertyUnhandled, property ) );
	return( false );
    }

    /*
     *  Send the command to the camera.
     */
    int shift[256];	// unclear how large this needs to be, but > 8
    shift[0] = s[0];
    shift[1] = s[1];
    int result = EdsSetPropertyData(
			camera->handle(),
			property,
			0,
			8,	// size of data in bytes
			&shift );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::SetPropertyFailure,
	    property, "setProperty" ) );
	return( false );
    }

    /*
     *  Commands are not accepted for a while when white
     *  balance has been shifted.
     */
    if( result == EDS_ERR_OK ) {
	Sleep( 250 );       // milliseconds
    }

    /*
     *  Update the camera model.
     */
    camera->setWhiteBalanceShift( shift );

    return( true );
}

bool
Controller::setFocusPropertyCommand( int selectionMode, int fp )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    /*
     *  Check camera model to ensure the new value is different
     *  from the current value.
     */
    EdsFocusInfo focusInfo = camera->getFocusInfo();
    if(    ((focusInfo.executeMode == Map::FP_AutomaticSelection) &&
	    (selectionMode == Map::FP_AutomaticSelection))
	|| ((focusInfo.executeMode == Map::FP_ManualSelection) &&
	    (selectionMode == Map::FP_ManualSelection) &&
	    (focusInfo.focusPoint[fp].selected == 1)) ) {
	return( true );
    }

    /*
     *  Send the command to the camera.
     *  If automatic selection mode, all focus points are selected.
     *  If manual selection mode, only one focus point is selected.
     */
    focusInfo.executeMode = selectionMode;
    const int mark = (selectionMode == Map::FP_AutomaticSelection);
    for( int i = 0; i < 128; i++ ) if( focusInfo.focusPoint[i].valid ) {
	focusInfo.focusPoint[i].selected = mark;
    }
    if( selectionMode == Map::FP_ManualSelection ) {
	focusInfo.focusPoint[fp].selected = 1;
    }

    int result = EdsSetPropertyData(
			camera->handle(),
			kEdsPropID_FocusInfo,
			0,
			4120,
			&focusInfo );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::SetPropertyFailure,
	    kEdsPropID_FocusInfo, "setFocusProperty" ) );
	return( false );
    }

    /*
     *  Update the camera model.
     */
    camera->setFocusInfo( focusInfo );

    return( true );
}

/*
 *  Command to move the zoom rectangle in live view.
 */
bool
Controller::moveZoomRectCommand( int x, int y )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    int result = EDS_ERR_OK;

    /*
     *  Check camera model to ensure the new values are
     *  different from the current values.
     */
    QPoint currentPosition = camera->getEvfZoomPosition();
    if( (currentPosition.x() == x) && (currentPosition.y() == y) ) {
	return( true );
    }

    /*
     *  Send the command to the camera.
     */
    EdsPoint point;
    point.x = x;
    point.y = y;
    result = EdsSetPropertyData(
		    camera->handle(),
		    kEdsPropID_Evf_ZoomPosition,
		    0,
		    sizeof(point),
		    &point );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::SetPropertyFailure,
		kEdsPropID_Evf_ZoomPosition, "moveZoomRect" ) );
	return( false );
    }

    /*
     *  Update the camera model.
     */
    camera->setEvfZoomPosition( QPoint(x, y) );

    return( true );
}


