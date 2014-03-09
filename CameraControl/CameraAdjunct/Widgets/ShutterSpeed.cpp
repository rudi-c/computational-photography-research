/*
 *  Shutter speed setting and display widget.
 */

#include <QtGui>
#include "ShutterSpeed.h"
#include "Camera.h"
#include "Map.h"
#include "size.h"


ShutterSpeed::ShutterSpeed( QWidget *parent )
    : QWidget(parent)
{
    n = 0;
    v = 0;

    maxCols = 3;
    maxRows = Map::MAX_Tv / maxCols;
    if( (Map::MAX_Tv % maxCols) != 0 ) {
	maxRows++;
    }

    setFixedSize( 80, ROW_HEIGHT );

    initialize();
}

ShutterSpeed::~ShutterSpeed()
{
    delete[] item;
}

void
ShutterSpeed::initialize()
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

    view = new QTableView( this );
    view->setModel( model );
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();
    for( int row = 0; row < maxRows; row++ ) {
	view->setRowHeight( row, ROW_HEIGHT );
    }
    for( int col = 0; col < maxCols; col++ ) {
	view->setColumnWidth( col, COL_WIDTH );
    }
    view->setFixedWidth( COL_WIDTH * maxCols + 15 ); // +15 for scroll bar
    view->setShowGrid( false );

    comboBox = new QComboBox( this );
    comboBox->setMaxVisibleItems( 18 );
    comboBox->setModel( model );
    comboBox->setFont( font );
    comboBox->setView( view );
    comboBox->setFixedSize( 80, 23 );
    comboBox->setCurrentIndex( 0 );
    comboBox->setModelColumn( 0 );
    comboBox->installEventFilter( this );
    QObject::connect( comboBox, SIGNAL(activated(int)),
	this, SLOT(toValue()));
}

/*
 *  Intercept wheel events from comboBox.
 */
bool
ShutterSpeed::eventFilter( QObject *target, QEvent *event )
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
	    emit propertyChanged( kEdsPropID_Tv, v[index] );
	}
	return( true );
    }
    return( QWidget::eventFilter( target, event ) );
}

void
ShutterSpeed::updateValue( Camera *camera )
{
    int index, value;

    /*
     *  Get the current shutter speed.
     */
    value = camera->getTv();

    if( camera->getShootingMode() == kEdsAEMode_Bulb ) {
	/*
	 *  Shooting mode is set to Bulb shooting.
	 *  On some cameras Bulb mode is a shooting mode;
	 *  on others it is a shutter speed setting.
	 */
	item[0]->setText( QString( "Bulb" ) );
	index = 0;
    }
    else
    if( value == 0 ) {
	/*
	 *  Auto shutter speed value, so don't display.
	 */
	item[0]->setText( QString() );
	index = 0;
    }
    else
    if( n == 0 ) {
	/*
	 *  Auto shutter speed mode, current auto setting.
	 */
	item[0]->setText( Map::toString_Tv( value ) );
	index = 0;
    }
    else {
	/*
	 *  Current user setting of shutter speed.
	 */
	for( index = 0; index < n; index++ ) {
	    if( value == v[index] ) break;
	}
	if( index >= n ) {
	    /*
	     *  Value not found. This can happen when changing the focal
	     *  length of a zoom lens. The value of the current shutter speed
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
ShutterSpeed::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for Tv value.
     */
    camera->getTv_List( &n, &v );

    if( n == 0 ) {
	/*
	 *  Auto shutter speed mode.
	 */
	comboBox->setEnabled( false );
    }
    else {
	/*
	 *  Current list of choices for shutter speed.
	 */
	for( int i = 0; i < n; i++ ) {
	    item[i]->setText( Map::toString_Tv( v[i] ) );
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
 *  represents a valid Canon ISO internal value.
 */
void
ShutterSpeed::toValue()
{
    if( n > 0 ) {
	QPoint point = view->viewport()->mapFromGlobal( QCursor::pos() );
	int row = view->rowAt( point.y() );
	int col = view->columnAt( point.x() );
	int index = row * maxCols + col;
	emit propertyChanged( kEdsPropID_Tv, v[index] );
    }
}

