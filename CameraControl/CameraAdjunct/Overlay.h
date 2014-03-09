/*
 *  Parameters for overlays in live view.
 */
#ifndef _OVERLAY_H_
#define _OVERLAY_H_


class Overlay
{
    public:
	Overlay() {}
	~Overlay() {}

	/*
	 *  Percentage of overlap when positioning images.
	 */
	int overlap;

	/*
	 *  Percentage of opacity when painting images.
	 */
	int opacity;

	/*
	 *  Number of overlays.
	 */
	int n;

	/*
	 *  Where to place overlay.
	 */
	enum {
	    Center = 0,
	    Top,
	    Bottom,
	    Left,
	    Right
	};
	int type[3];

	/*
	 *  Images to overlay.
	 */
	QImage image[3];
};

#endif // _OVERLAY_H_
