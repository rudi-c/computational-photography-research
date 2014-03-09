/*
 *  Controller in model-view-controller design.
 */

#include <QMetaType>
#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "Controller.h"
#include "Camera.h"
#include "ShotSeq.h"
#include "Import.h"
#include "Listener.h"
#include "CustomFunction.h"
#include "Event.h"
#include "Map.h"


/*
 *  Constructor for class.
 */
Controller::Controller( Camera *camera, Import *import )
{
    sdkLoaded = false;
    doLive = false;
    bulbLock = false;
    evfImageData = NULL;
    shotSeq = NULL;

    this->camera = camera;
    this->import = import;

    listener = new Listener;
    qRegisterMetaType<EdsPropertyEvent>("EdsPropertyEvent");
    qRegisterMetaType<EdsPropertyID>("EdsPropertyID");
    qRegisterMetaType<EdsUInt32>("EdsUInt32");
    qRegisterMetaType<EdsObjectEvent>("EdsObjectEvent");
    qRegisterMetaType<EdsBaseRef>("EdsBaseRef");
    qRegisterMetaType<EdsStateEvent>("EdsStateEvent");
    QObject::connect(
	listener, SIGNAL(propertyEvent(EdsPropertyEvent,EdsPropertyID,EdsUInt32)),
	this, SLOT(handlePropertyEvent(EdsPropertyEvent,EdsPropertyID,EdsUInt32)));

    QObject::connect(
	listener, SIGNAL(objectEvent(EdsObjectEvent,EdsBaseRef)),
	this, SLOT(handleObjectEvent(EdsObjectEvent,EdsBaseRef)));

    QObject::connect(
	listener, SIGNAL(stateEvent(EdsStateEvent,EdsUInt32)),
	this, SLOT(handleStateEvent(EdsStateEvent,EdsUInt32)));

    QObject::connect(
	listener, SIGNAL(cameraAddedEvent()),
	this, SLOT(handleCameraAddedEvent()));

    //qRegisterMetaType<Event>("Event");
    QObject::connect(
	listener, SIGNAL(eventReport(Event *)),
	this, SIGNAL(eventReport(Event *)));

    timer = new QTimer(this);
    QObject::connect(
	timer, SIGNAL(timeout()),
	this, SLOT(messagePump()) );
    timer->start( Controller::timerPeriod );
}

/*
 *  Destructor for class.
 */
Controller::~Controller()
{
    closeSessionCommand();
    timer->stop();
    delete timer;
    delete listener;
}

const int Controller::initValueOnly[] = {
    kEdsPropID_ProductName,
    kEdsPropID_Artist,
    kEdsPropID_Copyright,
    kEdsPropID_BodyIDEx,
    kEdsPropID_FirmwareVersion,
    kEdsPropID_DateTime,
    kEdsPropID_BatteryLevel,
    kEdsPropID_ColorSpace,
    kEdsPropID_WhiteBalanceShift,
    kEdsPropID_Evf_Mode,
    kEdsPropID_Evf_OutputDevice,
    kEdsPropID_Evf_DepthOfFieldPreview,
    kEdsPropID_SaveTo,
    kEdsPropID_AFMode,
    kEdsPropID_Evf_AFMode,
    -1
};

const int Controller::initValueAndList[] = {
    kEdsPropID_AEMode,
    kEdsPropID_Av,
    kEdsPropID_ISOSpeed,
    kEdsPropID_ImageQuality,
    kEdsPropID_MeteringMode,
    kEdsPropID_PictureStyle,
    kEdsPropID_Tv,
    kEdsPropID_WhiteBalance,
    kEdsPropID_DriveMode,
    kEdsPropID_AFMode,
    kEdsPropID_Evf_AFMode,
    kEdsPropID_ExposureCompensation,
    -1
};

const int Controller::initCFnValueOnly[] = {
    CFn_ExposureLevelIncrements,
    //CFn_MirrorLockup,
    //CFn_HighISOSpeedNoiseReduction,
    //CFn_LongExposureNoiseReduction,
    -1
};

/*
 *  A session has been established so initialize the camera model
 *  by querying all the properties of interest.
 */
