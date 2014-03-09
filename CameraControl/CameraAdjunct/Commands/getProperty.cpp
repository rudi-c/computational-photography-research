/*
 *  Commands to retrieve properties from the camera:
 *	getPropertyCommand( EdsPropertyID property, int parameter )
 *	getFocusPropertyCommand()
 *	getPropertyListCommand( EdsPropertyID property )
 * If a change is made directly at the camera, the camera will signal via
 * the listener that a value or a list of values should be retrieved.
 */

#include <QtGui>
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include "Controller.h"
#include "Camera.h"
#include "LiveImageData.h"
#include "CustomFunction.h"
#include "Event.h"


bool
Controller::getPropertyCommand( EdsPropertyID property, int parameter )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    uint  data = 0;
    int  result = EDS_ERR_OK;
    char s[EDS_MAX_NAME];
    EdsFocusInfo focusInfo;
    EdsTime time;	// for kEdsPropID_DateTime
    int shift[256];	// for kEdsPropID_WhiteBalanceShift

    /*
     *  Retry if camera device is busy.
     */
    do {

	/*
	 *  Request value for changed property from the camera.
	 */
	switch( property ) {
	    case kEdsPropID_CFn:	  // parameter != 0
	    case kEdsPropID_BatteryLevel: // parameter == 0 from here down
	    case kEdsPropID_LensStatus:
	    case kEdsPropID_AEMode:
	    case kEdsPropID_AEModeSelect:
	    case kEdsPropID_Av:
	    case kEdsPropID_Tv:
	    case kEdsPropID_ISOSpeed:
	    case kEdsPropID_MeteringMode:
	    case kEdsPropID_WhiteBalance:
	    case kEdsPropID_ColorSpace:
	    case kEdsPropID_ImageQuality:
	    case kEdsPropID_PictureStyle:

	    case kEdsPropID_DriveMode:
	    case kEdsPropID_AFMode:
	    case kEdsPropID_ExposureCompensation:
	    case kEdsPropID_Evf_Mode:
	    case kEdsPropID_Evf_OutputDevice:
	    case kEdsPropID_Evf_DepthOfFieldPreview:
	    case kEdsPropID_Evf_AFMode:
	    case kEdsPropID_Record:
	    case kEdsPropID_SaveTo:
	        result = EdsGetPropertyData(
				camera->handle(),
				property,
				parameter,
				4,	// size of data in bytes
				&data );
	        break;
	    case kEdsPropID_Artist:
	    case kEdsPropID_BodyIDEx:
	    case kEdsPropID_Copyright:
	    case kEdsPropID_FirmwareVersion:
	    case kEdsPropID_ProductName:
		EdsDataType dataType;
		EdsUInt32   dataSize;
		EdsGetPropertySize(
				camera->handle(),
				property,
				0,
				&dataType,
				&dataSize );
	        result = EdsGetPropertyData(
				camera->handle(),
				property,
				0,
				dataSize,	// size in bytes
				&s );
	        break;
	    case kEdsPropID_DateTime:
	        result = EdsGetPropertyData(
				camera->handle(),
				property,
				0,
				28,	// size of data in bytes
				&time );
	        break;
	    case kEdsPropID_WhiteBalanceShift:
	        result = EdsGetPropertyData(
				camera->handle(),
				property,
				parameter,
				8,	// size of data in bytes
				&shift );
	        break;
	    case kEdsPropID_FocusInfo:
	        result = EdsGetPropertyData(
				camera->handle(),
				property,
				0,
				4120,	// size of focusInfo in bytes
				&focusInfo );
	        break;
	    /*
	     *  Ignored cases.
	     */
	    // storage media in camera: CF, SD, or HDD
	    case kEdsPropID_CurrentStorage:
	    case kEdsPropID_CurrentFolder:
	    case kEdsPropID_LensName:
	    case kEdsPropID_AvailableShots:
	    case kEdsPropID_PictureStyleDesc:
	    // does not contain useful values for T2i
	    case kEdsPropID_DepthOfField:
	    // live view white balance mirrors regular white balance
	    case kEdsPropID_Evf_WhiteBalance:
	    // reading/writing flash compensation not supported in T2i
	    case kEdsPropID_FlashCompensation:
	    // possible to set AEBracket, which in turn sets Bracket,
	    // but it only takes a sequence of three shots
	    case kEdsPropID_Bracket:
	    case kEdsPropID_AEBracket:
		return( true );
	    default:
		emit eventReport( new Event( Event::GetPropertyUnhandled,
			property ) );
	        return( false );
	} // switch

	if( result == EDS_ERR_OK ) {

	    /*
	     *  Update the model.
	     */
	    switch( property ) {
		case kEdsPropID_BatteryLevel:
		    camera->setBatteryLevel( data );
		    break;
		case kEdsPropID_LensStatus:
		    camera->setLensStatus( data );
		    break;
		case kEdsPropID_AEMode:
		case kEdsPropID_AEModeSelect:
		    camera->setShootingMode( data );
		    break;
		case kEdsPropID_Av:
		    camera->setAv( data );
		    break;
		case kEdsPropID_Tv:
		    camera->setTv( data );
		    break;
		case kEdsPropID_ISOSpeed:
		    camera->setISO( data );
		    break;
		case kEdsPropID_MeteringMode:
		    camera->setMeteringMode( data );
		    break;
		case kEdsPropID_DriveMode:
		    camera->setDriveMode( data );
		    break;
		case kEdsPropID_AFMode:
		    camera->setAFMode( data );
		    break;
		case kEdsPropID_ExposureCompensation:
		    camera->setExposureComp( data );
		    break;
		case kEdsPropID_ImageQuality:
		    camera->setImageQuality( data );
		    break;
		case kEdsPropID_PictureStyle:
		    camera->setPictureStyle( data );
		    break;
		case kEdsPropID_WhiteBalance:
		    camera->setWhiteBalance( data );
		    break;
		case kEdsPropID_ColorSpace:
		    camera->setColorSpace( data );
		    break;
		case kEdsPropID_Evf_Mode:
		    camera->setEvfMode( data );
		    break;
		case kEdsPropID_Evf_OutputDevice:
		    camera->setEvfOutputDevice( data );
		    break;
		case kEdsPropID_Evf_DepthOfFieldPreview:
		    camera->setEvfDepthOfFieldPreview( data );
		    break;
		case kEdsPropID_Evf_AFMode:
		    camera->setEvfAFMode( data );
		    break;
		case kEdsPropID_Record:
		    camera->setRecordMode( data );
		    break;
		case kEdsPropID_SaveTo:
		    camera->setSaveTo( data );
		    break;
		case kEdsPropID_CFn:
		    switch( parameter ) {
			case CFn_ExposureLevelIncrements:
			    camera->setExposureLevelIncrements( data );
			    break;
			case CFn_FlashSyncSpeedInAvMode:
			    camera->setFlashSyncSpeedInAvMode( data );
			    break;
			case CFn_LongExposureNoiseReduction:
			    camera->setLongExposureNoiseReduction( data );
			    break;
			case CFn_HighISOSpeedNoiseReduction:
			    camera->setHighISOSpeedNoiseReduction( data );
			    break;
			case CFn_AutoFocusAssistBeamFiring:
			    camera->setAutoFocusAssistBeamFiring( data );
			    break;
			case CFn_MirrorLockup:
			    camera->setMirrorLockup( data );
			    break;
			case CFn_ShutterAELockButton:
			    camera->setShutterAELockButton( data );
			    break;
		    }
		    break;
		case kEdsPropID_FocusInfo:
		    camera->setFocusInfo( focusInfo );
		    break;
		case kEdsPropID_WhiteBalanceShift:
		    camera->setWhiteBalanceShift( shift );
		    break;
		case kEdsPropID_Artist:
		    camera->setAuthorName( s );
		    break;
		case kEdsPropID_BodyIDEx:
		    camera->setSerialNumber( s );
		    break;
		case kEdsPropID_Copyright:
		    camera->setCopyright( s );
		    break;
		case kEdsPropID_FirmwareVersion:
		    camera->setFirmware( s );
		    break;
		case kEdsPropID_ProductName:
		    camera->setModelName( s );
		    break;
		case kEdsPropID_DateTime:
		    camera->setDateTime(QDateTime(
				QDate( time.year, time.month, time.day ),
				QTime( time.hour, time.minute, time.second )));
		    break;
	    } // switch( property )

	    /*
	     *  Let the view know the model has changed.
	     */
	    emit propertyChanged( property, parameter );

	} // if ok

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy, 0,
		QString( "getProperty" ) ) );
	    QCoreApplication::processEvents();
	    Sleep( 500 );	// milliseconds
	}

    } while( result == EDS_ERR_DEVICE_BUSY );

    if( (result != EDS_ERR_OK) &&
	(property != kEdsPropID_FocusInfo) ) {
	emit eventReport( new Event( Event::GetPropertyFailure,
	    property, "getProperty" ) );
	return( false );
    }

    return( true );
}

