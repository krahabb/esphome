/* This file is based on unreality's FujiHeatPump project */
#pragma once

//#include "esphome/core/defines.h"
//#include "esphome/core/hal.h"
#include "arduino.h"

/*const byte kModeIndex = 3;
const byte kModeMask = 0b00001110;
const byte kModeOffset = 1;

const byte kFanIndex = 3;
const byte kFanMask = 0b01110000;
const byte kFanOffset = 4;

const byte kEnabledIndex = 3;
const byte kEnabledMask = 0b00000001;
const byte kEnabledOffset = 0;

const byte kErrorIndex = 3;
const byte kErrorMask = 0b10000000;
const byte kErrorOffset = 7;

const byte kEconomyIndex = 4;
const byte kEconomyMask = 0b10000000;
const byte kEconomyOffset = 7;

const byte kTemperatureIndex = 4;
const byte kTemperatureMask = 0b01111111;
const byte kTemperatureOffset = 0;

const byte kUpdateMagicIndex = 5;
const byte kUpdateMagicMask = 0b11110000;
const byte kUpdateMagicOffset = 4;

const byte kSwingIndex = 5;
const byte kSwingMask = 0b00000100;
const byte kSwingOffset = 2;

const byte kSwingStepIndex = 5;
const byte kSwingStepMask = 0b00000010;
const byte kSwingStepOffset = 1;

const byte kControllerPresentIndex = 6;
const byte kControllerPresentMask = 0b00000001;
const byte kControllerPresentOffset = 0;

const byte kControllerTempIndex = 6;
const byte kControllerTempMask = 0b00111110;
const byte kControllerTempOffset = 1;

const byte kOnOffUpdateMask = 0b10000000;
const byte kTempUpdateMask = 0b01000000;
const byte kModeUpdateMask = 0b00100000;
const byte kFanModeUpdateMask = 0b00010000;
const byte kEconomyModeUpdateMask = 0b00001000;
const byte kSwingModeUpdateMask = 0b00000100;
const byte kSwingStepUpdateMask = 0b00000010;
*/

#define FUJITSUAC_FRAMESIZE 8

#define FUJITSUAC_ADDR_MASTER 0
#define FUJITSUAC_ADDR_UNIT 1
#define FUJITSUAC_ADDR_PRIMARYCONTROLLER 32
#define FUJITSUAC_ADDR_SECONDARYCONTROLLER 33

#define FUJITSUAC_FRAMETYPE_STATUS 0
#define FUJITSUAC_FRAMETYPE_ERROR 1
#define FUJITSUAC_FRAMETYPE_LOGIN 2
#define FUJITSUAC_FRAMETYPE_UNKNOWN 3

#define FUJITSUAC_ACMODE_UNKNOWN 0
#define FUJITSUAC_ACMODE_FAN 1
#define FUJITSUAC_ACMODE_DRY 2
#define FUJITSUAC_ACMODE_COOL 3
#define FUJITSUAC_ACMODE_HEAT 4
#define FUJITSUAC_ACMODE_AUTO 5

#define FUJITSUAC_FANMODE_AUTO 0
#define FUJITSUAC_FANMODE_QUIET 1
#define FUJITSUAC_FANMODE_LOW 2
#define FUJITSUAC_FANMODE_MEDIUM 3
#define FUJITSUAC_FANMODE_HIGH 4

/*enum class FujiMessageType : byte {
    STATUS = 0,
    ERROR = 1,
    LOGIN = 2,
    UNKNOWN = 3,
};

enum class FujiMode : byte {
    UNKNOWN = 0,
    FAN = 1,
    DRY = 2,
    COOL = 3,
    HEAT = 4,
    AUTO = 5,
};*/

/*enum class FujiAddress : byte {
    START = 0,
    UNIT = 1,
    PRIMARY = 32,
    SECONDARY = 33,
};

enum class FujiFanMode : byte {
    FAN_AUTO = 0,
    FAN_QUIET = 1,
    FAN_LOW = 2,
    FAN_MEDIUM = 3,
    FAN_HIGH = 4
};*/

#pragma pack(push, 1)
typedef struct _FujiFrame {
    unsigned long millis = {};
    uint8_t datalen = {};
    union {
        byte data[FUJITSUAC_FRAMESIZE] = {};
        struct {
            //data[0]
            unsigned int addr_src : 8;
            //data[1]
            unsigned int addr_dst : 7;
            unsigned int bit_unknown : 1;            
            //data[2]
            unsigned int _dummy_2_0 : 3;
            unsigned int bit_write : 1;            
            unsigned int type : 2;
            unsigned int _dummy_2_1 : 2;
            //data[3]
            unsigned int enabled : 1;//on/off
            unsigned int acmode : 3;
            unsigned int fanmode : 3;
            unsigned int bit_error : 1;
            //data[4]
            unsigned int temp_target : 5;
            unsigned int _dummy_4_0 : 2;
            unsigned int economy : 1;
            //data[5]
            unsigned int _dummy_5_0 : 1;
            unsigned int swingstep : 1;
            unsigned int swing : 1;
            unsigned int _dummy_5_1 : 1;
            unsigned int updatemagic : 4;
            //data[6]
            unsigned int bit_controllerpresent : 1;
            unsigned int temp_room : 5;
            unsigned int _dummy_6_0 : 2;
            //data[7]
        };
    };

    /*byte onOff = 0;
    byte temperature = 16;
    byte acMode = 0;
    byte fanMode = 0;
    byte acError = 0;
    byte economyMode = 0;
    byte swingMode = 0;
    byte swingStep = 0;
    byte controllerPresent = 0;
    byte updateMagic = 0;  // unsure what this value indicates
    byte controllerTemp = 16;

    bool writeBit = false;
    bool loginBit = false;
    bool unknownBit = false;  // unsure what this bit indicates

    byte messageType = 0;
    byte messageSource = 0;
    byte messageDest = 0;
    */

    //void decode();
    //void encode();

    const char* dump_payload(char buffer[]);
    const char* dump_decoded(char buffer[]);
} FujiFrame;
#pragma pack(pop)

/*typedef struct FujiFrames {
    byte onOff = 0;
    byte temperature = 16;
    byte acMode = 0;
    byte fanMode = 0;
    byte acError = 0;
    byte economyMode = 0;
    byte swingMode = 0;
    byte swingStep = 0;
    byte controllerPresent = 0;
    byte updateMagic = 0;  // unsure what this value indicates
    byte controllerTemp = 16;

    bool writeBit = false;
    bool loginBit = false;
    bool unknownBit = false;  // unsure what this bit indicates

    byte messageType = 0;
    byte messageSource = 0;
    byte messageDest = 0;

    void decode(FujiFrameRaw& rawframe);
    void encode(FujiFrameRaw& rawframe);
    const char* dump(char buffer[]);
} FujiFrame;*/

