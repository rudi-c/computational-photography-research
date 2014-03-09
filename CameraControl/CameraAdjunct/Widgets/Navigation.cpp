/*
 *  Navigation widget for setting zoom levels of live image and
 *  setting and displaying zoom and focus rectangles.
 */

#include <QtGui>
#include "Navigation.h"
#include "LiveImageData.h"
#include "ZoomRect.h"
#include "Camera.h"
#include "Options.h"
#include "Map.h"


/*
 *  Constructor for navigation widget.
 */
Navigation::Navigation( Camera *camera, QWidget *parent )
    : QWidget( parent )
{
    this->camera = camera;
    validData = false;
    moving = false;
    viewSize = 194;

    worldWidth = 1;
    worldHeight = 1;
    viewWidth = 1;
    viewHeight = 1;
    viewX = 0;
    viewY = 0;

    zoomRect = new ZoomRect( camera, this );
    QObject::connect(
	zoomRect, SIGNAL(zoomRectChanged(int,int)),
	this, SIGNAL(zoomRectChanged(int,int)) );

    setFixedSize( viewSize, viewSize );
    setEnabled( false );

    setFocusPolicy( Qt::StrongFocus );
}

/*
 *  Destructor for navigation widget.
 */
Navigation::~Navigation()
{
    delete zoomRect;
}

/*
 *  New information for the navigation panel has arrived.
 */
void
Navigation::setData( EvfImageData *data, QRectF rect )
{
    if( data == NULL ) {
	validData = false;
	update();
	return;
    }

    validData = true;
    zoom = data->zoom;
    rotationFlag = data->rotationFlag;

    /*
     *  Determine world coordinate system and viewport
     *  coordinate system for painting and zooming.
     */
    worldWidth = data->coordinateSystem.width;
    worldHeight = data->coordinateSystem.height;
    QSize s( worldWidth, worldHeight );
    s.scale( viewSize, viewSize, Qt:: KeepAspectRatio );
    viewWidth = s.width();
    viewHeight = s.height();
    if( (rotationFlag == EvfImageData::RotationNone) ||
	(rotationFlag == EvfImageData::Rotation180) ) {
	viewX = 0;
	viewY = (viewSize-viewHeight)/2; // center the image in the window
	zoomRect->setWindow( 0, 0, worldWidth, worldHeight );
	zoomRect->setViewport( 0, 0, viewWidth, viewHeight );
    }
    else {
	viewX = (viewSize-viewHeight)/2;
	viewY = 0;			 // center the image in the window
	zoomRect->setWindow( 0, 0, worldWidth, worldHeight );
	zoomRect->setViewport( 0, 0, viewHeight, viewWidth );
    }

    /*
     *  Correct the visible rectangle for rotation.
     */
    switch( rotationFlag ) {
	case EvfImageData::RotationNone:
	    visibleRect = rect;
	    break;
	case EvfImageData::RotationCW90:
	    visibleRect.setRect( rect.y(), 1.0 - (rect.x()+rect.width()),
				 rect.height(), rect.width() );
	    break;
	case EvfImageData::RotationCCW90:
	    visibleRect.setRect( 1.0 - (rect.y()+rect.height()), rect.x(),
				 rect.height(), rect.width() );
	    break;
	case EvfImageData::Rotation180:
	    visibleRect.setRect( 1.0 - (rect.x()+rect.width()),
				 1.0 - (rect.y()+rect.height()),
				 rect.width(), rect.height() );
	    break;
    }

    /*
     *  Update the background image if not zooming (zoom == 1x).
     *  If zooming, the most recent non-zoomed image is used as
     *  the background.
     */
    if( zoom == 1 ) {
	image = data->image;
    }

    /*
     *  Update zoom rectangle using camera model, but
     *  only if mouse isn't currently dragging it.
     */
    if( !moving ) {
	zoomRect->setRect( camera->getEvfZoomRect() );
    }

    bool zoomMoveable =
	    (zoom == 1) ||
	    ((zoom != 1) &&
		(camera->getEvfAFMode() != EvfImageData::AFMODE_Face));
    setEnabled( zoomMoveable );

    if( isVisible() ) {
	update();
    }
}

/*
 *  Mouse moves the view window for panning the image if zoom = 1x,
 *  and moves the zoom rectangle if zoom = 5x, 10x.
 */
void
Navigation::mousePressEvent( QMouseEvent *event )
{
    moving = false;
    setCursor( Qt::ArrowCursor );
    if( !validData ) {
	return;
    }
    if( zoom == 1 ) {
	// not handled, but could be
	return;
    }

    if( event->button() == Qt::LeftButton ) {
	if( (camera->getEvfAFMode() == EvfImageData::AFMODE_Live) ||
	    (camera->getEvfAFMode() == EvfImageData::AFMODE_Quick) ) {
	    /*
	     *  Moving the zoom rectangle.
	     *
	     *  If cursor is outside zoom rectangle
	     *  first center zoom rectangle on cursor.
	     */
	    moving = true;
	    setCursor( Qt::SizeAllCursor );
	    lastPosition =
		zoomRect->transform( event->pos() - QPoint( viewX, viewY ) );
	    if( !zoomRect->contains( lastPosition ) ) {
	        zoomRect->centerZoomRect( lastPosition.x(),
					  lastPosition.y(), moving );
	    }
	}
    }
}

