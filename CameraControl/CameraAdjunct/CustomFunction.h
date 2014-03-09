/*
 *  Custom Function values.
 *  These are not documented and are specific to a camera.
 *  The following are valid for the 5D Mark III.
 *  Only the first is interesting.
 *       257,
 *       258,
 *       260,
 *       261,
 *       264,
 *      1034,
 *      1035,
 *      1798,
 *      2062,
 *      2067
 */
enum CustomFunctionValues {
    /*
     *  Exposure level increments:
     *    0 - 1/3-stop increments
     *    1 - 1/2-stop increments
     *  5D Mark III and 550D/Rebel T2i.
     */
    CFn_ExposureLevelIncrements = 257,

    /*
     *  ISO expansion:
     *    0 - off
     *    1 - on
     *  Only 550D/Rebel T2i.
     *  Not currently used.
     */
    CFn_ISOexpansion = 259,

    /*
     *  Flash synchronization speed in Av mode:
     *    0 - auto
     *    1 - 1/200 - 1/60 sec. auto
     *    2 - 1/200 sec. fixed
     *  Not currently used.
     */
    CFn_FlashSyncSpeedInAvMode = 271,

    /*
     *  Long exposure noise reduction:
     *    0 - off
     *    1 - auto
     *    2 - on
     *  Only 550D/Rebel T2i.
     *  Not currently used.
     */
    CFn_LongExposureNoiseReduction = 513,

    /*
     *  High ISO speed noise reduction:
     *    0 - standard
     *    1 - low
     *    2 - strong
     *    3 - disable
     *  Only 550D/Rebel T2i.
     *  Not currently used.
     */
    CFn_HighISOSpeedNoiseReduction = 514,

    /*
     *  Highlight tone priority:
     *    0 - disable
     *    1 - enable
     *  Only 550D/Rebel T2i.
     *  Not currently used.
     */
    CFn_HighlightTonePriority = 515,

    /*
     *  Auto focus assist beam firing:
     *    0 - enable
     *    1 - disable
     *    2 - enable external flash only
     *    3 - IR AF assist beam only
     *  Not currently used.
     */
    CFn_AutoFocusAssistBeamFiring = 1294,

    /*
     *  Mirror lockup:
     *    0 - disable
     *    1 - enable
     *  Only 550D/Rebel T2i.
     *  Not currently used.
     */
    CFn_MirrorLockup = 1551,

    /*
     *  Shutter/AE lock button:
     *    0 - AF/AE lock
     *    1 - AE lock/AF
     *    2 - AF/AF lock, no AE lock
     *    3 - AE/AF, no AE lock
     *  Not currently used.
     */
    CFn_ShutterAELockButton = 1793,

    /*
     *  Assign SET button:
     *    0 - Normal (disabled)
     *    1 - Image quality
     *    2 - Flash exposure compensation
     *    3 - LCD monitor on/off
     *    4 - Menu display
     *    5 - ISO speed
     */
    // not needed

    /*
     *  LCD display when power on:
     *    0 - Display on
     *    1 - Previous display status
     */
    // not needed

    /*
     *  Add image verification data:
     *    0 - disable
     *    1 - enable
     */
    // not needed
};

