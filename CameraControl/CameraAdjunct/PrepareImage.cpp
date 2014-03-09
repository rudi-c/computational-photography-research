/*
 *  Prepare image for display in live view.
 */

#include <QtGui>
#include "PrepareImage.h"
#include "LiveImageData.h"

/*
 *  Constructor for transaction thread.
 */
TransactionThread::TransactionThread()
{
}

/*
 *  Destructor for transaction thread.
 */
TransactionThread::~TransactionThread()
{
    wait();
};

void
TransactionThread::setData( EvfImageData *data )
{
    this->data = data;
    start();
}

/*
 *  Load JPEG image into a QImage. The QImage is stored using a
 *  32-bit RGB format (0xffRRGGBB); i.e., QImage::Format_RGB32.
 *  Routine is run in a separate thread.
 */
void
TransactionThread::run()
{
    if( data != NULL ) {
	bool load = data->image.loadFromData(
			data->buffer,
			data->len,
			"JPEG" );
	if( !load || data->image.isNull() ) {
	    return;
	}
    }
    emit imageReady( data );
};

/*
 *  Constructor for preparing image.
 */
PrepareImage::PrepareImage()
{
    lastFrame = false;

    nThreads = QThread::idealThreadCount() - 1;
    if( nThreads > MaxThreadCount ) {
	nThreads = MaxThreadCount;
    }
    if( nThreads < 1 ) {
	nThreads = 1;
    }
    for( int i = 0; i < nThreads; i++ ) {
	thread[i] = new TransactionThread();
	QObject::connect(
	    thread[i], SIGNAL(imageReady(EvfImageData *)),
	    this, SLOT(imageReady(EvfImageData *)) );
    }
}

/*
 *  Destructor for preparing image.
 */
PrepareImage::~PrepareImage()
{
    for( int i = 0; i < nThreads; i++ ) {
	delete thread[i];
    }
}

/*
 *  Convert the jpeg image in the buffer to a QImage.
 */
void
PrepareImage::updateLiveView( EvfImageData *data )
{
    if( data == NULL ) {
	imageReady( data );
	return;
    }

    /*
     *  Use next available thread.
     *  Drop frame if all threads busy.
     */
    int i;
    lastFrame = false;
    for( i = 0; i < nThreads; i++ ) {
	if( !thread[i]->isRunning() ) {
	    thread[i]->setData( data );
	    break;
	}
    }
    if( (i >= nThreads) && (data != NULL) ) {
	qDebug() << "frame dropped";
	delete data;
	data = NULL;
    }
}

void
PrepareImage::imageReady( EvfImageData *data )
{
    if( !lastFrame ) {
	emit liveViewChanged( data );
    }

    /*
     *  Once the end of live view signal (data == NULL)
     *  has been emitted, drop any remaining images that
     *  came earlier but are still in flight on a thread.
     */
    if( data == NULL ) {
	lastFrame = true;
    }
}

