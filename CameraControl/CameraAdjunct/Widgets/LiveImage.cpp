/*
 *  Show live view.
 */

#include <QtGui>
#include "LiveImage.h"
#include "LiveImageData.h"
#include "Overlay.h"
#include "ZoomRect.h"
#include "Camera.h"
#include "Options.h"
#include "Map.h"


/*
 *  Constructor for live image widget.
 */
LiveImage::LiveImage( Camera *camera, QWidget *parent )
    : QWidget( parent )
{
    this->camera = camera;
    validData = false;
    overlay = NULL;
    moving = false;
    recording = false;

    worldWidth = 1;
    worldHeight = 1;
    viewWidth = 1;
    viewHeight = 1;
    pixelWidth = 1.0;

    zoomRect = new ZoomRect( camera, this );
    QObject::connect(
	zoomRect, SIGNAL(zoomRectChanged(int,int)),
	this, SIGNAL(zoomRectChanged(int,int)) );

    setFocusPolicy( Qt::StrongFocus );
}

/*
 *  Destructor for live image widget.
 */
LiveImage::~LiveImage()
{
    delete zoomRect;
}

/*
 *  A new image has arrived.
 */
void
LiveImage::setData( EvfImageData *data, double scaling )
{
    if( data == NULL ) {
	validData = false;
	update();
	return;
    }

    validData = true;
    zoom = data->zoom;
    rotationFlag = data->rotationFlag;
    focusInfo = data->focusInfo;

    /*
     *  Update image to display.
     */
    if( zoom == 1 ) {
	image = data->image;
    }
    else {
	image = data->image.copy(
			data->zoomRect.point.x - data->imagePosition.x,
			data->zoomRect.point.y - data->imagePosition.y,
			data->zoomRect.size.width,
			data->zoomRect.size.height );
    }

    QPainter painter( &image );
    painter.setRenderHints(
		QPainter::Antialiasing |
		QPainter::SmoothPixmapTransform, true );

    /*
     *  Resize/rescale the window for displaying the image.
     */
    if( (rotationFlag == EvfImageData::RotationNone) ||
	(rotationFlag == EvfImageData::Rotation180) ) {
	resize( int( image.width() * scaling ),
		int( image.height() * scaling ) ); // landscape mode
    }
    else {
	resize( int( image.height() * scaling ),
		int( image.width() * scaling ) ); // portrait mode
    }

    /*
     *  Set world coordinate system and viewport
     *  coordinate system for painting on image.
     */
    QRect coordinateSystem;
    if( zoom == 1 ) {
	coordinateSystem = camera->getEvfCoordinateSystem();
    }
    else {
	coordinateSystem = camera->getEvfZoomRect();
    }
    worldWidth = coordinateSystem.width();
    worldHeight = coordinateSystem.height();
    viewWidth = size().width();
    viewHeight = size().height();
    painter.setWindow( coordinateSystem );
    painter.setViewport( 0, 0, image.width(), image.height() );
    zoomRect->setWindow( coordinateSystem ); 
    zoomRect->setViewport( 0, 0, viewWidth, viewHeight );
    pixelWidth = double( worldWidth ) / ( image.width() * scaling );

    /*
     *  Update zoom rectangle using camera model, but
     *  only if mouse isn't currently dragging it.
     */
    if( !moving ) {
	zoomRect->setRect( camera->getEvfZoomRect() );
    }

    /*
     *  Draw on the image.
     */
    if( zoom == 1 ) {
	showClipping( data->clippingFlag );
	drawCompositionOverlay( painter, data->overlayFlag );
	drawVideoRecord( painter );
    }
    drawFocusPoints( painter );
    painter.end();

    /*
     *  Rotate the image.
     */
    if( rotationFlag != EvfImageData::RotationNone ) {
	QMatrix matrix;
	switch( rotationFlag ) {
	    case EvfImageData::RotationCW90:  matrix.rotate(  90 ); break;
	    case EvfImageData::RotationCCW90: matrix.rotate( -90 ); break;
	    case EvfImageData::Rotation180:   matrix.rotate( 180 ); break;
	}
	image = image.transformed( matrix );
    }

    /*
     *  Scale the image.
     */
    image = image.scaled( viewWidth, viewHeight, Qt::KeepAspectRatio ); 

    update();
}

