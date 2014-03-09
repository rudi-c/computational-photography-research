/*
 *  Widget for displaying depth of field calculations.
 */

#include <QtGui>
#include <math.h>
#include "DepthOfField.h"


/*
 *  Constructor for depth of field widget.
 */
DepthOfField::DepthOfField( QString focusText, QWidget *parent )
    : QWidget( parent )
{
    this->focusText = focusText;

    focusImage = new QImage( ":/Resources/Misc/dof-focus.png", "PNG" );
    unfocusImage = new QImage( ":/Resources/Misc/dof-unfocus.png", "PNG" );

    setFixedSize( windowWidth, windowHeight );
}

/*
 *  Destructor for depth of field widget.
 */
DepthOfField::~DepthOfField()
{
    delete focusImage;
    delete unfocusImage;
}

void
DepthOfField::setData( double focusDistance,
		       double nearDistance,
		       double farDistance,
		       int units )
{
    this->focusDistance = focusDistance;
    this->nearDistance = nearDistance;
    this->farDistance = farDistance;
    this->units = units;

    drawImage();
    update();
}

/*
 *  Draw the depth of field widget.
 */
void
DepthOfField::drawImage()
{
    const double focus = toImageCoordinate( focusDistance );
    const double near = toImageCoordinate( nearDistance );
    const double far = toImageCoordinate( farDistance );

    const QString focusString = toString( focusDistance );
    const QString nearString = toString( nearDistance );
    const QString farString = toString( farDistance );

    /*
     *  Construct the image:
     *  green and infocus for part that is within depth of field,
     *  gray and out of focus for part that is outside depth of field.
     */
    image = unfocusImage->copy();
    for( int row = 0; row < image.height(); row++ ) {
	unsigned int *rowData =
	    (unsigned int *)image.scanLine( row );
	unsigned int *rowDataFocus =
	    (unsigned int *)focusImage->scanLine( row );
	for( int col = near; col <= far; col++ ) {
	    rowData[col] = rowDataFocus[col];
	}
    }

    /*
     *  Set the tick marks and tick labels.
     */
    QPen pen;
    pen.setColor( Qt::darkGray );
    pen.setWidth( 1 );
    QPainter painter( &image );
    painter.initFrom( this );
    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.setPen( pen );
    double factor;
    if(      units == UNITS_Feet   ) factor =  304.8;
    else if( units == UNITS_Metres ) factor = 1000.0;
    for( double l = 0.1; l <= 1000.0; l *= 10.0 ) {
	for( double d = l; d <= 10*l; d += l ) {
	    double x = toImageCoordinate( d * factor );
	    painter.drawLine( QLineF( x, vOffset+3, x, vOffset+7 ) );
	}
    }
    for( double l = 0.1; l <= 10000.0; l *= 10.0 ) {
	double x = toImageCoordinate( l * factor );
	painter.drawLine( QLineF( x, vOffset+3, x, vOffset+10 ) );
	QString tickText = toString( l * factor );
	if( tickText.size() == 1 ) {
	    painter.drawText(
		x - fontMetrics().width( tickText, 1 )/2,
		vOffset + 8 + fontMetrics().height(),
		tickText );
	}
	else {
	    painter.drawText(
		x - fontMetrics().width( tickText, 2 ),
		vOffset + 8 + fontMetrics().height(),
		tickText );
	}
    }

    /*
     *  Set the focus distance.
     */
    const QString legend = focusText + " distance";

    pen.setColor( Qt::black );
    painter.setPen( pen );
    painter.drawLine( QLineF( focus, vOffset, focus, vOffset-77.0 ) );

    /*
     *  Draw arrows.
     */
    double arrowSize = 8;
    const double M_PI_3 = M_PI/3.0;
    if( (focus + fontMetrics().width( legend ) + 20) < windowWidth ) {
	painter.drawText(
	    focus - fontMetrics().width( focusString ) - 2,
	    vOffset - 68,
	    focusString );
	painter.drawText(
	    focus + 20,
	    vOffset - 68,
	    legend );
	QPointF srcPoint( focus + 1, vOffset - 72  );
	QPointF dstPoint( focus + 18, vOffset - 72 );
	QLineF line( srcPoint, dstPoint );
	painter.drawLine( line ); 
	QPointF srcArrowP1 = srcPoint + QPointF(sin(M_PI_3) * arrowSize,
						cos(M_PI_3) * arrowSize);
	QPointF srcArrowP2 = srcPoint + QPointF(sin(M_PI - M_PI_3) * arrowSize,
						cos(M_PI - M_PI_3) * arrowSize);
	painter.setBrush( Qt::black );
	painter.drawPolygon(
	    QPolygonF() << line.p1() << srcArrowP1 << srcArrowP2 );
    }
    else {
	painter.drawText(
	    focus + 4,
	    vOffset - 68,
	    focusString );
	painter.drawText(
	    focus - fontMetrics().width( legend ) - 20,
	    vOffset - 68,
	    legend );
	QPointF srcPoint( focus - 18, vOffset - 72  );
	QPointF dstPoint( focus -  1, vOffset - 72 );
	QLineF line( srcPoint, dstPoint );
	painter.drawLine( line ); 
	QPointF dstArrowP1 = dstPoint + QPointF(sin(-M_PI_3) * arrowSize,
						cos(-M_PI_3) * arrowSize);
	QPointF dstArrowP2 = dstPoint + QPointF(sin(-M_PI + M_PI_3) * arrowSize,
						cos(-M_PI + M_PI_3) * arrowSize);
	painter.setBrush( Qt::black );
	painter.drawPolygon(
	    QPolygonF() << line.p2() << dstArrowP1 << dstArrowP2 );
    }

    /*
     *  Set the depth of field:
     *      2.5 m |<----->| 12.0m
     *         Depth of field
     */
    const QString dof = tr("Depth of field");
    const int top = vOffset + 2*fontMetrics().height();
    painter.drawLine( QLineF( near, top+10, near, top ) );
    painter.drawLine( QLineF( far, top+10, far, top ) );
    painter.drawText( near - fontMetrics().width( nearString ) - 2,
	top+10, nearString );
    painter.drawText( far + 4,
	top+10, farString );
    painter.drawText( near + (far - near)/2 - fontMetrics().width( dof )/2,
	top+10 + fontMetrics().height(), dof );
    arrowSize = (far - near)/2 - 1;
    if( arrowSize > 8 ) arrowSize = 8;
    QPointF srcPoint( near+1, top+5  );
    QPointF dstPoint( far-1, top+5 );
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
    painter.setBrush( Qt::black );
    painter.drawPolygon(
	QPolygonF() << line.p1() << srcArrowP1 << srcArrowP2 );
    painter.drawPolygon(
	QPolygonF() << line.p2() << dstArrowP1 << dstArrowP2 );

}

