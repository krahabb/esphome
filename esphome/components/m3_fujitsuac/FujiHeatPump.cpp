/* This file is based on unreality's FujiHeatPump project */

// #define DEBUG_FUJI
#include "FujiHeatPump.h"
#include "esphome/core/log.h"
// The esphome ESP_LOGx macros expand to reference esp_log_printf_, but do so
// without using its namespace. https://github.com/esphome/issues/issues/3196
// The workaround is to pull that particular function into this namespace.
using esphome::esp_log_printf_;

/*FujiFrame FujiHeatPump::decodeFrame(byte readbuf[8]) {
    FujiFrame ff;

    ff.messageSource = readbuf[0];
    ff.messageDest = readbuf[1] & 0b01111111;
    ff.loginBit = (readbuf[1] & 0b00100000) != 0;
    ff.unknownBit = (readbuf[1] & 0b10000000) > 0;
    ff.messageType = (readbuf[2] & 0b00110000) >> 4;
    ff.writeBit = (readbuf[2] & 0b00001000) != 0;

    ff.acError = (readbuf[kErrorIndex] & kErrorMask) >> kErrorOffset;
    ff.temperature =
        (readbuf[kTemperatureIndex] & kTemperatureMask) >> kTemperatureOffset;
    ff.acMode = (readbuf[kModeIndex] & kModeMask) >> kModeOffset;
    ff.fanMode = (readbuf[kFanIndex] & kFanMask) >> kFanOffset;
    ff.economyMode = (readbuf[kEconomyIndex] & kEconomyMask) >> kEconomyOffset;
    ff.swingMode = (readbuf[kSwingIndex] & kSwingMask) >> kSwingOffset;
    ff.swingStep =
        (readbuf[kSwingStepIndex] & kSwingStepMask) >> kSwingStepOffset;
    ff.controllerPresent =
        (readbuf[kControllerPresentIndex] & kControllerPresentMask) >>
        kControllerPresentOffset;
    ff.updateMagic =
        (readbuf[kUpdateMagicIndex] & kUpdateMagicMask) >> kUpdateMagicOffset;
    ff.onOff = (readbuf[kEnabledIndex] & kEnabledMask) >> kEnabledOffset;
    ff.controllerTemp = (readbuf[kControllerTempIndex] & kControllerTempMask) >>
                        kControllerTempOffset;  // there are 2 leading bits here
                                                // that are unknown

    return ff;
}

void FujiHeatPump::encodeFrame(FujiFrame ff) {
    memset(writeBuf, 0, 8);

    writeBuf[0] = ff.messageSource;

    writeBuf[1] &= 0b10000000;
    writeBuf[1] |= ff.messageDest & 0b01111111;

    writeBuf[2] &= 0b11001111;
    writeBuf[2] |= ff.messageType << 4;

    if (ff.writeBit) {
        writeBuf[2] |= 0b00001000;
    } else {
        writeBuf[2] &= 0b11110111;
    }

    writeBuf[1] &= 0b01111111;
    if (ff.unknownBit) {
        writeBuf[1] |= 0b10000000;
    }

    if (ff.loginBit) {
        writeBuf[1] |= 0b00100000;
    } else {
        writeBuf[1] &= 0b11011111;
    }

    writeBuf[kModeIndex] =
        (writeBuf[kModeIndex] & ~kModeMask) | (ff.acMode << kModeOffset);
    writeBuf[kModeIndex] = (writeBuf[kEnabledIndex] & ~kEnabledMask) |
                           (ff.onOff << kEnabledOffset);
    writeBuf[kFanIndex] =
        (writeBuf[kFanIndex] & ~kFanMask) | (ff.fanMode << kFanOffset);
    writeBuf[kErrorIndex] =
        (writeBuf[kErrorIndex] & ~kErrorMask) | (ff.acError << kErrorOffset);
    writeBuf[kEconomyIndex] = (writeBuf[kEconomyIndex] & ~kEconomyMask) |
                              (ff.economyMode << kEconomyOffset);
    writeBuf[kTemperatureIndex] =
        (writeBuf[kTemperatureIndex] & ~kTemperatureMask) |
        (ff.temperature << kTemperatureOffset);
    writeBuf[kSwingIndex] =
        (writeBuf[kSwingIndex] & ~kSwingMask) | (ff.swingMode << kSwingOffset);
    writeBuf[kSwingStepIndex] = (writeBuf[kSwingStepIndex] & ~kSwingStepMask) |
                                (ff.swingStep << kSwingStepOffset);
    writeBuf[kControllerPresentIndex] =
        (writeBuf[kControllerPresentIndex] & ~kControllerPresentMask) |
        (ff.controllerPresent << kControllerPresentOffset);
    writeBuf[kUpdateMagicIndex] =
        (writeBuf[kUpdateMagicIndex] & ~kUpdateMagicMask) |
        (ff.updateMagic << kUpdateMagicOffset);
    writeBuf[kControllerTempIndex] =
        (writeBuf[kControllerTempIndex] & ~kControllerTempMask) |
        (ff.controllerTemp << kControllerTempOffset);
}*/

