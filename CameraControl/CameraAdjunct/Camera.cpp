/*
 *  Model of camera in model-view-controller design.
 */

#include <QObject>
#include "EDSDKTypes.h"
#include "Camera.h"
#include "Map.h"


/*
 *  Constructor.
 */
Camera::Camera()
{
    _handle = 0;

    initialize();
}

/*
 *  Destructor.
 */
Camera::~Camera()
{
    _handle = 0;
}

/*
 *  Initialize the camera model.
 */
void
Camera::initialize()
{
    /*
     *  Camera setting properties.
     */
    modelName[0] = '\0';
    serialNumber[0] = '\0';
    firmware[0] = '\0';
    authorName[0] = '\0';
    copyright[0] = '\0';
    dateTime = QDateTime::currentDateTime();
    autoFocusAssistBeamFiring = -1;
    batteryLevel = -1;
    downloadInProgress = false;
    inBulbMode = false;
    inISOAutoMode = true;
    exposureLevelIncrements = 0;
    flashSyncSpeedInAvMode = -1;
    highISOSpeedNoiseReduction = -1;
    longExposureNoiseReduction = -1;
    mirrorLockup = -1;
    nFP = 0;
    saveTo = -1;
    shutterAELockButton = -1;

    /*
     *  Image properties.
     */
    imageQuality = EdsImageQuality_LR;	// RAW
    imageQuality_List.n = 0;
    whiteBalance = kEdsWhiteBalance_Auto;
    whiteBalance_List.n = 0;
    pictureStyle = kEdsPictureStyle_Standard;
    pictureStyle_List.n = 0;
    wbShift[0] = 0;
    wbShift[1] = 0;

    /*
     *  Capture properties.
     */
    shootingMode = kEdsAEMode_Unknown;
    shootingMode_List.n = 0;
    driveMode = 0x00000000;	// Single-Frame Shooting
    driveMode_List.n = 0;
    ISO = 0x00;			// Auto
    ISO_List.n = 0;
    meteringMode = 3;		// Evaluative
    meteringMode_List.n = 0;
    AFMode = 0;			// One-Shot AF
    AFMode_List.n = 0;
    Av = 0x38;			// 8
    Av_List.n = 0;
    Tv = 0x60;			// 1/30
    Tv_List.n = 0;
    exposureComp = 0x00;	// 0
    exposureComp_List.n = 0;
    lensStatus = 0;
    rotation = 0;
    focusMap = 0;

    /*
     *  EVF properties.
     */
    evfOutputDevice = 0;	// not active
    evfMode = 0;
    evfAFMode = 0x00;		// Live mode
    evfAFMode_List.n = 0;
    evfDepthOfFieldPreview = 0;
    evfZoom = 1;
    evfZoomPosition.setX( 0 );
    evfZoomPosition.setY( 0 );
    evfZoomSize.setWidth( 0 );
    evfZoomSize.setHeight( 0 );
    evfCoordinateSystem.setRect( 0, 0, 5184, 3456 );
    recordMode = 0;		// not recording
}

void Camera::setModelName(    char *s ) { strncpy(modelName,s,EDS_MAX_NAME); }
void Camera::setFirmware(     char *s ) { strncpy(firmware,s,EDS_MAX_NAME); }
void Camera::setAuthorName(   char *s ) { strncpy(authorName,s,EDS_MAX_NAME); }
void Camera::setCopyright(    char *s ) { strncpy(copyright,s,EDS_MAX_NAME); }
void Camera::setSerialNumber( char *s ) { strncpy(serialNumber,s,EDS_MAX_NAME);}
void Camera::setDateTime(  QDateTime dt ) { dateTime = dt; }

char *Camera::getModelName()    { return( modelName ); }
char *Camera::getFirmware()     { return( firmware ); }
char *Camera::getAuthorName()   { return( authorName ); }
char *Camera::getCopyright()    { return( copyright ); }
char *Camera::getSerialNumber() { return( serialNumber ); }
QDateTime Camera::getDateTime() { return( dateTime ); }