void
LiveImage::setImageOverlay( Overlay *overlay )
{
    this->overlay = overlay;
}

/*
 *  Mouse moves the focus point in Live mode (zoom = 1x, 5x, 10x)
 *  and selects a focus point in Quick mode (zoom = 1x)
 */
void
LiveImage::mousePressEvent( QMouseEvent *event )
{
    moving = false;
    setCursor( Qt::ArrowCursor );
    if( !validData ) {
	return;
    }

    if( event->button() == Qt::LeftButton ) {
	lastPosition = zoomRect->transform( event->pos() );
	if( camera->getEvfAFMode() == EvfImageData::AFMODE_Live ) {
	    /*
	     *  Moving the focus point in Live focus mode.
	     *
	     *  If cursor is outside focus rectangle (but within
	     *  image) first center zoom rectangle on cursor.
	     */
	    moving = true;
	    setCursor( Qt::SizeAllCursor );
	    if( !zoomRect->focusRect().contains( lastPosition ) ) {
		zoomRect->centerZoomRect( lastPosition.x(),
					  lastPosition.y(), moving );
	    }
	} // if Live mode
	else
	if( (zoom == 1) &&
	    (camera->getEvfAFMode() == EvfImageData::AFMODE_Quick) ) {
	    /*
	     *  Selecting a focus point in Quick focus mode.
	     *
	     *  Find closest focus point p.
	     */
	    int n;
	    QRect *fp;
	    camera->getFocusPoint_List( &n, &fp );
	    int minLen = worldWidth + worldHeight;
	    int p;
	    for( int i = 0; i < n; i++ ) {
		QPoint point = lastPosition - fp[i].center();
		int length = point.manhattanLength();
		if( minLen > length ) {
		    minLen = length;
		    p = i;
		}
	    }

	    //EdsFocusInfo focusInfo = camera->getFocusInfo();
	    bool allFPSelected = true;
	    for( int i = 0; i < n; i++ ) {
		allFPSelected = allFPSelected &&
				    focusInfo.focusPoint[i].selected;
	    }

	    if(  (focusInfo.executeMode == Map::FP_AutomaticSelection) ||
		((focusInfo.executeMode == Map::FP_LensSetToManual) &&
		    allFPSelected) ) {
		/*
		 *  Current mode: automatic focus point selection.
		 *  switch to manual focus point selection mode and select p.
		 *  center zoom rectangle on p.
		 *  A side effect of moving the zoom rectangle is that the
		 *  focus point in live mode is also moved.
		 */
		emit focusPropertyChanged( Map::FP_ManualSelection, p );
		zoomRect->centerZoomRect( fp[p].center().x(),
					  fp[p].center().y(), moving );
	    }
	    else {
		/*
		 *  Current mode: manual focus point selection.
		 *  if p is the center focus point and p is already selected:
		 *      switch to automatic focus point selection mode
		 *      center zoom rectangle on center focus point
		 *  else
		 *      select p
		 *      center zoom rectangle on p
		 *  A side effect of moving the zoom rectangle is that the
		 *  focus point in live mode is also moved.
		 */
		int cfp = n/2;
		if( (p == cfp) &&
		    (focusInfo.focusPoint[p].selected == 1) ) {
		    emit focusPropertyChanged( Map::FP_AutomaticSelection, 0 );
		    zoomRect->centerZoomRect( fp[cfp].center().x(),
					      fp[cfp].center().y(), moving );
		}
		else {
		    emit focusPropertyChanged( Map::FP_ManualSelection, p );
		    zoomRect->centerZoomRect( fp[p].center().x(),
					      fp[p].center().y(), moving );
		}
	    }
	} // if Quick mode
	else
	if( (zoom == 1) &&
	    (camera->getEvfAFMode() == EvfImageData::AFMODE_Face) ) {
	    // not handled, but could be
	} // if Face mode
    }
}

void
LiveImage::mouseMoveEvent( QMouseEvent *event )
{
    if( (event->buttons() & Qt::LeftButton) && moving ) {
	/*
	 *  Moving the focus point in Live focus mode.
	 */
	QPoint newPosition =
	    zoomRect->transform( event->pos() );
	QPoint delta = newPosition - lastPosition;
	zoomRect->scrollZoomRect( delta.x(), delta.y(), moving );
	lastPosition = newPosition;
    }
}

