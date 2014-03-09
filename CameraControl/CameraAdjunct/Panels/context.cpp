/*
 *  Create and display context menu.
 */

#include <QtGui>
#include "View.h"
#include "LiveImageData.h"


const char *View::menuText[] = {
    /*
     *  Clipping (6).
     */
    "None",
    "Shadow",
    "Highlight",
    "Both",
    "Shadow (alt)",
    "Highlight (alt)",
    /*
     *  Composition overlay (16).
     */
    "None",
    "Grid",
    "Thirds",
    "Golden ratio",
    "Diagonals",
    "Triangles 1",
    "Triangles 2",
    "Spiral 1",
    "Spiral 2",
    "Spiral 3",
    "Spiral 4",
    "Spiral 5",
    "Spiral 6",
    "Spiral 7",
    "Spiral 8",
    "Passport",
    /*
     *  Rotation (4).
     */
    "None",
    "90\xB0 CW",
    "90\xB0 CCW",
    "180\xB0",
    NULL
};

void
View::createContextMenu()
{
    menu = new QMenu( this );
    QMenu *clipping = new QMenu( "Clipping", this );
    QMenu *focusMap = new QMenu( "Focus map", this );
    QMenu *overlay = new QMenu( "Overlay", this );
    QMenu *rotation = new QMenu( "Rotation", this );
    menu->addMenu( clipping );
    menu->addMenu( focusMap );
    menu->addMenu( overlay );
    menu->addMenu( rotation );

    QActionGroup *clippingGroup = new QActionGroup( this );
    for( int i = EvfImageData::ShowClippingNone;
	     i <= EvfImageData::ShowClippingLast; i++ ) {
	action[i] = new QAction( menuText[i], this );
	action[i]->setData( i );
	action[i]->setCheckable( true );
	clipping->addAction( action[i] );
	clippingGroup->addAction( action[i] );
    }
    action[EvfImageData::ShowClippingNone]->setChecked( true );

    QActionGroup *overlayGroup = new QActionGroup( this );
    for( int i = EvfImageData::OverlayNone;
	     i <= EvfImageData::OverlayLast; i++ ) {
	action[i] = new QAction( menuText[i], this );
	action[i]->setData( i );
	action[i]->setCheckable( true );
	overlay->addAction( action[i] );
	overlayGroup->addAction( action[i] );
    }
    action[EvfImageData::OverlayNone]->setChecked( true );

    QActionGroup *rotationGroup = new QActionGroup( this );
    for( int i = EvfImageData::RotationNone;
	     i <= EvfImageData::RotationLast; i++ ) {
	action[i] = new QAction( menuText[i], this );
	action[i]->setData( i );
	action[i]->setCheckable( true );
	rotation->addAction( action[i] );
	rotationGroup->addAction( action[i] );
    }
    action[EvfImageData::RotationNone]->setChecked( true );
}

void
View::showContextMenu()
{
    action[clippingComboBox->currentIndex()
	 + EvfImageData::ShowClippingNone]->setChecked( true );
    action[overlayComboBox->currentIndex()
	 + EvfImageData::OverlayNone]->setChecked( true );
    action[rotationComboBox->currentIndex()
	 + EvfImageData::RotationNone]->setChecked( true );

    QAction *selectedItem = menu->exec( QCursor::pos() );

    if( selectedItem == 0 ) {
	return;
    }

    int index = selectedItem->data().toInt();
    if( index >= EvfImageData::ShowClippingNone &&
	index <= EvfImageData::ShowClippingLast ) {
	index -= EvfImageData::ShowClippingNone;
	clippingComboBox->setCurrentIndex( index );
    }
    else
    if( index >= EvfImageData::OverlayNone &&
	index <= EvfImageData::OverlayLast ) {
	index -= EvfImageData::OverlayNone;
	overlayComboBox->setCurrentIndex( index );
    }
    else
    if( index >= EvfImageData::RotationNone &&
	index <= EvfImageData::RotationLast ) {
	index -= EvfImageData::RotationNone;
	rotationComboBox->setCurrentIndex( index );
	setRotation( index );
    }
}

