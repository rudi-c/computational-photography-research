/*
 *  Model of camera in model-view-controller design.
 */
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <QObject>
#include <QRect>
#include <QDateTime>
#include "EDSDKTypes.h"

class Camera : public QObject
{
    public:
	Camera();
	~Camera();
	void initialize();

	/*
	 *  Camera properties.
	 */
	void setModelName( char *s );
	void setSerialNumber( char *s );
	void setFirmware( char *s );
	void setAuthorName( char *s );
	void setCopyright( char *s );
	void setDateTime( QDateTime dateTime );
	void setSaveTo( int value );
	void setBatteryLevel( int value );
	void setExposureLevelIncrements( int value );
	void setFlashSyncSpeedInAvMode( int value );
	void setLongExposureNoiseReduction( int value );
	void setHighISOSpeedNoiseReduction( int value );
	void setAutoFocusAssistBeamFiring( int value );
	void setMirrorLockup( int value );
	void setShutterAELockButton( int value );
	void setFocusPoint_List( EdsFocusInfo value );
	void setObjectsWaiting( bool b );

	char *getModelName();
	char *getSerialNumber();
	char *getFirmware();
	char *getAuthorName();
	char *getCopyright();
	QDateTime getDateTime();
	int  getSaveTo();
	int  getBatteryLevel();
	int  getExposureLevelIncrements();
	int  getFlashSyncSpeedInAvMode();
	int  getLongExposureNoiseReduction();
	int  getHighISOSpeedNoiseReduction();
	int  getAutoFocusAssistBeamFiring();
	int  getMirrorLockup();
	int  getShutterAELockButton();
	void getFocusPoint_List( int *n, QRect *list[] );
	bool objectsWaiting();
	bool shutterInBulbMode();
	bool isoInAutoMode();

	/*
	 *  Image properties.
	 */
	void setFocusInfo( EdsFocusInfo value );
	void setImageQuality( int value );
	void setImageQuality_List( int n, int list[] );
	void setWhiteBalance( int value );
	void setWhiteBalance_List( int n, int list[] );
	void setColorSpace( int value );
	void setPictureStyle( int value );
	void setPictureStyle_List( int n, int list[] );
	void setWhiteBalanceShift( int *shift );

	EdsFocusInfo& getFocusInfo();
	int  getImageQuality();
	void getImageQuality_List( int *n, int *list[] );
	int  getWhiteBalance();
	void getWhiteBalance_List( int *n, int *list[] );
	int  getColorSpace();
	int  getPictureStyle();
	void getPictureStyle_List( int *n, int *list[] );
	int  *getWhiteBalanceShift();

	/*
	 *  Capture properties.
	 */
	void setShootingMode( int value );
	void setShootingMode_List( int n, int list[] );
	void setDriveMode( int value );
	void setDriveMode_List( int n, int list[] );
	void setISO( int value );
	void setISO_List( int n, int list[] );
	void setMeteringMode( int value );
	void setMeteringMode_List( int n, int list[] );
	void setAFMode( int value );
	void setAFMode_List( int n, int list[] );
	void setAv( int value );
	void setAv_List( int n, int list[] );
	void setTv( int value );
	void setTv_List( int n, int list[] );
	void setExposureComp( int value );
	void setExposureComp_List( int n, int list[] );
	void setLensStatus( int value );

	int  getShootingMode();
	void getShootingMode_List( int *n, int *list[] );
	int  getDriveMode();
	void getDriveMode_List( int *n, int *list[] );
	int  getISO();
	void getISO_List( int *n, int *list[] );
	int  getMeteringMode();
	void getMeteringMode_List( int *n, int *list[] );
	int  getAFMode();
	void getAFMode_List( int *n, int *list[] );
	int  getAv();
	void getAv_List( int *n, int *list[] );
	int  getTv();
	void getTv_List( int *n, int *list[] );
	int  getExposureComp();
	void getExposureComp_List( int *n, int *list[] );
	int  getLensStatus();

