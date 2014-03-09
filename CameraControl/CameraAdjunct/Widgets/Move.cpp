/*
 *  Widget for moving around in image by stepping zoom rectangle
 *  left, right, up, and down. Also centers zoom rectangle.
 */

#include <QtGui>
#include <QTimer>
#include "Move.h"


Move::Move( QWidget *parent )
    : QWidget( parent )
{
    quadrant = 0;
    viewRect.setRect( 0, 0, 42, 42 );
    worldRect.setRect( -50, -50, 100, 100 );

    /*
     *  Auto repeat
     */
    timer.setInterval( 250 );	// milliseconds
    QObject::connect( &timer, SIGNAL(timeout()), this, SLOT(processMove()) );

    setMouseTracking( true );
    setStatusTip(tr("Center or move navigation window" ));

    setFixedSize( viewRect.size() );
    setFocusPolicy( Qt::TabFocus );
}

Move::~Move()
{
}

void
Move::processMove()
{
    if( quadrant != 0 ) {	// set in mouseMoveEvent or keyPressEvent
	emit moveDirection( quadrant );
    }
}

void
Move::mousePressEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton ) {
	processMove();
	if( (quadrant != 0) && (quadrant != Qt::Key_Home) ) {
	    timer.start();
	}
    }
}

void
Move::mouseMoveEvent( QMouseEvent *event )
{
    timer.stop();
    QPoint p = event->pos() - viewRect.center();
    if( (qAbs(p.x()) < 8) && (qAbs(p.y()) < 8) ) {
	quadrant = Qt::Key_Home; // approx. as a rectangle is accurate enough
    }
    else {
	QMatrix matrix;  
	matrix.rotate( 45 );
	QPoint q = matrix.map( p );
	if(      (q.x() <= 0) && (q.y() <  0) ) quadrant = Qt::Key_Left;
	else if( (q.x() >  0) && (q.y() <= 0) ) quadrant = Qt::Key_Up;
	else if( (q.x() <  0) && (q.y() >= 0) ) quadrant = Qt::Key_Down;
	else				        quadrant = Qt::Key_Right;
    }
    update();
}

void
Move::mouseReleaseEvent( QMouseEvent *event )
{
    timer.stop();
}

void
Move::leaveEvent( QEvent *event )
{
    timer.stop();
    quadrant = 0;
    event->accept();
    update();
}

void
Move::keyPressEvent( QKeyEvent *event )
{
    if( (event->key() == Qt::Key_Home) ||
	(event->key() == Qt::Key_Left) ||
	(event->key() == Qt::Key_Right) ||
	(event->key() == Qt::Key_Up) ||
	(event->key() == Qt::Key_Down) ) {
	quadrant = event->key();
	processMove();
	update();
    }
    else {
	QWidget::keyPressEvent( event );
    }
}

void
Move::keyReleaseEvent( QKeyEvent *event )
{
    if( (event->key() == Qt::Key_Home) ||
	(event->key() == Qt::Key_Left) ||
	(event->key() == Qt::Key_Right) ||
	(event->key() == Qt::Key_Up) ||
	(event->key() == Qt::Key_Down) ) {
	quadrant = 0;
	update();
    }
    else {
	QWidget::keyPressEvent( event );
    }
}

void
Move::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setRenderHints( QPainter::Antialiasing, true );
    painter.setWindow( worldRect );
    painter.setViewport( viewRect );

    painter.setPen( Qt::NoPen );

    QBrush brushMedium( QColor( 127, 127, 127 ) );
    painter.setBrush( brushMedium );
    painter.drawEllipse( worldRect );

    /*
     *  Highlight the direction the mouse is hovering over
     *  or the key that was pressed.
     */
    if( (quadrant != 0) /*&& underMouse()*/ ) {
	QBrush brushLight( QColor( 165, 165, 165 ) );
	painter.setBrush( brushLight );
	switch( quadrant ) {
	    case Qt::Key_Home:
		    painter.drawEllipse( QPoint( 0, 0 ), 16, 16 );
		    break;
	    case Qt::Key_Left:
		    painter.drawPie( worldRect, 135*16, 90*16 );
		    break;
	    case Qt::Key_Right:
		    painter.drawPie( worldRect, -45*16, 90*16 );
		    break;
	    case Qt::Key_Up:
		    painter.drawPie( worldRect, 45*16, 90*16 );
		    break;
	    case Qt::Key_Down:
		    painter.drawPie( worldRect, -135*16, 90*16 );
		    break;
	}
    }

    /*
     *  Show the centering button if it wasn't drawn
     *  highlighted above.
     */
    if( (quadrant != Qt::Key_Home) /*|| !underMouse()*/ ) {
	QBrush brushDark( QColor( 93, 93, 93 ) );
	painter.setBrush( brushDark );
	painter.drawEllipse( QPoint( 0, 0 ), 16, 16 );
    }

    QPen pen;
    pen.setWidth( 3 );
    pen.setColor( Qt::white );
    painter.setPen( pen );

    /*
     *  Draw the angles indicating direction.
     */
    const int r = 35;
    const int l =  6;
    static const int line[8][4] = {
	{  r,  0,  r-l,   -l },
	{  r,  0,  r-l,    l },
	{ -r,  0, -r+l,    l },
	{ -r,  0, -r+l,   -l },
	{  0,  r,   -l,  r-l },
	{  0,  r,    l,  r-l },
	{  0, -r,   -l, -r+l },
	{  0, -r,    l, -r+l }
    };
    for( int i = 0; i < 8; i++ ) {
	painter.drawLine( line[i][0], line[i][1], line[i][2], line[i][3] );
    }

    /*
     *  Indicate whether the widget has keyboard focus
     *  (has been tabbed to).
     */
    if( hasFocus() ) {
	pen.setWidth( 2 );
	pen.setStyle( Qt::DotLine );
	pen.setColor( Qt::black );
	painter.setPen( pen );
	painter.setBrush( Qt::NoBrush );
	painter.drawRoundedRect( -50, -50, 100, 100, 20, 20, Qt::RelativeSize );
    }

    /*
     *  Indicate whether the widget is disabled.
     */
    if( !isEnabled() ) {
	painter.fillRect( worldRect, QColor(214,214,214,128) );
    }
}

