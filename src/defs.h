#ifndef DEFS_H
#define DEFS_H  // prevent multiple definitions on Arduino IDE.

#include <Arduino.h> // For unsigned long and float types

// structure to hold the hw status going up to the webpage.
struct RAS_HW_STATUS_STRUCT {
    bool PWR_Button;
    unsigned long disturber_accum;  // disturber accumulated count since reset
    unsigned long disturber_et;     // disturber elapsed time in seconds since last disturber event
    unsigned long noise_accum;
    unsigned long noise_et;
    unsigned long strike_accum;
    unsigned long strike_et;
    float strike_distance;  // distance est from storm/strike as a float 16-May-2025 w9zv
    unsigned long strike_energy;
    const char *pSoftwareVersion;
    
    // added rates 16-May-2025 w9zv
    float strikeRate;
    float disturberRate;
    float noiseRate;
    float purgeRate;
};

// Declare the possible valid host commands in meaningful English. Enums start enumerating
// at zero, incrementing in steps of 1 unless overridden. We use an
// enum 'class' here for type safety and code readability
enum class HostCmdEnum : byte {
    UNUSED, // unused/invalid, defaults to 0
    PWR_SELECT,             // power button on GUI

// these are unused, but here to allow compilation of the RAS handlers (until removed)
// when web was added, started with copying over the RAS 3 web code... 
    SELECT_RADIO_1, // defaults to 1
    SELECT_RADIO_2, // defaults to 2
    SELECT_RADIO_3, // defaults to 3
    SELECT_RADIO_4, // defaults to 4
    AUX_1_ON, // defaults to 5
    AUX_2_ON, // ...
    AUX_1_OFF,
    AUX_2_OFF,
    ALL_GROUNDED,

    STATUS_REQUEST,
    RQST_MAX_EXEC_LOOP_TIME,
    NO_OP = 255
};

#define TFT_IM_HERE() _tft_im_here(__LINE__);

struct statistics_struct {
    unsigned long strikeCount;
    unsigned long disturberCount;
    unsigned long noiseCount;
    unsigned long purgeCount;

    float strikeRate;
    float disturberRate;
    float noiseRate;
    float purgeRate;

    float maxStrikeRate;
    float maxDisturberRate;
    float maxNoiseRate;
    float maxPurgeRate;

    unsigned long lastRateCalcTick;
};

#endif
