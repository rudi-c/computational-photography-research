/*
 *  Auto focus mode setting and display.
 */

#include <QtGui>
#include "FocusMode.h"
#include "Camera.h"
#include "Map.h"


FocusMode::FocusMode( QWidget *parent )
    : QWidget(parent)
{
    n = 0;
    v = 0;

    setMinimumWidth( 80 );
    setMinimumHeight( 19 );

    comboBox = new QComboBox( this );
    comboBox->setMaxCount( Map::MAX_FocusMode );
    comboBox->setMinimumWidth( 80 );
    comboBox->setMinimumHeight( 19 );
    QObject::connect( comboBox, SIGNAL(activated(int)),
		this, SLOT(toValue(int)));
}

FocusMode::~FocusMode()
{
}

void
FocusMode::updateValue( Camera *camera )
{
    int index, value;

    /*
     *  Get the current auto focus mode.
     */
    value = camera->getAFMode();
    index = 0;

    if( value == Map::AFMODE_Manual ) {
	/*
	 *  Lens is in manual mode.
	 */
	comboBox->insertItem( 0, QString( Map::toString_AFMode( value ) ) );
	setEnabled( false );
    }
    else {
	/*
	 *  Current user setting of auto focus mode.
	 */
	camera->getAFMode_List( &n, &v );
	comboBox->clear();
	for( int i = 0; i < n; i++ ) {
	    comboBox->insertItem(
		    i, QString( Map::toString_AFMode( v[i] ) ) );
	    if( value == v[i] ) index = i;
	}
	setEnabled( n > 0 );
    }

    comboBox->setCurrentIndex( index );
    update();
}

void
FocusMode::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for focus mode.
     */
    camera->getAFMode_List( &n, &v );

    comboBox->clear();

    if( n == 0 ) {
	/*
	 *  No valid focusing mode (happens when in live
	 *  view or video mode).
	 */
	setEnabled( false );
    }
    else {
	/*
	 *  Set current list of choices for focus mode.
	 */
	for( int i = 0; i < n; i++ ) {
	    comboBox->insertItem(
		i, QString( Map::toString_AFMode( v[i] ) ) );
	}
	setEnabled( true );
    }

    updateValue( camera );
}

/*
 *  Convert from a combo box value to a value that
 *  represents a valid Canon Metering Mode internal value.
 */
void
FocusMode::toValue( int index )
{
    if( n > 0 ) {
	emit propertyChanged( kEdsPropID_AFMode, v[index] );
    }
}

