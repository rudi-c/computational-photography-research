/*
 *  Convert from internal Canon EDSDK values to external names.
 */

#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "EDSDKErrors.h"
#include "Map.h"


/*
 *  Pairs of value and display name for shooting mode.
 */
const int Map::MAX_ShootingMode = 21;
const Map::PairType Map::_ShootingMode_Map[] = {
    { kEdsAEMode_Program,"P" },
    { kEdsAEMode_Tv,"Tv" },
    { kEdsAEMode_Av,"Av" },
    { kEdsAEMode_Manual,"M" },
    { kEdsAEMode_Bulb,"Bulb" },
    { kEdsAEMode_A_DEP,"A-DEP" },
    { kEdsAEMode_DEP,"DEP" },
    { kEdsAEMode_Custom,"C" },
    { kEdsAEMode_Lock,"Lock" },
    { kEdsAEMode_Green,"Full Auto" },
    { kEdsAEMode_NightPortrait,"Night Portrait" },
    { kEdsAEMode_Sports,"Sports" },
    { kEdsAEMode_Portrait,"Portrait" },
    { kEdsAEMode_Landscape,"Landscape" },
    { kEdsAEMode_Closeup,"Close Up" },
    { kEdsAEMode_FlashOff,"Flash Off" },
    { kEdsAEMode_CreativeAuto,"Creative Auto" },
    { kEdsAEMode_Movie,"Movie" },
    { kEdsAEMode_PhotoInMovie,"Photo in Movie" },
    { kEdsAEMode_SceneIntelligentAuto,"Intelligent Auto" },
    { kEdsAEMode_Unknown,"Unknown" },
    { -1,NULL }
};

/*
 *  Pairs of value and display name for Av.
 */
const int Map::MAX_Av = 55;
const Map::PairType Map::_Av_Map[] = {
    { 0x00,"Auto" },
    { 0x08,"1.0" },
    { 0x0B,"1.1" },
    { 0x0C,"1.2" },
    { 0x0D,"1.2" },
    { 0x10,"1.4" },
    { 0x13,"1.6" },
    { 0x14,"1.8" },
    { 0x15,"1.8" },
    { 0x18,"2.0" },
    { 0x1B,"2.2" },
    { 0x1C,"2.5" },
    { 0x1D,"2.5" },
    { 0x20,"2.8" },
    { 0x23,"3.2" },
    { 0x24,"3.5" },
    { 0x25,"3.5" },
    { 0x28,"4.0" },
    { 0x2B,"4.5" },
    { 0x2C,"4.5" },
    { 0x2D,"5.0" },
    { 0x30,"5.6" },
    { 0x33,"6.3" },
    { 0x34,"6.7" },
    { 0x35,"7.1" },
    { 0x38,"8.0" },
    { 0x3B,"9.0" },
    { 0x3C,"9.5" },
    { 0x3D,"10" },
    { 0x40,"11" },
    { 0x43,"13" },
    { 0x44,"13" },
    { 0x45,"14" },
    { 0x48,"16" },
    { 0x4B,"18" },
    { 0x4C,"19" },
    { 0x4D,"20" },
    { 0x50,"22" },
    { 0x53,"25" },
    { 0x54,"27" },
    { 0x55,"29" },
    { 0x58,"32" },
    { 0x5B,"36" },
    { 0x5C,"38" },
    { 0x5D,"40" },
    { 0x60,"45" },
    { 0x63,"51" },
    { 0x64,"54" },
    { 0x65,"57" },
    { 0x68,"64" },
    { 0x6B,"72" },
    { 0x6C,"76" },
    { 0x6D,"80" },
    { 0x70,"91" },
    { 0xffffffff,"Unknown" },
    { -1,NULL }
};

/*
 *  Pairs of value and display name for Tv.
 */
const int Map::MAX_Tv = 76;
const Map::PairType Map::_Tv_Map[] = {
    { 0x00,"Auto" },
    { 0x0c,"Bulb" },
    { 0x10,"30\"" },
    { 0x13,"25\"" },
    { 0x14,"20\"" },
    { 0x15,"20\"" },
    { 0x18,"15\"" },
    { 0x1B,"13\"" },
    { 0x1C,"10\"" },
    { 0x1D,"10\"" },
    { 0x20,"8\"" },
    { 0x23,"6\"" },
    { 0x24,"6\"" },
    { 0x25,"5\"" },
    { 0x28,"4\"" },
    { 0x2B,"3\"2" },
    { 0x2C,"3\"" },
    { 0x2D,"2\"5" },
    { 0x30,"2\"" },
    { 0x33,"1\"6" },
    { 0x34,"1\"5" },
    { 0x35,"1\"3" },
    { 0x38,"1\"" },
    { 0x3B,"0\"8" },
    { 0x3C,"0\"7" },
    { 0x3D,"0\"6" },
    { 0x40,"0\"5" },
    { 0x43,"0\"4" },
    { 0x44,"0\"3" },
    { 0x45,"0\"3" },
    { 0x48,"1/4" },
    { 0x4B,"1/5" },
    { 0x4C,"1/6" },
    { 0x4D,"1/6" },
    { 0x50,"1/8" },
    { 0x53,"1/10" },
    { 0x54,"1/10" },
    { 0x55,"1/13" },
    { 0x58,"1/15" },
    { 0x5B,"1/20" },
    { 0x5C,"1/20" },
    { 0x5D,"1/25" },
    { 0x60,"1/30" },
    { 0x63,"1/40" },
    { 0x64,"1/45" },
    { 0x65,"1/50" },
    { 0x68,"1/60" },
    { 0x6B,"1/80" },
    { 0x6C,"1/90" },
    { 0x6D,"1/100" },
    { 0x70,"1/125" },
    { 0x73,"1/160" },
    { 0x74,"1/180" },
    { 0x75,"1/200" },
    { 0x78,"1/250" },
    { 0x7B,"1/320" },
    { 0x7C,"1/350" },
    { 0x7D,"1/400" },
    { 0x80,"1/500" },
    { 0x83,"1/640" },
    { 0x84,"1/750" },
    { 0x85,"1/800" },
    { 0x88,"1/1000" },
    { 0x8B,"1/1250" },
    { 0x8C,"1/1500" },
    { 0x8D,"1/1600" },
    { 0x90,"1/2000" },
    { 0x93,"1/2500" },
    { 0x94,"1/3000" },
    { 0x95,"1/3200" },
    { 0x98,"1/4000" },
    { 0x9B,"1/5000" },
    { 0x9C,"1/6000" },
    { 0x9D,"1/6400" },
    { 0xA0,"1/8000" },
    { 0xffffffff,"Unknown" },
    { -1,NULL }
};

