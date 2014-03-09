/*
 *  Aperture setting and displaying widget.
 */

#include <QtGui>
#include "Aperture.h"
#include "Camera.h"
#include "Map.h"
#include "size.h"


Aperture::Aperture( QWidget *parent )
    : QWidget(parent)
{
    n = 0;
    v = 0;

    maxCols = 3;
    maxRows = Map::MAX_Av / maxCols;
    if( (Map::MAX_Av % maxCols) != 0 ) {
	maxRows++;
    }

    setFixedSize( 80, ROW_HEIGHT );

    initialize();
}

Aperture::~Aperture()
{
    delete[] item;
}

void
Aperture::initialize()
{
    QFont font;
    font.setFamily( "Calibri" );
    font.setPointSize( FONT_SIZE );

    item = new QStandardItem *[maxRows * maxCols];
    QStandardItemModel *model =
	new QStandardItemModel( maxRows, maxCols, this );
    int i = 0;
    for( int row = 0; row < maxRows; row++ ) {
	for( int col = 0; col < maxCols; col++ ) {
	    item[i] = new QStandardItem();
	    item[i]->setData( Qt::AlignCenter, Qt::TextAlignmentRole );
	    model->setItem( row, col, item[i] );
	    i++;
	}
    }

    view = new QTableView();
    view->setModel( model );
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();
    for( int row = 0; row < maxRows; row++ ) {
	view->setRowHeight( row, ROW_HEIGHT );
    }
    for( int col = 0; col < maxCols; col++ ) {
	view->setColumnWidth( col, COL_WIDTH );
    }
    view->setFixedWidth( COL_WIDTH * maxCols );
    view->setShowGrid( false );

    comboBox = new QComboBox( this );
    comboBox->setMaxVisibleItems( 16 );
    comboBox->setModel( model );
    comboBox->setFont( font );
    comboBox->setView( view );
    comboBox->setFixedSize( 80, 23 );
    comboBox->setCurrentIndex( 0 );
    comboBox->setModelColumn( 0 );
    comboBox->installEventFilter( this );
    QObject::connect(
	comboBox, SIGNAL(activated(int)),
	this, SLOT(toValue()));
}

/*
 *  Intercept wheel events from comboBox.
 */
bool
Aperture::eventFilter( QObject *target, QEvent *event )
{
    if( (target == comboBox) && (event->type() == QEvent::Wheel) ) {
	QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
	int step;
	if( wheelEvent->delta() < 0 ) step = -1;
	else			      step = +1;
	int row = comboBox->currentIndex();
	int col = comboBox->modelColumn();
	int index = row * maxCols + col;
	index += step;
	if( index < 0 ) index = 0;
	if( index > n-1 ) index = n-1;
	row = index / maxCols;
	col = index % maxCols;
	comboBox->setCurrentIndex( row );
	comboBox->setModelColumn( col );
	if( n > 0 ) {
	    emit propertyChanged( kEdsPropID_Av, v[index] );
	}
	return( true );
    }
    return( QWidget::eventFilter( target, event ) );
}

void
Aperture::updateValue( Camera *camera )
{
    int index, value;

    /*
     *  Get the current aperture.
     */
    value = camera->getAv();

    if( value == 0 ) {
	/*
	 *  "Auto" aperture value, so don't display.
	 */
	item[0]->setText( QString() );
	index = 0;
    }
    else
    if( n == 0 ) {
	/*
	 *  Auto aperture mode, current auto setting.
	 */
	QString text;
	text.sprintf( "F%s", Map::toString_Av( value ) );
	item[0]->setText( text );
	index = 0;
    }
    else {
	/*
	 *  Current user setting of aperture.
	 */
	for( index = 0; index < n; index++ ) {
	    if( value == v[index] ) break;
	}
	if( index >= n ) {
	    /*
	     *  Value not found. This can happen when changing the focal
	     *  length of a zoom lens. The value of the current aperture
	     *  isn't in the list of choices as the list hasn't been
	     *  updated yet. Just set the index to a valid value and
	     *  wait for the signal from the camera to update the list.
	     */
	    index = 0;
	}
    }

    int row = index / maxCols;
    int col = index % maxCols;
    comboBox->setCurrentIndex( row );
    comboBox->setModelColumn( col );
    update();
}

void
Aperture::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for Av value.
     */
    camera->getAv_List( &n, &v );

    if( n == 0 ) {
	/*
	 *  Auto aperture mode.
	 */
	comboBox->setEnabled( false );
    }
    else {
	/*
	 *  Current list of choices for aperture.
	 */
	for( int i = 0; i < n; i++ ) {
	    QString text;
	    text.sprintf( "F%s", Map::toString_Av( v[i] ) );
	    item[i]->setText( text );
	    item[i]->setEnabled( true );
	    if( (i % maxCols) == 0 ) {
		view->showRow( i / maxCols );
	    }
	}
	for( int i = n; i < maxRows*maxCols; i++ ) {
	    item[i]->setText( QString() );
	    item[i]->setEnabled( false );
	    // starting an empty row
	    if( (i % maxCols) == 0 ) {
		view->hideRow( i / maxCols );
	    }
	}
	comboBox->setEnabled( true );
    }

    updateValue( camera );
}

/*
 *  Convert from a row, col position to a value that
 *  represents a valid Canon Av internal value.
 */
void
Aperture::toValue()
{
    if( n > 0 ) {
	QPoint point = view->viewport()->mapFromGlobal( QCursor::pos() );
	int row = view->rowAt( point.y() );
	int col = view->columnAt( point.x() );
	int index = row * maxCols + col;
	emit propertyChanged( kEdsPropID_Av, v[index] );
    }
}