/*
void FujiHeatPump::connect(HardwareSerial *serial, bool secondary) {
    return this->connect(serial, secondary, -1, -1, -1);
}

void FujiHeatPump::connect(
    HardwareSerial *serial, 
    bool secondary,
    int rxPin = -1, 
    int txPin = -1, 
    int txEnablePin = -1) {
    _serial = serial;
    _txenablepin = txEnablePin;
    if (txEnablePin != -1) {
        pinMode(txEnablePin, OUTPUT);
    }
    if (rxPin != -1 && txPin != -1) {
#ifdef ESP32
        _serial->begin(500, SERIAL_8E1, rxPin, txPin);
#else        
        _serial->begin(500, SERIAL_8E1);
        if ((rxPin == 13) && (txPin==15)) {
            _serial->swap();
        }
#endif
    } else {
        _serial->begin(500, SERIAL_8E1);
    }
    _serial->setTimeout(200);

    if (secondary) {
        controllerIsPrimary = false;
        controllerAddress = static_cast<byte>(FujiAddress::SECONDARY);
    } else {
        controllerIsPrimary = true;
        controllerAddress = static_cast<byte>(FujiAddress::PRIMARY);
    }

    lastFrameReceived = 0;
}*/

/*bool FujiHeatPump::readFrame(FujiFrameRaw rawframe) {
    if (_serial->available()) {
        memset(&rawframe, 0, sizeof(rawframe));
        rawframe.datalen = _serial->readBytes(rawframe.data, 8);
        rawframe.millis = millis();
        for (int i = rawframe.datalen; i > 0;) {
            rawframe.data[--i] ^= 0xFF;
        }        
        return true;
    }
    return false;

        ESP_LOGD("fuji", "%08d RECV(%d): %X %X %X %X %X %X %X %X", (int)millis(), bytesRead, readbuf[0], readbuf[1], readbuf[2],
             readbuf[3], readbuf[4], readbuf[5], readbuf[6], readbuf[7]);

        if (rawframe.datalen < 8) {
            // skip incomplete frame
            return false;
        }


}*/

/*void FujiHeatPump::sendPendingFrame() {
    if (pendingFrame) {
        unsigned long waitfor = (lastFrameReceived + 50) - millis();
        if (waitfor > 0)
            delay(waitfor);
        if (_txenablepin != -1) {
            digitalWrite(_txenablepin, 1);
        }            
        _serial->write(writeBuf, 8);
        _serial->flush();
        if (_txenablepin != -1) {
            digitalWrite(_txenablepin, 0);
        }            
        pendingFrame = false;
        updateFields = 0;
        _serial->readBytes(
            writeBuf,
            8);  // read back our own frame so we dont process it again
    }
}*/

