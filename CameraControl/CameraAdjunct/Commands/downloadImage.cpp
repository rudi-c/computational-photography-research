/*
 *  Commands to download or register a file from the camera.
 */

//temporary
#include <QtGui>
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include "Controller.h"
#include "Import.h"
#include "Listener.h"
#include "ShotSeq.h"
#include "Event.h"

/*
 *  Download an image or movie file from the camera.
 */
bool
Controller::downloadFile( EdsDirectoryItemRef itemRef, bool isMovie )
{
    int result = EDS_ERR_OK;

    emit eventReport( new Event( Event::DownloadInitiated ) );

    /*
     *  Get information about the image to download.
     *
     *     typedef struct {
     *         EdsUInt32   size;        // file size (0 if folder)
     *         EdsBool     isFolder;
     *         EdsUInt32   groupID;     // same group ID is assigned to
     *                  		// all files that belong to the
     *                  		// same group (e.g., RAW+JPEG)
     *         EdsUInt32   option;      // applicable to cameras with a
     *                  		// direct transfer button
     *         EdsChar     szFileName[ EDS_MAX_NAME ];
     *                  		// folder or file name
     *     } EdsDirectoryItemInfo;
     */
    EdsDirectoryItemInfo itemInfo;
    result = EdsGetDirectoryItemInfo( itemRef, &itemInfo );

    /*
     *  Possibly rename file and check if it exists.
     */
    char fileName[EDS_MAX_NAME];
    uint fileSize = 0;
    if( result == EDS_ERR_OK ) {
	import->rename( fileName, itemInfo.szFileName );
	if( QFile::exists(QString(fileName)) ) {
	    if( !import->getSaveFile( fileName ) ) {
		EdsDownloadCancel( itemRef );
		result = EDS_ERR_OPERATION_CANCELLED;
	    }
	}
	fileSize = itemInfo.size;
    }

    /*
     *  Create a new file on computer (or open an existing file) and create
     *  a file stream for accessing the file. A version (EdsCreateFileStreamEx)
     *  is available that allows Unicode file names.
     *
     *      kEdsFileCreateDisposition_CreateNew
     *          Create a new file. An error occurs if the designated file
     *          already exists.
     *      kEdsFileCreateDisposition_CreateAlways
     *          Create a new file. If the designated file already exists,
     *          that file is overwritten and existing attributes erased.
     *      kEdsFileCreateDisposition_OpenExisting
     *          Open a file. An error occurs if the designated file does
     *          not exist.
     *      kEdsFileCreateDisposition_OpenAlways
     *          Open a file. If the designated file does not exist, a new
     *          file is created.
     *      kEdsFileCreateDisposition_TruncateExisting
     *          Opens a file and set the file size to 0 bytes.
     *
     *      kEdsAccess_Read
     *      kEdsAccess_Write
     *      kEdsAccess_ReadWrite
     */
    EdsStreamRef fileStream = NULL;
    if( result == EDS_ERR_OK ) {
	result = EdsCreateFileStream(
			fileName,
			kEdsFileCreateDisposition_CreateAlways,
			kEdsAccess_ReadWrite,
			&fileStream );
    }

    /*
     *  Set progress callback routine.
     *  Possible parameters:
     *      kEdsProgressOption_NoReport
     *      kEdsProgressOption_Done
     *      kEdsProgressOption_Periodically
     */
    EdsProgressOption option;
    if( isMovie ) option = kEdsProgressOption_Periodically;
    else	  option = kEdsProgressOption_NoReport;
    if( (result == EDS_ERR_OK) && isMovie ) {
	result = EdsSetProgressCallback(
			fileStream,
			Listener::progressCallback,
			option,
			(EdsVoid *)listener );
    }

    /*
     *  Downloads a file from the camera (either in the camera memory
     *  or on a memory card) to the computer.
     */
    if( result == EDS_ERR_OK ) {
	result = EdsDownload( itemRef, fileSize, fileStream );
    }

    /*
     *  Let the camera know that file transmission is complete.
     */
    if( result == EDS_ERR_OK ) {
	result = EdsDownloadComplete( itemRef );
    }

//begin temporary
    QImage image;
    bool load = image.load( fileName, "JPEG" );

    if( load ) {
	const int w = image.width();
	const int h = image.height();
	const int n = w * h;

        uchar *gray = new uchar[n];
        QRgb *bitsdata = (QRgb *)image.bits();
        for( int k = 0; k < n; k++ ) {
            gray[k] = ( 54 * qRed(   bitsdata[k] ) +
                       183 * qGreen( bitsdata[k] ) +
                        19 * qBlue(  bitsdata[k] ) ) / 256;
        }

        int j;
	char grayFileName[EDS_MAX_NAME];
        for( j = 0; fileName[j] != '.'; j++ ) {
	    grayFileName[j] = fileName[j];
        }
        grayFileName[j] = '.'; j++;
        grayFileName[j] = 'g'; j++;
        grayFileName[j] = 'r'; j++;
        grayFileName[j] = 'a'; j++;
        grayFileName[j] = 'y'; j++;
        grayFileName[j] = '\0';

        FILE *fp;

        fp = fopen( grayFileName, "wb" );
        fwrite( gray, 1, n, fp );
        fclose( fp );

        delete[] gray;
    } // if load
//end temporary
/*
*/

    /*
     *  Release the resources.
     */
    if( fileStream != NULL ) {
	EdsRelease( fileStream );
	fileStream = NULL;
    }
    if( itemRef != NULL) {
	EdsRelease( itemRef );
	itemRef = NULL;
    }

    /*
     *  Send notice of successful download along with information
     *  about the image. Check if image is part of a multi-shot
     *  sequence.
     */
    if( result == EDS_ERR_OK ) {
	if( isMovie ) {
	    emit eventReport( new Event( Event::DownloadProgress, 100 ) );
	}
	emit eventReport( new Event( Event::DownloadComplete,
				0, imageProperties( fileName ) ) );
	if( isMovie ) {
	    emit eventReport( new Event( Event::DownloadProgress, -1 ) );
	}
	updateMultiShot( fileName );
    }

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::DownloadFailure, result ) );
    }

    return( result == EDS_ERR_OK );
};