void
LiveImage::mouseReleaseEvent( QMouseEvent *event )
{
    if( (event->button() == Qt::LeftButton) && moving ) {
	/*
	 *  Moving the focus point in Live focus mode.
	 */
	moving = false;
	setCursor( Qt::ArrowCursor );
	QPoint newPosition =
	    zoomRect->transform( event->pos() );
	QPoint delta = newPosition - lastPosition;
	zoomRect->scrollZoomRect( delta.x(), delta.y(), moving );
	lastPosition = QPoint();
    }
}

/*
 *  Move the focus rectangle in live view focusing mode
 *  using the arrow keys.
 */
void
LiveImage::keyPressEvent( QKeyEvent *event )
{
    static QTime lastRepeat;

    moving = false;
    setCursor( Qt::ArrowCursor );
    if( !validData ) {
	return;
    }
    if( (event->key() == Qt::Key_Left) ||
	(event->key() == Qt::Key_Right) ||
	(event->key() == Qt::Key_Up) ||
	(event->key() == Qt::Key_Down) ) {
	event->accept();
	if( event->isAutoRepeat() && lastRepeat.elapsed() < 100 ) {
	    return; // ignore if auto repeat too fast for camera
	}
	lastRepeat.start();
	int newKey = zoomRect->transform( event->key() );
	zoomRect->scrollZoomRect( newKey, moving );
    }
    else {
	QWidget::keyPressEvent( event );
    }
}

/*
 *  Manipulate the image to show clipping.
 *  Assumes zoom = 1x.
 */
void
LiveImage::showClipping( int clippingFlag )
{
    if( clippingFlag == EvfImageData::ShowClippingNone ) {
	return;
    }

    int n = image.width() * image.height();
    unsigned int *bitdata = (unsigned int *)image.bits();

    if( clippingFlag == EvfImageData::ShowClippingShadowAlt ) {
	/*
	 *  Image turns white, clipped areas appear:
	 *	black (all 3 channels clipped);
	 *	cyan, magenta, yellow (2 channels clipped);
	 *	red, green, blue (1 channel clipped).
	 */
	for( int i = 0; i < n; i++ ) {
	    int r, g, b;
	    if( qRed(   bitdata[i] ) == 0 ) r = 255; else r = 0;
	    if( qGreen( bitdata[i] ) == 0 ) g = 255; else g = 0;
	    if( qBlue(  bitdata[i] ) == 0 ) b = 255; else b = 0;
	    // now swap black and white
	    if( (r == 0) && (g == 0) && (b == 0) ) {
		r = 255; g = 255; b = 255;
	    }
	    else
	    if( (r == 255) && (g == 255) && (b == 255) ) {
		r = 0; g = 0; b = 0;
	    }
	    bitdata[i] = qRgb( r, g, b );
	}
    }
    else
    if( clippingFlag == EvfImageData::ShowClippingHighlightAlt ) {
	/*
	 *  Image turns black, clipped areas appear:
	 *	white (all 3 channels clipped);
	 *	cyan, magenta, yellow (2 channels clipped);
	 *	red, green, blue (1 channel clipped).
	 */
	for( int i = 0; i < n; i++ ) {
	    int r, g, b;
	    if( qRed(   bitdata[i] ) == 255 ) r = 255; else r = 0;
	    if( qGreen( bitdata[i] ) == 255 ) g = 255; else g = 0;
	    if( qBlue(  bitdata[i] ) == 255 ) b = 255; else b = 0;
	    bitdata[i] = qRgb( r, g, b );
	}
    }
    else {

	if( (clippingFlag == EvfImageData::ShowClippingShadow) ||
	    (clippingFlag == EvfImageData::ShowClippingBoth) ) {
	    /*
	     *  Show shadow clipping if all three of the
	     *  channels are clipped.
	     */
	    for( int i = 0; i < n; i++ ) {
		if( (qRed(   bitdata[i] ) == 0) &&
		    (qGreen( bitdata[i] ) == 0) &&
		    (qBlue(  bitdata[i] ) == 0) ) {
		    bitdata[i] = qRgb( 0, 0, 255 );
		}
	    }
	} // if show shadow clipping

	if( (clippingFlag == EvfImageData::ShowClippingHighlight) ||
	    (clippingFlag == EvfImageData::ShowClippingBoth) ) {
	    /*
	     *  Show highlight clipping if two or more of the
	     *  channels are clipped.
	     */
	    for( int i = 0; i < n; i++ ) {
		int count = 0;
		if( qRed(   bitdata[i] ) == 255 ) count++;
		if( qGreen( bitdata[i] ) == 255 ) count++;
		if( qBlue(  bitdata[i] ) == 255 ) count++;
		if( count >= 2 ) {
		    bitdata[i] = qRgb( 255, 0, 0 );
		}
	    }
	} // if show highlight clipping

    }
}

