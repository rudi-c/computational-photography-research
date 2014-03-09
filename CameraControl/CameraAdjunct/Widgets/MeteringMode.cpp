/*
 *  Metering mode setting and display widget.
 */

#include <QtGui>
#include "MeteringMode.h"
#include "Camera.h"
#include "Map.h"
#include "size.h"


MeteringMode::MeteringMode( QWidget *parent )
    : QWidget(parent)
{
    n = 0;
    v = 0;

    setFixedSize( 80, ROW_HEIGHT );

    initialize();
}

MeteringMode::~MeteringMode()
{
}

void
MeteringMode::initialize()
{
    model = new QStandardItemModel( Map::MAX_MeteringMode, 2, this ); // rows, cols
    for( int row = 0; row < Map::MAX_MeteringMode; row++ ) {
	model->setItem( row, 0, new QStandardItem() );
	model->setItem( row, 1, new QStandardItem() );
    }

    view = new QTableView( this );
    view->setModel( model );
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();
    view->setShowGrid( false );
    view->setIconSize( QSize( ICON_WIDTH, ICON_HEIGHT ) );
    for( int row = 0; row < Map::MAX_MeteringMode; row++ ) {
	view->setRowHeight( row, ROW_HEIGHT );
    }
    view->setColumnWidth( 0, ICON_COL_WIDTH );
    view->setColumnWidth( 1, COL_WIDTH );
    view->setFixedWidth( ICON_COL_WIDTH + COL_WIDTH );

    comboBox = new QComboBox( this );
    comboBox->setModel( model );
    comboBox->setModelColumn( 0 );
    comboBox->setView( view );
    comboBox->setIconSize( QSize( ICON_WIDTH, ICON_HEIGHT ) );
    comboBox->setFixedSize( 59, ROW_HEIGHT );
comboBox->setFixedSize( 80, ROW_HEIGHT );
    QObject::connect(
	comboBox, SIGNAL(activated(int)),
	this, SLOT(toValue(int)) );
}

void
MeteringMode::updateValue( Camera *camera )
{
    int index, value;

    /*
     *  Get the current metering mode.
     */
    value = camera->getMeteringMode();

    if( n == 0 ) {
	/*
	 *  Auto metering mode, current auto setting.
	 */
	setPage( 0, value );
	index = 0;
    }
    else {
	/*
	 *  Current user setting of metering mode.
	 */
	for( index = 0; index < n; index++ ) {
	    if( value == v[index] ) break;
	}
	if( index >= n ) {
	    // doesn't happen
	    index = 0;
	}
    }

    comboBox->setCurrentIndex( index );
    update();
}

void
MeteringMode::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for metering mode.
     */
    camera->getMeteringMode_List( &n, &v );

    if( n == 0 ) {
	/*
	 *  Auto metering mode.
	 */
	setEnabled( false );
    }
    else {
	/*
	 *  Set current list of choices for metering mode.
	 */
	for( int i = 0; i < n; i++ ) {
	    setPage( i, v[i] );
	}
	for( int i = n; i < Map::MAX_MeteringMode; i++ ) {
	    //model->item( i, 0 )->setIcon( QIcon() );
	    //model->item( i, 1 )->setText( QString() );
	    view->hideRow( i );
	}
	setEnabled( true );
    }

    updateValue( camera );
}


/*
 *  Convert from a combo box index to a value that
 *  represents a valid Canon Metering Mode internal value.
 */
void
MeteringMode::toValue( int index )
{
    comboBox->setCurrentIndex( index ); // needed because of a bug in Qt
    if( n > 0 ) {
	emit propertyChanged( kEdsPropID_MeteringMode, v[index] );
    }
}

void
MeteringMode::setPage( int index, int value )
{
    model->item( index, 1 )->setText(
	QString( Map::toString_MeteringMode( value ) ) );
    switch( value ) {
	case Map::METERING_Spot:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/MeteringMode/spot.png") );
	    break;
	case Map::METERING_Evaluative:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/MeteringMode/evaluative.png") );
	    break;
	case Map::METERING_Partial:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/MeteringMode/partial.png") );
	    break;
	case Map::METERING_Center:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/MeteringMode/center-weighted.png") );
	    break;
    }
}