/*
 *  Pairs of value and display name for ISO speeds.
 */
const int Map::MAX_ISO = 33;
const Map::PairType Map::_ISO_Map[] = {
    { 0x00,"Auto" },
    { 0x28,"6" },
    { 0x30,"12" },
    { 0x38,"25" },
    { 0x40,"50" },
    { 0x48,"100" },
    { 0x4b,"125" },
    { 0x4d,"160" },
    { 0x50,"200" },
    { 0x53,"250" },
    { 0x55,"320" },
    { 0x58,"400" },
    { 0x5b,"500" },
    { 0x5d,"640" },
    { 0x60,"800" },
    { 0x63,"1000" },
    { 0x65,"1250" },
    { 0x68,"1600" },
    { 0x6B,"2000" },
    { 0x6D,"2500" },
    { 0x70,"3200" },
    { 0x73,"4000" },
    { 0x75,"5000" },
    { 0x78,"6400" },
    { 0x7B,"8000" },
    { 0x7D,"H1" },	//  10000
    { 0x80,"H2" },	//  12800
    { 0x83,"H3" },	//  16000
    { 0x85,"H4" },	//  20000
    { 0x88,"H5" },	//  25600
    { 0x90,"H6" },	//  51200
    { 0x98,"H7" },	// 102400
    { 0xffffffff,"Unknown" },
    { -1,NULL }
};

/*
 *  Pairs of value and display name for metering mode.
 */
const int Map::MAX_MeteringMode = 4;
const Map::PairType Map::_MeteringMode_Map[] = {
    { 0,"Unknown" },
    { METERING_Spot,"Spot" },
    { METERING_Evaluative,"Evaluative" },
    { METERING_Partial,"Partial" },
    { METERING_Center,"Center-wgt" },
    { 0xffffffff,"Unknown" },
    { -1,NULL }
};

/*
 *  Pairs of value and display name for EvfAF mode.
 */
const Map::PairType Map::_EvfAFMode_Map[] = {
    { Evf_AFMode_Quick,"Quick" },
    { Evf_AFMode_Live,"Live" },
    { Evf_AFMode_LiveFace,"Face" },
    { 0xffffffff,"Unknown"},
    { -1,NULL }
};

/*
 *  Pairs of value and display name for Exposure Compensation.
 */
/*
const Map::PairType Map::_ExposureComp_Map[] = {
    { 0x40,"+8"},
    { 0x3d,"+7 2/3"},
    { 0x3c,"+7 1/2"},
    { 0x3b,"+7 1/3"},
    { 0x38,"+7"},
    { 0x35,"+6 2/3"},
    { 0x34,"+6 1/2"},
    { 0x33,"+6 1/3"},
    { 0x30,"+6"},
    { 0x2d,"+5 2/3"},
    { 0x2c,"+5 1/2"},
    { 0x2b,"+5 1/3"},
    { 0x28,"+5"},
    { 0x25,"+4 2/3"},
    { 0x24,"+4 1/2"},
    { 0x23,"+4 1/3"},
    { 0x20,"+4"},
    { 0x1d,"+3 2/3"},
    { 0x1c,"+3 1/2"},
    { 0x1b,"+3 1/3"},
    { 0x18,"+3"},
    { 0x15,"+2 2/3"},
    { 0x14,"+2 1/2"},
    { 0x13,"+2 1/3"},
    { 0x10,"+2"},
    { 0x0d,"+1 2/3"},
    { 0x0c,"+1 1/2"},
    { 0x0b,"+1 1/3"},
    { 0x08,"+1"},
    { 0x05,"+2/3"},
    { 0x04,"+1/2"},
    { 0x03,"+1/3"},
    { 0x00,"0"},
    { 0xfd,"-1/3"},
    { 0xfc,"-1/2"},
    { 0xfb,"-2/3"},
    { 0xf8,"-1"},
    { 0xf5,"-1 1/3"},
    { 0xf4,"-1 1/2"},
    { 0xf3,"-1 2/3"},
    { 0xf0,"-2"},
    { 0xed,"-2 1/3"},
    { 0xec,"-2 1/2"},
    { 0xeb,"-2 2/3"},
    { 0xe8,"-3"},
    { 0xe5,"-3 1/3"},
    { 0xe4,"-3 1/2"},
    { 0xe3,"-3 2/3"},
    { 0xe0,"-4"},
    { 0xdd,"-4 1/3"},
    { 0xdc,"-4 1/2"},
    { 0xdb,"-4 2/3"},
    { 0xd8,"-5"},
    { 0xd5,"-5 1/3"},
    { 0xd4,"-5 1/2"},
    { 0xd3,"-5 2/3"},
    { 0xd0,"-6"},
    { 0xcd,"-6 1/3"},
    { 0xcc,"-6 1/2"},
    { 0xcb,"-6 2/3"},
    { 0xc8,"-7"},
    { 0xc5,"-7 1/3"},
    { 0xc4,"-7 1/2"},
    { 0xc3,"-7 2/3"},
    { 0xc0,"-8"},
    { 0xffffffff,"Unknown"},
    { -1,NULL }
};
*/

/*
 *  Pairs of value and display name for image quality.
 */
