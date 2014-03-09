/*
 *  Prepare image for display in live view.
 */
#ifndef _PREPAREIMAGE_H
#define _PREPAREIMAGE_H

#include <QObject>
#include <QThread>
#include <QImage>
#include "LiveImageData.h"

class TransactionThread : public QThread
{
    Q_OBJECT

    public:
	TransactionThread();
	~TransactionThread();
	void setData( EvfImageData *data );

    signals:
	void imageReady( EvfImageData *data );

    protected:
	void run();
	void constructFocusMap( EvfImageData *data );
	EvfImageData *data;
};

class PrepareImage : public QObject
{
    Q_OBJECT

    public:
	PrepareImage();
	~PrepareImage();

    public slots:
	void updateLiveView( EvfImageData *data );

    signals:
	void liveViewChanged( EvfImageData *data );

    protected slots:
	void imageReady( EvfImageData *data );

    protected:
	bool lastFrame;
	int nThreads;
	static const int MaxThreadCount = 8;
	TransactionThread *thread[MaxThreadCount];
};

#endif // _PREPAREIMAGE_H
