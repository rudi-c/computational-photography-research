/*
 *  Events representing:
 *	status changes,
 *	warnings and failures, and
 *	progress on long running tasks.
 */
#ifndef _EVENT_H_
#define _EVENT_H_

class Event
{
    public:
	Event( int type, int data = 0, QString description = QString() )
	{
	    this->type = type;
	    this->data = data;
	    this->time = QTime::currentTime();
	    this->description = description;
	}
	Event() {}
	~Event() {}

	/*
	 *  Event types: warnings, failures, internal messages
	 *  while under development, and progress/status changes.
	 */
	enum {
	    /*
	     *  Warnings.
	     */
	    CameraBusy = 100,
	    ConnectionLost,
	    LensInManualFocus,
	    NotEvfMode,
	    NotVideoMode,
	    NotZoomFaceMode,
	    ISOAuto,
	    /*
	     *  Failures.
	     */
	    SetPropertyFailure = 200,
	    GetPropertyFailure,
	    DownloadFailure,
	    RegisterFailure,
	    LockFailure,
	    ShutterFailure,
	    LiveviewFailure,
	    CaptureFailure,
	    /*
	     *  Internal.
	     */
	    SetPropertyUnhandled = 300,
	    GetPropertyUnhandled,
	    GetPropertyListUnhandled,
	    AFResultUnhandled,
	    /*
	     *  Progress/status.
	     */
	    DownloadInitiated = 400,
	    DownloadProgress,
	    DownloadComplete,
	    RegisterComplete,
	    BulbExposureTime,
	    SequenceInitiated,
	    SequenceProgress,
	    SequenceCancelled,
	    SequenceCancelledNoConnection,
	    SequenceCancelledMirrorLockup,
	    SequenceCancelledNotEvfMode,
	    SequenceComplete
	};
	int type;

	int data;
	QTime time;		// time of event
	QString description;	// description of event
};

#endif // _EVENT_H_
