/*
 *  Live view image data.
 */
#ifndef _EVFIMAGEDATA_H_
#define _EVFIMAGEDATA_H_

typedef unsigned int uint;
typedef unsigned char uchar;


class EvfImageData
{
    public:
	EvfImageData() {
	    len = 0;
	    memset( &focusInfo, 0, sizeof( EvfFocusInfo ) );
	}
	~EvfImageData() {}

	/*
	 *  Size of largest JPEG.
	 */
	static const uint MaxBufferSize = 2 * 1024 * 1024;

	/*
	 *  Flag for how to show clipping in the live view image.
	 *
	 *  ShowClippingShadow: show black clipping on live view image if
	 *	all three of the channels are clipped.
	 *
	 *  ShowClippingHighlight: show highlight clipping on live view
	 *	image if two or more of the channels are clipped.
	 *
	 *  ShowClippingBoth: ShowClippingShadow and ShowClippingHighlight.
	 *
	 *  ShowClippingShadowAlt: image turns white, clipped areas appear
	 *	black (all 3 channels clipped);
	 *	cyan, magenta, yellow (2 channels clipped);
	 *	red, green, blue (1 channel clipped).
	 *
	 *  ShowClippingHighlightAlt: image turns black, clipped areas appear
	 *	white (all 3 channels clipped);
	 *	cyan, magenta, yellow (2 channels clipped);
	 *	red, green, blue (1 channel clipped).
	 */
	enum ShowClippingFlag {
	    ShowClippingNone = 0,
	    ShowClippingShadow,
	    ShowClippingHighlight,
	    ShowClippingBoth,
	    ShowClippingShadowAlt,
	    ShowClippingHighlightAlt,
	    ShowClippingLast = ShowClippingHighlightAlt
	};
	int clippingFlag;

	/*
	 *  Flag for overlays for aiding composition in live view.
	 *  Enumeration should be consecutive with FocusMapFlag.
	 *
	 *  OverlayNone:         no overlay.
	 *  OverlayGrid:         show finely spaced grid.
	 *  OverlayThirds:       show grid for "Rule of Thirds".
	 *  OverlayGoldenRatio:  show grid based on golden ratio.
	 *  OverlayDiagonals:    show diagonal lines from each corner.
	 *  OverlayTriangles:    show diagonal line and two perpendicular lines.
	 *  OverlayGoldenSpiral: show grid based on golden ratio.
	 *  OverlayPassport:     show head-shot outline for passport photo.
	 */
	enum OverlayFlag {
	    OverlayNone = ShowClippingLast + 1,
	    OverlayGrid,
	    OverlayThirds,
	    OverlayGoldenRatio,
	    OverlayDiagonals,
	    OverlayTriangles1,    // two versions, depending on diagonal
	    OverlayTriangles2,
	    OverlayGoldenSpiral1, // eight versions, depending on starting
	    OverlayGoldenSpiral2, // corner and initial trajectory
	    OverlayGoldenSpiral3,
	    OverlayGoldenSpiral4,
	    OverlayGoldenSpiral5,
	    OverlayGoldenSpiral6,
	    OverlayGoldenSpiral7,
	    OverlayGoldenSpiral8,
	    OverlayPassport,
	    OverlayLast = OverlayPassport
	};
	int overlayFlag;

	/*
	 *  Flag for how to rotate the live view image.
	 *  Enumeration should be consecutive with OverlayFlag.
	 *
	 *  RotationCW90: rotate clockwise 90 degrees.
	 *  RotationCCW90: rotate counterclockwise 90 degrees.
	 *  Rotation180: rotate 180 degrees.
	 */
	enum RotationFlag {
	    RotationNone = OverlayLast + 1,
	    RotationCW90,
	    RotationCCW90,
	    Rotation180,
	    RotationLast = Rotation180
	};
	int rotationFlag;

	/*
	 *  Focus mode in live view.
	 */
	enum {
	    AFMODE_Quick = 0,
	    AFMODE_Live = 1,
	    AFMODE_Face = 2
	};

	/*
	 *  Camera zoom.
	 */
	enum {
	    ZOOM_Fit = 1,
	    ZOOM_5x = 5,
	    ZOOM_10x = 10
	};