double
DepthOfField::toImageCoordinate( double x )
{
    if(      units == UNITS_Feet   ) x /=  304.8;
    else if( units == UNITS_Metres ) x /= 1000.0;

    if( 0 <= x && x <     0.1 ) x =     0.1;
    if( 0 >  x || x > 10000.0 ) x = 10000.0;

    return( (log10( x ) * step) + hOffset );
}

/*
 *  Convert from millimetres to feet/miles or metres/km and format.
 *  Assume three significant digits.
 */
QString
DepthOfField::toString( double x )
{
    const QString infinity = QString(QChar(8734));

    QString text;

    if( units == UNITS_Feet ) {
	x /= 304.8; // conversion from millimetres to feet

	if(                   x <     0 ) text = infinity;
	else if(    0 <= x && x <    10 ) text.sprintf( "%0.2f ft", x );
	else if(   10 <= x && x <   100 ) text.sprintf( "%0.1f ft", x );
	else if(  100 <= x && x <  5280 ) text.sprintf( "%0.0f ft", x );
	else if( 5280 <= x && x < 10000 ) text.sprintf( "%0.1f mi", x/5280.0 );
	else				  text = infinity;
    }
    else
    if( units == UNITS_Metres ) {
	x /= 1000.0; // conversion from millimetres to metres

	if(                   x <     0 ) text = infinity;
	else if(    0 <= x && x <    10 ) text.sprintf( "%0.2f m", x );
	else if(   10 <= x && x <   100 ) text.sprintf( "%0.1f m", x );
	else if(  100 <= x && x <  1000 ) text.sprintf( "%0.0f m", x );
	else if( 1000 <= x && x < 10000 ) text.sprintf( "%0.1f km", x/1000.0 );
	else				  text = infinity;
    }

    return( text );
}

/*
 *  Paint the depth of field widget.
 */
void
DepthOfField::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.drawImage( 0, 0, image );
}

