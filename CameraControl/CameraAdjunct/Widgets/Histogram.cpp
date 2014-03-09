/*
 *  Histogram display widget.
 */

#include <QtGui>
#include "Histogram.h"
#include "LiveImageData.h"


/*
 *  Constructor for histogram widget.
 */
Histogram::Histogram( QWidget *parent )
    : QWidget( parent )
{
    validData = false;
    normal = false;
    luminosity = false;

    QPalette palette;
    QBrush darkBrush( QColor( 50, 50, 50, 255 ) );
    darkBrush.setStyle(Qt::SolidPattern);
    palette.setBrush( QPalette::Active, QPalette::Window, darkBrush );
    palette.setBrush( QPalette::Disabled, QPalette::Window, darkBrush );
    palette.setBrush( QPalette::Inactive, QPalette::Window, Qt::lightGray );
    setPalette( palette );
    setAutoFillBackground( true );

    setFixedSize( 194, 130 ); // +2 for the box
    //setFixedSize( 258, 130 ); // +2 for the box
}

/*
 *  Destructor for histogram widget.
 */
Histogram::~Histogram()
{
}

/*
 *  A new histogram has arrived.
 */
void
Histogram::setData( EvfImageData *data )
{
    if( data == NULL ) {
	validData = false;
	update();
	return;
    }

    validData = true;
    /*
     *  The camera only supplies histogram data if the zoom factor
     *  is 1x, and grays out the histogram during focusing.
     */
    if( data->zoom == 1 ) {
	normal = data->histogramStatus & EvfImageData::HISTOGRAM_Normal;
    }
    else {
	normal = false;
    }
    /*
     *  The histogram is either a luminosity histogram or an RGB histogram.
     */
    luminosity = data->histogramStatus & EvfImageData::HISTOGRAM_Luminosity;
    for( int i = 0; i < 256; i++ ) {
	histogram_Y[i] = data->histogram_Y[i];
	histogram_R[i] = data->histogram_R[i];
	histogram_G[i] = data->histogram_G[i];
	histogram_B[i] = data->histogram_B[i];
    }

    if( isVisible() ) {
	update();
    }
}

/*
 *  Draw the histogram.
 */
void
Histogram::paintEvent( QPaintEvent *event )
{
    if( !validData ) {
	return;
    }

    QPainter painter(this);
    QPen pen;
    painter.setPen( pen );

    const double width = 192.0;
    const double height = 128.0;

    const double x_offset =  1.0;
    const double step = width / 256.0;
    double x;
    double y;

    /*
     *  The histogram has four guide bars.
     */
    pen.setColor( Qt::darkGray );
    pen.setWidth( step );
    painter.setPen( pen );
    const int barWidth = width / 5;
    for( int i = 1; i <= 4; i++ ) {
	x = x_offset + i*barWidth;
	painter.drawLine( QLineF( x, 0, x, height ) );
    }

    /*
     *  Find the average histogram value. The values are then
     *  scaled so that the average value is placed 1/6 of the
     *  way up in the window. This appears to correspond to the
     *  way the camera and Lightroom display the histogram.
     */
    int total = 0;
    for( int i = 0; i < 256; i++ ) {
	total += histogram_Y[i];
    }
    const double scale = height / static_cast<double>((total / 256) * 6);

    if( luminosity ) {
	/*
	 *  Luminosity histogram.
	 */
	pen.setWidth( step );

	if( normal ) pen.setColor( QColor( 240, 240, 190 ) ); // light yellow
	else	     pen.setColor( Qt::gray );
	painter.setPen( pen );
	for( int i = 0; i < 256; i++ ) if( histogram_Y[i] ) {
	    x = x_offset + i*step;
	    y = histogram_Y[i] * scale;
	    painter.drawLine( QLineF( x, height - y, x, height ) );
	}
    }
    else {
	/*
	 *  RGB histogram.
	 */
	if( normal ) {
	    painter.save();
	    painter.setCompositionMode( QPainter::CompositionMode_Screen );
	}
	pen.setWidth( step );

	if( normal ) pen.setColor( Qt::red );
	else	     pen.setColor( Qt::gray );
	painter.setPen( pen );
	for( int i = 0; i < 256; i++ ) if( histogram_R[i] ) {
	    x = x_offset + i*step;
	    y = histogram_R[i] * scale;
	    painter.drawLine( QLineF( x, height - y, x, height ) );
	}

	if( normal ) pen.setColor( Qt::green );
	else	     pen.setColor( Qt::gray );
	painter.setPen( pen );
	for( int i = 0; i < 256; i++ ) if( histogram_G[i] ) {
	    x = x_offset + i*step;
	    y = histogram_G[i] * scale;
	    painter.drawLine( QLineF( x, height - y, x, height ) );
	}

	if( normal ) pen.setColor( Qt::blue );
	else	     pen.setColor( Qt::gray );
	painter.setPen( pen );
	for( int i = 0; i < 256; i++ ) if( histogram_B[i] ) {
	    x = x_offset + i*step;
	    y = histogram_B[i] * scale;
	    painter.drawLine( QLineF( x, height - y, x, height ) );
	}

	if( normal ) {
	    painter.restore();
	}
    }

    /*
     *  Draw boxes in upper left and upper right indicating degree
     *  of shadow and highlight clipping. The contribution of each
     *  channel (r, g, b) to the final color of the box is weighted
     *  by the degree of clipping in that channel. The RGB values
     *  are scaled to the equivalent of:
     *	  min( 255, (int)( 255 * (histogram_R[0]/512.0) ) )
     *  where 512 was chosen through some experimentation.
     */
    const int bw = 9;
    int r, g, b;
    pen.setWidth( 1 );
    pen.setColor( Qt::darkGray );
    painter.setPen( pen );
    // shadow clipping
    r = histogram_R[0] / 2;
    g = histogram_G[0] / 2;
    b = histogram_B[0] / 2;
    if( r > 255 ) r = 255;
    if( g > 255 ) g = 255;
    if( b > 255 ) b = 255;
    painter.fillRect( QRect( 1, 1, bw, bw ), QColor( r, g, b ) );
    painter.drawRect( QRect( 1, 1, bw, bw ) );
    // highlight clipping
    r = histogram_R[255] / 2;
    g = histogram_G[255] / 2;
    b = histogram_B[255] / 2;
    if( r > 255 ) r = 255;
    if( g > 255 ) g = 255;
    if( b > 255 ) b = 255;
    painter.fillRect( QRect( width - bw, 1, bw, bw ), QColor( r, g, b ) );
    painter.drawRect( QRect( width - bw, 1, bw, bw ) );

    /*
     *  Draw a border around the histogram.
     */
    pen.setWidth( 1 );
    pen.setColor( Qt::darkGray );
    painter.setPen( pen );
    painter.drawRect( QRect( 0, 0, width+1, height+1 ) );
}