const int Map::MAX_ImageQuality = 59;
const Map::PairType Map::_ImageQuality_Map[] = {
    { 0,"Unknown" },
    { EdsImageQuality_LR,     "RAW"},
    { EdsImageQuality_LRLJF,  "RAW + JPEG Large Fine"},
    { EdsImageQuality_LRMJF,  "RAW + JPEG Middle Fine"},
    { EdsImageQuality_LRSJF,  "RAW + JPEG Small Fine"},
    { EdsImageQuality_LRLJN,  "RAW + JPEG Large Normal"},
    { EdsImageQuality_LRMJN,  "RAW + JPEG Middle Normal"},
    { EdsImageQuality_LRSJN,  "RAW + JPEG Small Normal"},
    { EdsImageQuality_LRS1JF, "RAW + JPEG Small1 Fine"},
    { EdsImageQuality_LRS1JN, "RAW + JPEG Small1 Normal"},
    { EdsImageQuality_LRS2JF, "RAW + JPEG Small2"},
    { EdsImageQuality_LRS3JF, "RAW + JPEG Small3"},
    { EdsImageQuality_LRLJ,   "RAW + JPEG Large"},
    { EdsImageQuality_LRM1J,  "RAW + JPEG Middle1"},
    { EdsImageQuality_LRM2J,  "RAW + JPEG Middle2"},
    { EdsImageQuality_LRSJ,   "RAW + JPEG Small"},
    { EdsImageQuality_MR,     "Middle Raw"},
    { EdsImageQuality_MRLJF,  "Middle Raw + JPEG Large Fine"},
    { EdsImageQuality_MRMJF,  "Middle Raw + JPEG Middle Fine"},
    { EdsImageQuality_MRSJF,  "Middle Raw + JPEG Small Fine"},
    { EdsImageQuality_MRLJN,  "Middle Raw + JPEG Large Normal"},
    { EdsImageQuality_MRMJN,  "Middle Raw + JPEG Middle Normal"},
    { EdsImageQuality_MRSJN,  "Middle Raw + JPEG Small Normal"},
    { EdsImageQuality_MRS1JF, "Middle RAW + JPEG Small1 Fine"},
    { EdsImageQuality_MRS1JN, "Middle RAW + JPEG Small1 Normal"},
    { EdsImageQuality_MRS2JF, "Middle RAW + JPEG Small2"},
    { EdsImageQuality_MRS3JF, "Middle RAW + JPEG Small3"},
    { EdsImageQuality_MRLJ,   "Middle Raw + JPEG Large"},
    { EdsImageQuality_MRM1J,  "Middle Raw + JPEG Middle1"},
    { EdsImageQuality_MRM2J,  "Middle Raw + JPEG Middle2"},
    { EdsImageQuality_MRSJ,   "Middle Raw + JPEG Small"},
    { EdsImageQuality_SR,     "Small RAW"},
    { EdsImageQuality_SRLJF,  "Small RAW + JPEG Large Fine"},
    { EdsImageQuality_SRMJF,  "Small RAW + JPEG Middle Fine"},
    { EdsImageQuality_SRSJF,  "Small RAW + JPEG Small Fine"},
    { EdsImageQuality_SRLJN,  "Small RAW + JPEG Large Normal"},
    { EdsImageQuality_SRMJN,  "Small RAW + JPEG Middle Normal"},
    { EdsImageQuality_SRSJN,  "Small RAW + JPEG Small Normal"},
    { EdsImageQuality_SRS1JF, "Small RAW + JPEG Small1 Fine"},
    { EdsImageQuality_SRS1JN, "Small RAW + JPEG Small1 Normal"},
    { EdsImageQuality_SRS2JF, "Small RAW + JPEG Small2"},
    { EdsImageQuality_SRS3JF, "Small RAW + JPEG Small3"},
    { EdsImageQuality_SRLJ,   "Small RAW + JPEG Large"},
    { EdsImageQuality_SRM1J,  "Small RAW + JPEG Middle1"},
    { EdsImageQuality_SRM2J,  "Small RAW + JPEG Middle2"},
    { EdsImageQuality_SRSJ,   "Small RAW + JPEG Small"},
    { EdsImageQuality_LJF,    "JPEG Large Fine"},
    { EdsImageQuality_LJN,    "JPEG Large Normal"},
    { EdsImageQuality_MJF,    "JPEG Middle Fine"},
    { EdsImageQuality_MJN,    "JPEG Middle Normal"},
    { EdsImageQuality_SJF,    "JPEG Small Fine"},
    { EdsImageQuality_SJN,    "JPEG Small Normal"},
    { EdsImageQuality_S1JF,   "JPEG Small1 Fine"},
    { EdsImageQuality_S1JN,   "JPEG Small1 Normal"},
    { EdsImageQuality_S2JF,   "JPEG Small2"},
    { EdsImageQuality_S3JF,   "JPEG Small3"},
    { EdsImageQuality_LJ,     "JPEG Large"},
    { EdsImageQuality_M1J,    "JPEG Middle1"},
    { EdsImageQuality_M2J,    "JPEG Middle2"},
    { EdsImageQuality_SJ,     "JPEG Small"},
    { -1,NULL }
};

/*
 *  Pairs of value and display name for AF mode.
 */
const int Map::MAX_FocusMode = 4;
const Map::PairType Map::_AFMode_Map[] = {
    { AFMODE_OneShot,"One-Shot" },
    { AFMODE_AIServo,"AI Servo"},
    { AFMODE_AIFocus,"AI Focus"},
    { AFMODE_Manual,"Manual"},
    { 0xffffffff,"Unknown"},
    { -1,NULL }
};

const int Map::MAX_WhiteBalance = 18;
const Map::PairType Map::_WhiteBalance_Map[] = {
    { kEdsWhiteBalance_Auto,	    "Auto" },
    { kEdsWhiteBalance_Daylight,    "Daylight" },
    { kEdsWhiteBalance_Cloudy,	    "Cloudy" },
    { kEdsWhiteBalance_Tangsten,    "Tungsten" },
    { kEdsWhiteBalance_Fluorescent, "Fluorescent" },
    { kEdsWhiteBalance_Strobe,	    "Flash" },
    { kEdsWhiteBalance_Shade,	    "Shade" },
    { kEdsWhiteBalance_ColorTemp,   "ColorTemp" },
    { kEdsWhiteBalance_WhitePaper,  "Custom" },
    { kEdsWhiteBalance_WhitePaper2, "WhitePaper2" },
    { kEdsWhiteBalance_WhitePaper3, "WhitePaper3" },
    { kEdsWhiteBalance_WhitePaper4, "WhitePaper4" },
    { kEdsWhiteBalance_WhitePaper5, "WhitePaper5" },
    { kEdsWhiteBalance_PCSet1,	    "PCSet1" },
    { kEdsWhiteBalance_PCSet2,	    "PCSet2" },
    { kEdsWhiteBalance_PCSet3,	    "PCSet3" },
    { kEdsWhiteBalance_PCSet4,	    "PCSet4" },
    { kEdsWhiteBalance_PCSet5,	    "PCSet5" },
    { -1,NULL }
};

