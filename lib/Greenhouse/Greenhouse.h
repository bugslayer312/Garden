//
//  Greenhouse.h
//  Greenhouse
//
//  Created by Bugslayer on 23.08.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#ifndef __Greenhouse__Greenhouse__
#define __Greenhouse__Greenhouse__

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <Time.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <GUtil.h>
#include <GSMService.h>

#define DEVICE_SAVE_BEGIN_MARKER 10

struct TimeOfDay
{
    uint8_t Hour;
    uint8_t Minute;
    TimeOfDay();
    TimeOfDay(uint8_t hour, uint8_t minute);
    TimeOfDay(String& str);
    int compareTo(const TimeOfDay& tm);
    void decrease(uint16_t minutes);
    String toString();
};

typedef struct
{
    tmElements_t Parsed;
    time_t NotParsed;
} GTime, *GTimePtr;

typedef struct GDate
{
    uint8_t day;
    uint8_t month;
    uint8_t year;
    
    GDate();
    GDate(uint8_t day, uint8_t month, uint8_t year);
    GDate(String& dateAsStr);
    static uint8_t daysPerMonth(uint8_t month, bool leapYear);
    static int32_t daysDelta(GDate& date1, GDate& date2);
    uint16_t daysFromYearBegin();
    String toString();
    void incDay();
    void decDay();
    bool isLeapYear();
};

bool operator==(const GDate& op1, const GDate& op2);
bool operator!=(const GDate& op1, const GDate& op2);
bool operator<(const GDate& op1, const GDate& op2);

typedef enum {
    dmOff,
    dmAuto,
    dmOn
} GDeviceMode;

String devMode2String(GDeviceMode value);
GDeviceMode devModeFromString(String& value);
String tmElements2String(tmElements_t val);

struct RunSleepSettings
{
    uint16_t RunItv;
    uint16_t SleepItv;
    RunSleepSettings();
    RunSleepSettings(uint16_t runItv, uint16_t sleepItv);
    RunSleepSettings(String& str);
    String toString();
};

typedef enum{
    dtUnknown,
    dtLed,
    dtDnat,
    dtTubeFan,
    dtBlowFan,
    dtEmergencyLamp,
    dtWaterPump,
    dtHumidifier
} GDeviceType;

class GScheduleDevice{
protected:
    GDeviceType nType;
    uint32_t dwFlags;  // для базового класса первый байт, для потомков записывать дальше
    TimeOfDay tmStartTime;  // расписание вкл
    TimeOfDay tmEndTime;    // расписание выкл
    GTimePtr ptmSysTime;    // ссылка на время из бокса
    RunSleepSettings runSleepItv;   // интервал работы и отдыха
    TimeOfDay tmRunSleepStartTime;  // время начала отсчета интервала работы
    bool isWithinTimeInterval();    // текущее время попадает в интервал расписания
    int getMinutesPassedFromStart(const TimeOfDay& start);    // кол-во минут, прошедших от start
    // текущее время попадает в первый временной отрезок из двух чередующихся
    bool matchFirstInterval(const TimeOfDay& start, int firstItv, int secondItv);
    bool getScheduled();
    void setScheduled(bool value);
    void setDeviceId(uint8_t value);    // 0..127
    uint16_t nMemAddress;
    void saveFlagsToBuffer(uint8_t* buff);
    void saveFlags();
    void saveScheduleToBuffer(uint8_t* buff);
    void saveSchedule();
    void saveRunSleepSettinsToBuffer(uint8_t* buff);
    void saveRunSleepSettins();
    
    time_t tmDelayEndTime;
    void setIsDelayShutdown(bool value);
    void setIsDelayRun(bool value);
    void checkDelay();
    void init(GTimePtr sysTime, uint8_t id);
    
    void setRunSleepPreset(int num, bool toSleepItv);
    
public:
    GScheduleDevice(unsigned int memAddress);
    GDeviceType getType();
    bool getConnected();
    void setConnected(bool value);
    void setSchedule(TimeOfDay tmStartTime, TimeOfDay tmEndTime);
    TimeOfDay getScheduleStart();
    TimeOfDay getScheduleEnd();
    RunSleepSettings getRunSleepItv();
    void setRunSleepItv(RunSleepSettings value);
    void setMode(GDeviceMode value);
    GDeviceMode getMode();
    uint8_t getDeviceId();
    virtual bool readSettings();
    virtual void writeSettings();
    virtual bool isRunning();
    // прекращение работы на n секунд
    void delayShutdown(uint16_t delay);
    bool getIsDelayShutdown();
    // включение на n секунд
    void delayRun(uint16_t delay);
    bool getIsDelayRun();
    