/*
 *  Register an image file, where the file stays on the camera.
 */
void
Controller::registerFile( EdsDirectoryItemRef itemRef )
{
    int result = EDS_ERR_OK;

    /*
     *  Get the file name of the image.
     */
    EdsDirectoryItemInfo itemInfo;
    result = EdsGetDirectoryItemInfo( itemRef, &itemInfo );

    /*
     *  Release the resources.
     */
    if( itemRef != NULL) {
	EdsRelease( itemRef );
	itemRef = NULL;
    }

    /*
     *  Send notice of successful register. Check
     *  if image is part of a multi-shot sequence.
     */
    if( result == EDS_ERR_OK ) {
	emit eventReport( new Event( Event::RegisterComplete,
				0, QString( itemInfo.szFileName ) ) );
	updateMultiShot( itemInfo.szFileName );
    }

    if( result != EDS_ERR_OK ) {
	emit eventReport( new Event( Event::RegisterFailure, result ) );
    }
};

/*
 *  Retrieve image properties from an image file.
 */
QString
Controller::imageProperties( char *fileName )
{
    int result;
    QString capture = QString(fileName);

    /*
     *  Get the file stream and image reference.
     */
    EdsStreamRef fileStream = NULL;
    result = EdsCreateFileStream(
		fileName,
		kEdsFileCreateDisposition_OpenExisting,
		kEdsAccess_Read,
		&fileStream );
    EdsImageRef imageRef = NULL;
    if( result == EDS_ERR_OK ) {
	result = EdsCreateImageRef( fileStream, &imageRef );
    }

    /*
     *  Get capture properties.
     */
    if( result == EDS_ERR_OK ) {
	/*
	 *  Shutter speed.
	 */
	EdsRational rational;
	result = EdsGetPropertyData( imageRef,
	    kEdsPropID_Tv, 0, sizeof(rational), &rational);
	if( rational.denominator == 1 ) {
	    capture += ", "
			+ QString::number(rational.numerator)
			+ "s";
	}
	else
	if( rational.numerator == 1 ) {
	    capture += ", "
			+ QString::number(rational.numerator)
			+ "/"
			+ QString::number(rational.denominator)
			+ "s";
	}
	else {
	    double speed = double(rational.numerator)
				/ double(rational.denominator);
	    capture += ", "
			+ QString::number(speed,'f',1)
			+ "s";
	}
    }
    if( result == EDS_ERR_OK ) {
	/*
	 *  Aperture.
	 */
	EdsRational rational;
	result = EdsGetPropertyData( imageRef,
	    kEdsPropID_Av, 0, sizeof(rational), &rational);
	double aperture = double(rational.numerator)
				/ double(rational.denominator);
	if( aperture < 10.0 ) {
	    capture += ", f/"
			+ QString::number(aperture,'f',1);
	}
	else {
	    capture += ", f/"
			+ QString::number(aperture,'f',0);
	}
    }
    if( result == EDS_ERR_OK ) {
	/*
	 *  ISO speed.
	 */
	uint data;
	result = EdsGetPropertyData( imageRef,
	    kEdsPropID_ISOSpeed, 0, sizeof(data), &data);
	capture += ", ISO "
		   + QString::number(data);
    }
    if( result == EDS_ERR_OK ) {
	/*
	 *  Lens (wide-telephoto), focal length.
	 */
	EdsRational focal[3];
	result = EdsGetPropertyData( imageRef,
	    kEdsPropID_FocalLength, 0, sizeof(focal), &focal);
	if( focal[1].numerator == focal[2].numerator ) {
	    capture += ", "
			+ QString::number(focal[0].numerator)
			+ "mm";
	}
	else {
	    capture += ", "
			+ QString::number(focal[1].numerator)
			+ "-"
			+ QString::number(focal[2].numerator)
			+ "@"
			+ QString::number(focal[0].numerator)
			+ "mm";
	}
    }

    /*
     *  Release the file stream and image reference.
     */
    if( imageRef != NULL ) {
	EdsRelease( imageRef );
    }
    if( fileStream != NULL ) {
	EdsRelease( fileStream );
    }

    if( result != EDS_ERR_OK ) {
	return( QString(fileName) );
    }

    return( capture );
}