/*
 *  Get focus point properties, which are only available
 *  in Quick mode. Called once at initialization.
 */
bool
Controller::getFocusPropertyCommand()
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    int saveEvfAFMode;
    int result = EDS_ERR_OK;
    EdsFocusInfo focusInfo;

    /*
     *  Retry if camera device is busy.
     */
    do {

	/*
	 *  Camera must be in Quick mode to retrieve focus points.
	 */
	saveEvfAFMode = camera->getEvfAFMode();
	setPropertyCommand(
			kEdsPropID_Evf_AFMode,
			EvfImageData::AFMODE_Quick );
	result = EdsGetPropertyData(
			camera->handle(),
			kEdsPropID_FocusInfo,
			0,
			4120,	// size of focusInfo in bytes
			&focusInfo );

	setPropertyCommand( kEdsPropID_Evf_AFMode, saveEvfAFMode );

	if( result == EDS_ERR_OK ) {
	    camera->setFocusInfo( focusInfo );
	    camera->setFocusPoint_List( focusInfo );
	}

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy, 0,
		QString( "getFocusProperty" ) ) );
	    QCoreApplication::processEvents();
	    Sleep( 500 );	// milliseconds
	}

    } while( result == EDS_ERR_DEVICE_BUSY );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::GetPropertyFailure,
		kEdsPropID_FocusInfo, "getFocusProperty" ) );
	return( false );
    }

    return( true );
}

