/*
 *  Commands to open and close a session with the camera.
 */

#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include "Controller.h"
#include "Camera.h"
#include "Listener.h"
#include "Import.h"


bool
Controller::openSessionCommand()
{
    if( camera->handle() != 0 ) {
	closeSessionCommand();
    }

    listener->setInActive();
    int result = EDS_ERR_OK;

    if( !sdkLoaded ) {
	/*
	 *  Initialize the SDK.
	 */
	result = EdsInitializeSDK();
	if( result == EDS_ERR_OK ) {
	    sdkLoaded = true;
	}

	/*
	 *  Register the camera added callback function.
	 *  Invoked if the camera is plugged in after startup or
	 *  if the camera is unplugged and later plugged in again.
	 *  The handler re-establishes the connection.
	 */
	if( result == EDS_ERR_OK ) {
	    result = EdsSetCameraAddedHandler(
			Listener::cameraAddedHandler,
			(EdsVoid *)listener );
	}
    }

    /*
     *  Get list of connected cameras.
     */
    EdsCameraListRef cameraListRef;
    if( result == EDS_ERR_OK ) {
	result = EdsGetCameraList( &cameraListRef );
    }

    /*
     *  Get the number of connected cameras and
     *  check if there is at least one connected.
     */
    EdsUInt32 nCameras;
    if( result == EDS_ERR_OK ) {
	result = EdsGetChildCount( cameraListRef, &nCameras );
	if( nCameras == 0 ) {
	    result = EDS_ERR_DEVICE_NOT_FOUND;
	}
    }

    /*
     *  Get the camera at index 0.
     */
    EdsCameraRef cameraRef;
    if( result == EDS_ERR_OK ) {
	result = EdsGetChildAtIndex( cameraListRef, 0, &cameraRef );
	camera->setHandle( cameraRef );
    }

    /*
     *  Determine whether legacy camera or more modern PTP protocol.
     */
    EdsDeviceInfo deviceInfo;
    if( result == EDS_ERR_OK ) {
	result = EdsGetDeviceInfo( cameraRef, &deviceInfo );    
	if( result == EDS_ERR_OK && cameraRef == NULL ) {
	    result = EDS_ERR_DEVICE_NOT_FOUND;
	}
    }
    if( result == EDS_ERR_OK ) {
	if( deviceInfo.deviceSubType == 0 ) {
	    // legacy camera and protocol
	}
	else {
	    // PTP protocol
	}
    }

    /*
     *  Release camera list.
     */
    if( cameraListRef != NULL ) {
	EdsRelease( cameraListRef );
    }

    /*
     *  Register the property event callback function.
     */
    if( result == EDS_ERR_OK ) {
	result = EdsSetPropertyEventHandler(
		    camera->handle(),
		    kEdsPropertyEvent_All,
		    Listener::propertyEventHandler,
		    (EdsVoid *)listener );
    }

    /*
     *  Register the object event callback function.
     */
    if( result == EDS_ERR_OK ) {
	result = EdsSetObjectEventHandler(
		    camera->handle(),
		    kEdsObjectEvent_All,
		    &Listener::objectEventHandler,
		    (EdsVoid *)listener );
    }

    /*
     *  Register the camera state event callback function.
     */
    if( result == EDS_ERR_OK ) {
	result = EdsSetCameraStateEventHandler(
		    camera->handle(),
		    kEdsStateEvent_All,
		    &Listener::stateEventHandler,
		    (EdsVoid *)listener );
    }

    /*
     *  Open a session with the camera.
     */
    if( result == EDS_ERR_OK ) {
	result = EdsOpenSession( camera->handle() );
    }

    /*
     *  Set where to save images. Default is to save on computer.
     */
    if( result == EDS_ERR_OK ) {
	setPropertyCommand( kEdsPropID_SaveTo, import->getSaveTo() );
    }

    if( result != EDS_ERR_OK ) {
	camera->setHandle( 0 );
	return( false );
    }

    listener->setActive();
    initializeCamera();

    return( true );
}

bool
Controller::closeSessionCommand()
{
    if( camera->handle() != 0 ) {
	EdsCloseSession( camera->handle() );
	EdsRelease( camera->handle() );
	camera->setHandle( 0 );
    }

    if( sdkLoaded ) {
	EdsTerminateSDK();
	sdkLoaded = false;
    }

    listener->setInActive();

    return( true );
}