void
LiveImage::drawLineWithArrows(
	QPainter &painter,
	int x1, int y1,
	int x2, int y2 )
{
    const int arrowSize = 20;
    const double M_PI_3 = M_PI/3.0;

    QPointF srcPoint( x1, y1 );
    QPointF dstPoint( x2, y2 );
    QLineF line( srcPoint, dstPoint );
    painter.drawLine( line );
    QPointF srcArrowP1 = srcPoint + QPointF(sin(M_PI_3) * arrowSize,
					    cos(M_PI_3) * arrowSize);
    QPointF srcArrowP2 = srcPoint + QPointF(sin(M_PI - M_PI_3) * arrowSize,
					    cos(M_PI - M_PI_3) * arrowSize);
    QPointF dstArrowP1 = dstPoint + QPointF(sin(-M_PI_3) * arrowSize,
					    cos(-M_PI_3) * arrowSize);
    QPointF dstArrowP2 = dstPoint + QPointF(sin(-M_PI + M_PI_3) * arrowSize,
					    cos(-M_PI + M_PI_3) * arrowSize);
    painter.drawPolygon(
	QPolygonF() << line.p1() << srcArrowP1 << srcArrowP2 );
    painter.drawPolygon(
	QPolygonF() << line.p2() << dstArrowP1 << dstArrowP2 );
}

/*
 *  Draw an overlay for aiding composition.
 *  Assumes zoom = 1x.
 */