    bool canIncreaseDutyRatio();
    bool canDecreaseDutyRatio();
    void increaseDutyRatio();
    void decreaseDutyRatio();
    void resetRunSleepStartTime(bool toSleepItv);
    float getDutyRatio();
    int calcStartInSec();
    
    void debugInfo();
};

typedef enum {
    lmeFullPower,
    lmeHalfPower
} LedModeEx;

class GLedLamp : public GScheduleDevice
{
private:
    uint8_t pin1;
    uint8_t pin2;
    time_t tmDelayHalfModeEndTime;
    void setIsOnFirst(bool value);
    bool getIsOnFirst();
    void setIsOnSecond(bool value);
    bool getIsOnSecond();
    void setIsDelayShutdownHalf(bool value);
public:
    GLedLamp(uint16_t memAddress);
    void init(uint8_t pin1, uint8_t pin2, GTimePtr sysTime, uint8_t id);
    void run(bool bStealth);
    void setModeEx(LedModeEx value);
    LedModeEx getModeEx();
    virtual bool isRunning();
    void reset();
    void delayShutdownHalf(uint16_t delay);
    bool getIsDelayShutdownHalf();
};

#define DNAT_COOLING_TIME 300

class GDnatLamp : public GScheduleDevice
{
private:
    uint8_t powerPin;
    uint8_t coolPin;
    
    time_t tmStartCoolingTime;
    
    void setPowerOn(bool value);
    bool getPowerOn();
    void setCoolOn(bool value);
    bool getCoolOn();
public:
    GDnatLamp(uint16_t memAddress);
    void init(uint8_t powerPin, uint8_t coolPin, GTimePtr sysTime, uint8_t id);
    void run(bool bStealth);
    virtual bool isRunning();
    void reset();
    void startCooling();
    void stopCooling();
    int getLostCoolingTime();
};

class GEmergencyLamp : public GScheduleDevice
{
private:
    uint8_t powerPin;
    time_t startTime;
    void setIsOn(bool value);
public:
    GEmergencyLamp(uint16_t memAddress);
    void init(uint8_t powerPin, GTimePtr sysTime, uint8_t id);
    virtual bool isRunning();
    void reset();
    void run(bool emergencyOn);
};

typedef enum {
    tfmeLowPower,
    tfmeHighPower
} TubeFanModeEx;

class GTubeFan : public GScheduleDevice
{
private:
    uint8_t powerPin;
    uint8_t speedSwitchPin;
    time_t tmDelayHighPowerEndTime;
    
    void setPowerOn(bool value);
    bool getPowerOn();
    void setIsDelayHighPower(bool value);
    void setHighPower(bool value);
public:
    GTubeFan(uint16_t memAddress);
    void init(uint8_t powerPin, uint8_t speedSwitchPin, GTimePtr sysTime, uint8_t id);
    void run(bool bStealth);
    void setModeEx(TubeFanModeEx value);
    TubeFanModeEx getModeEx();
    void reset();
    virtual bool isRunning();
    bool getHighPower();
    bool getIsDelayHighPower();
    void delayHighPower(uint16_t delay);
};

class GBlowFan : public GScheduleDevice
{
    uint8_t powerPinLeft;
    uint8_t powerPinRight;
    void setIsOn(bool value);
    bool getIsOn();
public:
    GBlowFan(uint16_t memAddress);
    void init(uint8_t powerPinLeft, uint8_t powerPinRight, GTimePtr sysTime, uint8_t id);
    void run(bool bStealth);
    virtual bool isRunning();
    void reset();
};

#define PUMP_FLOWSPEED_MLPERSEC 32

#define WATERPUMP_SAVE_BEGIN_MARKER 91

class GWaterPump : public GScheduleDevice
{
private:
    uint8_t tank1PowerPin;
    uint8_t tank2PowerPin;
    uint16_t tank1PortionMlValue;
    uint16_t tank2PortionMlValue;
    uint16_t tank1PortionMlLost;
    uint16_t tank2PortionMlLost;
    GDate lastWateringDate;
    GDate referenceDate;
    