/*bool FujiHeatPump::waitForFrame() {    

    if (_serial->available()) {

        FujiFrame ff;
        memset(ff.data, 0, sizeof(ff.data));
        ff.datalen = _serial->readBytes(ff.data, 8);
        ff.millis = millis();
        for (int i = ff.datalen; i > 0;) {
            ff.data[--i] ^= 0xFF;
        }        
        ff.decode();
        // collect the first transactions on the bus...
        if (frameLog.size() < 30)
        {
            frameLog.push_back(ff);
        }
        
        char buffer[256];
        ESP_LOGD("fuji", "%s", ff.dump_payload(buffer));
        ESP_LOGD("fuji", "%s", ff.dump_decoded(buffer));

        if (ff.datalen < 8) {
            // skip incomplete frame
            return false;
        }

        if (ff.messageDest == controllerAddress) {
            lastFrameReceived = ff.millis;

            if (ff.messageType == static_cast<byte>(FujiMessageType::STATUS)) {
                if (ff.controllerPresent == 1) {
                    // we have logged into the indoor unit
                    // this is what most frames are
                    ff.messageSource = controllerAddress;

                    if (seenSecondaryController) {
                        ff.messageDest =
                            static_cast<byte>(FujiAddress::SECONDARY);
                        ff.loginBit = true;
                        ff.controllerPresent = 0;
                    } else {
                        ff.messageDest = static_cast<byte>(FujiAddress::UNIT);
                        ff.loginBit = false;
                        ff.controllerPresent = 1;
                    }

                    ff.updateMagic = 0;
                    ff.unknownBit = true;
                    ff.writeBit = 0;
                    ff.messageType = static_cast<byte>(FujiMessageType::STATUS);
                } else {
                    if (controllerIsPrimary) {
                        // if this is the first message we have received,
                        // announce ourselves to the indoor unit
                        ff.messageSource = controllerAddress;
                        ff.messageDest = static_cast<byte>(FujiAddress::UNIT);
                        ff.loginBit = false;
                        ff.controllerPresent = 0;
                        ff.updateMagic = 0;
                        ff.unknownBit = true;
                        ff.writeBit = 0;
                        ff.messageType =
                            static_cast<byte>(FujiMessageType::LOGIN);

                        ff.onOff = 0;
                        ff.temperature = 0;
                        ff.acMode = 0;
                        ff.fanMode = 0;
                        ff.swingMode = 0;
                        ff.swingStep = 0;
                        ff.acError = 0;
                    } else {
                        // secondary controller never seems to get any other
                        // message types, only status with controllerPresent ==
                        // 0 the secondary controller seems to send the same
                        // flags no matter which message type

                        ff.messageSource = controllerAddress;
                        ff.messageDest = static_cast<byte>(FujiAddress::UNIT);
                        ff.loginBit = false;
                        ff.controllerPresent = 1;
                        ff.updateMagic = 2;
                        ff.unknownBit = true;
                        ff.writeBit = 0;
                    }
                }

                // if we have any updates, set the flags
                if (updateFields) {
                    ff.writeBit = 1;
                }

                if (updateFields & kOnOffUpdateMask) {
                    ff.onOff = updateState.onOff;
                }

                if (updateFields & kTempUpdateMask) {
                    ff.temperature = updateState.temperature;
                }

                if (updateFields & kModeUpdateMask) {
                    ff.acMode = updateState.acMode;
                }

                if (updateFields & kFanModeUpdateMask) {
                    ff.fanMode = updateState.fanMode;
                }

                if (updateFields & kSwingModeUpdateMask) {
                    ff.swingMode = updateState.swingMode;
                }

                if (updateFields & kSwingStepUpdateMask) {
                    ff.swingStep = updateState.swingStep;
                }

                memcpy(&currentState, &ff, sizeof(FujiFrame));
            } else if (ff.messageType ==
                       static_cast<byte>(FujiMessageType::LOGIN)) {
                // received a login frame OK frame
                // the primary will send packet to a secondary controller to see
                // if it exists
                ff.messageSource = controllerAddress;
                ff.messageDest = static_cast<byte>(FujiAddress::SECONDARY);
                ff.loginBit = true;
                ff.controllerPresent = 1;
                ff.updateMagic = 0;
                ff.unknownBit = true;
                ff.writeBit = 0;

                ff.onOff = currentState.onOff;
                ff.temperature = currentState.temperature;
                ff.acMode = currentState.acMode;
                ff.fanMode = currentState.fanMode;
                ff.swingMode = currentState.swingMode;
                ff.swingStep = currentState.swingStep;
                ff.acError = currentState.acError;
            } else if (ff.messageType ==
                       static_cast<byte>(FujiMessageType::ERROR)) {
                ESP_LOGD("fuji", "AC ERROR RECV: ");
                // handle errors here
                return false;
            }

            pendingFrame = true;
            //we're replying asap
            ff.encode();
            for (int i = 0; i < 8; i++) {
                ff.data[i] ^= 0xFF;
            }


        } else if (ff.messageDest ==
                   static_cast<byte>(FujiAddress::SECONDARY)) {
            seenSecondaryController = true;
            currentState.controllerTemp =
                ff.controllerTemp;  // we dont have a temp sensor, use the temp
                                    // reading from the secondary controller
        }

        return true;
    }

    return false;
}

bool FujiHeatPump::isBound() {
    if (millis() - lastFrameReceived < 1000) {
        return true;
    }
    return false;
}

bool FujiHeatPump::updatePending() {
    if (updateFields) {
        return true;
    }
    return false;
}

void FujiHeatPump::setOnOff(bool o) {
    updateFields |= kOnOffUpdateMask;
    updateState.onOff = o ? 1 : 0;
}
void FujiHeatPump::setTemp(byte t) {
    updateFields |= kTempUpdateMask;
    updateState.temperature = t;
}
void FujiHeatPump::setMode(byte m) {
    updateFields |= kModeUpdateMask;
    updateState.acMode = m;
}
void FujiHeatPump::setFanMode(byte fm) {
    updateFields |= kFanModeUpdateMask;
    updateState.fanMode = fm;
}
void FujiHeatPump::setEconomyMode(byte em) {
    updateFields |= kEconomyModeUpdateMask;
    updateState.economyMode = em;
}
void FujiHeatPump::setSwingMode(byte sm) {
    updateFields |= kSwingModeUpdateMask;
    updateState.swingMode = sm;
}
void FujiHeatPump::setSwingStep(byte ss) {
    updateFields |= kSwingStepUpdateMask;
    updateState.swingStep = ss;
}

bool FujiHeatPump::getOnOff() { return currentState.onOff == 1 ? true : false; }
byte FujiHeatPump::getTemp() { return currentState.temperature; }
byte FujiHeatPump::getMode() { return currentState.acMode; }
byte FujiHeatPump::getFanMode() { return currentState.fanMode; }
byte FujiHeatPump::getEconomyMode() { return currentState.economyMode; }
byte FujiHeatPump::getSwingMode() { return currentState.swingMode; }
byte FujiHeatPump::getSwingStep() { return currentState.swingStep; }
byte FujiHeatPump::getControllerTemp() { return currentState.controllerTemp; }

FujiFrame *FujiHeatPump::getCurrentState() { return &currentState; }

FujiFrame *FujiHeatPump::getUpdateState() { return &updateState; }

void FujiHeatPump::setState(FujiFrame *state) {
    FujiFrame *current = this->getCurrentState();
    if (state->onOff != current->onOff) {
        this->setOnOff(state->onOff);
    }

    if (state->temperature != current->temperature) {
        this->setTemp(state->temperature);
    }

    if (state->acMode != current->acMode) {
        this->setMode(state->acMode);
    }

    if (state->fanMode != current->fanMode) {
        this->setFanMode(state->fanMode);
    }

    if (state->economyMode != current->economyMode) {
        this->setEconomyMode(state->economyMode);
    }

    if (state->swingMode != current->swingMode) {
        this->setSwingMode(state->swingMode);
    }

    if (state->swingStep != current->swingStep) {
        this->setSwingStep(state->swingStep);
    }
}

byte FujiHeatPump::getUpdateFields() { return updateFields; }
*/