void
LiveImage::drawCompositionOverlay( QPainter &painter, int overlayFlag )
{
    if( overlayFlag == EvfImageData::OverlayNone ) {
	return;
    }

    int width = worldWidth;
    int height = worldHeight;
    const double lineWidth = 1.25;

    QPen pen;
    pen.setColor( Qt::darkGray );
    pen.setWidthF( lineWidth * pixelWidth );
    painter.setPen( pen );

    if( overlayFlag == EvfImageData::OverlayGrid ) {
	/*
	 *  Draw grid: 40 pixels wide; grid squares that
	 *	are undersized are evenly placed at ends.
	 */
	const int gridStep = 40 * pixelWidth;
	int hInitialStep = (width - (gridStep*(width/gridStep))) / 2;
	int vInitialStep = (height - (gridStep*(height/gridStep))) / 2;
	for( int h = hInitialStep; h < width; h += gridStep ) {
	    painter.drawLine( QLine( h, 0, h, height ) );
	}
	for( int v = vInitialStep; v < height; v += gridStep ) {
	    painter.drawLine( QLine( 0, v, width, v ) );
	}
    }
    else
    if( overlayFlag == EvfImageData::OverlayThirds ) {
	/*
	 *  Draw thirds:
	 *	    1 | 1 | 1
	 *	    ---------
	 *	    1 | 1 | 1
	 *	    ---------
	 *	    1 | 1 | 1
	 */
	int hStep = width / 3;
	int vStep = height / 3;
	painter.drawLine( QLine(   hStep, 0,   hStep, height ) );
	painter.drawLine( QLine( 2*hStep, 0, 2*hStep, height ) );
	painter.drawLine( QLine( 0,   vStep, width,   vStep ) );
	painter.drawLine( QLine( 0, 2*vStep, width, 2*vStep ) );
    }
    else
    if( overlayFlag == EvfImageData::OverlayGoldenRatio ) {
	/*
	 *  Draw golden ratio, phi = 1.61803399...
	 *	    1       | 1 - phi | 1
	 *	    ---------------------------
	 *	    1 - phi |         | 1 - phi
	 *	    ---------------------------
	 *	    1       | 1 - phi | 1
	 */
	double phi = (1.0 + sqrt(5.0)) / 2.0;
	double hStep = width / (1.0 + phi);
	double vStep = height / (1.0 + phi);
	painter.drawLine( QLineF( hStep, 0, hStep, height ) );
	painter.drawLine( QLineF( phi*hStep, 0,
			      phi*hStep, height ) );
	painter.drawLine( QLineF( 0, vStep, width, vStep ) );
	painter.drawLine( QLineF( 0,     phi*vStep,
			      width, phi*vStep ) );
    }
    else
    if( overlayFlag == EvfImageData::OverlayDiagonals ) {
	/*
	 *  Draw diagonal lines (at 45 degree angles)
	 *  from each corner. Relies on clipping.
	 */
	painter.drawLine( QLine( 0, 0, height, height ) );
	painter.drawLine( QLine( 0, height, height, 0 ) );
	painter.drawLine( QLine( width, height, width-height, 0 ) );
	painter.drawLine( QLine( width, 0, width-height, height ) );
    }
    else
    if( (overlayFlag == EvfImageData::OverlayTriangles1) ||
	(overlayFlag == EvfImageData::OverlayTriangles2) ) {
	/*
	 *  Draw triangles: diagonal line and two
	 *  perpendicular lines. Relies on clipping.
	 *  Two versions, depending on diagonal drawn.
	 */
	double alpha = atan( (double)height/(double)width );
	double beta = 1.57079632679489661923 - alpha; // PI/2 - alpha
	double a = width * sin( alpha ) * sin( beta );
	double b = a / tan( beta );
	if( overlayFlag == EvfImageData::OverlayTriangles1 ) {
	    painter.drawLine( QLineF( 0, 0, width, height ) );
	    painter.drawLine( QLineF( width, 0, width-b, a ) );
	    painter.drawLine( QLineF( 0, height, b, height-a ) );
	}
	else {
	    painter.drawLine( QLineF( 0, height, width, 0 ) );
	    painter.drawLine( QLineF( 0, 0, b, a ) );
	    painter.drawLine( QLineF( width-b, height-a, width, height ) );
	}
    }
    else
    if( (overlayFlag >= EvfImageData::OverlayGoldenSpiral1) &&
	(overlayFlag <= EvfImageData::OverlayGoldenSpiral8) ) {
	painter.save();
	QPen pen;
	pen.setColor( Qt::darkGray );
	pen.setWidthF( lineWidth * 1618.0 * (pixelWidth/double(width)) );
	painter.setPen( pen );
	if( overlayFlag <= EvfImageData::OverlayGoldenSpiral4 ) {
	    painter.setWindow( 0, 0, 1618, 1000 );
	}
	else {
	    painter.setWindow( 0, 0, 1000, 1618 );
	    painter.rotate( 90.0 );
	    painter.translate( 0, -1000 );
	}
	if( overlayFlag == EvfImageData::OverlayGoldenSpiral3 ||
	    overlayFlag == EvfImageData::OverlayGoldenSpiral4 ||
	    overlayFlag == EvfImageData::OverlayGoldenSpiral7 ||
	    overlayFlag == EvfImageData::OverlayGoldenSpiral8 ) {
	    painter.rotate( -180.0 );
	    painter.translate( -1618, -1000 );
	}
	double phi = (1.0 + sqrt(5.0)) / 2.0;
	double w = 1618;
	while( w > 50 ) {
	    double a = w/phi;
	    double b = w - a;
	    if( overlayFlag == EvfImageData::OverlayGoldenSpiral1 ||
		overlayFlag == EvfImageData::OverlayGoldenSpiral3 ||
		overlayFlag == EvfImageData::OverlayGoldenSpiral5 ||
		overlayFlag == EvfImageData::OverlayGoldenSpiral7 ) {
		// spiral 1, 3, 5, 7
		painter.drawRect( QRectF( 0, 0, a, a ) );
		painter.drawArc( QRectF( 0, -a, 2*a, 2*a ), 180*16, 90*16 );
		painter.translate( a, a );
		painter.rotate( -90.0 );
	    }
	    else {
		// spiral 2, 4, 6, 8
		painter.drawRect( QRectF( 0, 0, a, a ) );
		painter.drawArc( QRectF( 0, 0, 2*a, 2*a ), 90*16, 90*16 );
		painter.translate( w, 0 );
		painter.rotate( 90.0 );
	    }
	    w = a;
	}
	painter.restore();
    }
    else
    if( overlayFlag == EvfImageData::OverlayPassport ) {
	/*
	 *  Draw passport specification.
	 *  Assumes portrait mode.
	 */
	painter.save();
	QPen pen;
	pen.setColor( Qt::darkGray );
	// line width is 2 pixels
	pen.setWidthF( 2.0 * 700.0 * (pixelWidth/double(width)) );
	painter.setPen( pen );
	QSize s( 700, 500 );
	s.scale( image.width(), image.height(), Qt::KeepAspectRatio );
	int w = s.width();
	int h = s.height();
	painter.setViewport( QRect( image.width()/2 - w/2,
				    image.height()/2 - h/2, w, h ) );
	painter.setWindow( 0, 0, 700, 500 );
	painter.translate( 350, 250 );
	if( rotationFlag == EvfImageData::RotationCW90 ) {
	    painter.rotate( -90.0 );
	}
	else {
	    painter.rotate( 90.0 );
	}
	painter.translate( -250, -350 );
	painter.drawRect( QRect( 0, 0, 500, 700 ) );
	// draw face boundaries
	painter.drawEllipse( QPointF( 255, 300 ), 120, 155 );
	painter.drawEllipse( QPointF( 255, 275 ), 130, 180 );
	// draw neck & shoulders
	painter.drawLine( 195, 445, 195, 465 );
	painter.drawLine( 195, 465,  35, 540 );
	painter.drawLine( 320, 445, 320, 465 );
	painter.drawLine( 320, 465, 480, 540 );
	// draw min & max guidelines
	pen.setColor( Qt::lightGray );
	pen.setStyle( Qt::DashLine );
	painter.setPen( pen );
	painter.drawLine( 100,  92, 410,  92 );
	painter.drawLine( 100, 142, 410, 142 );
	painter.drawLine( 100, 458, 410, 458 );
	painter.translate( 80, 422 );
	painter.rotate( -90.0 );
	pen.setStyle( Qt::SolidLine );
	painter.setPen( pen );
	painter.setBrush( Qt::darkGray );
	drawLineWithArrows( painter, -30,  20, 274,  20 );
	drawLineWithArrows( painter, -30, 330, 324, 330 );
	QFont f("Calibri", 26);
	painter.setFont( f );
	painter.drawText( 0, 0, "Min Face Height" );
	painter.drawText( 0, 370, "Max Face Height" );
	painter.restore();
    }
}