const int Map::MAX_DriveMode = 10;
const Map::PairType Map::_DriveMode_Map[] = {
    { DRIVE_Single,"Single shooting" },
    { DRIVE_Continuous,"Continuous shooting" },
    { DRIVE_Video,"Video" },
    { DRIVE_NotUsed,"Not used" },
    { DRIVE_ContinuousHigh,"High-Speed continuous" },
    { DRIVE_ContinuousLow,"Low-Speed continuous" },
    { DRIVE_Silent,"Silent single shooting" },
    { DRIVE_Self10C,"Self-timer 10s continuous" },
    { DRIVE_Self10,"Self-timer 10s remote" },
    { DRIVE_Self2,"Self-timer 2-seconds" },
    { DRIVE_SilentSingle,"Silent single shooting" },
    { DRIVE_SilentContinuous,"Silent continuous" },
    { -1,NULL }
};

const int Map::MAX_PictureStyle = 13;
const Map::PairType Map::_PictureStyle_Map[] = {
    { kEdsPictureStyle_Standard,"Standard" },
    { kEdsPictureStyle_Portrait,"Portrait" },
    { kEdsPictureStyle_Landscape,"Landscape" },
    { kEdsPictureStyle_Neutral,"Neutral" },
    { kEdsPictureStyle_Faithful,"Faithful" },
    { kEdsPictureStyle_Monochrome,"Monochrome" },
    { kEdsPictureStyle_Auto,"Auto" },
    { kEdsPictureStyle_User1,"User Def. 1" },
    { kEdsPictureStyle_User2,"User Def. 2" },
    { kEdsPictureStyle_User3,"User Def. 3" },
    { kEdsPictureStyle_PC1,"PC 1" },
    { kEdsPictureStyle_PC2,"PC 2" },
    { kEdsPictureStyle_PC3,"PC 3" },
    { -1,NULL }
};

/*
 *  Pairs of error code and error message.
 */