void
Camera::setShootingMode( int value )
{
    shootingMode = value;

    switch( value ) {
	case kEdsAEMode_Bulb:
	case kEdsAEMode_Av:
		Tv = 0x00;	// "Auto"
		break;
	case kEdsAEMode_Manual:
		break;
	case kEdsAEMode_Tv:
		Av = 0x00;	// "Auto"
		break;
	case kEdsAEMode_Program:
	case kEdsAEMode_Green:
	case kEdsAEMode_SceneIntelligentAuto:
		Tv = 0x00;	// "Auto"
		Av = 0x00;	// "Auto"
		break;
    }
}

void Camera::setAv(           int value ) { Av = value; }
void Camera::setTv(           int value ) { Tv = value;
					    inBulbMode = (Tv == 0x0c); }
void Camera::setISO(          int value ) { ISO = value;
					    inISOAutoMode = (ISO == 0x00); }
void Camera::setMeteringMode( int value ) { meteringMode = value; }
void Camera::setAFMode(       int value ) { AFMode = value; }
void Camera::setDriveMode(    int value ) { driveMode = value; }
void Camera::setExposureComp( int value ) { exposureComp = value; }
void Camera::setImageQuality( int value ) { imageQuality = value; }
void Camera::setPictureStyle( int value ) { pictureStyle = value; }
void Camera::setWhiteBalance( int value ) { whiteBalance = value; }
void Camera::setColorSpace(   int value ) { colorSpace = value; }
void Camera::setEvfAFMode(    int value ) { evfAFMode = value; }
void Camera::setBatteryLevel( int value ) { batteryLevel = value; }
void Camera::setSaveTo(       int value ) { saveTo = value; }
void Camera::setExposureLevelIncrements(    int value ) { exposureLevelIncrements = value; }
void Camera::setFlashSyncSpeedInAvMode(     int value ) { flashSyncSpeedInAvMode = value; }
void Camera::setLongExposureNoiseReduction( int value ) { longExposureNoiseReduction = value; }
void Camera::setHighISOSpeedNoiseReduction( int value ) { highISOSpeedNoiseReduction = value; }
void Camera::setAutoFocusAssistBeamFiring(  int value ) { autoFocusAssistBeamFiring = value; }
//TODO: this should be deleted as it only works for T2i?
void Camera::setMirrorLockup(               int value ) { mirrorLockup = value; }
void Camera::setShutterAELockButton(        int value ) { shutterAELockButton = value; }
void Camera::setLensStatus(                 int value ) { lensStatus = value; }

int  Camera::getShootingMode()	{ return( shootingMode ); }
int  Camera::getAv()		{ return( Av ); }
int  Camera::getTv()		{ return( Tv ); }
int  Camera::getISO()		{ return( ISO ); }
int  Camera::getMeteringMode()	{ return( meteringMode ); }
int  Camera::getAFMode()	{ return( AFMode ); }
int  Camera::getDriveMode()	{ return( driveMode ); }
int  Camera::getExposureComp()	{ return( exposureComp ); }
int  Camera::getImageQuality()	{ return( imageQuality ); }
int  Camera::getPictureStyle()	{ return( pictureStyle ); }
int  Camera::getWhiteBalance()	{ return( whiteBalance ); }
int  Camera::getColorSpace()	{ return( colorSpace ); }
int  Camera::getEvfAFMode()	{ return( evfAFMode ); }
int  Camera::getEvfMode()	{ return( evfMode ); }
int  Camera::getEvfOutputDevice() { return( evfOutputDevice ); }
int  Camera::getBatteryLevel()	{ return( batteryLevel ); }
int  Camera::getSaveTo()	{ return( saveTo ); }
int  Camera::getExposureLevelIncrements()    { return( exposureLevelIncrements ); }
int  Camera::getFlashSyncSpeedInAvMode()     { return( flashSyncSpeedInAvMode ); }
int  Camera::getLongExposureNoiseReduction() { return( longExposureNoiseReduction ); }
int  Camera::getHighISOSpeedNoiseReduction() { return( highISOSpeedNoiseReduction ); }
int  Camera::getAutoFocusAssistBeamFiring()  { return( autoFocusAssistBeamFiring ); }
int  Camera::getMirrorLockup()		     { return( mirrorLockup ); }
int  Camera::getShutterAELockButton()	     { return( shutterAELockButton ); }
int  Camera::getLensStatus()		     { return( lensStatus ); }

