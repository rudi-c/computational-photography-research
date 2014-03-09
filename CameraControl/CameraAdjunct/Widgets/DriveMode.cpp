/*
 *  Drive mode setting and display widget.
 */

#include <QtGui>
#include "DriveMode.h"
#include "Camera.h"
#include "Map.h"
#include "size.h"


DriveMode::DriveMode( QWidget *parent )
    : QWidget(parent)
{
    n = 0;
    v = 0;

    setFixedSize( 80, ROW_HEIGHT );

    initialize();
}

DriveMode::~DriveMode()
{
}

void
DriveMode::initialize()
{
    model = new QStandardItemModel( Map::MAX_DriveMode, 2, this ); // rows, cols
    for( int row = 0; row < Map::MAX_DriveMode; row++ ) {
	model->setItem( row, 0, new QStandardItem() );
	model->setItem( row, 1, new QStandardItem() );
    }

    view = new QTableView( this );
    view->setModel( model );
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();
    view->setShowGrid( false );
    view->setIconSize( QSize( ICON_WIDTH, ICON_HEIGHT ) );
    for( int row = 0; row < Map::MAX_DriveMode; row++ ) {
	view->setRowHeight( row, ROW_HEIGHT );
    }
    view->setColumnWidth( 0, ICON_COL_WIDTH );
    view->setColumnWidth( 1, 2*COL_WIDTH );
    view->setFixedWidth( ICON_COL_WIDTH + 2*COL_WIDTH );

    comboBox = new QComboBox( this );
    comboBox->setModel( model );
    comboBox->setModelColumn( 0 );
    comboBox->setView( view );
    comboBox->setIconSize( QSize( ICON_WIDTH, ICON_HEIGHT ) );
    comboBox->setFixedSize( 80, ROW_HEIGHT );
    QObject::connect(
	comboBox, SIGNAL(activated(int)),
	this, SLOT(toValue(int)) );
}

void
DriveMode::updateValue( Camera *camera )
{
    int index, value;

    /*
     *  Get the current drive mode.
     */
    value = camera->getDriveMode();

    if( n == 0 ) {
	/*
	 *  Auto drive mode.
	 */
	setPage( 0, value );
	index = 0;
    }
    else {
	/*
	 *  Current user setting of drive mode.
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
DriveMode::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for drive mode.
     */
    camera->getDriveMode_List( &n, &v );

    if( n == 0 ) {
	/*
	 *  Auto drive mode.
	 */
	setEnabled( false );
    }
    else {
	/*
	 *  Current list of choices for drive mode.
	 */
	for( int i = 0; i < n; i++ ) {
	    setPage( i, v[i] );
	}
	for( int i = n; i < Map::MAX_DriveMode; i++ ) {
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
 *  represents a valid Canon Drive Mode internal value.
 */
void
DriveMode::toValue( int index )
{
    comboBox->setCurrentIndex( index ); // needed because of a bug in Qt
    if( n > 0 ) {
	emit propertyChanged( kEdsPropID_DriveMode, v[index] );
    }
}

void
DriveMode::setPage( int index, int value )
{
    model->item( index, 1 )->setText(
	QString( Map::toString_DriveMode( value ) ) );
    switch( value ) {
	case Map::DRIVE_Single:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/single.png") );
	    break;
	case Map::DRIVE_Silent:
	case Map::DRIVE_SilentSingle:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/single-silent.png") );
	    break;
	case Map::DRIVE_Continuous:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/continuous.png") );
	    break;
	case Map::DRIVE_ContinuousHigh:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/continuous-high.png") );
	    break;
	case Map::DRIVE_ContinuousLow:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/continuous-low.png") );
	    break;
	case Map::DRIVE_SilentContinuous:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/continuous-silent.png") );
	    break;
	case Map::DRIVE_Video:
	case Map::DRIVE_NotUsed:
	    break;
	case Map::DRIVE_Self2:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/self-timer2.png") );
	    break;
	case Map::DRIVE_Self10C:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/self-timerC.png") );
	    break;
	case Map::DRIVE_Self10:
	    model->item( index, 0 )->setIcon(
		QIcon(":/Resources/DriveMode/self-timerR.png") );
	    break;
    }
}

