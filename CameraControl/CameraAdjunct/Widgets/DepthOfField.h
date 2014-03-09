/*
 *  Widget for displaying depth of field calculations.
 */
#ifndef _DEPTHOFFIELD_H_
#define _DEPTHOFFIELD_H_

#include <QWidget>

class QImage;


class DepthOfField : public QWidget
{
    public:
	DepthOfField( QString focusText, QWidget *parent = 0 );
	~DepthOfField();
	void setData(
	    double focusDistance,
	    double nearDistance,
	    double farDistance,
	    int units );

	enum {
	    UNITS_Feet = 0,
	    UNITS_Metres = 1
	};

    protected:
	void drawImage();
	QString toString( double x );
	double toImageCoordinate( double x );
	void paintEvent( QPaintEvent *event );
	QImage image;
	QImage *focusImage;
	QImage *unfocusImage;
	QString focusText;
	double focusDistance;
	double nearDistance;
	double farDistance;
	int units;
	static const int windowWidth = 480;
	static const int windowHeight = 140;
	static const int hOffset = 134;  // where distance 1 is in the image
	static const int step = 75;    // step size between 0.1, 1, 10, ...
	static const int vOffset = 85;  // where ground is in the image
};

#endif // _DEPTHOFFIELD_H_