    time_t lastTime;
    
    void savePortionMlValuesToBuffer(uint8_t* buff);
    void saveWaterLostPortionToBuffer(uint8_t* buff);
    void saveReferenceDateToBuffer(uint8_t* buff);
    void saveLastWateringDateToBuffer(uint8_t* buff);
    
    void setIsOn1(bool value);
    bool getIsOn1();
    void setIsOn2(bool value);
    bool getIsOn2();
    
    bool isOnePin();
    void writeLostPortion();
    void writeLastWateringDate();
public:
    GWaterPump(uint16_t memAddress);
    void init(uint8_t tank1PowerPin, uint8_t tank2PowerPin, GTimePtr sysTime, uint8_t id);
    virtual bool readSettings();
    virtual void writeSettings();
    void run();
    void runSchedule();
    void setTank1PortionMl(uint16_t value);
    void setTank2PortionMl(uint16_t value);
    uint16_t getTank1PortionMl();
    uint16_t getTank2PortionMl();
    void startWatering();
    void startWatering(uint16_t portion1, uint16_t portion2);
    void stopWatering();
    bool isWatering();
    uint16_t getLostWater();
    GDate getReferenceDate();
    void setReferenceDate(GDate& value);
    GDate getNextWateringDate();
};

class GHumidifier : public GScheduleDevice
{
private:
    uint8_t powerPin;
    
    void setIsOn(bool value);
    bool getIsOn();
public:
    GHumidifier(uint16_t memAddress);
    void init(uint8_t powerPin, GTimePtr sysTime, uint8_t id);
    void run(bool bStealth);
    virtual bool isRunning();
};

typedef struct {
    uint8_t LedLampLeft1_1;
    uint8_t LedLampLeft1_2;
    uint8_t LedLampLeft2_1;
    uint8_t LedLampLeft2_2;
    uint8_t LedLampRight1_1;
    uint8_t LedLampRight1_2;
    uint8_t LedLampRight2_1;
    uint8_t LedLampRight2_2;
    uint8_t DnatPower;
    uint8_t DnatCool;
    uint8_t TubeFanPower;
    uint8_t TubeFanSwitch;
    uint8_t BlowFanPowerLeft;
    uint8_t BlowFanPowerRight;
    uint8_t EmergencyLamp;
    uint8_t WaterPumpTank1Pin;
    uint8_t WaterPumpTank2Pin;
    uint8_t Humidifier;
    uint8_t SystemSwitchOff;
    uint8_t SystemSwitchOn;
    uint8_t GreenWarnLed;
    uint8_t YellowWarnLed;
    uint8_t RedWarnLed;
    uint8_t DCAdapterCooler;
    uint8_t SoilMoistureAnalog;
    uint8_t ArduinoReset;
    uint8_t GsmResetPin;
} GreenhouseInitPins;

struct GreenhouseCreatePins{
    uint8_t TempHumiditySensor;
    uint8_t DallasSensors;
    uint8_t GsmRX;
    uint8_t GsmTX;
    
    GreenhouseCreatePins(uint8_t tempHumiditySensor, uint8_t dallasSensors, uint8_t gsmRX, uint8_t gsmTX);
};

class TempSensorInfo
{
private:
    uint8_t nFlag;
    uint8_t nDeviceId;
    DeviceAddress address;
    float temp;
    
public:
    TempSensorInfo();
    TempSensorInfo(const TempSensorInfo& info);
    TempSensorInfo& operator=(TempSensorInfo& rhs);
    
    //void setValid(bool value);
    //bool getValid();
    void setHasTemp(bool value);
    bool getHasTemp();
    void setDeviceId(uint8_t value);
    uint8_t getDeviceId();
    void getAddress(DeviceAddress outVal);
    void setAddress(DeviceAddress value);
    float getTemp();
    void setTemp(float value);
    bool compareAddress(DeviceAddress value);
};

#define TEMPSENSORS_SAVE_BEGIN_MARKER 30
#define TEMPSENSOR_SAVE_END_MARKER 60

#define MAX_SENSORS_COUNT 8

