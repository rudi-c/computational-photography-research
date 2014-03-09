/*
 *  Track and move the zoom rectangle.
 */

#include <QtGui>
#include "ZoomRect.h"
#include "LiveImageData.h"
#include "Camera.h"
#include "Options.h"
#include "Map.h"


/*
 *  Constructor for zoom rectangle.
 */
ZoomRect::ZoomRect( Camera *camera, QWidget *parent )
    : QWidget( parent )
{
    origin.setX( 0 );
    origin.setY( 0 );
    worldWidth = 1;
    worldHeight = 1;
    viewWidth = 1;
    viewHeight = 1;

    this->camera = camera;
}

/*
 *  Destructor for zoom rectangle.
 */
ZoomRect::~ZoomRect()
{
}

/*
 *  Center the zoom rectangle at (x, y).
 *  Assumes world coordinate system.
 */
void
ZoomRect::centerZoomRect( int x, int y, bool moving )
{
    /*
     *  Convert the point to the upper left position.
     */
    x -= zRect.width() / 2;
    y -= zRect.height() / 2;

    moveZoomRect( x, y, moving );
}

/*
 *  Relative move of the zoom rectangle.
 *  Assumes world coordinate system.
 */
void
ZoomRect::scrollZoomRect( int deltaX, int deltaY, bool moving )
{
    int x = zRect.x() + deltaX;
    int y = zRect.y() + deltaY;

    moveZoomRect( x, y, moving );
}

/*
 *  Relative move of the zoom rectangle.
 *  Assumes world coordinate system.
 */
void
ZoomRect::scrollZoomRect( int key, bool moving )
{
    const int scrollStep = 68; // camera uses this step
    switch( key ) {
	case Qt::Key_Home:
	    centerZoomRect( worldWidth/2, worldHeight/2, moving );
	    break;
	case Qt::Key_Left:  scrollZoomRect( -scrollStep, 0, moving ); break;
	case Qt::Key_Right: scrollZoomRect( +scrollStep, 0, moving ); break;
	case Qt::Key_Up:    scrollZoomRect( 0, -scrollStep, moving ); break;
	case Qt::Key_Down:  scrollZoomRect( 0, +scrollStep, moving ); break;
	default: /* ignore */ break;
    }
}

/*
 *  Absolute move of zoom rectangle.
 *  Assumes world coordinate system.
 */
void
ZoomRect::moveZoomRect( int x, int y, bool moving )
{
    /*
     *  Zoom rectangle not allowed within these
     *  distances from the edge of the image.
     */
    int xDistance;
    int yDistance;
    if( (camera->getAFMode() == Map::AFMODE_Manual) ||
	(camera->getEvfAFMode() != EvfImageData::AFMODE_Live) ) {
	xDistance = 50;
	yDistance = 50;
    }
    else
    if( camera->getEvfZoom() == 1 ) {
	xDistance = 230;
	yDistance = 350;
    }
    else
    if( camera->getEvfZoom() == 5 ) {
	xDistance = 240;
	yDistance = 380;
    }
    else {
	xDistance = 500;
	yDistance = 550;
    }

    const int globalWidth = camera->getEvfCoordinateSystem().width();
    const int globalHeight = camera->getEvfCoordinateSystem().height();
    const int w = zRect.width();
    const int h = zRect.height();
    if( x < xDistance ) x = xDistance; // left
    if( x >= globalWidth - (w + xDistance) ) {
	x  = globalWidth - (w + xDistance); // right
    }
    if( y < yDistance ) y = yDistance; // top
    if( y >= globalHeight - (h + yDistance) ) {
	y  = globalHeight - (h + yDistance); // bottom
    }

    zRect.moveTo( x, y );
    update();

    /*
     *  Update the position of the zoom rectangle in the
     *  camera if the zoom rectangle is no longer moving.
     */
    if( !moving ) {
	emit zoomRectChanged( zRect.x(), zRect.y() );
    }
}

/*
 *  Transform a point from widget coordinates to world coordinates.
 */
