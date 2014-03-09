/*
 *  ISO speed setting and display widget.
 */

#include <QtGui>
#include "ISOSpeed.h"
#include "Camera.h"
#include "Map.h"
#include "size.h"


ISOSpeed::ISOSpeed( QWidget *parent )
    : QWidget(parent)
{
    n = 0;
    v = 0;

    maxCols = 3;
    maxRows = Map::MAX_ISO / maxCols;
    if( (Map::MAX_ISO % maxCols) != 0 ) {
	maxRows++;
    }

    setFixedSize( 80, ROW_HEIGHT );

    initialize();
}

ISOSpeed::~ISOSpeed()
{
    delete ISO_icon;
    delete[] item;
    //QObject::dumpObjectInfo();
    //QObject::dumpObjectTree();
}

void
ISOSpeed::initialize()
{
    QFont font;
    font.setFamily( "Calibri" );
    font.setPointSize( FONT_SIZE );

    ISO_icon = new QIcon( ":/Resources/Misc/ISO.png" );

    item = new QStandardItem *[maxRows * maxCols];
    QStandardItemModel *model =
	new QStandardItemModel( maxRows, maxCols, this );
    int i = 0;
    for( int row = 0; row < maxRows; row++ ) {
	for( int col = 0; col < maxCols; col++ ) {
	    item[i] = new QStandardItem();
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
    view->setIconSize( QSize( 20, 13 ) );

    comboBox = new QComboBox( this );
    comboBox->setMaxVisibleItems( 16 );
    comboBox->setModel( model );
    comboBox->setFont( font );
    comboBox->setView( view );
    comboBox->setIconSize( QSize( 20, 13 ) );
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
ISOSpeed::eventFilter( QObject *target, QEvent *event )
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
	    emit propertyChanged( kEdsPropID_ISOSpeed, v[index] );
	}
	return( true );
    }
    return( QWidget::eventFilter( target, event ) );
}

void
ISOSpeed::updateValue( Camera *camera )
{
    int index, value;

    /*
     *  Get the current ISO value.
     */
    value = camera->getISO();

    if( n == 0 ) {
	/*
	 *  Auto ISO speed mode, either "Auto" value or current auto setting.
	 */
	item[0]->setIcon( *ISO_icon );
	item[0]->setText( Map::toString_ISO( value ) );
	index = 0;
    }
    else {
	/*
	 *  Current user setting of ISO speed.
	 */
	for( index = 0; index < n; index++ ) {
	    if( value == v[index] ) break;
	}
	if( index >= n ) {
	    index = 0;	// Shouldn't happen
	}
    }

    int row = index / maxCols;
    int col = index % maxCols;
    comboBox->setCurrentIndex( row );
    comboBox->setModelColumn( col );
    update();
}

void
ISOSpeed::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for ISO value.
     */
    camera->getISO_List( &n, &v );

    if( n == 0 ) {
	/*
	 *  Auto ISO speed mode.
	 */
	comboBox->setEnabled( false );
    }
    else {
	/*
	 *  Current list of choices for ISO speed.
	 */
	for( int i = 0; i < n; i++ ) {
	    item[i]->setIcon( *ISO_icon );
	    item[i]->setText( Map::toString_ISO( v[i] ) );
	    item[i]->setEnabled( true );
	    if( (i % maxCols) == 0 ) {
		view->showRow( i / maxCols );
	    }
	}
	for( int i = n; i < maxRows*maxCols; i++ ) {
	    item[i]->setIcon( QIcon() );
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
ISOSpeed::toValue()
{
    if( n > 0 ) {
	QPoint point = view->viewport()->mapFromGlobal( QCursor::pos() );
	int row = view->rowAt( point.y() );
	int col = view->columnAt( point.x() );
	int index = row * maxCols + col;
	emit propertyChanged( kEdsPropID_ISOSpeed, v[index] );
    }
}