class DallasTemperatureSensors
{
private:
    uint8_t nFlags;
    OneWire oneWire;
    DallasTemperature dallasSensors;
    TempSensorInfo sensorsData[MAX_SENSORS_COUNT];
    uint8_t sensorsCount;
    GTimePtr ptmSysTime;
    GScheduleDevice* devicesToLink[MAX_SENSORS_COUNT];
    uint8_t devicesToLinkCount;
    uint8_t testingDeviceNo;
    time_t startTestingTime;
    time_t maxLinkingTime;
    float tempSnapshot[MAX_SENSORS_COUNT] = {NAN, NAN, NAN, NAN, NAN, NAN, NAN, NAN};
    
    const int coolDownSec = 120;
    const int testRunSec = 240;
    const int testPauseSec = 120;

    void setLinking(bool value);
    void setTesting(bool value);
    bool getTesting();
    
    uint16_t nMemAddress;
    
    void writeLinks();
    void readLinks();

public:
    DallasTemperatureSensors(uint8_t wirePin, uint16_t nMemTAddress);
    void init(GTimePtr sysTime);
    void read();    // читает за раз один сенсор
    void run();
    void linkToDevices(GScheduleDevice** devices, uint8_t count);
    bool isLinking();
    //void printTemp();
    int getSensorsCount();
    TempSensorInfo& getSensorInfo(int idx);
    float getMaxTemp();
    float getMinTemp();
    bool getNeedLink();
    void setNeedLink(bool value);
    float getDeviceTemp(uint8_t id);
    void setSensorDeviceLink(int sensidx, uint8_t dev_id);
    void stopLinking();
};

typedef enum {
    wlmOff,
    wlmBlink,
    wlmOn
} GWarnLedMode;

#define DOT_TICK_COUNT 1
#define DASH_TICK_COUNT 3
#define PAUSE_TICK_COUNT 2
#define END_TICK_COUNT 16

class GWarnLed{
private:
    uint16_t dwFlags;
    uint8_t pin;
    uint8_t counter;
public:
    GWarnLed();
    void init(uint8_t pin);
    void switchOn();
    void switchOff();
    void setBlink(uint8_t dashCnt, uint8_t dotCnt);
    void check();
    GWarnLedMode getMode();
    int getDashCount();
    int getDotCount();
private:
    bool getLedOn();
    void setLedOn(bool value);
    void setMode(GWarnLedMode value);
    void setDashCount(int value);
    void setDotCount(int value);
    bool checkBlinkOn();
    void checkOn();
};

#define GREENHOUSE_SAVE_START_MARKER 81
#define GREENHOUSE_CURRENT_PROGRAM 1

typedef enum GStatType{
    stDevTempMax,
    stDevTempMin,
    stSysTempMax,
    stSysTempMin,
    stSysHumMax,
    stSysHumMin
};

#define TOTAL_DEVICES_COUNT 10

class Greenhouse : public GreenhouseControl{
private:
    GSMService gsm;
    
    unsigned long dwFlags;
    GTime tmCurrent;
    GTime tmStart;
    time_t tmTimer1LastTick;
    time_t tmTimer2LastTick;
    time_t tmTimer3LastTick;
    time_t tmTimer4LastTick;
    time_t tmTimer5MinPeriodStart;
    time_t tmTimerShutdown;
    time_t tmTimerPowerOn;
    uint8_t programNum;
    
    // devices
    GLedLamp ledLampLeft1;
    GLedLamp ledLampLeft2;
    GLedLamp ledLampRight1;
    GLedLamp ledLampRight2;
    GDnatLamp dnatLamp;
    GTubeFan tubeFan;
    GBlowFan blowFan;
    GEmergencyLamp emergencyLamp;
    GWaterPump waterPump;
    GHumidifier humidifier;
    
    GScheduleDevice* allDevices[TOTAL_DEVICES_COUNT] = {
        &ledLampLeft1,
        &ledLampLeft2,
        &ledLampRight1,
        &ledLampRight2,
        &dnatLamp,
        &tubeFan,
        &blowFan,
        &emergencyLamp,
        &waterPump,
        &humidifier
    };
    
    // warn leds
    GWarnLed greenWarnLed;
    GWarnLed yellowWarnLed;
    GWarnLed redWarnLed;
    
    // sensors
    DHT dht;

    DallasTemperatureSensors tempSensors;
    
    uint8_t shutDownPin;
    uint8_t systemStartPin;
    uint8_t dcAdapterCoolerPin;
    uint8_t soilMoistureAnalogPin;
    uint8_t arduinoResetPin;
    
