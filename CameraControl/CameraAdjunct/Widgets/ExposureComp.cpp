/*
 *  Exposure compensation setting and displaying widget.
 */

#include <QtGui>
#include "Camera.h"
#include "ExposureComp.h"
#include "Map.h"
#include "size.h"


ExposureComp::ExposureComp( Camera *camera, int bound, QWidget *parent )
    : QWidget(parent)
{
    this->camera = camera;

    n = 0;
    v = 0;
    center = 0;
    compensation = 0.0;

    nBars = 0;
    incr = 0;

    sw = bound;			// adjustable sliding window size
    width = 20*sw + 12;		// width of painter window
    height = 24;		// height of painter window

    /*
     *  Viewport size is adjustable but must maintain
     *  aspect ratio of painter window.
     */
    viewWidth = 45 * sw;
    viewHeight = viewWidth * double(height)/double(width);
    setFixedSize( viewWidth, viewHeight );
    setFocusPolicy( Qt::TabFocus );
}

ExposureComp::~ExposureComp()
{
}

void
ExposureComp::setCompensation( int value )
{
    /*
     *  Convert from Canon internal value to actual
     *  exposure compensation.
     */
    compensation = Map::toCompensation( value );

    /*
     *  Update the center of the sliding window.
     */
    center = qCeil( qAbs(compensation) ) - sw;
    if( center < 0 ) {
	center = 0;
    }
    else
    if( compensation < 0 ) {
	center = -center;
    }
}

/*
 *  Find the index of the current exposure value.
 */
int
ExposureComp::findIndex()
{
    int value = camera->getExposureComp();

    for( int index = 0; index < n; index++ ) {
	if( value == v[index] ) {
	    return( index );
	}
    }

    return( 0 );
}

void
ExposureComp::keyPressEvent( QKeyEvent *event )
{
    int index = findIndex();

    if( event->key() == Qt::Key_Right ) {
	toValue( index+1 );
    }
    else
    if( event->key() == Qt::Key_Left ) {
	toValue( index-1 );
    }
    else {
	QWidget::keyPressEvent( event );
    }
}

void
ExposureComp::mousePressEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton ) {
	int index = findIndex();
	/*
	 *  Determine which direction to change the exposure compensation.
	 *  Transform the current exposure compensation from world coords
	 *  to viewport coords and compare to mouse x position, which is
	 *  in viewport coords.
	 */
	if( event->x() > ((compensation-center)*10 + width/2)
				* double(viewWidth)/double(width) ) {
	    toValue( index+1 );
	}
	else {
	    toValue( index-1 );
	}
    }
}

void
ExposureComp::wheelEvent( QWheelEvent *event )
{
    int index = findIndex();

    if( event->delta() > 0 ) {
	toValue( index+1 );
    }
    else {
	toValue( index-1 );
    }
}

void
ExposureComp::updateValue( Camera *camera )
{
    /*
     *  Get the current exposure compensation.
     */
    int value = camera->getExposureComp();
    setCompensation( value );

    update();
}

void
ExposureComp::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for exposure compensation.
     */
    camera->getExposureComp_List( &n, &v );

    /*
     *  Exposure compensation only in P, Tv, Av, A-DEP (not in M).
     */
    setEnabled( n > 0 );
    if( n > 0 ) {
	setCursor( Qt::SizeHorCursor );
    }
    else {
	setCursor( Qt::ArrowCursor );
    }

    updateValue( camera );
}


/*
 *  Convert from a slider index value to a value that
 *  represents a valid Canon White Balance internal value.
 */
void
ExposureComp::toValue( int index )
{
    if( n > 0 ) {
	if( index < 0   ) index = 0;
	if( index > n-1 ) index = n-1;
	emit propertyChanged( kEdsPropID_ExposureCompensation, v[index] );
    }
}

void
ExposureComp::updateExposures( int index )
{
    nBars = index;	// nBars: 0, 1, ...,  7 represents 1, 3, 5, ..., 15
    update();
}

void
ExposureComp::updateIncrement( int index )
{
    incr = index; // incr: 0, 1, ... represents 0/3, 1/3, ... or 0/2, 1/2, ...
    update();
}

