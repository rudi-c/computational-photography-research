/*
 *  Display an m x n table of images for stitching into a panorama.
 */
#ifndef _IMAGETABLE_H_
#define _IMAGETABLE_H_

#include <QWidget>

class QButtonGroup;
class Overlay;


class ImageTable : public QWidget
{
    Q_OBJECT

    public:
	ImageTable( QWidget *parent = 0 );
	~ImageTable();
	void setImage( QImage image, int rotationFlag, int shot );
	Overlay *getOverlay( int shot );

    public slots:
	void setRows( int value );
	void setColumns( int value );
	void setOrder( int value );
	void clearTable();

    protected:
	/*
	 *  Size of view box for each image.
	 */
	//static const int viewSize = 160;	// Thumbnail size
	static const int viewSize = 128;
	static const int maxItems = 15;

	/*
	 *  Images and their positions in the table.
	 */
	typedef struct {
	    QImage image;
	    QImage thumbnail;
	    int x;
	    int y;
	} ItemType;
	ItemType item[maxItems];
	int nItems;

	int rows;
	int cols;
	int order;

	Overlay *overlay;

	void paintEvent( QPaintEvent *event );
};

#endif // _IMAGETABLE_H_