/*
 *  Draw red circle to indicate video record, duration of
 *  video, and bars top and bottom. The bars are appropriate
 *  for the most common video resolutions, but not for all.
 */
void
LiveImage::drawVideoRecord( QPainter &painter )
{
    /*
     *  Movie  recordMode  recording  recording'  action
     *  -----------------------------------------------------------
     *  yes    yes         yes        yes         draw
     *  yes    yes         no         yes         start timer, draw
     *  yes    no          yes        no          return
     *  yes    no          no         no          return
     *  no     yes         yes        no          return
     *  no     yes         no         no          return
     *  no     no          yes        no          return
     *  no     no          no         no          return
     */
    if( //(camera->getShootingMode() != kEdsAEMode_Movie) ||
	(camera->getRecordMode() != Map::RecordModeStart) ) {
	recording = false;
	return;
    }

    if( !recording ) {
	/*
	 *  Starting a new video recording session.
	 */
	recording = true;
	videoTime.start();
    }

    painter.save();
    // bars
    const double offset = 20*pixelWidth;
    painter.fillRect( QRectF( 0, 0, worldWidth, offset ),
		      QColor(32,32,32,128) );
    painter.fillRect( QRectF( 0, worldHeight-offset, worldWidth, worldHeight ),
		      QColor(32,32,32,128) );
    // red dot
    QPen pen;
    pen.setWidthF( pixelWidth );
    pen.setColor( Qt::red );
    painter.setPen( pen );
    QBrush brush( Qt::red );
    painter.setBrush( brush );
    painter.drawEllipse( QPointF( worldWidth-offset, worldHeight-(offset/2.0) ),
			 3.5*pixelWidth, 3.5*pixelWidth );
    // duration
    QFont f( "Calibri" );
    f.setPointSizeF( 11 * pixelWidth );
    painter.setFont( f );
    pen.setColor( Qt::white );
    painter.setPen( pen );
    int seconds =  videoTime.elapsed()/1000;
    int minutes = seconds/60;
    seconds = seconds % 60;
    QTime t( 0, minutes, seconds );
    painter.drawText( QPointF( offset, worldHeight-(offset/4.0) ),
		      t.toString( "mm':'ss" ) );
    painter.restore();
}

