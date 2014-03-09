/*
 *  Show live view.
 */
#ifndef _LIVEIMAGE_H_
#define _LIVEIMAGE_H_

#include <QWidget>
#include <QTime>
#include "EDSDKTypes.h"
#include "LiveImageData.h"

class QImage;
class Camera;
class Options;
class Overlay;
class ZoomRect;


class LiveImage: public QWidget
{
    Q_OBJECT

    public:
	LiveImage( Camera *camera, QWidget *parent = 0 );
	~LiveImage();
	void setData( EvfImageData *data, /*QImage image,*/ double scaling );
	void setImageOverlay( Overlay *overlay );

    signals:
	void focusPropertyChanged( int selectionMode, int fp );
	void zoomRectChanged( int x, int y );

    protected:
	void paintEvent( QPaintEvent *event );
	void mousePressEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void mouseReleaseEvent( QMouseEvent *event );
	void keyPressEvent( QKeyEvent *event );
	void showClipping( int clippingFlag );
	void drawLineWithArrows( QPainter &painter,
		int x1, int y1, int x2, int y2 );
	void drawCompositionOverlay( QPainter &painter, int overlayFlag );
	void drawFocusPoints( QPainter &painter );
	void drawVideoRecord( QPainter &painter );
	void drawImageOverlay( QPainter &painter );

	Camera *camera;
	QImage image;
	bool validData;
	uint zoom;
	int rotationFlag;
	Overlay *overlay;
	EvfImageData::EvfFocusInfo focusInfo;
	QTime videoTime;	// duration of video if recording
	bool recording;		// whether recording a video
	int worldWidth;
	int worldHeight;
	int viewWidth;
	int viewHeight;
	double pixelWidth;
	ZoomRect *zoomRect;

	/*
	 *  The boolean "moving" tracks whether the user is currently
	 *  dragging the zoom rectangle to a new location in the window.
	 *  The camera is only updated with the position of the rectangle
	 *  once the mouse is released and the zoom rectangle is in a
	 *  resting position (the camera could not handle all of the
	 *  intermediate updates).
	 */
	bool moving;
	QPoint lastPosition;
};

#endif // _LIVEIMAGE_H_