/*
const Map::PairType Map::_error_Map[] = {
    // Miscellaneous errors
    { EDS_ERR_UNIMPLEMENTED,"UNIMPLEMENTED" },
    { EDS_ERR_INTERNAL_ERROR,"INTERNAL_ERROR" },
    { EDS_ERR_MEM_ALLOC_FAILED,"MEM_ALLOC_FAILED" },
    { EDS_ERR_MEM_FREE_FAILED,"MEM_FREE_FAILED" },
    { EDS_ERR_OPERATION_CANCELLED,"OPERATION_CANCELLED" },
    { EDS_ERR_INCOMPATIBLE_VERSION,"INCOMPATIBLE_VERSION" },
    { EDS_ERR_NOT_SUPPORTED,"NOT_SUPPORTED" },
    { EDS_ERR_UNEXPECTED_EXCEPTION,"UNEXPECTED_EXCEPTION" },
    { EDS_ERR_PROTECTION_VIOLATION,"PROTECTION_VIOLATION" },
    { EDS_ERR_MISSING_SUBCOMPONENT,"MISSING_SUBCOMPONENT" },
    { EDS_ERR_SELECTION_UNAVAILABLE,"SELECTION_UNAVAILABLE" },
    // File errors
    { EDS_ERR_FILE_IO_ERROR,"FILE_IO_ERROR" },
    { EDS_ERR_FILE_TOO_MANY_OPEN,"FILE_TOO_MANY_OPEN" },
    { EDS_ERR_FILE_NOT_FOUND,"FILE_NOT_FOUND" },
    { EDS_ERR_FILE_OPEN_ERROR,"FILE_OPEN_ERROR" },
    { EDS_ERR_FILE_CLOSE_ERROR,"FILE_CLOSE_ERROR" },
    { EDS_ERR_FILE_SEEK_ERROR,"FILE_SEEK_ERROR" },
    { EDS_ERR_FILE_TELL_ERROR,"FILE_TELL_ERROR" },
    { EDS_ERR_FILE_READ_ERROR,"FILE_READ_ERROR" },
    { EDS_ERR_FILE_WRITE_ERROR,"FILE_WRITE_ERROR" },
    { EDS_ERR_FILE_PERMISSION_ERROR,"FILE_PERMISSION_ERROR" },
    { EDS_ERR_FILE_DISK_FULL_ERROR,"FILE_DISK_FULL_ERROR" },
    { EDS_ERR_FILE_ALREADY_EXISTS,"FILE_ALREADY_EXISTS" },
    { EDS_ERR_FILE_FORMAT_UNRECOGNIZED,"FILE_FORMAT_UNRECOGNIZED" },
    { EDS_ERR_FILE_DATA_CORRUPT,"FILE_DATA_CORRUPT" },
    { EDS_ERR_FILE_NAMING_NA,"FILE_NAMING_NA" },
    // Directory errors
    { EDS_ERR_DIR_NOT_FOUND,"DIR_NOT_FOUND" },
    { EDS_ERR_DIR_IO_ERROR,"DIR_IO_ERROR" },
    { EDS_ERR_DIR_ENTRY_NOT_FOUND,"DIR_ENTRY_NOT_FOUND" },
    { EDS_ERR_DIR_ENTRY_EXISTS,"DIR_ENTRY_EXISTS" },
    { EDS_ERR_DIR_NOT_EMPTY,"DIR_NOT_EMPTY" },
    // Property errors
    { EDS_ERR_PROPERTIES_UNAVAILABLE,"PROPERTIES_UNAVAILABLE" },
    { EDS_ERR_PROPERTIES_MISMATCH,"PROPERTIES_MISMATCH" },
    { EDS_ERR_PROPERTIES_NOT_LOADED,"PROPERTIES_NOT_LOADED" },
    // Function parameter errors
    { EDS_ERR_INVALID_PARAMETER,"INVALID_PARAMETER" },
    { EDS_ERR_INVALID_HANDLE,"INVALID_HANDLE" },
    { EDS_ERR_INVALID_POINTER,"INVALID_POINTER" },
    { EDS_ERR_INVALID_INDEX,"INVALID_INDEX" },
    { EDS_ERR_INVALID_LENGTH,"INVALID_LENGTH" },
    { EDS_ERR_INVALID_FN_POINTER,"INVALID_FN_POINTER" },
    { EDS_ERR_INVALID_SORT_FN,"INVALID_SORT_FN" },
    // Device errors
    { EDS_ERR_DEVICE_NOT_FOUND,"No camera detected" },
    { EDS_ERR_DEVICE_BUSY,"Camera busy" },
    { EDS_ERR_DEVICE_INVALID,"Camera Invalid" },
    { EDS_ERR_DEVICE_EMERGENCY,"Camera Emergency" },
    { EDS_ERR_DEVICE_MEMORY_FULL,"Camera Memory Full" },
    { EDS_ERR_DEVICE_INTERNAL_ERROR,"Camera Internal Error" },
    { EDS_ERR_DEVICE_INVALID_PARAMETER,"Camera Invalid Parameter" },
    { EDS_ERR_DEVICE_NO_DISK,"Camera No Disk" },
    { EDS_ERR_DEVICE_DISK_ERROR,"Camera Disk Error" },
    { EDS_ERR_DEVICE_CF_GATE_CHANGED,"Camera CF Gate Changed" },
    { EDS_ERR_DEVICE_DIAL_CHANGED,"Camera Dial Changed" },
    { EDS_ERR_DEVICE_NOT_INSTALLED,"Camera Not Installed" },
    { EDS_ERR_DEVICE_STAY_AWAKE,"Camera Stay Awake" },
    { EDS_ERR_DEVICE_NOT_RELEASED,"Camera Not Released" },
    // Stream errors
    { EDS_ERR_STREAM_IO_ERROR,"STREAM_IO_ERROR" },
    { EDS_ERR_STREAM_NOT_OPEN,"STREAM_NOT_OPEN" },
    { EDS_ERR_STREAM_ALREADY_OPEN,"STREAM_ALREADY_OPEN" },
    { EDS_ERR_STREAM_OPEN_ERROR,"STREAM_OPEN_ERROR" },
    { EDS_ERR_STREAM_CLOSE_ERROR,"STREAM_CLOSE_ERROR" },
    { EDS_ERR_STREAM_SEEK_ERROR,"STREAM_SEEK_ERROR" },
    { EDS_ERR_STREAM_TELL_ERROR,"STREAM_TELL_ERROR" },
    { EDS_ERR_STREAM_READ_ERROR,"STREAM_READ_ERROR" },
    { EDS_ERR_STREAM_WRITE_ERROR,"STREAM_WRITE_ERROR" },
    { EDS_ERR_STREAM_PERMISSION_ERROR,"STREAM_PERMISSION_ERROR" },
    { EDS_ERR_STREAM_COULDNT_BEGIN_THREAD,"STREAM_COULDNT_BEGIN_THREAD" },
    { EDS_ERR_STREAM_BAD_OPTIONS,"STREAM_BAD_OPTIONS" },
    { EDS_ERR_STREAM_END_OF_STREAM,"STREAM_END_OF_STREAM" },
    // Communications errors
    { EDS_ERR_COMM_PORT_IS_IN_USE,"COMM_PORT_IS_IN_USE" },
    { EDS_ERR_COMM_DISCONNECTED,"COMM_DISCONNECTED" },
    { EDS_ERR_COMM_DEVICE_INCOMPATIBLE,"COMM_DEVICE_INCOMPATIBLE" },
    { EDS_ERR_COMM_BUFFER_FULL,"COMM_BUFFER_FULL" },
    { EDS_ERR_COMM_USB_BUS_ERR,"COMM_USB_BUS_ERR" },
    // Lock/unlock
    { EDS_ERR_USB_DEVICE_LOCK_ERROR,"USB_DEVICE_LOCK_ERROR" },
    { EDS_ERR_USB_DEVICE_UNLOCK_ERROR,"USB_DEVICE_UNLOCK_ERROR" },
    // STI/WIA
    { EDS_ERR_STI_UNKNOWN_ERROR,"STI_UNKNOWN_ERROR" },
    { EDS_ERR_STI_INTERNAL_ERROR,"STI_INTERNAL_ERROR" },
    { EDS_ERR_STI_DEVICE_CREATE_ERROR,"STI_DEVICE_CREATE_ERROR" },
    { EDS_ERR_STI_DEVICE_RELEASE_ERROR,"STI_DEVICE_RELEASE_ERROR" },
    { EDS_ERR_DEVICE_NOT_LAUNCHED,"DEVICE_NOT_LAUNCHED" },
    { EDS_ERR_ENUM_NA,"ENUM_NA" },
    { EDS_ERR_INVALID_FN_CALL,"INVALID_FN_CALL" },
    { EDS_ERR_HANDLE_NOT_FOUND,"HANDLE_NOT_FOUND" },
    { EDS_ERR_INVALID_ID,"INVALID_ID" },
    { EDS_ERR_WAIT_TIMEOUT_ERROR,"WAIT_TIMEOUT_ERROR" },
    // PTP
    { EDS_ERR_SESSION_NOT_OPEN,"SESSION_NOT_OPEN" },
    { EDS_ERR_INVALID_TRANSACTIONID,"INVALID_TRANSACTIONID" },
    { EDS_ERR_INCOMPLETE_TRANSFER,"INCOMPLETE_TRANSFER" },
    { EDS_ERR_INVALID_STRAGEID,"INVALID_STRAGEID" },
    { EDS_ERR_DEVICEPROP_NOT_SUPPORTED,"DEVICEPROP_NOT_SUPPORTED" },
    { EDS_ERR_INVALID_OBJECTFORMATCODE,"INVALID_OBJECTFORMATCODE" },
    { EDS_ERR_SELF_TEST_FAILED,"SELF_TEST_FAILED" },
    { EDS_ERR_PARTIAL_DELETION,"PARTIAL_DELETION" },
    { EDS_ERR_SPECIFICATION_BY_FORMAT_UNSUPPORTED,"SPECIFICATION_BY_FORMAT_UNSUPPORTED" },
    { EDS_ERR_NO_VALID_OBJECTINFO,"NO_VALID_OBJECTINFO" },
    { EDS_ERR_INVALID_CODE_FORMAT,"INVALID_CODE_FORMAT" },
    { EDS_ERR_UNKNOWN_VENDOR_CODE,"UNKNOWN_VENDOR_CODE" },
    { EDS_ERR_CAPTURE_ALREADY_TERMINATED,"CAPTURE_ALREADY_TERMINATED" },
    { EDS_ERR_INVALID_PARENTOBJECT,"INVALID_PARENTOBJECT" },
    { EDS_ERR_INVALID_DEVICEPROP_FORMAT,"INVALID_DEVICEPROP_FORMAT" },
    { EDS_ERR_INVALID_DEVICEPROP_VALUE,"INVALID_DEVICEPROP_VALUE" },
    { EDS_ERR_SESSION_ALREADY_OPEN,"SESSION_ALREADY_OPEN" },
    { EDS_ERR_TRANSACTION_CANCELLED,"TRANSACTION_CANCELLED" },
    { EDS_ERR_SPECIFICATION_OF_DESTINATION_UNSUPPORTED,"SPECIFICATION_OF_DESTINATION_UNSUPPORTED" },
    { EDS_ERR_NOT_CAMERA_SUPPORT_SDK_VERSION,"NOT_CAMERA_SUPPORT_SDK_VERSION" },
    // PTP vendor
    { EDS_ERR_UNKNOWN_COMMAND,"UNKNOWN_COMMAND" },
    { EDS_ERR_OPERATION_REFUSED,"OPERATION_REFUSED" },
    { EDS_ERR_LENS_COVER_CLOSE,"LENS_COVER_CLOSE" },
    { EDS_ERR_LOW_BATTERY,"LOW_BATTERY" },
    { EDS_ERR_OBJECT_NOTREADY,"OBJECT_NOTREADY" },
    { EDS_ERR_CANNOT_MAKE_OBJECT,"CANNOT_MAKE_OBJECT" },
    // Take picture errors
    { EDS_ERR_TAKE_PICTURE_AF_NG,"TAKE_PICTURE_AF_NG" },
    { EDS_ERR_TAKE_PICTURE_RESERVED,"TAKE_PICTURE_RESERVED" },
    { EDS_ERR_TAKE_PICTURE_MIRROR_UP_NG,"TAKE_PICTURE_MIRROR_UP_NG" },
    { EDS_ERR_TAKE_PICTURE_SENSOR_CLEANING_NG,"TAKE_PICTURE_SENSOR_CLEANING_NG" },
    { EDS_ERR_TAKE_PICTURE_SILENCE_NG,"TAKE_PICTURE_SILENCE_NG" },
    { EDS_ERR_TAKE_PICTURE_NO_CARD_NG,"TAKE_PICTURE_NO_CARD_NG" },
    { EDS_ERR_TAKE_PICTURE_CARD_NG,"TAKE_PICTURE_CARD_NG" },
    { EDS_ERR_TAKE_PICTURE_CARD_PROTECT_NG,"TAKE_PICTURE_CARD_PROTECT_NG" },
    { EDS_ERR_TAKE_PICTURE_MOVIE_CROP_NG,"TAKE_PICTURE_MOVIE_CROP_NG" },
    { EDS_ERR_TAKE_PICTURE_STROBO_CHARGE_NG,"TAKE_PICTURE_STROBO_CHARGE_NG" },
    { EDS_ERR_LAST_GENERIC_ERROR_PLUS_ONE,"LAST_GENERIC_ERROR_PLUS_ONE" },
    { 0xffffffff,"Unknown" },
    { -1,NULL }
};
*/

