/* This file is based on unreality's FujiHeatPump project */
#pragma once

//#include "esphome/core/defines.h"
//#include "esphome/core/hal.h"

/*
class FujiHeatPump {
   private:
    HardwareSerial *_serial;
    uint8_t         _txenablepin;

    //byte            writeBuf[8];

    byte controllerAddress;
    bool controllerIsPrimary = true;
    bool seenSecondaryController = false;
    bool controllerLoggedIn = false;
    unsigned long lastFrameReceived;

    byte updateFields;
    FujiFrame updateState;
    FujiFrame currentState;

    //FujiFrame decodeFrame(byte readbuf[8]);
    //void encodeFrame(FujiFrame ff);

    //bool pendingFrame = false;


   public:
    void connect(HardwareSerial *serial, bool secondary);
    void connect(HardwareSerial *serial, bool secondary, int rxPin, int txPin, int txEnablePin);


    //these are 'do-it-all' members which run the protocol handler
    bool waitForFrame();
    //void sendPendingFrame();
    bool isBound();
    bool updatePending();

    void setOnOff(bool o);
    void setTemp(byte t);
    void setMode(byte m);
    void setFanMode(byte fm);
    void setEconomyMode(byte em);
    void setSwingMode(byte sm);
    void setSwingStep(byte ss);
    void setState(FujiFrame * state);

    bool getOnOff();
    byte getTemp();
    byte getMode();
    byte getFanMode();
    byte getEconomyMode();
    byte getSwingMode();
    byte getSwingStep();
    byte getControllerTemp();

    FujiFrame *getCurrentState();
    FujiFrame *getUpdateState();

    byte getUpdateFields();

};
*/