void
ExposureComp::drawSigns( QPainter &painter, int i, QString s, int w )
{
    painter.save();

    QFont fontS;
    fontS.setFamily( "Calibri" );
    fontS.setPointSize( 4 );
    painter.setFont( fontS );
    QFontMetrics fmS( fontS );

    const QString minus( QChar(8722) );
    const int wm = fmS.width(minus);
    const QString plus( QChar(43) );
    const int wp = fmS.width(plus);

    if( i < 0 ) {
	painter.drawText( QPointF( i*10.0 - w - wm - 0.5, 8 ), minus );
    }
    else
    if( i > 0 ) {
	painter.drawText( QPointF( i*10.0 - w - wp - 0.5, 8 ), plus );
    }

    painter.restore();
}

void
ExposureComp::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setRenderHints( QPainter::Antialiasing, true );

    /*
     *  Sliding window with a range of seven digits over -8 .. 0 .. +8.
     */
    painter.setWindow( 10*center - width/2, 0, width, height );

    QFont font;
    font.setFamily( "Calibri" );
    font.setPointSize( 6 );
    painter.setFont( font );
    QFontMetrics fm( font );
    painter.setPen( Qt::black );

    /*
     *  Draw frame with a gradient background to indicate
     *  direction of darker and of lighter compensation.
     */
    QLinearGradient g( QPoint(-80, height/2),
		       QPoint( 80, height/2) );
    g.setColorAt( 0.0, Qt::darkGray );
    g.setColorAt( 1.0, Qt::white );
    painter.setBrush( g );
    painter.drawRoundedRect( 10*center - width/2, 0, width, height,
	7.5, 7.5, Qt::RelativeSize );

    /*
     *  Draw the integer exposure values and draw dots for
     *  the fractional values.
     */
    for( int i = (center-sw); i <= (center+sw); i++ ) {
	QString s( QString::number( qAbs(i) ) );
	int w = fm.width( s ) / 2;
	painter.drawText( i*10 - w, 10, s );
	if( i == (center-sw) || i == (center+sw) ) {
	    drawSigns( painter, i, s, w );
	}
    }
    const QString dot( "." );
    const int d = fm.width( dot ) / 2;
    for( int i = (center-sw); i < (center+sw); i++ ) {
	if( camera->getExposureLevelIncrements() == 0 ) {
	    // 1/3 - Stop
	    painter.drawText( QPointF( i*10 + 3.333 - d, 10 ), dot );
	    painter.drawText( QPointF( i*10 + 6.667 - d, 10 ), dot );
	}
	else {
    	    // 1/2 - Stop
	    painter.drawText( i*10 + 5 - d, 10, dot );
	}
    }

    int mode = camera->getShootingMode();
    if( (mode == kEdsAEMode_A_DEP)   ||
	(mode == kEdsAEMode_Manual)  ||
	(mode == kEdsAEMode_Av)      ||
	(mode == kEdsAEMode_Tv)      ||
	(mode == kEdsAEMode_Program) ) {
	/*
	 *  Draw bar representing current value.
	 */
	painter.setPen( Qt::black );
	painter.setBrush( Qt::white );
	painter.drawRect( QRectF( compensation*10.0 - 0.75, 12, 1.5, 6.5 ) );
	/*
	 *  Draw any additional exposures.
	 */
	double step = 0.0;
	if( camera->getExposureLevelIncrements() == 0 ) {
	    step = double(incr) / 3.0;	// 1/3 - Stop
	}
	else {
	    step = double(incr) / 2.0;	// 1/2 - Stop
	}
	if( (nBars > 0) && (step > 0.0) ) {
	    for( int i = 0; i < nBars; i++ ) {
		painter.drawRect( QRectF(
		    (compensation-(i+1)*step)*10.0 - 0.75, 12, 1.5, 4.5 ) );
		painter.drawRect( QRectF(
		    (compensation+(i+1)*step)*10.0 - 0.75, 12, 1.5, 4.5 ) );
	    }
	}
    }

    /*
     *  Indicate whether the widget has keyboard focus
     *  (has been tabbed to).
     */
    if( hasFocus() ) {
	QPen pen;
	pen.setWidth( 0 );
	pen.setStyle( Qt::DotLine );
	pen.setColor( Qt::black );
	painter.setPen( pen );
	painter.setBrush( Qt::NoBrush );
	painter.drawRect( 10*center - width/2 + 1, 1, width-2, height-2 );
    }

    /*
     *  Indicate whether the widget is disabled.
     */
    if( !isEnabled() ) {
	painter.fillRect( QRect( 10*center - width/2, 0, width, height ),
			  QColor(214,214,214,128) );
    }
}

