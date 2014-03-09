/*
 *  Parameters for controlling a sequence of shots.
 */
#ifndef _SHOTSEQ_H_
#define _SHOTSEQ_H_


class ShotSeq
{
    public:
	ShotSeq() {}
	~ShotSeq() {}

	/*
	 *  Type of shot sequence.
	 */
	enum {
	    ExposureBracketingManual = 1,
	    ExposureBracketingAv,
	    FocusBracketing,
	    Interval,
	    Stitch
	};
	int type;

	/*
	 *  Number of frames to take in sequence.
	 */
	int frames;

	/*
	 *  Current shot in sequence.
	 */
	int shot;

	/*
	 *  Exposure and focus bracketing.
	 *
	 *  Exposure bracketing in Manual shooting mode: List
	 *  of shutter speeds (aperture is held constant).
	 *
	 *  Exposure bracketing in Av shooting mode: List of
	 *  exposure compensations (aperture is held constant).
	 *
	 *  Focus bracketing: List of shots, each represented
	 *  as a step from current focal plane.
	 */
	static const int MAX_BRACKET = 250;
	int bracket[MAX_BRACKET];

	/*
	 *  Interval: Delay in milliseconds before first shot.
	 */
	int delay;

	/*
	 *  Interval: Interval of time in milliseconds between shots.
	 *  If interval is zero, take the sequence of shots as quickly
	 *  as possible.
	 */
	int interval;

	/*
	 *  Interval: Shutter speed in milliseconds. If bulbMode is
	 *  false, use current shutter speed setting of camera.
	 */
	bool bulbMode;
	int shutter;
};

#endif // _SHOTSEQ_H_