	/*
	 *  So far:
	 *  "Live mode"
	 *      16: regular
	 *      17: in focus (signaled after a focusing event)
	 *      18: focus not achieved (signaled after a focusing event)
	 *      32: blank (don't show the focus rectangle)
	 *  "Live face detection mode"
	 *      16: 
	 *      17: 
	 *      18:
	 *      32: 
	 *      33: 
	 *  "Quick mode"
	 *      0/1
	 */
	enum {
	    FOCUS_Regular = 16,
	    FOCUS_Infocus = 17,
	    FOCUS_FocusNotAchieved = 18,
	    FOCUS_Blank = 32
	};

	typedef struct {
	    int  width;
	    int  height;
	} EvfSize;

	typedef struct {
	    int  x;
	    int  y;
	} EvfPoint;

	typedef struct {
	    EvfPoint point;
	    EvfSize  size;
	} EvfRect;

	/*
	 *  Image, buffer for JPEG image data, length of image data.
	 */
	QImage image;
	uchar buffer[MaxBufferSize];
	uint len;

	/*
	 *  Histogram data.
	 */
	uint histogram_Y[256];
	uint histogram_R[256];
	uint histogram_G[256];
	uint histogram_B[256];

	/*
	 *  Histogram status. Hide, normal, or gray come from the
	 *  camera, and are or'd with either luminosity or RGB.
	 */
	enum HistogramFlags {
	    HISTOGRAM_Hide = 0,
	    HISTOGRAM_Normal = 1,
	    HISTOGRAM_Gray = 2,
	    HISTOGRAM_Luminosity = 4,
	    HISTOGRAM_RGB = 8
	};
	uint histogramStatus;

	/*
	 *  The width and height of the coordinate system used in
	 *  live view image. The values are constant and all focus
	 *  point and zoom rectangle positions are specified in this
	 *  coordinate system. The top-left point is the origin (0,0).
	 */
	EvfSize coordinateSystem;

	/*
	 *  The upper left coordinates of an enlarged image.
	 *  The coordinates are (0, 0) if the zoom factor is 1x;
	 *  otherwise the coordinates designate the source point
	 *  of the enlarged image in the original image.
	 */
	EvfPoint imagePosition;

	/*
	 *  Zoom ratio. For example, zoom = 5 means 5x.
	 *  Can be set using setProperty().
	 */
	uint zoom;

	/*
	 *  The focus and zoom border rectangle. The "point" member is the
	 *  upper left coordinates of the rectangle; the "size" member
	 *  gives the width and height of the rectangle. For a given zoom
	 *  level (1x, 5x, 10x), the width and height of the focus rectangle
	 *  are constant.
	 */
	EvfRect zoomRect;

	/*
	 *  A single focus point.
	 */
	typedef struct {
	    /*
	     *  Valid/invalid focus point (0/1). There are as many
	     *  valid focus points as "nFocusPoints" and they appear
	     *  to be recorded consecutively starting at 0.
	     */
	    uint valid;

	    /*
	     *  Selected/unselected focus point.
	     *  So far:
	     *  "Live mode"
	     *  "Quick mode"
	     *	0/1
	     *  "Live face detection mode"
	     *      1: 
	     *      2: 
	     */
	    uint selected;

	    /*
	     *  In focus/out of focus. Set even during manual shooting.
	     */
	    uint justFocus;

	    /*
	     *  Upper-left and lower-right coordinates of the focus point.
	     */
	    EvfRect rect;

	    /*
	     *  Unknown.
	     */
	    uint reserved;
	} EvfFocusPoint;

	/*
	 *  A collection of focus points.
	 */
	typedef struct {
	    /*
	     *  The upper-left coordinates of the image, the width and height.
	     */
	    EvfRect imageRect;

	    /*
	     *  Number of focus points.
	     */
	    uint nFocusPoints;

	    /*
	     *  Description of each focus point.
	     */
	    EvfFocusPoint focusPoint[128];

	    /*
	     *  So far:
	     *  "Live mode"
	     *      0: always
	     *  "Quick mode"
	     *      0: lens is set to manual focus
	     *      2: manual selection of focus point
	     *      4: auto selection of focus points
	     *  "Live face detection mode"
	     *      0: always
	     */
	    uint executeMode;

	} EvfFocusInfo;

	EvfFocusInfo focusInfo;
};

#endif // _EVFIMAGEDATA_H_