/*
 *  Draw the focus point(s).
 *
 *                        |  1x      5x      10x
 *  -----------------------------------------------
 *  Manual   focus        |  --      --      --
 *  AF Quick focus points |  yes     --      --
 *  AF Live  focus point  |  yes     yes     yes
 *  AF Face  focus point  |  yes     N/A     N/A
 */
void
LiveImage::drawFocusPoints( QPainter &painter )
{
    /*
     *  Width of lines for focus points scaled so that
     *  lines will be 2 pixels wide.
     */
    QPen pen;
    const double lineWidth = 2.0 * pixelWidth;
    pen.setWidthF( lineWidth );

    if( (zoom == 1) &&
	(camera->getEvfAFMode() == EvfImageData::AFMODE_Quick) ) {

	/*
	 *  Focus points (dedicated AF sensor is used to focus).
	 *
	 *	selected justFocus action
	 *	-------- --------- ------
	 *	   0         0      gray
	 *	   0         1      gray
	 *	   1         0      white
	 *	   1         1      green
	 */
	EvfImageData::EvfFocusPoint *fp = focusInfo.focusPoint;

	for( int i = 0; i < focusInfo.nFocusPoints; i++ ) {
	    if( !fp[i].selected ) {
		pen.setColor( Qt::darkGray );
	    }
	    else
	    if( fp[i].justFocus & 1 ) {
		pen.setColor( Qt::green );
	    }
	    else {
		pen.setColor( Qt::white );
	    }
	    painter.setPen( pen );

	    painter.drawRect( fp[i].rect.point.x,
			      fp[i].rect.point.y,
			      fp[i].rect.size.width,
			      fp[i].rect.size.height );
	}
    } // AF Quick mode
    else
    if( camera->getEvfAFMode() == EvfImageData::AFMODE_Live ) {

	/*
	 *  Focus rectangle (image sensor is used to focus).
	 *
	 *	justFocus == FOCUS_Regular: (white)
	 *	justFocus == FOCUS_Infocus: (green)
	 *	justFocus == FOCUS_FocusNotAchieved: (red)
	 *	justFocus == FOCUS_Blank: (blank)
	 */
	if( (focusInfo.nFocusPoints == 1) &&
	    (focusInfo.focusPoint[0].justFocus ==
		EvfImageData::FOCUS_Regular) ) {
	    pen.setColor( Qt::white );
	}
	else
	if( (focusInfo.nFocusPoints == 1) &&
	    (focusInfo.focusPoint[0].justFocus ==
		EvfImageData::FOCUS_Infocus) ) {
	    pen.setColor( Qt::green );
	}
	else
	if( (focusInfo.nFocusPoints == 1) &&
	    (focusInfo.focusPoint[0].justFocus ==
		EvfImageData::FOCUS_FocusNotAchieved) ) {
	    pen.setColor( Qt::red );
	}
	else {
	    pen.setColor( Qt::transparent );
	}
	painter.setPen( pen );

	/*
	 *  Draw focus rectangle.
	 *
	 *  Not using focus point coordinates in
	 *  focusInfo.focusPoint[0]... as it may not have
	 *  latest information if focus rectangle is being moved.
	 */
	painter.drawRect( zoomRect->focusRect( zoom ) );

    } // AF Live
    else
    if( (zoom == 1) &&
	(camera->getEvfAFMode() == EvfImageData::AFMODE_Face) ) {

	/*
	 *  Focus rectangle, if face found in image.
	 */
	EvfImageData::EvfFocusPoint *fp = focusInfo.focusPoint;

	for( int i = 0; i < focusInfo.nFocusPoints; i++ ) {
	    if( !fp[i].selected ) {
		pen.setColor( Qt::transparent );
	    }
	    else
	    if( fp[i].justFocus & 1 ) {
		pen.setColor( Qt::green );
	    }
	    else {
		pen.setColor( Qt::white );
	    }
	    painter.setPen( pen );

	    painter.drawRect( fp[i].rect.point.x,
			      fp[i].rect.point.y,
			      fp[i].rect.size.width,
			      fp[i].rect.size.height );

	    /*
	     *  Draw triangles to indicate more faces have been found.
	     */
	    if( (pen.color() == Qt::white) &&
		(focusInfo.nFocusPoints > 1) ) {
		int a = fp[i].rect.point.x;
		int b = fp[i].rect.point.y + fp[i].rect.size.height/2;
		QPointF triangle1[3] = {
		    QPointF( a - 30, b-50 ),
		    QPointF( a - 30, b+50 ),
		    QPointF( a - 85, b    )
		};
		a += fp[i].rect.size.width;
		QPointF triangle2[3] = {
		    QPointF( a + 30, b-50 ),
		    QPointF( a + 30, b+50 ),
		    QPointF( a + 85, b    )
		};
		QBrush brush( Qt::white );
		painter.setBrush( brush );
		painter.drawPolygon( triangle1, 3 );
		painter.drawPolygon( triangle2, 3 );
		painter.setBrush( Qt::NoBrush );
	    }
	}
    } // AF Face mode
}