/*
 *  Pairs of property ids and display name.
 */
/*
const Map::PairType Map::_propertyID_Map[] = {
    { kEdsPropID_Unknown,"kEdsPropID_Unknown" },
    // Camera setting properties
    { kEdsPropID_ProductName,"kEdsPropID_ProductName" },
    { kEdsPropID_BodyIDEx,"kEdsPropID_BodyIDEx" },
    { kEdsPropID_OwnerName,"kEdsPropID_OwnerName" },
    { kEdsPropID_MakerName,"kEdsPropID_MakerName" },
    { kEdsPropID_DateTime,"kEdsPropID_DateTime" },
    { kEdsPropID_FirmwareVersion,"kEdsPropID_FirmwareVersion" },
    { kEdsPropID_BatteryLevel,"kEdsPropID_BatteryLevel" },
    { kEdsPropID_CFn,"kEdsPropID_CFn" },
    { kEdsPropID_SaveTo,"kEdsPropID_SaveTo" },
    { kEdsPropID_CurrentStorage,"kEdsPropID_CurrentStorage" },
    { kEdsPropID_CurrentFolder,"kEdsPropID_CurrentFolder" },
    { kEdsPropID_MyMenu,"kEdsPropID_MyMenu" },
    { kEdsPropID_BatteryQuality,"kEdsPropID_BatteryQuality" },
    { kEdsPropID_HDDirectoryStructure,"kEdsPropID_HDDirectoryStructure" },
    // Image properties
    { kEdsPropID_ImageQuality,"kEdsPropID_ImageQuality" },
    { kEdsPropID_JpegQuality,"kEdsPropID_JpegQuality" },
    { kEdsPropID_Orientation,"kEdsPropID_Orientation" },
    { kEdsPropID_ICCProfile,"kEdsPropID_ICCProfile" },
    { kEdsPropID_FocusInfo,"kEdsPropID_FocusInfo" },
    { kEdsPropID_DigitalExposure,"kEdsPropID_DigitalExposure" },
    { kEdsPropID_WhiteBalance,"kEdsPropID_WhiteBalance" },
    { kEdsPropID_ColorTemperature,"kEdsPropID_ColorTemperature" },
    { kEdsPropID_WhiteBalanceShift,"kEdsPropID_WhiteBalanceShift" },
    { kEdsPropID_Contrast,"kEdsPropID_Contrast" },
    { kEdsPropID_ColorSaturation,"kEdsPropID_ColorSaturation" },
    { kEdsPropID_ColorTone,"kEdsPropID_ColorTone" },
    { kEdsPropID_Sharpness,"kEdsPropID_Sharpness" },
    { kEdsPropID_ColorSpace,"kEdsPropID_ColorSpace" },
    { kEdsPropID_ToneCurve,"kEdsPropID_ToneCurve" },
    { kEdsPropID_PhotoEffect,"kEdsPropID_PhotoEffect" },
    { kEdsPropID_FilterEffect,"kEdsPropID_FilterEffect" },
    { kEdsPropID_ToningEffect,"kEdsPropID_ToningEffect" },
    { kEdsPropID_ParameterSet,"kEdsPropID_ParameterSet" },
    { kEdsPropID_ColorMatrix,"kEdsPropID_ColorMatrix" },
    { kEdsPropID_PictureStyle,"kEdsPropID_PictureStyle" },
    { kEdsPropID_PictureStyleDesc,"kEdsPropID_PictureStyleDesc" },
    { kEdsPropID_PictureStyleCaption,"kEdsPropID_PictureStyleCaption" },
    // Image processing properties
    { kEdsPropID_Linear,"kEdsPropID_Linear" },
    { kEdsPropID_ClickWBPoint,"kEdsPropID_ClickWBPoint" },
    { kEdsPropID_WBCoeffs,"kEdsPropID_WBCoeffs" },
    // Image GPS properties
    { kEdsPropID_GPSVersionID,"kEdsPropID_GPSVersionID" },
    { kEdsPropID_GPSLatitudeRef,"kEdsPropID_GPSLatitudeRef" },
    { kEdsPropID_GPSLatitude,"kEdsPropID_GPSLatitude" },
    { kEdsPropID_GPSLongitudeRef,"kEdsPropID_GPSLongitudeRef" },
    { kEdsPropID_GPSLongitude,"kEdsPropID_GPSLongitude" },
    { kEdsPropID_GPSAltitudeRef,"kEdsPropID_GPSAltitudeRef" },
    { kEdsPropID_GPSAltitude,"kEdsPropID_GPSAltitude" },
    { kEdsPropID_GPSTimeStamp,"kEdsPropID_GPSTimeStamp" },
    { kEdsPropID_GPSSatellites,"kEdsPropID_GPSSatellites" },
    { kEdsPropID_GPSStatus,"kEdsPropID_GPSStatus" },
    { kEdsPropID_GPSMapDatum,"kEdsPropID_GPSMapDatum" },
    { kEdsPropID_GPSDateStamp,"kEdsPropID_GPSDateStamp" },
    // Property mask
    { kEdsPropID_AtCapture_Flag,"kEdsPropID_AtCapture_Flag" },
    // Capture properties
    { kEdsPropID_AEMode,"kEdsPropID_AEMode" },
    { kEdsPropID_DriveMode,"kEdsPropID_DriveMode" },
    { kEdsPropID_ISOSpeed,"kEdsPropID_ISOSpeed" },
    { kEdsPropID_MeteringMode,"kEdsPropID_MeteringMode" },
    { kEdsPropID_AFMode,"kEdsPropID_AFMode" },
    { kEdsPropID_Av,"kEdsPropID_Av" },
    { kEdsPropID_Tv,"kEdsPropID_Tv" },
    { kEdsPropID_ExposureCompensation,"kEdsPropID_ExposureCompensation" },
    { kEdsPropID_FlashCompensation,"kEdsPropID_FlashCompensation" },
    { kEdsPropID_FocalLength,"kEdsPropID_FocalLength" },
    { kEdsPropID_AvailableShots,"kEdsPropID_AvailableShots" },
    { kEdsPropID_Bracket,"kEdsPropID_Bracket" },
    { kEdsPropID_WhiteBalanceBracket,"kEdsPropID_WhiteBalanceBracket" },
    { kEdsPropID_LensName,"kEdsPropID_LensName" },
    { kEdsPropID_AEBracket,"kEdsPropID_AEBracket" },
    { kEdsPropID_FEBracket,"kEdsPropID_FEBracket" },
    { kEdsPropID_ISOBracket,"kEdsPropID_ISOBracket" },
    { kEdsPropID_NoiseReduction,"kEdsPropID_NoiseReduction" },
    { kEdsPropID_FlashOn,"kEdsPropID_FlashOn" },
    { kEdsPropID_RedEye,"kEdsPropID_RedEye" },
    { kEdsPropID_FlashMode,"kEdsPropID_FlashMode" },
    { kEdsPropID_LensStatus,"kEdsPropID_LensStatus" },
    { kEdsPropID_Artist,"kEdsPropID_Artist" },
    { kEdsPropID_Copyright,"kEdsPropID_Copyright" },
    { kEdsPropID_DepthOfField,"kEdsPropID_DepthOfField" },
    { kEdsPropID_EFCompensation,"kEdsPropID_EFCompensation" },
    { kEdsPropID_AEModeSelect,"kEdsPropID_AEModeSelect" },
    // EVF properties
    { kEdsPropID_Evf_OutputDevice,"kEdsPropID_Evf_OutputDevice" },
    { kEdsPropID_Evf_Mode,"kEdsPropID_Evf_Mode" },
    { kEdsPropID_Evf_WhiteBalance,"kEdsPropID_Evf_WhiteBalance" },
    { kEdsPropID_Evf_ColorTemperature,"kEdsPropID_Evf_ColorTemperature" },
    { kEdsPropID_Evf_DepthOfFieldPreview,"kEdsPropID_Evf_DepthOfFieldPreview" },
    // EVF image data properties
    { kEdsPropID_Evf_Zoom,"kEdsPropID_Evf_Zoom" },
    { kEdsPropID_Evf_ZoomPosition,"kEdsPropID_Evf_ZoomPosition" },
    { kEdsPropID_Evf_FocusAid,"kEdsPropID_Evf_FocusAid" },
    { kEdsPropID_Evf_Histogram,"kEdsPropID_Evf_Histogram" },
    { kEdsPropID_Evf_ImagePosition,"kEdsPropID_Evf_ImagePosition" },
    { kEdsPropID_Evf_HistogramStatus,"kEdsPropID_Evf_HistogramStatus" },
    { kEdsPropID_Evf_AFMode,"kEdsPropID_Evf_AFMode" },
    { kEdsPropID_Record,"kEdsPropID_Record" },
    { kEdsPropID_Evf_HistogramY,"kEdsPropID_Evf_HistogramY" },
    { kEdsPropID_Evf_HistogramR,"kEdsPropID_Evf_HistogramR" },
    { kEdsPropID_Evf_HistogramG,"kEdsPropID_Evf_HistogramG" },
    { kEdsPropID_Evf_HistogramB,"kEdsPropID_Evf_HistogramB" },
    { kEdsPropID_Evf_CoordinateSystem,"kEdsPropID_Evf_CoordinateSystem" },
    { kEdsPropID_Evf_ZoomRect,"kEdsPropID_Evf_ZoomRect" },
    { kEdsPropID_Evf_ImageClipRect,"kEdsPropID_Evf_ImageClipRect" },
    { -1,NULL }
};
*/