/*
 *  For a given configurable property, command to get from the
 *  camera the list of currently valid choices.
 */
bool
Controller::getPropertyListCommand( EdsPropertyID property )
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    int  result = EDS_ERR_OK;
    EdsPropertyDesc properties = {0};

    /*
     *  Retry if camera device is busy.
     */
    do {

	/*
	 *  Request list of values for changed property from the camera.
	 */
	switch( property ) {
	    case kEdsPropID_AEMode:
	    case kEdsPropID_AEModeSelect:
	    case kEdsPropID_Av:
	    case kEdsPropID_Tv:
	    case kEdsPropID_ISOSpeed:
	    case kEdsPropID_MeteringMode:
	    case kEdsPropID_WhiteBalance:
	    case kEdsPropID_ImageQuality:
	    case kEdsPropID_PictureStyle:
	    case kEdsPropID_DriveMode:
	    case kEdsPropID_AFMode:
	    case kEdsPropID_ExposureCompensation:
	    case kEdsPropID_Evf_AFMode:
		result = EdsGetPropertyDesc(
				camera->handle(),
				property,
				&properties );
		break;
	    /*
	     *  Ignored cases.
	     */
	    // possibilities are known
	    case kEdsPropID_Evf_OutputDevice:
	    case kEdsPropID_Evf_Mode:
	    // two possibilities are known
	    case kEdsPropID_ColorSpace:
	    // does not contain useful values for T2i
	    case kEdsPropID_DepthOfField:
	    // Evf WB mirrors regular WB
	    case kEdsPropID_Evf_WhiteBalance:
	    // possible to set AEBracket, which in turn sets Bracket,
	    // but it only takes a sequence of three shots.
	    case kEdsPropID_AEBracket:
		return( true );
	    default:
		emit eventReport( new Event( Event::GetPropertyListUnhandled,
			property ) );
		return( false );
	} // switch

	if( result == EDS_ERR_OK ) {

	    int n = properties.numElements;
	    int v[128];
	    for( int i = 0; i < n; i++ ) {
		v[i] = properties.propDesc[i];
	    }

	    /*
	     *  Update the model.
	     */
	    switch( property ) {
		case kEdsPropID_AEMode:
		case kEdsPropID_AEModeSelect:
		    camera->setShootingMode_List( n, v );
		    break;
		case kEdsPropID_Av:
		    camera->setAv_List( n, v );
		    break;
		case kEdsPropID_Tv:
		    camera->setTv_List( n, v );
		    break;
		case kEdsPropID_ISOSpeed:
		    camera->setISO_List( n, v );
		    break;
		case kEdsPropID_MeteringMode:
		    camera->setMeteringMode_List( n, v );
		    break;
		case kEdsPropID_WhiteBalance:
		    camera->setWhiteBalance_List( n, v );
		    break;
		case kEdsPropID_ImageQuality:
		    camera->setImageQuality_List( n, v );
		    break;
		case kEdsPropID_PictureStyle:
		    camera->setPictureStyle_List( n, v );
		    break;
		case kEdsPropID_DriveMode:
		    camera->setDriveMode_List( n, v );
		    break;
		case kEdsPropID_AFMode:
		    camera->setAFMode_List( n, v );
		    break;
		case kEdsPropID_ExposureCompensation:
		    camera->setExposureComp_List( n, v );
		    break;
		case kEdsPropID_Evf_AFMode:
		    camera->setEvfAFMode_List( n, v );
		    break;
	    }

	    /*
	     *  Let the view know the model has changed.
	     */
	    emit propertyListChanged( property );

	} // if ok

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy, 0,
		QString( "getPropertyList" ) ) );
	    QCoreApplication::processEvents();
	    Sleep( 500 );	// milliseconds
	}

    } while( result == EDS_ERR_DEVICE_BUSY );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::GetPropertyFailure,
		property, "getPropertyList" ) );
	return( false );
    }

    return( true );
}


/*
*********************************************
Data sizes and types
*********************************************
kEdsPropID_CurrentFolder	dataType: String	dataSize: 16
EdsDataType dataType;
EdsUInt32   dataSize;
EdsGetPropertySize( camera->handle(),
kEdsPropID_CurrentFolder, 0, &dataType, &dataSize );
*/