void
Controller::initializeCamera()
{
    for( int i = 0; initValueOnly[i] != -1; i++ ) {
	getPropertyCommand( initValueOnly[i] );
    }

    for( int i = 0; initValueAndList[i] != -1; i++ ) {
	getPropertyCommand( initValueAndList[i] );
	getPropertyListCommand( initValueAndList[i] );
    }

    for( int i = 0; initCFnValueOnly[i] != -1; i++ ) {
	getPropertyCommand( kEdsPropID_CFn, initCFnValueOnly[i] );
    }

   /*
    *  Comes last as relies on knowing the value of Evf_AFMode.
    */
    getFocusPropertyCommand();
}

/*
 *  SDK relies on Microsoft Window's message pump.
 */
void
Controller::messagePump()
{
    MSG msg;

    while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) != 0 ) {
	TranslateMessage( &msg );
	DispatchMessage( &msg );
    }
}

/*
 *  Handle property events from the camera.
 */
void
Controller::handlePropertyEvent(
	EdsPropertyEvent inEvent,
	EdsPropertyID	 inPropertyID,
	EdsUInt32	 inParam )
{
    if( inEvent == kEdsPropertyEvent_PropertyChanged ) {
	/*
	 *  A camera property has changed.
	 */
	getPropertyCommand( inPropertyID, inParam );
    }
    else
    if( inEvent == kEdsPropertyEvent_PropertyDescChanged ) {
	/*
	 *  A list of configurable camera properties has changed.
	 */
	getPropertyListCommand( inPropertyID );
    }
}

void
Controller::handleObjectEvent(
	EdsObjectEvent	inEvent,
	EdsBaseRef	inRef )
{
    switch( inEvent ) {
	/*
	 *  Notifies that the volume (memory card) has been:
	 *    1. added
	 *    2. removed
	 *    3. changed
	 *    4. formatted
	 *  Use EdsGetVolumeInfo to get volume information.
	 *  Get subitems of the designated volume again as needed.
	 */
	case kEdsObjectEvent_VolumeAdded:
	case kEdsObjectEvent_VolumeRemoved:
	case kEdsObjectEvent_VolumeInfoChanged:
	case kEdsObjectEvent_VolumeUpdateItems:
	    //static_cast<EdsVolumeRef>(inRef)
	    EdsRelease( inRef );
	    break;
	/*
	 *  Notifies that images were deleted in a designated folder on camera.
	 *  If notification of this event is received, get sub-items of the
	 *  designated folder again as needed. Changed folders (specifically,
	 *  directory item objects) can be retrieved from event data.
	 */
	case kEdsObjectEvent_FolderUpdateItems:
	    break;
	/*
	 *  Notifies of the creation of objects such as new folders or files,
	 *  such as a movie file, on a camera compact flash card or the like.
	 *  Newly created objects are indicated by event data.
	 */
	case kEdsObjectEvent_DirItemCreated:
	    if( camera->getSaveTo() == kEdsSaveTo_Camera ) {
		registerFile( static_cast<EdsDirectoryItemRef>(inRef) );
	    }
	    else
	    if( isMovieFile( static_cast<EdsDirectoryItemRef>(inRef) ) ) {
		downloadFile( static_cast<EdsDirectoryItemRef>(inRef), true );
	    }
	    else {
		EdsRelease( inRef );
	    }
	    break;
	/*
	 *  Notifies of the deletion of objects such as folders or files on a
	 *  camera compact flash card or the like. Deleted objects are indicated
	 *  in event data.
	 */
	case kEdsObjectEvent_DirItemRemoved:
	    break;
	/*
	 *  Notifies that information of DirItem objects has been
	 *  changed. Changed objects are indicated by event data. The changed
	 *  value can be retrieved by means of EdsGetDirectoryItemInfo.
	 */
	case kEdsObjectEvent_DirItemInfoChanged:
	    break;
	/*
	 *  Notifies that header information has been updated, as for rotation
	 *  information of image files on the camera. If this event is received,
	 *  get the file header information again, as needed.
	 */
	case kEdsObjectEvent_DirItemContentChanged:
	    break;
	/*
	 *  Notifies that there are objects on a camera to be transferred to
	 *  a computer. This event is generated after remote release from a
	 *  computer or local release from a camera.
	 */
	case kEdsObjectEvent_DirItemRequestTransfer:
	    downloadFile( static_cast<EdsDirectoryItemRef>(inRef) );
	    break;
	/*
	 *  Notifies that camera's direct transfer button was pressed.
	 */
	case kEdsObjectEvent_DirItemRequestTransferDT:
	    EdsDownloadCancel( static_cast<EdsDirectoryItemRef>(inRef) );
	    EdsRelease( inRef );
	    break;
	/*
	 *  Notifies that camera's direct transfer cancel button was pressed.
	 */
	case kEdsObjectEvent_DirItemCancelTransferDT:
	    // ignore
	    break;
    }
}

