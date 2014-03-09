/*
 *  Controller in model-view-controller design.
 */
#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QTime>
#include <QImage>
#include "EDSDKTypes.h"

class Camera;
class Event;
class ShotSeq;
class Import;
class Listener;
class EvfImageData;

class Controller : public QObject
{
    Q_OBJECT

    public:
	Controller( Camera *camera, Import *import );
	~Controller();

	/*
	 *  Manage a session with the camera.
	 */
	bool openSessionCommand();
	bool closeSessionCommand();

    public slots:
	/*
	 *  Setting properties commands.
	 */
	bool setPropertyCommand( int property, int newValue, int param = 0 );
	bool setPropertyCommand( int property, QString string );
	bool setPropertyCommand( int property, QDateTime dateTime );
	bool setPropertyCommand( int property, int *shift );
	bool setFocusPropertyCommand( int selectionMode, int fp = 0 );
	bool moveZoomRectCommand( int x, int y );

	/*
	 *  Live view commands.
	 */
	bool startLiveView();
	void stopLiveView();

	/*
	 *  Video commands.
	 */
	bool startVideo();
	void stopVideo();

	/*
	 *  Focusing commands.
	 */
	bool focusAdjustment( int value );
	bool autoFocusChanged( int status );

	/*
	 *  Shooting commands.
	 */
	bool shutterCommand( int value );
	void startSequence( ShotSeq *shotSeq );
	void cancelSequence();
	void highISOtest( int shutterSpeed );

	/*
	 *  Communication with camera listener.
	 */
	void handlePropertyEvent(
		EdsPropertyEvent  inEvent,
		EdsPropertyID	  inPropertyID,
		EdsUInt32	  inParam );

	void handleObjectEvent(
		EdsObjectEvent	inEvent,
		EdsBaseRef	inRef );

	void handleStateEvent(	
		EdsStateEvent	inEvent,
		EdsUInt32	inEventData );

	void handleCameraAddedEvent();

	void messagePump();

    signals:
	void propertyChanged( int property, int parameter );
	void propertyListChanged( int property );
	void liveViewChanged( EvfImageData *evfImageData );
	void updatePanorama( QImage image, int shot );
	void eventReport( Event *event );

    protected slots:
	void firstShot();
	void takePictureCommand();
	bool lockCommand( int value );
	void bulbOpenShutter();
	void bulbCloseShutter();

    protected:
	void initializeCamera();
	Camera *camera;
	Import *import;

	bool startLiveViewCommand();
	bool doLiveViewCommand();
	bool stopLiveViewCommand();

	bool sdkLoaded;

	/*
	 *  Getting properties commands.
	 */
	bool getPropertyCommand( EdsPropertyID propertyID, int param = 0 );
	bool getPropertyListCommand( EdsPropertyID propertyID );
	bool getFocusPropertyCommand();

	/*
	 *  Shooting a sequence of shots.
	 */
	ShotSeq *shotSeq;
	QTimer intervalTimer;
	bool doSeq;
	int bulbShutterTime;
	bool bulbLock;	// track whether camera locked for bulb shooting

	/*
	 *  File management.
	 */
	bool downloadFile( EdsDirectoryItemRef itemRef, bool isMovie = false );
	void registerFile( EdsDirectoryItemRef itemRef );
	bool isMovieFile( EdsDirectoryItemRef itemRef );
	void updateMultiShot( char *fileName );
	QString imageProperties( char *fileName );
	QImage loadImage( char *fileName );

	/*
	 *  During video recording, destination for the movie must
	 *  be the camera. Save the previous destination for images 
	 *  so it can be restored once video recording ends.
	 */
	uint saveSaveTo;

	/*
	 *  Camera listener for passing on changes that result
	 *  from accessing the controls directly on the camera.
	 */
	Listener *listener;

	/*
	 *  Data for streaming live view data from camera.
	 */
	bool doLive;
	EvfImageData *evfImageData;

	/*
	 *  Timer mechanism to periodically call the message pump
	 *  for communicating with the camera.
	 */
	QTimer *timer;
	static const int timerPeriod = 500; // milliseconds

	/*
	 *  Properties to initialize when a session is opened.
	 */
	static const int initValueOnly[];
	static const int initValueAndList[];
	static const int initCFnValueOnly[];
};

#endif // _CONTROLLER_H_
