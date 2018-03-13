//
//  GSMService.h
//  Greenhouse
//
//  Created by Bugslayer on 10.11.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#ifndef __Greenhouse__GSMService__
#define __Greenhouse__GSMService__

#include <SoftwareSerial.h>
#include <GUtil.h>

//#define __GSM_DEBUG_OUTPUT

#ifdef __GSM_DEBUG_OUTPUT
#define gsmDebugOutput(_val_) Serial.print((_val_))
#define gsmDebugOutputln(_val_) Serial.println((_val_))
#else
#define gsmDebugOutput(_val_)
#define gsmDebugOutputln(_val_)
#endif

typedef enum {
    atOk = 0,
    atError = 1,
    atTimeout = 2
} atResult;

typedef enum {
    gsResetting = 0x00000001,
    gsModuleStarting = 0x00000004,
    gsModuleReady = 0x00000008,
    gsCsq = 0x00000010,
    gsCreg = 0x00000020,
    gsNetworkOk = 0x00000030,
    gsClip = 0x00000040,
    gsCsms = 0x00000080,
    gsCnmi = 0x00000100,
    gsCmgf = 0x00000200,
    gsCscs = 0x00000400,
    gsInitParamsOk = 0x000007C0,
    gsAll = 0x000007FD
} GsmState;

class GSMService
{
    String output;
    SoftwareSerial serial;
    uint8_t resetPin = 0;
    uint32_t nFlags = 0;
    uint32_t nResetStart = 0;
    String master = "+79125958230";
    uint8_t diagnoseCount = 0;
    uint8_t initParamsCount = 0;
    uint8_t askSmsPacketLostCount = 0;
    uint8_t askBalanceCount = 0;
    uint16_t nSmsPacketLost = 0;
    uint32_t nAskSmsPacketLost = 0;
    uint16_t nSentSmsCount = 0;
    float fBalance = 0.0f;
    uint32_t nAskBalance = 0;
    
    bool getResetting();
    void setResetting(bool value);
    bool getInitializedSerial();
    void setInitializedSerial(bool value);
    bool getStartupProcessing();                // MODEM:STARTUP
    void setStartupProcessing(bool value);
    bool getStartupReady();                     // +PBREADY
    void setStartupReady(bool value);
    bool getSignalQualityOk();                  // AT+CSQ
    void setSignalQualityOk(bool value);
    bool getGsmRegistrationOk();                // AT+CREG?
    void setGsmRegistrationOk(bool value);
    bool getNetworkOk();                        // CSQ & CREG is OK
    bool getCallIdentification();               // AT+CLIP
    void setCallIdentification(bool value);
    bool getChoseSmsService();                  // AT+CSMS
    void setChoseSmsService(bool value);
    bool getSmsIndication();                    // AT+CNMI
    void setSmsIndication(bool value);
    bool getSmsMode();                          // AT+CMGF
    void setSmsMode(bool value);
    bool getCharacterSet();                     // AT+CSCS
    void setCharacterSet(bool value);
    bool getInitParams();                       // CLIP + CSMS + CNMI + CMGF + CSCS
    bool getAskSmsPacketLost();
    void setAskSmsPacketLost(bool value);
    bool getNeedAskSmsPacketLost();
    void setNeedAskSmsPacketLost();
    void askSmsPackedLost();
    bool getAskBalance();
    void setAskBalance(bool value);
    bool getNeedAskBalance();
    void setNeedAskBalance();
    void askBalance();
    void initParams();
    void initSerial();
    void outputPrintln(String txt);
    bool watingFor(String response, uint32_t timeout = 1000);
    void clearSerial();
    atResult executeATCommandInt(String cmd, String& result, uint32_t timeout);
    atResult executeATCommand(String cmd, String& result, uint32_t timeout = 1000, int trycnt = 1);
    void parseResponseResult(String cmd, String& response, String& result);
    void diagnose();
    void parseBalanceResponse(String& response, int pos);
    
public:
    GSMService(uint8_t rx, uint8_t tx);
    void init(uint8_t resetPin);
    void loop();
    void reset();
    void sendsms(String text);
    void runatcmd(String cmd);
    void runcmd(String text);
    String getOutput();
    int getSmsPacketLost();
    float getBalance();
    uint32_t getState();
};

#endif /* defined(__Greenhouse__GSMService__) */
