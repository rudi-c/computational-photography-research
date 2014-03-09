/*
 *  Navigation widget for setting zoom levels of live image and
 *  setting and displaying zoom and focus rectangles.
 */
#ifndef _NAVIGATION_H_
#define _NAVIGATION_H_

#include <QWidget>

class QButtonGroup;
class Camera;
class Options;
class EvfImageData;
class ZoomRect;


class Navigation : public QWidget
{
    Q_OBJECT

    public:
	Navigation( Camera *camera, QWidget *parent = 0 );
	~Navigation();
	void setData( EvfImageData *data, QRectF visibleRect );

    public slots:
	void moveDirection( int key );

    signals:
	void zoomRectChanged( int x, int y );

    protected:
	void paintEvent( QPaintEvent *event );
	void mousePressEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void mouseReleaseEvent( QMouseEvent *event );
	void keyPressEvent( QKeyEvent *event );

	Camera *camera;
	bool validData;
	uint zoom;
	int rotationFlag;
	QRectF visibleRect;
	QImage image;
	int worldWidth;
	int worldHeight;
	int viewWidth;
	int viewHeight;
	int viewX;
	int viewY;
	int viewSize;
	ZoomRect *zoomRect;

	/*
	 *  Track whether a mouse move or release event was initiated
	 *  in the navigation window. Only act on these events if true.
	 */
	bool moving;
	QPoint lastPosition;
};

#endif // _NAVIGATION_H_
