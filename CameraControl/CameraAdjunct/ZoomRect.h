/*
 *  Track and move the zoom rectangle.
 */
#ifndef _ZOOMRECT_H_
#define _ZOOMRECT_H_

#include <QWidget>

class Camera;
class Options;


class ZoomRect: public QWidget
{
    Q_OBJECT

    public:
	ZoomRect( Camera *camera, QWidget *parent = 0 );
	~ZoomRect();
	void centerZoomRect( int x, int y, bool moving );
	void moveZoomRect( int x, int y, bool moving );
	void scrollZoomRect( int deltaX, int deltaY, bool moving );
	void scrollZoomRect( int key, bool moving );
	QPoint transform( QPoint p );
	int transform( int key );
	QRect focusRect( int zoom = 1 );
	int x();
	int y();
	int width();
	int height();
	bool contains( QPoint p );
	void setRect( QRect rect );
	void setRect( int x, int y, int width, int height );
	void setWindow( QRect rect );
	void setWindow( int x, int y, int width, int height );
	void setViewport( int x, int y, int width, int height );

    signals:
	void zoomRectChanged( int x, int y );

    protected:
	QPoint origin;
	int worldWidth;
	int worldHeight;
	int viewWidth;
	int viewHeight;
	Camera *camera;
	QRect zRect;
};

#endif // _ZOOMRECT_H_