void Camera::setEvfMode( int value ) { evfMode = value; }
void Camera::setEvfOutputDevice( int value ) { evfOutputDevice = (value & 0x00000003); }
void Camera::setEvfDepthOfFieldPreview( int value ) { evfDepthOfFieldPreview = value; }
int  Camera::getEvfDepthOfFieldPreview() { return( evfDepthOfFieldPreview ); }
void Camera::setEvfZoom( int value ) { evfZoom = value; }
int  Camera::getEvfZoom() { return( evfZoom ); }
void Camera::setEvfZoomPosition( const QPoint &p ) { evfZoomPosition = p; }
QPoint Camera::getEvfZoomPosition() const { return( evfZoomPosition ); }
void Camera::setEvfZoomSize( const QSize &size ) { evfZoomSize = size; }
QSize Camera::getEvfZoomSize() const { return( evfZoomSize ); }
QRect Camera::getEvfZoomRect() const { return( QRect( evfZoomPosition, evfZoomSize ) ); }
void Camera::setEvfCoordinateSystem( const QRect &rect ) { evfCoordinateSystem = rect; }
QRect Camera::getEvfCoordinateSystem() const { return( evfCoordinateSystem ); }
void Camera::setRecordMode( int value ) { recordMode = value; }
int  Camera::getRecordMode() { return( recordMode ); }
void Camera::setRotationSetting( int value ) { rotation = value; }
int  Camera::getRotationSetting() { return( rotation ); }
void Camera::setFocusMapSetting( int value ) { focusMap = value; }
int  Camera::getFocusMapSetting() { return( focusMap ); }

void Camera::setObjectsWaiting( bool b ) { downloadInProgress = b; }
bool Camera::objectsWaiting() { return( downloadInProgress ); }
bool Camera::isoInAutoMode() { return( inISOAutoMode ); }

bool
Camera::shutterInBulbMode()
{
    return( inBulbMode || (shootingMode == kEdsAEMode_Bulb) );
}

void
Camera::setWhiteBalanceShift( int *shift )
{
    wbShift[0] = shift[0];
    wbShift[1] = shift[1];
}
int *
Camera::getWhiteBalanceShift()
{
    return( wbShift );
}

void
Camera::setShootingMode_List( int n, int list[] )
{
    shootingMode_List.n = n;
    for( int i = 0; i < n; i++ ) {
	shootingMode_List.l[i] = list[i];
    }
}

void
Camera::setAv_List( int n, int list[] )
{
    Av_List.n = n;
    for( int i = 0; i < n; i++ ) {
	Av_List.l[i] = list[i];
    }
}

void
Camera::setTv_List( int n, int list[] )
{
    Tv_List.n = n;
    for( int i = 0; i < n; i++ ) {
	Tv_List.l[i] = list[i];
    }
}

void
Camera::setISO_List( int n, int list[] )
{
    ISO_List.n = n;
    for( int i = 0; i < n; i++ ) {
	ISO_List.l[i] = list[i];
    }
}

void
Camera::setMeteringMode_List( int n, int list[] )
{
    meteringMode_List.n = n;
    for( int i = 0; i < n; i++ ) {
	meteringMode_List.l[i] = list[i];
    }
}

void
Camera::setAFMode_List( int n, int list[] )
{
    AFMode_List.n = n;
    for( int i = 0; i < n; i++ ) {
	AFMode_List.l[i] = list[i];
    }
}

