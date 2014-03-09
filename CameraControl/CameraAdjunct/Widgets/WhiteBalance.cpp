/*
 *  White balance setting and display widget.
 */

#include <QtGui>
#include "WhiteBalance.h"
#include "Camera.h"
#include "Map.h"
#include "size.h"


WhiteBalance::WhiteBalance( QWidget *parent )
    : QWidget(parent)
{
    n = 0;
    v = 0;

    setFixedSize( 80, ROW_HEIGHT );

    initialize();
}

WhiteBalance::~WhiteBalance()
{
}

void
WhiteBalance::initialize()
{
    model = new QStandardItemModel( Map::MAX_WhiteBalance, 2, this ); // rows, cols
    for( int row = 0; row < Map::MAX_WhiteBalance; row++ ) {
	model->setItem( row, 0, new QStandardItem() );
	model->setItem( row, 1, new QStandardItem() );
    }

    view = new QTableView( this );
    view->setModel( model );
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();
    view->setShowGrid( false );
    view->setIconSize( QSize( ICON_WIDTH, ICON_HEIGHT ) );
    for( int row = 0; row < Map::MAX_WhiteBalance; row++ ) {
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
    //comboBox->setFixedSize( 59, ROW_HEIGHT );
comboBox->setFixedSize( 80, ROW_HEIGHT );
    QObject::connect(
	comboBox, SIGNAL(activated(int)),
	this, SLOT(toValue(int)) );
}

void
WhiteBalance::updateValue( Camera *camera )
{
    int index, value;

    /*
     *  Get the current white balance.
     */
    value = camera->getWhiteBalance();

    if( n == 0 ) {
	/*
	 *  Auto white balance mode.
	 */
	setPage( 0, value );
	index = 0;
    }
    else {
	/*
	 *  Current user setting of white balance.
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
WhiteBalance::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for white balance.
     */
    camera->getWhiteBalance_List( &n, &v );

    if( n == 0 ) {
	/*
	 *  Auto white balance mode.
	 */
	setEnabled( false );
    }
    else {
	/*
	 *  Current list of choices for white balance.
	 */
	for( int i = 0; i < n; i++ ) {
	    setPage( i, v[i] );
	}
	for( int i = n; i < Map::MAX_WhiteBalance; i++ ) {
	    //model->item( i, 0 )->setIcon( QIcon() );
	    //model->item( i, 1 )->setText( QString() );
	    view->hideRow( i );
	}
	setEnabled( true );
    }

    updateValue( camera );
}


/*
 *  Convert from a slider index value to a value that
 *  represents a valid Canon White Balance internal value.
 */
void
WhiteBalance::toValue( int index )
{
    comboBox->setCurrentIndex( index ); // needed because of a bug in Qt
    if( n > 0 ) {
	emit propertyChanged( kEdsPropID_WhiteBalance, v[index] );
    }
}

void
WhiteBalance::setPage( int index, int value )
{
    model->item( index, 1 )->setText(
	QString( Map::toString_WhiteBalance( value ) ) );
    switch( value ) {
	case kEdsWhiteBalance_Auto:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/WhiteBalance/awb.png") );
	    break;
	case kEdsWhiteBalance_Daylight:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/WhiteBalance/daylight.png") );
	    break;
	case kEdsWhiteBalance_Shade:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/WhiteBalance/shade.png") );
	    break;
	case kEdsWhiteBalance_Cloudy:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/WhiteBalance/cloudy.png") );
	    break;
	case kEdsWhiteBalance_Tangsten:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/WhiteBalance/tungsten.png") );
	    break;
	case kEdsWhiteBalance_Fluorescent:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/WhiteBalance/fluorescent.png") );
	    break;
	case kEdsWhiteBalance_Strobe:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/WhiteBalance/flash.png") );
	    break;
	case kEdsWhiteBalance_WhitePaper:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/WhiteBalance/custom.png") );
	    break;
	case kEdsWhiteBalance_PCSet1:
	    break;
	case kEdsWhiteBalance_ColorTemp:
	    break;
    }
}