    // sensors data
    float sysHumidity;
    float sysTemperature;
    uint16_t soilMoisture;
    
    // temp for warning and soft cooling
    float sysWarnTemp = 30.0f;
    float devicesWarnTemp = 45.0f;
    // temp for hard fast cooling
    float devicesExtremeTemp = 48.0f;
    // temp for shutdown devices
    float devicesOverheatTemp = 50.0f;
    // for shutdown immediately
    float systemShutdownTemp = 35.0f;
    float devicesShutdownTemp = 52.0f;
    
    float sysHumidityExtremeTop = 75.0f;
    
    uint8_t sysWarnTempDelta;
    uint8_t devicesWarnTempDelta;
    
    uint16_t nAutoSwitchOnItv = 30;
    TimeOfDay tmAutoResetTime;
    
    void updateWarnLeds();
    void updateEmergencyIsOn();
    
    uint8_t getConnectedLamps(GScheduleDevice** devices);
    
    bool getIsRunnig();
    void setIsRunning(bool value);
    bool getIsShutdowning();
    void setIsShutdowning(bool value);
    bool getIsSwitchingOn();
    void setIsSwitchingOn(bool value);
    
    void checkHumidity();
    void checkLedTemp(GLedLamp* ledLamp);
    void checkSystemTemp();
    
    bool getIsAdapterCoolerOn();
    void setIsAdapterCoolerOn(bool value);
    void checkDCAdapterCooler();
    
public:
    void systemShutdown();
    void systemPowerOn();
    void doArduinoReset();
    
    static void getFloatAsMemBuff(float num, uint8_t* buff);
    static float readFloatFromMemBuff(uint8_t* buff);
    
    Greenhouse(GreenhouseCreatePins& createPins);
    void init(GreenhouseInitPins &controlPins);
    void loop();
    void loopWarnLeds();
    void loopGSM();
    virtual String executeCommand(int target, String command);
    bool readFromMem();
    void writeToMem();
    bool readProgramNumFromMem(uint8_t* outval);
    //void ledStartOn();
private:
    //bool readSelfSettings();
    void readTime();
    void setupStartTime();
    void setClockIsOk(bool val);
    bool getClockIsOk();
    void setClockStartIsOk(bool val);
    bool getClockStartIsOk();
    void setTime(tmElements_t &tm);
    void setTime(TimeOfDay &tm);
    void setupProgram01();
    void setupProgram02();
    void setProgram(uint8_t programNum);
public:
    bool readSelfSettings();
    void writeSelfSettings();
    float getSystemTemp();
    void getDevicesTemp(float* temp, String* devNames, int* devcount);
    float getDeviceTemp(int devid);
    float getHumidity();
    uint16_t getSoilMoisture();
    DallasTemperatureSensors& getTempSensors();
    int getDevicesCount();
    GScheduleDevice* getDevice(int id);
    String getDeviceName(int id);
    static String devTypeToString(GDeviceType val);
    tmElements_t getStartTime();
    tmElements_t getCurrentTime();
    void setCurrentTime(tmElements_t& value);
    void getConnectedDevices(GScheduleDevice** devices, int* count);
    bool getStealthModeOn();
    void setStealthModeOn(bool value);
    
    float getDevWarnTemp();
    void setDevWarnTemp(float value);
    float getDevExtremeTemp();
    void setDevExtremeTemp(float value);
    float getDevOverheatTemp();
    void setDevOverheatTemp(float value);
    float getDevShutdownTemp();
    void setDevShutdownTemp(float value);
    
    float getSysWarnTemp();
    void setSysWarnTemp(float value);
    float getSysShutdownTemp();
    void setSysShutdownTemp(float value);
    float getSysHumidityExtremeTop();
    void setSysHumidityExtremeTop(float value);
    
    bool getAutoSwitchOn();
    void setAutoSwitchOn(bool value);
    uint16_t getAutoSwitchOnItv();
    void setAutoSwitchOnItv(uint16_t val);
    bool getAutoReset();
    void setAutoReset(bool value);
    TimeOfDay getAutoResetTime();
    void setAutoResetTime(TimeOfDay value);
    
    void linkTempSensors();
    void executeCommand(String command, String* params, int paramCount);
};

#endif /* defined(__Greenhouse__Greenhouse__) */