void
Navigation::mouseMoveEvent( QMouseEvent *event )
{
    if( (event->buttons() & Qt::LeftButton) && moving ) {
	/*
	 *  Moving the zoom rectangle.
	 */
	QPoint newPosition =
	    zoomRect->transform( event->pos() - QPoint( viewX, viewY ) );
	QPoint delta = newPosition - lastPosition;
	zoomRect->scrollZoomRect( delta.x(), delta.y(), moving );
	lastPosition = newPosition;
    }
}

void
Navigation::mouseReleaseEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton && moving ) {
	/*
	 *  Moving the zoom rectangle in Live focus mode.
	 */
	moving = false;
	setCursor( Qt::ArrowCursor );
	QPoint newPosition =
	    zoomRect->transform( event->pos() - QPoint( viewX, viewY ) );
	QPoint delta = newPosition - lastPosition;
	zoomRect->scrollZoomRect( delta.x(), delta.y(), moving );
	lastPosition = QPoint();
    }
}

/*
 *  Move the view window for panning the image if zoom = 1x,
 *  and move the zoom rectangle if zoom = 5x, 10x.
 */
void
Navigation::keyPressEvent( QKeyEvent *event )
{
    static QTime lastRepeat;

    moving = false;
    setCursor( Qt::ArrowCursor );
    if( !validData ) {
	return;
    }
    if( zoom == 1 ) {
	// not handled, but could be
	return;
    }

    if( (event->key() == Qt::Key_Home) ||
	(event->key() == Qt::Key_Right) ||
	(event->key() == Qt::Key_Up) ||
	(event->key() == Qt::Key_Down) ) {
	event->accept();
	if( event->isAutoRepeat() && lastRepeat.elapsed() < 100 ) {
	    return; // ignore if auto repeat too fast for camera
	}
	lastRepeat.start();
	int newKey = zoomRect->transform( event->key() );
	zoomRect->scrollZoomRect( newKey, false );
    }
    else {
	QWidget::keyPressEvent( event );
    }
}

/*
 *  Move the zoom rectangle using the "move" widget.
 */
void
Navigation::moveDirection( int key )
{
    zoomRect->scrollZoomRect( zoomRect->transform( key ), false );
}

/*
 *  Draw the navigation panel.
 */
void
Navigation::paintEvent( QPaintEvent *event )
{
    QPainter painter(this);
    painter.setRenderHints(
		QPainter::Antialiasing |
		QPainter::SmoothPixmapTransform, true );

    QPen pen;
    pen.setColor( Qt::darkGray );
    painter.setPen( pen );
    pen.setWidth( 0 );

    if( !validData ) {
	painter.drawRect( 1, 1, viewSize-2, viewSize-2 );
	return;
    }

    const double pixelWidth = double( worldWidth ) / double( viewSize );

    painter.setWindow( 0, 0, worldWidth, worldHeight );
    painter.setViewport( viewX, viewY, viewWidth, viewHeight );

    switch( rotationFlag ) {
	case EvfImageData::RotationNone:
	    break;
	case EvfImageData::Rotation180:
	    painter.rotate( 180.0 );
	    painter.translate( -worldWidth, -worldHeight );
	    break;
	case EvfImageData::RotationCW90:
	    painter.rotate( 90.0 );
	    painter.translate( 0, -worldHeight );
	    break;
	case EvfImageData::RotationCCW90:
	    painter.rotate( -90.0 );
	    painter.translate( -worldWidth, 0 );
	    break;
    }

    /*
     *  Draw image and a border around the image.
     */
    painter.drawImage( QRect( 0, 0, worldWidth, worldHeight ), image );
    painter.drawRect( 0, 0, worldWidth, worldHeight );

    /*
     *  Draw rectangle representing visible region.
     */
    pen.setColor( Qt::white );
    pen.setWidthF( 1.5 * pixelWidth );
    painter.setPen( pen );
    if( zoom == 1 ) {
	painter.drawRect(
		QRectF( visibleRect.x() * worldWidth,
			visibleRect.y() * worldHeight,
			visibleRect.width() * worldWidth,
			visibleRect.height() * worldHeight ) );
    }
    else
    if( (zoom != 1) &&
	(camera->getEvfAFMode() != EvfImageData::AFMODE_Face) ) {
	/*
	 *  Zoom rectangle (zoom = 5x or 10x).
	 */
	const int x = zoomRect->x();
	const int y = zoomRect->y();
	const int w = zoomRect->width();
	const int h = zoomRect->height();
	painter.drawRect(
		QRectF( x + visibleRect.x() * w,
			y + visibleRect.y() * h,
			visibleRect.width() * w,
			visibleRect.height() * h ) );
    }
}

