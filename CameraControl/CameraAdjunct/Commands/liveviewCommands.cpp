/*
 *  Commands to open and close a live view session.
 */

#include <QtGui>
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include "Controller.h"
#include "LiveImageData.h"
#include "Camera.h"
#include "Event.h"
#include "Map.h"

bool
Controller::startLiveView()
{
    if( startLiveViewCommand() ) {
	doLive = true;
	doLiveViewCommand();
	stopLiveViewCommand();
    }

    return( true );
}

void
Controller::stopLiveView()
{
    doLive = false;
}

bool
Controller::startLiveViewCommand()
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    int result = EDS_ERR_OK;

    /*
     *  Enable live view, if it is not already enabled.
     */
    if( camera->getEvfMode() == 0 ) {
	uint evfMode = 1;
	result = EdsSetPropertyData(
			camera->handle(),
			kEdsPropID_Evf_Mode,
			0,
			sizeof(evfMode),
			&evfMode );
	/*
	 *  Will also be set through a callback from the camera.
	 */
	if( result == EDS_ERR_OK ) {
	    camera->setEvfMode( 1 );
	}
    }

    /*
     *  Get current output device.
     *  Nothing to do if output destination of live view is
     *  already computer.
     */
    uint device = camera->getEvfOutputDevice();
    if( (device & kEdsEvfOutputDevice_PC) == 1 ) {
	return( true );
    }

    /*
     *  Set the output device for live view.
     *      0				- no live view
     *      kEdsEvfOutputDevice_TFT	- live view on camera
     *      kEdsEvfOutputDevice_PC	- live view on computer
     *      kEdsEvfOutputDevice_TFT |
     *      kEdsEvfOutputDevice_PC	- live view on both
     */
    if( result == EDS_ERR_OK ) {
	device |= kEdsEvfOutputDevice_PC;
	result = EdsSetPropertyData(
			camera->handle(),
			kEdsPropID_Evf_OutputDevice,
			0,
			sizeof(device),
			&device );
	/*
	 *  Will also be set through a callback from the camera.
	 */
	if( result == EDS_ERR_OK ) {
	    camera->setEvfOutputDevice( device );
	}
    }

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::LiveviewFailure,
		result, "startLiveview" ) );
	return( false );
    }

    return( true );
}

bool
Controller::stopLiveViewCommand()
{
    int result = EDS_ERR_OK;

    /*
     *  Retry if camera device is busy.
     */
    do {

	/*
	 *  Check whether in live view mode.
	 */
	if( camera->getEvfMode() == 0 ) {
	    return( true );
	}

	/*
	 *  Get current output device.
	 *  Nothing to do if output destination of live view isn't computer.
	 */
	EdsUInt32 device = camera->getEvfOutputDevice();
	if( (device & kEdsEvfOutputDevice_PC) == 0 ) {
	    return( true );
	}

	/*
	 *  Release depth of field preview, if not already released.
	 */
	int depthOfFieldPreview = camera->getEvfDepthOfFieldPreview();
	if( depthOfFieldPreview != 0 ) {
	    depthOfFieldPreview = 0;
	    result = EdsSetPropertyData( camera->handle(),
					 kEdsPropID_Evf_DepthOfFieldPreview,
					 0,
					 sizeof(depthOfFieldPreview),
					 &depthOfFieldPreview);
	    if (result == EDS_ERR_OK) {
		camera->setEvfDepthOfFieldPreview( depthOfFieldPreview );
		Sleep( 500 );
	    }
	}

	/*
	 *  Change the output device.
	 */
	if( result == EDS_ERR_OK ) {
	    device &= ~kEdsEvfOutputDevice_PC;
	    result = EdsSetPropertyData( camera->handle(),
				     kEdsPropID_Evf_OutputDevice,
				     0,
				     sizeof(device),
				     &device );
	    /*
	     *  Will also be set through a callback from the camera.
	     */
	    if( result == EDS_ERR_OK ) {
		camera->setEvfOutputDevice( device );
	    }
	} // if ok

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy,
		0, "stopLiveview" ) );
	    QCoreApplication::processEvents();
	    Sleep( 500 );	// milliseconds
	}

    } while( result == EDS_ERR_DEVICE_BUSY );

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::LiveviewFailure,
		result, "stopLiveview" ) );
	return( false );
    }

    return( true );
}

/*
 *  Retrieve the live view stream.
 */
