/*
 *  Widget for moving around in image by stepping zoom rectangle
 *  left, right, up, and down. Also centers zoom rectangle.
 */
#ifndef _MOVE_H_
#define _MOVE_H_

#include <QWidget>
#include <QTimer>


class Move : public QWidget
{
    Q_OBJECT

    public:
	Move( QWidget *parent = 0 );
	~Move();

    signals:
	void moveDirection( int quadrant );

    protected slots:
	void processMove();

    protected:
	void mousePressEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void mouseReleaseEvent( QMouseEvent *event );
	void keyPressEvent( QKeyEvent *event );
	void keyReleaseEvent( QKeyEvent *event );
	void leaveEvent( QEvent *event );
	void paintEvent( QPaintEvent *event );

	QTimer timer;
	int quadrant;
	QRect viewRect;
	QRect worldRect;
};

#endif // _MOVE_H_
