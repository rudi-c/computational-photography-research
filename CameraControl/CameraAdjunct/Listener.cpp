/*
 *  Camera listener.
 */

#include <QTime>
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include "Listener.h"
#include "Event.h"


Listener::Listener() { _active = false; }
Listener::~Listener() {}

void Listener::setActive()   { _active = true; }
void Listener::setInActive() { _active = false; }
bool Listener::isActive()    { return( _active ); }

/*
 *  Camera added handler.
 */
EdsError EDSCALLBACK
Listener::cameraAddedHandler( EdsVoid* inContext )
{
    Listener *l = (Listener *)inContext;
    emit l->cameraAddedEvent();

    return( EDS_ERR_OK );
}

/*
 *  Property event callback function.
 */
EdsError EDSCALLBACK
Listener::propertyEventHandler(
	EdsPropertyEvent inEvent,
	EdsPropertyID    inPropertyID,
	EdsUInt32        inParam,
	EdsVoid          *inContext )
{
    Listener *l = (Listener *)inContext;
    if( l->isActive() ) {
	emit l->propertyEvent( inEvent, inPropertyID, inParam );
    }

    return( EDS_ERR_OK );
}

/*
 *  Object event callback function.
 */
EdsError EDSCALLBACK
Listener::objectEventHandler(
	EdsObjectEvent	inEvent,
	EdsBaseRef	inRef,
	EdsVoid		*inContext )
{
    Listener *l = (Listener *)inContext;
    emit l->objectEvent( inEvent, inRef );

    return( EDS_ERR_OK );
}

/*
 *  State event callback function.
 */
EdsError EDSCALLBACK
Listener::stateEventHandler(
	EdsStateEvent	inEvent,
	EdsUInt32	inEventData,
	EdsVoid		*inContext )
{
    Listener *l = (Listener *)inContext;
    emit l->stateEvent( inEvent, inEventData );

    return( EDS_ERR_OK );
}

/*
 *  Progress callback function.
 *      inPercent	- progress in range 0 - 100%.
 *      inContext	- listener
 *      outCancel	- to cancel process, set to true
 */
EdsError EDSCALLBACK
Listener::progressCallback(
	EdsUInt32 inPercent,
	EdsVoid	  *inContext,
	EdsBool	  *outCancel )
{
    Listener *l = (Listener *)inContext;
    emit l->eventReport( new Event( Event::DownloadProgress, (int)inPercent ) );

    return( EDS_ERR_OK );
}

