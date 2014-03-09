/*
 *  Camera listener.
 *
 *  The EDSDK uses callback routines to communicate changes
 *  from the camera. The routines are static so that their
 *  address can be taken and passed in to the EDSDK.
 *
 *  The SDK starts a callback in a new thread. So must
 *  ensure that there is no contention for shared variables.
 *  Listener must emit signals rather than call routines directly.
 */
#ifndef _LISTENER_H_
#define _LISTENER_H_

#include <QObject>
#include "EDSDKTypes.h"

class Event;

class Listener : public QObject
{
    Q_OBJECT

public:
    Listener::Listener();
    Listener::~Listener();

    /*
     *  Routines for activating and deactiving the listener.
     *  The listener is activated after a session is fully open with 
     *  the camera. This avoids the problem of callbacks before the
     *  session has completely been established. Handling these
     *  callbacks would require queueing them and then handling
     *  them after the session has been fully opened (as not all of
     *  the properties are queriable when the session is in this state).
     *  To avoid this complication, the listener is inactive until the
     *  session is fully open. As a result, the camera model is
     *  initialized with explicit calls for information to the camera.
     */
    void setActive();
    void setInActive();
    bool isActive();

    /*
     *  Callback routines for handling property, object, state,
     *  and progress events.
     */
    static
    EdsError EDSCALLBACK propertyEventHandler(
	EdsPropertyEvent inEvent,
	EdsPropertyID	 inPropertyID,
	EdsUInt32	 inParam,
	EdsVoid		 *inContext );

    static
    EdsError EDSCALLBACK objectEventHandler(
	EdsObjectEvent	inEvent,
	EdsBaseRef	inRef,
	EdsVoid		*inContext );

    static
    EdsError EDSCALLBACK stateEventHandler(
	EdsStateEvent	inEvent,
	EdsUInt32	inEventData,
	EdsVoid		*inContext );

    static
    EdsError EDSCALLBACK cameraAddedHandler(
	EdsVoid* inContext );

    static
    EdsError EDSCALLBACK progressCallback(
	EdsUInt32	inPercent,
	EdsVoid		*inContext,
	EdsBool		*outCancel );

signals:

    /*
     *  Signals are connected to slots in controller.
     */
    void propertyEvent(
	EdsPropertyEvent inEvent,
	EdsPropertyID	 inPropertyID,
	EdsUInt32	 inParam );

    void objectEvent(
	EdsObjectEvent	 inEvent,
	EdsBaseRef	 inRef );

    void stateEvent(
	EdsStateEvent	 inEvent,
	EdsUInt32	 inEventData );

    void cameraAddedEvent();

    void eventReport( Event *event );

protected:
    volatile bool _active;

};

#endif // _LISTENER_H_