	/*
	 *  Evf properties.
	 */
	void setEvfAFMode( int value );
	void setEvfAFMode_List( int n, int list[] );
	void setEvfDepthOfFieldPreview( int value );
	void setEvfMode( int value );
	void setEvfOutputDevice( int value );
	void setEvfZoom( int value );
	void setRecordMode( int value );
	void setRotationSetting( int value );
	void setFocusMapSetting( int value );
	void setEvfCoordinateSystem( const QRect &rect ); // width and height
	void setEvfZoomPosition( const QPoint &p ); // top left coordinates
	void setEvfZoomSize( const QSize &size );   // width and height

	int  getEvfAFMode();
	void getEvfAFMode_List( int *n, int *list[] );
	int  getEvfDepthOfFieldPreview();
	int  getEvfMode();
	int  getEvfOutputDevice();
	int  getEvfZoom();
	int  getRecordMode();
	int  getRotationSetting();
	int  getFocusMapSetting();
	QRect getEvfCoordinateSystem() const;
	QPoint getEvfZoomPosition() const;
	QRect getEvfZoomRect() const;
	QSize getEvfZoomSize() const;

	/*
	 *  Camera handle.
	 */
	void setHandle( EdsCameraRef handle );
	EdsCameraRef handle();

    protected:
	/*
	 *  Camera properties.
	 */
	char modelName[EDS_MAX_NAME];		// read
	char serialNumber[EDS_MAX_NAME];	// read
	char firmware[EDS_MAX_NAME];		// read
	char authorName[EDS_MAX_NAME];		// read/write
	char copyright[EDS_MAX_NAME];		// read/write
	QDateTime dateTime;			// read/write
	int  saveTo;				// read/write
	int  batteryLevel;			// read
	int  exposureLevelIncrements;		// read/write
	int  iSOexpansion;			// read/write
	int  flashSyncSpeedInAvMode;		// read/write
	int  longExposureNoiseReduction;	// read/write
	int  highISOSpeedNoiseReduction;	// read/write
	int  autoFocusAssistBeamFiring;		// read/write
	int  mirrorLockup;			// read/write
	int  shutterAELockButton;		// read/write
	bool downloadInProgress;
	bool inBulbMode;
	bool inISOAutoMode;

	/*
	 *  Image properties.
	 */
	EdsFocusInfo focusInfo;		// read/write
	int  imageQuality;		// read/write
	int  whiteBalance;		// read/write
	int  colorSpace;		// read/write
	int  pictureStyle;		// read/write
	int  wbShift[2];		// read/write

	/*
	 *  Capture properties.
	 */
	int  shootingMode;		// read only (on T2i)
	int  driveMode;			// read/write
	int  ISO;			// read/write
	int  meteringMode;		// read/write
	int  AFMode;			// read/write
	int  Av;			// read/write
	int  Tv;			// read/write
	int  exposureComp;		// read/write
	int  lensStatus;		// read

	/*
	 *  Evf properties.
	 */
	uint evfOutputDevice;		// not used outside Controller
	int  evfMode;			// not used outside Controller
	int  evfAFMode;			// read/write
	int  evfDepthOfFieldPreview;	// read/write
	int  evfZoom;			// write
	QPoint evfZoomPosition;		// write
	QSize evfZoomSize;		// read
	QRect evfCoordinateSystem;	// read
	int  recordMode;		// read/write
	int  rotation;
	int  focusMap;

	/*
	 *  Reference to EDSDK internal structure.
	 */
	EdsCameraRef _handle;

	/*
	 *  Store list of possible values for a varying property.
	 *  These lists come from the camera, and depend on the current
	 *  settings of AEMode, lens on camera, and so on.
	 */
	typedef struct {
	    int  n;		// number of elements
	    int  l[128];	// elements
	} ListType;
	ListType shootingMode_List;
	ListType Av_List;
	ListType Tv_List;
	ListType ISO_List;
	ListType meteringMode_List;
	ListType AFMode_List;
	ListType driveMode_List;
	ListType exposureComp_List;
	ListType imageQuality_List;
	ListType pictureStyle_List;
	ListType whiteBalance_List;
	ListType evfAFMode_List;

	/*
	 *  Store number and coordinates of focus points.
	 */
	int  nFP;
	QRect lFP[128];
};

#endif // _CAMERA_H_