void
Camera::setDriveMode_List( int n, int list[] )
{
    driveMode_List.n = n;
    for( int i = 0; i < n; i++ ) {
	driveMode_List.l[i] = list[i];
    }
}

void
Camera::setExposureComp_List( int n, int list[] )
{
    exposureComp_List.n = n;
    for( int i = 0; i < n; i++ ) {
	exposureComp_List.l[i] = list[i];
    }
}

void
Camera::setImageQuality_List( int n, int list[] )
{
    imageQuality_List.n = n;
    for( int i = 0; i < n; i++ ) {
	imageQuality_List.l[i] = list[i];
    }
}

void
Camera::setPictureStyle_List( int n, int list[] )
{
    pictureStyle_List.n = n;
    for( int i = 0; i < n; i++ ) {
	pictureStyle_List.l[i] = list[i];
    }
}

void
Camera::setWhiteBalance_List( int n, int list[] )
{
    whiteBalance_List.n = n;
    for( int i = 0; i < n; i++ ) {
	whiteBalance_List.l[i] = list[i];
    }
}

void
Camera::setEvfAFMode_List( int n, int list[] )
{
    evfAFMode_List.n = n;
    for( int i = 0; i < n; i++ ) {
	evfAFMode_List.l[i] = list[i];
    }
}

void Camera::getShootingMode_List( int *n, int *list[] ) { *n = shootingMode_List.n; *list = shootingMode_List.l; }
void Camera::getAv_List(           int *n, int *list[] ) { *n = Av_List.n;	     *list = Av_List.l; }
void Camera::getTv_List(           int *n, int *list[] ) { *n = Tv_List.n;	     *list = Tv_List.l; }
void Camera::getISO_List(          int *n, int *list[] ) { *n = ISO_List.n;	     *list = ISO_List.l; } 
void Camera::getMeteringMode_List( int *n, int *list[] ) { *n = meteringMode_List.n; *list = meteringMode_List.l; }
void Camera::getAFMode_List(       int *n, int *list[] ) { *n = AFMode_List.n;	     *list = AFMode_List.l; }
void Camera::getDriveMode_List(    int *n, int *list[] ) { *n = driveMode_List.n;    *list = driveMode_List.l; }
void Camera::getExposureComp_List( int *n, int *list[] ) { *n = exposureComp_List.n; *list = exposureComp_List.l; }
void Camera::getImageQuality_List( int *n, int *list[] ) { *n = imageQuality_List.n; *list = imageQuality_List.l; }
void Camera::getPictureStyle_List( int *n, int *list[] ) { *n = pictureStyle_List.n; *list = pictureStyle_List.l; }
void Camera::getWhiteBalance_List( int *n, int *list[] ) { *n = whiteBalance_List.n; *list = whiteBalance_List.l; }
void Camera::getEvfAFMode_List(    int *n, int *list[] ) { *n = evfAFMode_List.n;    *list = evfAFMode_List.l; }

void
Camera::setFocusInfo( EdsFocusInfo value )
{
    focusInfo = value;
}

EdsFocusInfo&
Camera::getFocusInfo()
{
    return( focusInfo );
}

void
Camera::setFocusPoint_List( EdsFocusInfo value )
{
    nFP = value.pointNumber;
    for( int i = 0; i < nFP; i++ ) {
	lFP[i].setRect(
	    value.focusPoint[i].rect.point.x,
	    value.focusPoint[i].rect.point.y,
	    value.focusPoint[i].rect.size.width,
	    value.focusPoint[i].rect.size.height );
    }
}

void
Camera::getFocusPoint_List( int *n, QRect *list[] )
{
    *n = nFP;
    *list = (QRect *)&lFP;
}

void
Camera::setHandle( EdsCameraRef handle )
{
    _handle = handle;
}

EdsCameraRef
Camera::handle()
{
    return( _handle );
}

