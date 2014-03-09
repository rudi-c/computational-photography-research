/*
 *  Exposure compensation setting and displaying widget.
 */
#ifndef _EXPOSURECOMP_H_
#define _EXPOSURECOMP_H_

#include <QWidget>

class Camera;


class ExposureComp : public QWidget
{
    Q_OBJECT

    public:
	ExposureComp( Camera *camera, int bound, QWidget *parent = 0 );
	~ExposureComp();
	void updateValue( Camera *camera );
	void updateList( Camera *camera );

    signals:
	void propertyChanged( int property, int newValue );

    public slots:
	void updateExposures( int index );
	void updateIncrement( int index );

    protected:
	Camera *camera;
	void keyPressEvent( QKeyEvent *event );
	void mousePressEvent( QMouseEvent *event );
	void wheelEvent( QWheelEvent *event );
	void drawSigns( QPainter &painter, int i, QString s, int w );
	void paintEvent( QPaintEvent *event );

	int findIndex();
	void setCompensation( int value );
	void toValue( int index );
	int sw;			// sliding window -sw, .., +sw
	int center;		// center of sliding window
	int width;		// width of painter window
	int height;		// height of painter window
	int viewWidth;		// width of viewport window
	int viewHeight;		// height of viewport window
	int nBars;		// number of additional exposure bars
	int incr;		// increment for compensation
	double compensation;	// exposure compensation
	int n;			// list of choices for exposure compensation
	int *v;
};

#endif // _EXPOSURECOMP_H_