QPoint
ZoomRect::transform( QPoint p )
{
    QPoint q = p;
    int w = viewWidth;
    int h = viewHeight;

    switch( camera->getRotationSetting() ) {
	case EvfImageData::RotationNone:
		q = (p * worldWidth) / w;
		break;
	case EvfImageData::RotationCW90:
		q = (QPoint( p.y(), w - p.x() ) * worldWidth) / h;
		break;
	case EvfImageData::RotationCCW90:
		q = (QPoint( h - p.y(), p.x() ) * worldWidth) / h;
		break;
	case EvfImageData::Rotation180:
		q = (QPoint( w - p.x(), h - p.y() ) * worldWidth) / w;
		break;
    }

    return( q + origin );
}

/*
 *  Transform an arrow key from widget coordinates to world coordinates.
 */
int
ZoomRect::transform( int key )
{
    int tKey = key;
    switch( camera->getRotationSetting() ) {
	case EvfImageData::RotationNone:
		break;
	case EvfImageData::RotationCW90:
		switch( key ) {
		    case Qt::Key_Left:  tKey = Qt::Key_Down; break;
		    case Qt::Key_Right: tKey = Qt::Key_Up; break;
		    case Qt::Key_Up:    tKey = Qt::Key_Left; break;
		    case Qt::Key_Down:  tKey = Qt::Key_Right; break;
		}
		break;
	case EvfImageData::RotationCCW90:
		switch( key ) {
		    case Qt::Key_Left:  tKey = Qt::Key_Up; break;
		    case Qt::Key_Right: tKey = Qt::Key_Down; break;
		    case Qt::Key_Up:    tKey = Qt::Key_Right; break;
		    case Qt::Key_Down:  tKey = Qt::Key_Left; break;
		}
		break;
	case EvfImageData::Rotation180:
		switch( key ) {
		    case Qt::Key_Left:  tKey = Qt::Key_Right; break;
		    case Qt::Key_Right: tKey = Qt::Key_Left; break;
		    case Qt::Key_Up:    tKey = Qt::Key_Down; break;
		    case Qt::Key_Down:  tKey = Qt::Key_Up; break;
		}
		break;
    }

    return( tKey );
}

/*
 *  The focus rectangle associated with the zoom rectangle.
 *  When zoom = 1x, the focus rectangle is taller than it really is
 *  to be consistent with what the camera displays and to distinguish
 *  more clearly from Quick mode.
 *  When zoom = 5x, 10x, the focus rectangle is square h/2 x h/2.
 */
QRect
ZoomRect::focusRect( int zoom )
{
    const int x = zRect.x();
    const int y = zRect.y();
    const int w = zRect.width();
    const int h = zRect.height();
    if( zoom == 1 ) {
	return( QRect( x+w/4, y, w/2, h ) );
    }
    else {
	const int boxWidth = h/2;
	return( QRect( x + w/2 - boxWidth/2,
		       y + h/2 - boxWidth/2,
		       boxWidth,
		       boxWidth ) );
    }
}

int ZoomRect::x() { return( zRect.x() ); }
int ZoomRect::y() { return( zRect.y() ); }
int ZoomRect::width() { return( zRect.width() ); }
int ZoomRect::height() { return( zRect.height() ); }
bool ZoomRect::contains( QPoint p ) { return( zRect.contains( p ) ); }
void ZoomRect::setRect( QRect rect ) { zRect = rect; }

void
ZoomRect::setRect( int x, int y, int width, int height )
{
    zRect.setRect( x, y, width, height );
}

void
ZoomRect::setWindow( QRect rect )
{
    origin.setX( rect.x() );
    origin.setY( rect.y() );
    worldWidth = rect.width();
    worldHeight = rect.height();
}

void
ZoomRect::setWindow( int x, int y, int width, int height )
{
    origin.setX( x );
    origin.setY( y );
    worldWidth = width;
    worldHeight = height;
}

void
ZoomRect::setViewport( int x, int y, int width, int height )
{
    viewWidth = width;
    viewHeight = height;
}