QImage
Controller::loadImage( char *fileName )
{
    int result;

    /*
     *  Open the file.
     */
    EdsStreamRef fileStream = NULL;
    result = EdsCreateFileStream(
		fileName,
		kEdsFileCreateDisposition_OpenExisting,
		kEdsAccess_Read,
		&fileStream );

    EdsImageRef imageRef = NULL;
    if( result == EDS_ERR_OK ) {
	result = EdsCreateImageRef( fileStream, &imageRef );
    }

    /*
     *  Get information about the image itself.
     *
     *     typedef struct {
     *      EdsUInt32   width;              // image width
     *      EdsUInt32   height;             // image height
     *      EdsUInt32   numOfComponents;    // color components
     *      EdsUInt32   componentDepth;     // bits/sample (8,16)
     *      EdsRect     effectiveRect;      // image size excluding
     *      EdsUInt32   reserved1;          // black bars (present
     *      EdsUInt32   reserved2;          // on some thumbnails)
     *     } EdsImageInfo;
     *
     *  Only a subset work for a given image type.
     *     kEdsImageSrc_FullView            // full-sized image
     *     kEdsImageSrc_Thumbnail           // thumbnail image
     *     kEdsImageSrc_Preview             // preview image
     *     kEdsImageSrc_RAWThumbnail        // RAW thumbnail image
     *     kEdsImageSrc_RAWFullView         // RAW full-sized image
     */
    EdsImageInfo imageInfo = {0};
    EdsImageSource imageSource;
    if( QString(fileName).endsWith( ".JPG", Qt::CaseInsensitive ) ) {
	// JPEG image
	imageSource = kEdsImageSrc_FullView;
    }
    else {
	// RAW image: .CRW, .CR2, .RAW
	imageSource = kEdsImageSrc_Preview;
    }
//thumbnail sufficient or need above higher resolution for overlay?
//imageSource = kEdsImageSrc_Thumbnail;	// width is 160 pixels
    if( result == EDS_ERR_OK ) {
	result = EdsGetImageInfo( imageRef, imageSource, &imageInfo );
    }

    /*
     *  Get the image.
     */
    EdsStreamRef memoryStream = NULL;
    if( result == EDS_ERR_OK ) {
	result = EdsCreateMemoryStream( 0, &memoryStream );
    }

    EdsSize size;
    if( result == EDS_ERR_OK ) {
	/*
	 *  Source rectangle specifies portion of image to retrieve.
	 *  Target rectangle can be used to scale the image up or down.
	 */
	size.width = imageInfo.effectiveRect.size.width;
	size.height = imageInfo.effectiveRect.size.height;
	result = EdsGetImage(
			imageRef,
			imageSource,
			kEdsTargetImageType_RGB,
			imageInfo.effectiveRect,    // source rectangle
			size,			    // target rectangle
			memoryStream );
    }

    /*
     *  Convert to QImage format.
     */
    QImage final;
    if( result == EDS_ERR_OK ) {
	EdsVoid *ptr = NULL;
	EdsGetPointer( memoryStream, &ptr );
	if( ptr != NULL ) {
	    EdsUInt32 len = 0;
	    EdsGetLength( memoryStream, &len );
	    QImage image( size.width, size.height, QImage::Format_ARGB32 );
	    unsigned int *bitdata = (unsigned int *)image.bits();
	    uchar *p;
	    p = static_cast<uchar *>(ptr);
	    int j = 0;
	    for( int i = 0; i < len; i += 3 ) {
		int r, g, b;
		r = p[i];
		g = p[i+1];
		b = p[i+2];
		bitdata[j] = qRgb( r, g, b );
		j++;
	    }
	    final = image;
	} // ptr != NULL
    }

    /*
     *  Release the streams and reference.
     */
    if( memoryStream != NULL ) {
	EdsRelease( memoryStream );
    }
    if( imageRef != NULL ) {
	EdsRelease( imageRef );
    }
    if( fileStream != NULL ) {
	EdsRelease( fileStream );
    }

    if( result != EDS_ERR_OK ) {
	return( QImage() );
    }

    return( final );
}

/*
 *  Check if image is part of a multi-shot sequence.
 */
void
Controller::updateMultiShot( char *fileName )
{
    if( doSeq ) {

	shotSeq->shot++;
	emit eventReport( new Event( Event::SequenceProgress,
				 (100*shotSeq->shot)/shotSeq->frames ) );

	if( shotSeq->type == ShotSeq::Stitch ) {
	    emit updatePanorama( loadImage(fileName), shotSeq->shot );
	}

	if( shotSeq->shot >= shotSeq->frames ) {
	    if( shotSeq->type == ShotSeq::Interval ) {
		intervalTimer.stop();
	    }
	    doSeq = false;
	    emit eventReport( new Event( Event::SequenceComplete ) );
	}
    }
}

bool
Controller::isMovieFile( EdsDirectoryItemRef itemRef )
{
    int result = EDS_ERR_OK;

    EdsDirectoryItemInfo itemInfo;
    EdsGetDirectoryItemInfo( itemRef, &itemInfo );
    if( itemInfo.isFolder & 1 ) {
	return( false );
    }
    QString s( itemInfo.szFileName );
    return( s.endsWith( ".MOV", Qt::CaseInsensitive ) );
}