bool
Controller::doLiveViewCommand()
{
    if( camera->handle() == 0 ) {
	return( false );
    }

    /*
     *  Nothing to do if output destination of live view isn't computer.
     */
    if( (camera->getEvfOutputDevice() & kEdsEvfOutputDevice_PC) == 0 ) {
	return( true );
    }

    QTime t;
    t.start();

    uint frameCount = 0;
    int zoomLast = 1;

    while( doLive ) {

	QCoreApplication::processEvents();

	if( camera->handle() == 0 ) {
	    return( false );
	}

	evfImageData = new EvfImageData;

	/*
	 *  Create the memory stream.
	 */
	int result = EDS_ERR_OK;
	EdsStreamRef stream = NULL;
	result = EdsCreateMemoryStreamFromPointer(
			evfImageData->buffer,
			EvfImageData::MaxBufferSize,
			&stream );

	/*
	 *  Get the length of the stream.
	 */
	EdsUInt32 len = 0;
	if( result == EDS_ERR_OK ) {
	    EdsGetLength( stream, &len );
	}
	evfImageData->len = len;

	/*
	 *  Create a handle to get the live view image data set.
	 */
	EdsEvfImageRef evfImageRef = NULL;
	if( result == EDS_ERR_OK ) {
	    result = EdsCreateEvfImageRef( stream, &evfImageRef );
	}

	/*
	 *  Download the live view image data set into the memory stream.
	 */
	if( result == EDS_ERR_OK ) {
	    result = EdsDownloadEvfImage( camera->handle(), evfImageRef );
	}

	/*
	 *  Get the live view image and the meta data from the memory stream.
	 */
	if( result == EDS_ERR_OK ) {

	    /*
	     *  Get the histogram (YRGB).
	     */
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_HistogramY,
			0,
			sizeof(evfImageData->histogram_Y),
			evfImageData->histogram_Y );
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_HistogramR,
			0,
			sizeof(evfImageData->histogram_R),
			evfImageData->histogram_R );
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_HistogramG,
			0,
			sizeof(evfImageData->histogram_G),
			evfImageData->histogram_G );
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_HistogramB,
			0,
			sizeof(evfImageData->histogram_B),
			evfImageData->histogram_B );

	    /*
	     *  Get the histogram status:
	     *     kEdsEvfHistogramStatus_Hide
	     *     kEdsEvfHistogramStatus_Normal
	     *     kEdsEvfHistogramStatus_Grayout
	     */
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_HistogramStatus,
			0,
			sizeof(evfImageData->histogramStatus),
			&evfImageData->histogramStatus );

	    /*
	     *  Get the coordinate system of the live view image.
	     */
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_CoordinateSystem,
			0,
			sizeof(evfImageData->coordinateSystem),
			&evfImageData->coordinateSystem );

	    /*
	     *  Get the cropping position of the image data (when enlarging).
	     *  Upper left coordinate using JPEG large size as a reference.
	     */
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_ImagePosition,
			0,
			sizeof(evfImageData->imagePosition),
			&evfImageData->imagePosition );

	    /*
	     *  Get the magnification ratio (x1, x5, or x10).
	     */
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_Zoom,
			0,
			sizeof(evfImageData->zoom),
			&evfImageData->zoom );

	    /*
	     *  If zoom has changed, reset frame counter.
	     */
	    if( zoomLast != evfImageData->zoom ) {
	        zoomLast = evfImageData->zoom;
		frameCount = 0;
	    }

	    /*
	     *  Get the rectangle of the focus border.
	     */
	    EdsGetPropertyData( evfImageRef,
			kEdsPropID_Evf_ZoomRect,
			0,
			sizeof(evfImageData->zoomRect),
			&evfImageData->zoomRect );


	    /*
	     *  Get the focus point information.
	     */
	    int focusResult = EdsGetPropertyData(
					camera->handle(),
					kEdsPropID_FocusInfo,
					0,
					4120,
					&evfImageData->focusInfo );
	    if( focusResult != EDS_ERR_OK ) {
		evfImageData->focusInfo.nFocusPoints = 0;
	    }

	    /*
	     *  Get the rotation, if any, of the image.
	     */
	    evfImageData->rotationFlag = camera->getRotationSetting();

	    /*
	     *  Update the camera model.
	     *
	     *  Zoom events and zoom rectangle movements can be
	     *  initiated at both the computer and the camera. There
	     *  can be some delay before these events are processed and
	     *  the camera does not signal that they have happened.
	     *  So, if there is a mismatch between the values in
	     *  the image and the values recorded in the camera
	     *  model for 6 consecutive images, reset camera model.
	     */
	    camera->setEvfCoordinateSystem(
			QRect( 0, 0,
			       evfImageData->coordinateSystem.width,
			       evfImageData->coordinateSystem.height ) );
	    camera->setEvfZoomSize(
			QSize( evfImageData->zoomRect.size.width,
			       evfImageData->zoomRect.size.height ) );
	    static int zoomCount = 0;
	    if( evfImageData->zoom == camera->getEvfZoom() ) {
		zoomCount = 0;
	    }
	    else {
		zoomCount++;
		if( (frameCount == 0) || (zoomCount >= 6) ) {
		    camera->setEvfZoom( evfImageData->zoom );
		    zoomCount = 0;
		}
	    }
	    static int zoomPositionCount = 0;
	    QPoint position = camera->getEvfZoomPosition();
	    if( (evfImageData->zoomRect.point.x == position.x()) &&
	        (evfImageData->zoomRect.point.y == position.y()) ) {
		zoomPositionCount = 0;
	    }
	    else {
		zoomPositionCount++;
		if( (frameCount == 0) || (zoomPositionCount >= 6) ) {
		    camera->setEvfZoomPosition(
			QPoint( evfImageData->zoomRect.point.x,
			        evfImageData->zoomRect.point.y ) );
		    zoomPositionCount = 0;
		}
	    }