void
LiveImage::drawImageOverlay( QPainter &painter )
{
    painter.setOpacity( double(overlay->opacity)/100.0 );
    for( int i = 0; i < overlay->n; i++ ) {
	if( overlay->image[i].isNull() ) {
	    continue;
	}

	int imageWidth = overlay->image[i].width();
	int imageHeight = overlay->image[i].height();

	if( overlay->type[i] == Overlay::Center ) {
	    // not handled, but could be
	}
	else
	if( overlay->type[i] == Overlay::Top ) {
	    int imageSlice = (overlay->overlap*imageHeight)/100;
	    int viewSlice = (overlay->overlap*viewHeight)/100;
	    painter.drawImage (
		QRect( 0, 0, viewWidth, viewSlice ),
		overlay->image[i],
		QRect( 0, imageHeight-imageSlice, imageWidth, imageSlice )
	    );
	}
	else
	if( overlay->type[i] == Overlay::Bottom ) {
	    int imageSlice = (overlay->overlap*imageHeight)/100;
	    int viewSlice = (overlay->overlap*viewHeight)/100;
	    painter.drawImage (
		QRect( 0, viewHeight-viewSlice, viewWidth, viewSlice ),
		overlay->image[i],
		QRect( 0, 0, imageWidth, imageSlice )
	    );
	}
	else
	if( overlay->type[i] == Overlay::Left ) {
	    int imageSlice = (overlay->overlap*imageWidth)/100;
	    int viewSlice = (overlay->overlap*viewWidth)/100;
	    painter.drawImage (
		QRect( 0, 0, viewSlice, viewHeight ),
		overlay->image[i],
		QRect( imageWidth-imageSlice, 0, imageSlice, imageHeight )
	    );
	}
	else
	if( overlay->type[i] == Overlay::Right ) {
	    int imageSlice = (overlay->overlap*imageWidth)/100;
	    int viewSlice = (overlay->overlap*viewWidth)/100;
	    painter.drawImage (
		QRect( viewWidth-viewSlice, 0, viewSlice, viewHeight ),
		overlay->image[i],
		QRect( 0, 0, imageSlice, imageHeight )
	    );
	}
    } // for
}

/*
 *  Draw the image.
 */
void
LiveImage::paintEvent( QPaintEvent *event )
{
    if( !validData ) {
	return;
    }

    QPainter painter( this );
    painter.drawImage( 0, 0, image );

    if( (overlay != NULL) && (overlay->n > 0) ) {
	drawImageOverlay( painter );
    }
}