const Map::PairType Map::_Unknown = { 0, "Unknown" };

char *
Map::_toString( int value, const Map::PairType _map[] )
{
    int  i;

    for( i = 0; _map[i].description != NULL; i++ ) {
	if( _map[i].value == value ) break;
    }
    if( _map[i].description != NULL ) {
	return( _map[i].description );
    }
    else {
	return( _Unknown.description );
    }
}

char *
Map::toString_ShootingMode( int value )
{
    return( _toString( value, _ShootingMode_Map ) );
}

char *
Map::toString_Av( int value )
{
    return( _toString( value, _Av_Map ) );
}

char *
Map::toString_Tv( int value )
{
    return( _toString( value, _Tv_Map ) ) ;
}

char *
Map::toString_ISO( int value )
{
    return( _toString( value, _ISO_Map ) );
}

char *
Map::toString_MeteringMode( int value )
{
    return( _toString( value, _MeteringMode_Map ) );
}

char *
Map::toString_EvfAFMode( int value )
{
    return( _toString( value, _EvfAFMode_Map ) );
}

/*
char *
Map::toString_ExposureComp( int value )
{
    return( _toString( value, _ExposureComp_Map ) );
}
*/

char *
Map::toString_ImageQuality( int value )
{
    return( _toString( value, _ImageQuality_Map ) );
}

