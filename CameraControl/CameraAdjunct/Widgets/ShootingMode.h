/*
 *  Shooting mode display widget.
 */
#ifndef _SHOOTINGMODE_H_
#define _SHOOTINGMODE_H_

#include <QWidget>
#include "Map.h"

class QLabel;
class QStackedWidget;
class Camera;


class ShootingMode : public QWidget
{
    Q_OBJECT

    public:
	ShootingMode( QWidget *parent = 0 );
	~ShootingMode();
	void updateValue( Camera *camera );
	void updateList( Camera *camera );

    protected:
	void initialize();
	void setPage( int index, int value );
	QLabel **page;
	QStackedWidget *stackedWidget;
	int n;  // list of possibilities for shooting mode
	int *v;
	static const int width = 48;
	static const int height = 64;
};

#endif // _SHOOTINGMODE_H_
