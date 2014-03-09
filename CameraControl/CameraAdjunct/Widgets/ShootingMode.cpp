/*
 *  Shooting mode display widget.
 *
 *  Does not yet handle cameras where the shooting mode can be
 *  changed through the SDK, as the widget constructed here does
 *  not provide a way to select a value.
*/

#include <QtGui>
#include "ShootingMode.h"
#include "Camera.h"
#include "Map.h"


ShootingMode::ShootingMode( QWidget *parent )
    : QWidget(parent)
{
    n = 0;
    v = 0;

    setFixedSize( width, height );

    initialize();
}

ShootingMode::~ShootingMode()
{
}

void
ShootingMode::initialize()
{
    QFont font;
    font.setFamily( "Calibri" );
    //font.setPointSize( 28 );
    font.setPointSize( 24 );

    stackedWidget = new QStackedWidget( this );
    stackedWidget->setFixedSize( width, height );
    stackedWidget->setFont( font );
    page = new QLabel *[Map::MAX_ShootingMode];
    for( int i = 0; i < Map::MAX_ShootingMode; i++ ) {
	page[i] = new QLabel( stackedWidget );
	page[i]->setText( QString() );
	page[i]->setFixedSize( width, height );
	page[i]->setScaledContents( true );
	page[i]->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
	stackedWidget->insertWidget( i, page[i] );
    }
    stackedWidget->setCurrentIndex( 0 );
}

void
ShootingMode::updateValue( Camera *camera )
{
    int index, value;

    /*
     *  Get the current shooting mode.
     */
    value = camera->getShootingMode();

    /*
     *  For cameras where the shooting mode can only be changed
     *  on the camera itself, n = 0 always.
     */
    if( n == 0 ) {
	setPage( 0, value );
	index = 0;
    }
    else {
	for( index = 0; index < n; index++ ) {
	    if( value == v[index] ) break;
	}
	if( index >= n ) {
	    // does this ever happen?
	    index = 0;
	}
    }
    stackedWidget->setCurrentIndex( index );
    update();
}

void
ShootingMode::updateList( Camera *camera )
{
    /*
     *  Get the updated list of choices for shooting mode.
     *  For cameras where the shooting mode can only be changed
     *  on the camera itself, n = 0 always and this code has
     *  no effect.
     */
    camera->getShootingMode_List( &n, &v );

    for( int i = 0; i < n; i++ ) {
	setPage( i, v[i] );
    }
    stackedWidget->setCurrentIndex( 0 );
    update();
}

void
ShootingMode::setPage( int index, int value )
{
    switch( value ) {
	case kEdsAEMode_Custom:
	case kEdsAEMode_DEP:
	case kEdsAEMode_Lock:
	    page[index]->setText( Map::toString_ShootingMode( value ) );
	    break;
	case kEdsAEMode_Bulb:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/B.png"));
	    break;
	case kEdsAEMode_A_DEP:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/A-DEP.png"));
	    break;
	case kEdsAEMode_Manual:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/M.png"));
	    break;
	case kEdsAEMode_Av:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/Av.png"));
	    break;
	case kEdsAEMode_Tv:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/Tv.png"));
	    break;
	case kEdsAEMode_Program:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/P.png"));
	    break;
	case kEdsAEMode_Green:
	case kEdsAEMode_SceneIntelligentAuto:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/fullauto.png"));
	    break;
	case kEdsAEMode_NightPortrait:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/night.png"));
	    break;
	case kEdsAEMode_Sports:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/sports.png"));
	    break;
	case kEdsAEMode_Portrait:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/portrait.png"));
	    break;
	case kEdsAEMode_Landscape:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/landscape.png"));
	    break;
	case kEdsAEMode_Closeup:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/closeup.png"));
	    break;
	case kEdsAEMode_FlashOff:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/flashoff.png"));
	    break;
	case kEdsAEMode_CreativeAuto:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/creativeauto.png"));
	    break;
	case kEdsAEMode_Movie:
	    page[index]->setPixmap(
		QPixmap(":/Resources/ShootingMode/movie.png"));
	    break;
	case kEdsAEMode_PhotoInMovie:
	case kEdsAEMode_Unknown:
	    break;
    }
}

