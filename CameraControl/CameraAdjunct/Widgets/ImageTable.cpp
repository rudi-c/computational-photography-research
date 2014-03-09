/*
 *  Display an m x n table of images for stitching into a panorama.
 */

#include <QtGui>
#include "ImageTable.h"
#include "MultiShot.h"
#include "LiveImageData.h"
#include "Overlay.h"


/*
 *  Constructor for table of images widget.
 */
ImageTable::ImageTable( QWidget *parent )
    : QWidget( parent )
{
    rows = 1;
    cols = 1;
    setOrder( MultiShot::ORDER_LtoR );

    overlay = new Overlay();

    setMinimumSize( viewSize, viewSize );
    resize( viewSize, viewSize );
}

/*
 *  Destructor for table of images widget.
 */
ImageTable::~ImageTable()
{
    delete overlay;
}

/*
 *  A new image for the table of images has arrived.
 */
void
ImageTable::setImage( QImage image, int rotationFlag, int shot )
{
    /*
     *  Scale and rotate the image.
     */
    if( image.isNull() ) {
	item[shot].image = image;
	item[shot].thumbnail = image;
    }
    else {
	// size of live view jpeg
	image = image.scaled( 1056, 1056, Qt::KeepAspectRatio ); 
	if( rotationFlag != EvfImageData::RotationNone ) {
	    QMatrix matrix;
	    switch( rotationFlag ) {
		case EvfImageData::RotationCW90:  matrix.rotate(  90 ); break;
		case EvfImageData::RotationCCW90: matrix.rotate( -90 ); break;
		case EvfImageData::Rotation180:   matrix.rotate( 180 ); break;
	    }
	    image = image.transformed( matrix );
	}
	item[shot].image = image;
	item[shot].thumbnail =
		image.scaled( viewSize, viewSize, Qt::KeepAspectRatio ); 
    }

    update();
}

void
ImageTable::setRows( int value )
{
    if( value*cols < maxItems ) {
	rows = value;
	resize( cols*viewSize, rows*viewSize );
	setOrder( order );
    }
}

void
ImageTable::setColumns( int value )
{
    if( rows*value < maxItems ) {
	cols = value;
	resize( cols*viewSize, rows*viewSize );
	setOrder( order );
    }
}

/*
 *  Set image order.
 *
 *	0 1 2 3		l-to-r
 *	4 5 6 7
 *
 *	0 2 4 6		l-to-r (alt)
 *	1 3 5 7
 *
 *	3 2 1 0		r-to-l
 *	7 6 5 4
 *
 *	6 4 2 0		r-to-l (alt)
 *	7 5 3 1
 *
 *  This reduces to just two valid orders if number of rows is 1.
 */
void
ImageTable::setOrder( int value )
{
    order = value;

    nItems = 0;
    const int w = viewSize;
    const int h = viewSize;

    if( order == MultiShot::ORDER_LtoR ) {
	for( int i = 0; i < rows; i++ )
	for( int j = 0; j < cols; j++ ) {
	    item[nItems].x = j*w;
	    item[nItems].y = i*h;
	    nItems++;
	}
    }
    else
    if( order == MultiShot::ORDER_LtoR_alt ) {
	for( int j = 0; j < cols; j++ )
	for( int i = 0; i < rows; i++ ) {
	    item[nItems].x = j*w;
	    item[nItems].y = i*h;
	    nItems++;
	}
    }
    else
    if( order == MultiShot::ORDER_RtoL ) {
	for( int i = 0; i < rows; i++ )
	for( int j = 0; j < cols; j++ ) {
	    item[nItems].x = (cols-1-j)*w;
	    item[nItems].y = i*h;
	    nItems++;
	}
    }
    else
    if( order == MultiShot::ORDER_RtoL_alt ) {
	for( int j = 0; j < cols; j++ )
	for( int i = 0; i < rows; i++ ) {
	    item[nItems].x = (cols-1-j)*w;
	    item[nItems].y = i*h;
	    nItems++;
	}
    }

    update();
}

Overlay *
ImageTable::getOverlay( int shot )
{
    if( (shot <= 0) || (shot >= nItems) ) {
	return( NULL );
    }

    int n = 0;

    if( order == MultiShot::ORDER_LtoR ) {
	if( (shot % cols) != 0 ) {
	    overlay->type[n] = Overlay::Left;
	    overlay->image[n] = item[shot-1].image;
	    n++;
	}
	if( (shot / cols) != 0 ) {
	    overlay->type[n] = Overlay::Top;
	    overlay->image[n] = item[shot % cols].image;
	    n++;
	}
    }
    else
    if( order == MultiShot::ORDER_LtoR_alt ) {
	if( (shot / rows) != 0 ) {
	    overlay->type[n] = Overlay::Left;
	    overlay->image[n] = item[shot - rows].image;
	    n++;
	}
	if( (shot % rows) != 0 ) {
	    overlay->type[n] = Overlay::Top;
	    overlay->image[n] = item[shot-1].image;
	    n++;
	}
    }
    else
    if( order == MultiShot::ORDER_RtoL ) {
	if( (shot % cols) != 0 ) {
	    overlay->type[n] = Overlay::Right;
	    overlay->image[n] = item[shot-1].image;
	    n++;
	}
	if( (shot / cols) != 0 ) {
	    overlay->type[n] = Overlay::Top;
	    overlay->image[n] = item[shot % cols].image;
	    n++;
	}
    }
    else
    if( order == MultiShot::ORDER_RtoL_alt ) {
	if( (shot / rows) != 0 ) {
	    overlay->type[n] = Overlay::Right;
	    overlay->image[n] = item[shot - rows].image;
	    n++;
	}
	if( (shot % rows) != 0 ) {
	    overlay->type[n] = Overlay::Top;
	    overlay->image[n] = item[shot-1].image;
	    n++;
	}
    }

    overlay->n = n;
    overlay->overlap = 100;	// default values
    overlay->opacity = 100;

    return( overlay );
}

void
ImageTable::clearTable()
{
    for( int i = 0; i < maxItems; i++ ) {
	item[i].image = QImage();
	item[i].thumbnail = QImage();
    }

    update();
}

/*
 *  Draw the table of images.
 */
void
ImageTable::paintEvent( QPaintEvent *event )
{
    QPainter painter(this);
    painter.setRenderHints(
		QPainter::Antialiasing |
		QPainter::SmoothPixmapTransform, true );

    QPen pen;
    pen.setColor( Qt::darkGray );
    pen.setWidth( 0 );
    painter.setPen( pen );

    int pointSize = 24;
    QFont f("Calibri", pointSize);
    QFontMetrics fm( f );
    painter.setFont( f );

    const int w = viewSize;
    const int h = viewSize;
    for( int i = 0; i < nItems; i++ ) {
	if( item[i].thumbnail.isNull() ) {
	    /*
	     *  Draw the order of the shot.
	     */
	    QString s = QString::number( i+1 );
	    int sw = fm.width( s ) / 2;
	    painter.drawText( item[i].x + w/2 - sw,
			      item[i].y + h/2 + pointSize/2, s  );
	}
	else {
	    /*
	     *  Center the thumbnail in the view box.
	     */
	    painter.drawImage(
			item[i].x + (viewSize-item[i].thumbnail.width())/2,
			item[i].y + (viewSize-item[i].thumbnail.height())/2,
			item[i].thumbnail );
	}
	painter.drawRect( item[i].x+1,
			  item[i].y+1, w-1, h-1 );
    }
    painter.drawRect( 1, 1, cols*viewSize - 2, rows*viewSize - 2 );
}

