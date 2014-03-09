/*
 *  Histogram display widget.
 */
#ifndef _HISTOGRAM_H_
#define _HISTOGRAM_H_

#include <QWidget>

class EvfImageData;


class Histogram : public QWidget
{
    public:
	Histogram( QWidget *parent = 0 );
	~Histogram();
	void setData( EvfImageData *data );

    protected:
	void paintEvent( QPaintEvent *event );
	bool validData;
	bool normal;
	bool luminosity;
	uint histogram_Y[256];
	uint histogram_R[256];
	uint histogram_G[256];
	uint histogram_B[256];
};

#endif // _HISTOGRAM_H_
