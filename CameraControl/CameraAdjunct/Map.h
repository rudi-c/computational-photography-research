/*
 *  Mapping between internal Canon EDSDK values and external names.
 */
#ifndef _MAP_H_
#define _MAP_H_

class Map
{
    public:
	enum {
	    METERING_Spot = 1,
	    METERING_Evaluative = 3,
	    METERING_Partial = 4,
	    METERING_Center = 5
	};

	enum {
	    AFMODE_OneShot = 0,
	    AFMODE_AIServo = 1,
	    AFMODE_AIFocus = 2,
	    AFMODE_Manual = 3
	};

	enum {
	    FP_LensSetToManual = 0,
	    FP_ManualSelection = 2,
	    FP_AutomaticSelection = 4
	};

	enum {
	    DRIVE_Single = 0x00000000,
	    DRIVE_Continuous = 0x00000001,
	    DRIVE_Video = 0x00000002,
	    DRIVE_NotUsed = 0x00000003,
	    DRIVE_ContinuousHigh = 0x00000004,
	    DRIVE_ContinuousLow = 0x00000005,
	    DRIVE_Silent = 0x00000006,
	    DRIVE_Self10C = 0x00000007,
	    DRIVE_Self10 = 0x00000010,
	    DRIVE_Self2 = 0x00000011,
	    DRIVE_SilentSingle = 0x00000013,
	    DRIVE_SilentContinuous = 0x00000014
	};

	/*
	 *  Video recording.
	 */
	enum {
	    RecordModeStop = 0,
	    RecordModeStart = 4
	};

	static const int MAX_ShootingMode;
	static const int MAX_Av;
	static const int MAX_Tv;
	static const int MAX_ISO;
	static const int MAX_MeteringMode;
	static const int MAX_ImageQuality;
	static const int MAX_FocusMode;
	static const int MAX_WhiteBalance;
	static const int MAX_DriveMode;
	static const int MAX_PictureStyle;

	static char *toString_ShootingMode( int value );
	static char *toString_Av( int value );
	static char *toString_Tv( int value );
	static char *toString_ISO( int value );
	static char *toString_MeteringMode( int value );
	static char *toString_EvfAFMode( int value );
	static char *toString_ImageQuality( int value );
	static char *toString_AFMode( int value );
	static char *toString_WhiteBalance( int value );
	static char *toString_DriveMode( int value );
	static char *toString_PictureStyle( int value );
	//static char *toString_error( int value );
	//static char *toString_propertyID( int value );

	static double Map::toCompensation( int value );
	static double Map::toShutterSpeed( int value );
	static int Map::toISO( int value );
	static int Map::fromISO( int value );
	static int Map::fromCompensation( double value );

    protected:
	typedef struct {
	    int  value;
	    char *description;
	} PairType;

	static const PairType _ShootingMode_Map[];
	static const PairType _Av_Map[];
	static const PairType _Tv_Map[];
	static const PairType _ISO_Map[];
	static const PairType _MeteringMode_Map[];
	static const PairType _EvfAFMode_Map[];
	static const PairType _ImageQuality_Map[];
	static const PairType _AFMode_Map[];
	static const PairType _WhiteBalance_Map[];
	static const PairType _DriveMode_Map[];
	static const PairType _PictureStyle_Map[];
	//static const PairType _error_Map[];
	//static const PairType _propertyID_Map[];

	static const PairType _Unknown;

	static char *_toString( int value, const PairType _map[] );
};

#endif // _MAP_H_