void
Controller::handleStateEvent(
	EdsStateEvent	inEvent,
	EdsUInt32	inEventData )
{
    switch( inEvent ) {
	/*
	 *  Camera has lost connection to host computer, either because
	 *  it was disconnected by unplugging a cord, opening the
	 *  compact flash compartment, turning the camera off, auto
	 *  shut-off, or by other means.
	 */
	case kEdsStateEvent_Shutdown:
	    if( camera->handle() != 0 ) {
		stopLiveView();
		EdsCloseSession( camera->handle() );
		EdsRelease( camera->handle() );
		camera->setHandle( 0 );
	    }
	    // signal view of change in connection status
	    camera->initialize();
	    emit propertyChanged( kEdsPropID_ProductName, 0 );
	    emit eventReport( new Event( Event::ConnectionLost ) );
	    break;
	/*
	 *  Notifies of whether or not (0/1) there are objects waiting to
	 *  be transferred to the host computer. This is useful when
	 *  ensuring all shot images have been transferred when the
	 *  application is closed. A value of 1 is passed initially
	 *  (whether it is a single shot or a burst of shots to download),
	 *  followed by a 0 when downloading has completed.
	 */
	case kEdsStateEvent_JobStatusChanged:
	    camera->setObjectsWaiting( inEventData );
	    break;
	/*
	 *  Notifies that the camera will shut down after a specific period. 
	 *  Generated only if auto shut-off is set. Exactly when notification
	 *  is issued (that is, the number of seconds until shutdown) varies
	 *  depending on the camera model. If the camera is allowed to shut
	 *  down, the connection will be lost and can only be reestablished
	 *  by pressing the camera's shutter button half-way (or pressing
	 *  menu, disp). To continue operation without having the camera
	 *  shut down, use EdsSendCommand to extend the auto shut-off timer.
	 *  inEventData contains the number of seconds to shutdown.
	 */
	case kEdsStateEvent_WillSoonShutDown:
	    EdsSendCommand( camera->handle(),
			    kEdsCameraCommand_ExtendShutDownTimer,
			    0 );
	    break;
	/*
	 *  As the counterpart event to kEdsStateEvent_WillSoonShutDown,
	 *  this event notifies of updates to the number of seconds until
	 *  a camera shuts down. After the update, the period until shutdown
	 *  is model-dependent. Not all cameras notify of this event.
	 */
	case kEdsStateEvent_ShutDownTimerUpdate:
	    // ignore
	    break;
	/*
	 *  Notifies that a requested release has failed, due to focus
	 *  failure or similar factors: inEventData contains error code.
	 *  Hasn't occurred yet.
	 */
	case kEdsStateEvent_CaptureError:
	    emit eventReport( new Event( Event::CaptureFailure, inEventData ) );
	    break;
	/*
	 *  Notifies of internal SDK errors. If this error event is received,
	 *  the issuing device will probably not be able to continue working
	 *  properly, so cancel the remote connection.
	 */
	case kEdsStateEvent_InternalError:
	    stopLiveView();
	    closeSessionCommand();
	    camera->initialize();
	    // signal view of change in connection status
	    emit propertyChanged( kEdsPropID_ProductName, 0 );
	    emit eventReport( new Event( Event::ConnectionLost ) );
	    break;
	/*
	 *  Not documented and hasn't occurred yet.
	 */
	case kEdsStateEvent_AfResult:
	    emit eventReport(
		new Event( Event::AFResultUnhandled, inEventData ) );
	    break;
	/*
	 *  Notifies of the exposure time during bulb shooting. Events are
	 *  issued in about one-second intervals during bulb shooting. This
	 *  event is only issued when bulb shooting is started on the computer.
	 *  inEventData contains the exposure time in seconds.
	 */
	case kEdsStateEvent_BulbExposureTime:
	    emit eventReport(
		new Event( Event::BulbExposureTime, inEventData ) );
	    break;
    }
}

void
Controller::handleCameraAddedEvent()
{
    if( openSessionCommand() ) {
	messagePump();
    }
}