static int count = 0;
//begin temporary
int focusMapFlag = camera->getFocusMapSetting();
if( !focusMapFlag ) {
  count = 0;
}
else {
  if( frameCount > 50 ) {
    FILE *fp;

    char fileName[32];
    if(      count <  10 ) sprintf (fileName, "image00%d.jpg", count );
    else if( count < 100 ) sprintf (fileName, "image0%d.jpg", count );
    else                   sprintf (fileName, "image%d.jpg", count );

    fp = fopen(fileName,"wb");
    fwrite(evfImageData->buffer,1,evfImageData->len,fp);
    fclose(fp);
    QImage image;
    bool load = image.loadFromData(
                          evfImageData->buffer,
                          evfImageData->len,
                          "JPEG" );
    //qDebug() << "load" << load;
    const int w = image.width();
    const int h = image.height();
    const int n = w * h;

    uchar *gray = new uchar[n];
    QRgb *bitsdata = (QRgb *)image.bits();
    for( int i = 0; i < n; i++ ) {
          gray[i] = ( 54 * qRed(   bitsdata[i] ) +
                     183 * qGreen( bitsdata[i] ) +
                      19 * qBlue(  bitsdata[i] ) ) / 256;
    }

    if(      count <  10 ) sprintf (fileName, "image00%d.gray", count );
    else if( count < 100 ) sprintf (fileName, "image0%d.gray", count );
    else                   sprintf (fileName, "image%d.gray", count );

    fp = fopen(fileName,"wb");
    fwrite(gray,1,n,fp);
    fclose(fp);

    for( int i = 0; i < n; i++ ) {
          bitsdata[i] =  gray[i] * 0x00010101;
    }

    if(      count <  10 ) sprintf (fileName, "imagebw00%d.jpg", count );
    else if( count < 100 ) sprintf (fileName, "imagebw0%d.jpg", count );
    else                   sprintf (fileName, "imagebw%d.jpg", count );

    //image.save(fileName,"JPEG",100);
    delete[] gray;

    //if( shutterCommand( kEdsCameraCommand_ShutterButton_Completely_NonAF ) ) {
	//shutterCommand( kEdsCameraCommand_ShutterButton_OFF );
    //}

    count++;
    if( count > 250 ) exit(1);
    focusAdjustment( kEdsEvfDriveLens_Far1 );
    frameCount = 0;
  }
}
//end temporary

	    /*
	     *  Notify that live view image transfer is complete.
	     */
	    frameCount++;
	    emit liveViewChanged( evfImageData );

	} // get meta data and image

	if( result != EDS_ERR_OK ) {
	    delete evfImageData;
	    evfImageData = NULL;
	}

	if( stream != NULL ) {
	    EdsRelease( stream );
	    stream = NULL;
	}

	if( evfImageRef != NULL ) {
	    EdsRelease( evfImageRef );
	    evfImageRef = NULL;
	}

	/*
	 *  If retry is required, the camera may become unstable if multiple
	 *  commands are issued in succession without an intervening interval.
	 *  So, put a delay in before command is reissued.
	 */
	if( result == EDS_ERR_OBJECT_NOTREADY ) {
	    /*
	     *  If the shutter is open, send the end of stream
	     *  signal as the camera has stopped streaming.
	     */
	    if( bulbLock ) {
		emit liveViewChanged( NULL );
	    }
	    Sleep( 100 );	// milliseconds
	}
	if( result == EDS_ERR_DEVICE_BUSY ) {
	    emit eventReport( new Event( Event::CameraBusy, 0, "doLiveview" ) );
	    Sleep( 500 );	// milliseconds
	}

	if( (result != EDS_ERR_OK) &&
	    (result != EDS_ERR_OBJECT_NOTREADY) &&
	    (result != EDS_ERR_DEVICE_BUSY) ) {
	    emit eventReport( new Event( Event::LiveviewFailure,
		result, "doLiveview" ) );
	    return( false );
	}

    } // while

    emit liveViewChanged( NULL );

    /*
     *  Not the true frame rate, as must account for
     *  dropped frames in PrepareImage::updateLiveView().
     */
    qDebug( "FPS: %5.2f",
	static_cast<float>(frameCount) /
	static_cast<float>((t.elapsed())/1000.0) );

    return( true );
}