char *
Map::toString_AFMode( int value )
{
    return( _toString( value, _AFMode_Map ) );
}

char *
Map::toString_WhiteBalance( int value )
{
    return( _toString( value, _WhiteBalance_Map ) );
}

char *
Map::toString_DriveMode( int value )
{
    return( _toString( value, _DriveMode_Map ) );
}

char *
Map::toString_PictureStyle( int value )
{
    return( _toString( value, _PictureStyle_Map ) );
}

/*
char *
Map::toString_error( int value )
{
    return( _toString( value, _error_Map ) );
}
*/

/*
char *
Map::toString_propertyID( int value )
{
    return( _toString( value, _propertyID_Map ) );
}
*/

/*
 *  Convert from Canon internal value to actual ISO.
 */
int
Map::toISO( int value )
{
    char *s = Map::toString_ISO( value );
    if(      s[0] == 'A' ) return( 0 );
    else if( s[0] == 'H' ) return( 12800 );
    else	           return( atoi( s ) );
}

/*
 *  Convert from actual ISO to Canon internal value.
 */
int
Map::fromISO( int value )
{
    switch( value ) {
        case     6: return( 0x28 );
        case    12: return( 0x30 );
        case    25: return( 0x38 );
        case    50: return( 0x40 );
        case   100: return( 0x48 );
        case   125: return( 0x4b );
        case   160: return( 0x4d );
        case   200: return( 0x50 );
        case   250: return( 0x53 );
        case   320: return( 0x55 );
        case   400: return( 0x58 );
        case   500: return( 0x5b );
        case   640: return( 0x5d );
        case   800: return( 0x60 );
        case  1000: return( 0x63 );
        case  1250: return( 0x65 );
        case  1600: return( 0x68 );
        case  3200: return( 0x70 );
        case  6400: return( 0x78 );
        case 12800: return( 0x80 );
        default:    return( 0 );
    }
}

/*
 *  Convert from Canon internal value to actual
 *  exposure compensation.
 */
double
Map::toCompensation( int value )
{
    int e = value;
    e = e / 8;
    if( e >= 24 ) {
	e = e - 32;
    }

    double compensation;
    switch( value % 8 ) {
	case 0:  compensation = e; break;
	case 3:  compensation = e + 1.0/3.0; break;
	case 4:  compensation = e + 1.0/2.0; break;
	case 5:  compensation = e + 2.0/3.0; break;
	default: compensation = 0.0; break;
    }

    return( compensation );
}

/*
 *  Convert from actual exposure compensation to
 *  Canon internal value.
 */
int
Map::fromCompensation( double value )
{
    int e;
    double diff;

    if( value == 0.0 ) {
	return( 0 );
    }

    e = 8 * value;
    diff = (8.0 * value) - e;
    if( diff >  0.5 ) e++;
    if( diff < -0.5 ) e--;
    if( e < 0 ) e = 256 + e;

    return( e );
}

/*
 *  Convert from Canon internal value to approximate
 *  shutter speed (milliseconds).
 */
double
Map::toShutterSpeed( int value )
{
    double speed = 32000.0;	// 32 seconds;

    for( int i = 2; i < 20; i++ ) {
	if( i*8 == value ) {
	    return( speed );
	}
	else
	if( (i*8 < value) && (value < (i+1)*8) ) {
	    return( (speed + speed/2.0) / 2.0 );
	}
	speed /= 2.0;
    }

    return( 0.0 );
}

