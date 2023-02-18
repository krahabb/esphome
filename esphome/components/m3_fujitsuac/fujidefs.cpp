#include "fujidefs.h"


const char* FujiFrame::dump_payload(char buffer[]) {
    sprintf(buffer, "%08d DATA(%d): %X %X %X %X %X %X %X %X", (int)millis, datalen, data[0], data[1], data[2],
             data[3], data[4], data[5], data[6], data[7]);    
    return buffer;      
}
const char* FujiFrame::dump_decoded(char buffer[]) {
    sprintf(buffer,
            //"mSrc: %d mDst: %d mType: %d write: %d login: %d unknown: %d "
            //"onOff: %d temp: %d, mode: %d cP:%d uM:%d cTemp:%d acError:%d\n"
            "addr_src:%d addr_dst:%d type:%d updatemagic:%d "
            "bit_unknown:%d bit_write:%d bit_error:%d bit_controllerpresent:%d "
            "enabled:%d temp_target:%d acmode:%d fanmode:%d temp_room:%d "
            "economy:%d swing:%d swingstep:%d\n",
            //messageSource, messageDest, messageType, writeBit,
            //loginBit, unknownBit, onOff, temperature, acMode,
            //controllerPresent, updateMagic, controllerTemp, acError,
            addr_src, addr_dst, type, updatemagic,
            bit_unknown, bit_write, bit_error, bit_controllerpresent,
            enabled, temp_target, acmode, fanmode, temp_room,
            economy, swing, swingstep);
    return buffer;
}

/*
void FujiFrame::decode() {
    messageSource = data[0];
    messageDest = data[1] & 0b01111111;
    loginBit = (data[1] & 0b00100000) != 0;
    unknownBit = (data[1] & 0b10000000) > 0;
    messageType = (data[2] & 0b00110000) >> 4;
    writeBit = (data[2] & 0b00001000) != 0;

    acError = (data[kErrorIndex] & kErrorMask) >> kErrorOffset;
    temperature = (data[kTemperatureIndex] & kTemperatureMask) >> kTemperatureOffset;
    acMode = (data[kModeIndex] & kModeMask) >> kModeOffset;
    fanMode = (data[kFanIndex] & kFanMask) >> kFanOffset;
    economyMode = (data[kEconomyIndex] & kEconomyMask) >> kEconomyOffset;
    swingMode = (data[kSwingIndex] & kSwingMask) >> kSwingOffset;
    swingStep = (data[kSwingStepIndex] & kSwingStepMask) >> kSwingStepOffset;
    controllerPresent = (data[kControllerPresentIndex] & kControllerPresentMask) >>
        kControllerPresentOffset;
    updateMagic = (data[kUpdateMagicIndex] & kUpdateMagicMask) >> kUpdateMagicOffset;
    onOff = (data[kEnabledIndex] & kEnabledMask) >> kEnabledOffset;
    controllerTemp = (data[kControllerTempIndex] & kControllerTempMask) >>
                        kControllerTempOffset;  // there are 2 leading bits here
                                                // that are unknown
}

void FujiFrame::encode() {
    memset(data, 0, sizeof(data));

    data[0] = messageSource;

    data[1] &= 0b10000000;
    data[1] |= messageDest & 0b01111111;

    data[2] &= 0b11001111;
    data[2] |= messageType << 4;

    if (writeBit) {
        data[2] |= 0b00001000;
    } else {
        data[2] &= 0b11110111;
    }

    data[1] &= 0b01111111;
    if (unknownBit) {
        data[1] |= 0b10000000;
    }

    if (loginBit) {
        data[1] |= 0b00100000;
    } else {
        data[1] &= 0b11011111;
    }

    data[kModeIndex] =
        (data[kModeIndex] & ~kModeMask) | 
        (acMode << kModeOffset);
    data[kModeIndex] = 
        (data[kEnabledIndex] & ~kEnabledMask) | 
        (onOff << kEnabledOffset);
    data[kFanIndex] =
        (data[kFanIndex] & ~kFanMask) | 
        (fanMode << kFanOffset);
    data[kErrorIndex] =
        (data[kErrorIndex] & ~kErrorMask) | 
        (acError << kErrorOffset);
    data[kEconomyIndex] = 
        (data[kEconomyIndex] & ~kEconomyMask) | 
        (economyMode << kEconomyOffset);
    data[kTemperatureIndex] =
        (data[kTemperatureIndex] & ~kTemperatureMask) |
        (temperature << kTemperatureOffset);
    data[kSwingIndex] =
        (data[kSwingIndex] & ~kSwingMask) | 
        (swingMode << kSwingOffset);
    data[kSwingStepIndex] = 
        (data[kSwingStepIndex] & ~kSwingStepMask) |
        (swingStep << kSwingStepOffset);
    data[kControllerPresentIndex] =
        (data[kControllerPresentIndex] & ~kControllerPresentMask) |
        (controllerPresent << kControllerPresentOffset);
    data[kUpdateMagicIndex] =
        (data[kUpdateMagicIndex] & ~kUpdateMagicMask) |
        (updateMagic << kUpdateMagicOffset);
    data[kControllerTempIndex] =
        (data[kControllerTempIndex] & ~kControllerTempMask) |
        (controllerTemp << kControllerTempOffset);

}*/

