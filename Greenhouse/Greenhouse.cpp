//
//  Greenhouse.cpp
//  Greenhouse
//
//  Created by Bugslayer on 23.08.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#include "Greenhouse.h"
#include <DS13007RTC24C32.h>

String devMode2String(GDeviceMode value)
{
    switch (value) {
        case dmOff:
            return "off";
        case dmOn:
            return "on";
        default:
            return "auto";
    }
}

GDeviceMode devModeFromString(String& value)
{
    if(value == "off")
        return dmOff;
    if (value == "on")
        return dmOn;
    return dmAuto;
}

String tmElements2String(tmElements_t val)
{
    return String(tmYearToCalendar(val.Year)) + (val.Month < 10 ? "/0" : "/") + String(val.Month) +
        (val.Day < 10 ? "/0" : "/") + String(val.Day) + (val.Hour < 10 ? " 0" : " ") + String(val.Hour) +
        (val.Minute < 10 ? ":0" : ":") + String(val.Minute) + (val.Second < 10 ? ":0" : ":") + String(val.Second);
}


// for test

void print2digits(int number) {
    if (number >= 0 && number < 10) {
        Serial.print('0');
    }
    Serial.print(number);
}

void printTime(const tmElements_t& tm)
{
    Serial.print('[');
    print2digits(tm.Hour);
    Serial.print(':');
    print2digits(tm.Minute);
    Serial.print(':');
    print2digits(tm.Second);
    Serial.print(']');
}

//#define __ENABLE_DEBUG_OUTPUT

#ifdef __ENABLE_DEBUG_OUTPUT
#define debugOutput(_val_) Serial.print((_val_))
#define debugOutputts(_val_, _ts_) printTime((_ts_)); Serial.print((_val_))
#define debugOutputln(_val_) Serial.println((_val_))
#define debugOutputlnts(_val_, _ts_) printTime((_ts_)); Serial.println((_val_))
#else
#define debugOutput(_val_)
#define debugOutputts(_val_, _ts_)
#define debugOutputln(_val_)
#define debugOutputlnts(_val_, _ts_)
#endif

// TimeOfDay

TimeOfDay::TimeOfDay()
{
    
}

TimeOfDay::TimeOfDay(uint8_t hour, uint8_t minute)
{
    this->Hour = hour;
    this->Minute = minute;
}

TimeOfDay::TimeOfDay(String& str)
{
    int pos = str.indexOf(":");
    if(pos >= 0)
    {
        Hour = str.substring(0, pos).toInt();
        Minute = str.substring(pos+1, str.length()).toInt();
    }
    else{
        Hour = 0;
        Minute = 0;
    }
}

int TimeOfDay::compareTo(const TimeOfDay& tm)
{
    if(Hour > tm.Hour)
        return 1;
    if(Hour < tm.Hour)
        return -1;
    if(Minute > tm.Minute)
        return 1;
    if(Minute < tm.Minute)
        return -1;
    return 0;
}

void TimeOfDay::decrease(uint16_t minutes)
{
    uint16_t h = minutes / 60;
    if(h >= 24)
        h = h % 24;
    uint16_t m = minutes % 60;
    if(Minute >= m)
    {
        Minute -= m;
    }
    else
    {
        if(Hour == 0)
            Hour = 23;
        else
            Hour -= 1;
        Minute = 60 - (m - Minute);
    }
    if(Hour >= h)
    {
        Hour -= h;
    }
    else
    {
        Hour = 24 - (h - Hour);
    }
}

String TimeOfDay::toString()
{
    return (Hour < 10 ? "0" : "") + String(Hour) + ":" + (Minute < 10 ? "0" : "") + String(Minute);
}

// GDate

bool operator==(const GDate& op1, const GDate& op2)
{
    return op1.day == op2.day && op1.month == op2.month && op1.year == op2.year;
}

bool operator!=(const GDate& op1, const GDate& op2)
{
    return !(op1 == op2);
}

bool operator<(const GDate& op1, const GDate& op2)
{
    return (op1.year < op2.year) ||
            op1.year == op2.year && (op1.month < op2.month ||
                                     op1.month == op2.month && op1.day < op2.day);
}

GDate::GDate(uint8_t day, uint8_t month, uint8_t year)
{
    this->day = day;
    this->month = month;
    this->year = year;
}

GDate::GDate()
{
    this->day = 0;
    this->month = 0;
    this->year = 45; //2015 - 1970;
}

GDate::GDate(String& dateAsStr)
{
    int pos = dateAsStr.indexOf("-");
    if(pos > 0)
    {
        year = dateAsStr.substring(0, pos).toInt() - 1970;
        int pos1 = dateAsStr.indexOf("-", pos+1);
        if(pos1 > pos+1)
        {
            month = dateAsStr.substring(pos+1, pos1).toInt();
            if(pos1 < dateAsStr.length() - 1)
            {
                day = dateAsStr.substring(pos1+1, dateAsStr.length()).toInt();
            }
        }
    }
}

bool GDate::isLeapYear()
{
    return (((1970 + year) % 4) == 0);
}

uint16_t GDate::daysFromYearBegin()
{
    bool leapYear = isLeapYear();
    uint16_t res = 0;
    for(int i=0;i < month;i++)
    {
        res += daysPerMonth(i, leapYear);
    }
    return res + day;
}

uint8_t GDate::daysPerMonth(uint8_t month, bool leapYear)
{
    switch (month) {
        case 1:
            return (leapYear ? 29 : 28);
        case 0:
        case 2:
        case 4:
        case 6:
        case 7:
        case 9:
        case 11:
            return 31;
        default:
            return 30;
    }
}

int32_t GDate::daysDelta(GDate& date1, GDate& date2)
{
    if(date2 < date1)
        return -daysDelta(date2, date1);
    int32_t res = 0;
    uint8_t y = date2.year;
    for(int i=date2.year-1;i >= date1.year;i--)
    {
        res += 365;
        if(((1970 + i) % 4) == 0)
            res += 1;
    }
    res += (date2.daysFromYearBegin() - date1.daysFromYearBegin());
    return res;
}

String GDate::toString()
{
    return String(year + 1970) + "-" + (month < 10 ? ("0" + String(month)) : month) + "-" +
        (day < 10 ? ("0" + String(day)) : day);
}

void GDate::incDay()
{
    day++;
    if(day >= daysPerMonth(month, isLeapYear()))
    {
        day = 0;
        month++;
        if(month >= 12)
        {
            month = 0;
            year++;
        }
    }
}

void GDate::decDay()
{
    if(day > 0)
    {
        day--;
    }
    else
    {
        if(month > 0)
        {
            month--;
        }
        else
        {
            month = 11;
            year--;
        }
        day = daysPerMonth(month, isLeapYear()) - 1;
    }
}

// GreenhouseCreatePins

GreenhouseCreatePins::GreenhouseCreatePins(uint8_t tempHumiditySensor, uint8_t dallasSensors, uint8_t gsmRX, uint8_t gsmTX)
{
    TempHumiditySensor = tempHumiditySensor;
    DallasSensors = dallasSensors;
    GsmRX = gsmRX;
    GsmTX = gsmTX;
}

// TempSensorInfo

TempSensorInfo::TempSensorInfo()
{
    nFlag = 0;
    nDeviceId = 0;
    temp = NAN;
}

TempSensorInfo::TempSensorInfo(const TempSensorInfo& info)
{
    this->nFlag = info.nFlag;
    this->temp = info.temp;
    for(int i = 0;i < 8;i++)
        this->address[i] = info.address[i];
}

TempSensorInfo& TempSensorInfo::operator=(TempSensorInfo& rhs)
{
    if(this != &rhs)
    {
        this->nFlag = rhs.nFlag;
        this->temp = rhs.temp;
        for(int i = 0;i < 8;i++)
            this->address[i] = rhs.address[i];
    }
    return *this;
}

void TempSensorInfo::setHasTemp(bool value)
{
    if(value)
        nFlag |= 0x01;
    else
        nFlag &= 0xFE;
}

bool TempSensorInfo::getHasTemp()
{
    return (nFlag & 0x01) > 0;
}

//void TempSensorInfo::setValid(bool value)
//{
//    if(value)
//        nFlag |= 0x01;
//    else
//        nFlag &= 0xFE;
//}

//bool TempSensorInfo::getValid()
//{
//    return (nFlag & 0x01) > 0;
//}

void TempSensorInfo::setDeviceId(uint8_t value)
{
    nDeviceId = value;
}

uint8_t TempSensorInfo::getDeviceId()
{
    return nDeviceId;
}

void TempSensorInfo::getAddress(DeviceAddress outVal)
{
    for(int i=0;i < 8;i++)
        outVal[i] = address[i];
}

void TempSensorInfo::setAddress(DeviceAddress value)
{
    for(int i = 0;i < 8;i++)
        address[i] = value[i];
}

float TempSensorInfo::getTemp()
{
    return temp;
}

void TempSensorInfo::setTemp(float value)
{
    temp = value;
}

bool TempSensorInfo::compareAddress(DeviceAddress value)
{
    for(int i = 0;i < 8;i++)
    {
        if(address[i] != value[i])
            return false;
    }
    return true;
}

// DallasTemperatureSensors

DallasTemperatureSensors::DallasTemperatureSensors(uint8_t wirePin, uint16_t nMemAddress):
    oneWire(wirePin)
    , dallasSensors(&oneWire)
{
    this->nMemAddress = nMemAddress;
    nFlags = 0;
    sensorsCount = 0;
    devicesToLinkCount = 0;
    testingDeviceNo = 0;
    setNeedLink(true);
}

void DallasTemperatureSensors::init(GTimePtr sysTime)
{
    ptmSysTime = sysTime;
    dallasSensors.begin();
    int allSensorsCount = dallasSensors.getDeviceCount();
    if(allSensorsCount > 8)
        allSensorsCount = 8;
    oneWire.reset_search();
    DeviceAddress tmpAddress;
    sensorsCount = 0;
    for(int i=0;i < allSensorsCount; i++)
    {
        if(oneWire.search(tmpAddress))
        {
            TempSensorInfo& curSensorInfo = sensorsData[sensorsCount++];
            curSensorInfo.setAddress(tmpAddress);
            dallasSensors.setResolution(tmpAddress, 12);
        }
    }
    readLinks();
}

void DallasTemperatureSensors::read()
{
    dallasSensors.requestTemperatures();
    DeviceAddress tmpAddress;
    for(uint8_t i = 0;i < sensorsCount;i++)
    {
        TempSensorInfo& curSensorInfo = sensorsData[i];
        curSensorInfo.getAddress(tmpAddress);
        float tmp = dallasSensors.getTempC(tmpAddress);
        if(!isnan(tmp) && tmp > -100.0f)
        {
            curSensorInfo.setTemp(tmp);
            curSensorInfo.setHasTemp(true);
        }
    }
}

//void DallasTemperatureSensors::printTemp()
//{
//    debugOutputts("Devices temp ", ptmSysTime->Parsed);
//    for(uint8_t i=0;i<sensorsCount;i++)
//    {
//        debugOutput(i);
//        debugOutput(": ");
//        float tmp = sensorsData[i].getTemp();
//        if(isnan(tmp))
//            debugOutput("NAN");
//        else
//            debugOutput(tmp);
//        debugOutput(" *C; ");
//    }
//    debugOutputln("");
//}

void DallasTemperatureSensors::run()
{
    if(isLinking())
    {
        if(ptmSysTime->NotParsed > maxLinkingTime)
        {
            // защита от непредвиденных ситуаций
            setNeedLink(false);
            stopLinking();
        }
        if(getTesting())
        {
            if(ptmSysTime->NotParsed >= startTestingTime + testRunSec)
            {
                // finish test device
                debugOutputts("Finish testing device", ptmSysTime->Parsed);
                debugOutput(testingDeviceNo);
                debugOutput(", id = ");
                debugOutputln(devicesToLink[testingDeviceNo]->getDeviceId());
                GScheduleDevice* device = devicesToLink[testingDeviceNo];
                debugOutputlnts("Switch off device.", ptmSysTime->Parsed);
                device->setMode(dmOff);
                int maxTmpIncFreeSensor = -1;
                float tmpInc = 0.0f;
                debugOutputts("Get sensors end data: ", ptmSysTime->Parsed);
                for(uint8_t i = 0;i < sensorsCount;i++)
                {
                    TempSensorInfo& sensInfo = sensorsData[i];
                    bool skip = sensInfo.getDeviceId() > 0 || !sensInfo.getHasTemp();
                    debugOutput(i);
                    debugOutput(") ");
                    debugOutput(sensInfo.getTemp());
                    debugOutput("(dev=");
                    debugOutput(sensInfo.getDeviceId());
                    debugOutput(") ");
                    if(skip)
                        continue;
                    if(!isnan(tempSnapshot[i])) // просто паранойя
                    {
                        float tinc = sensInfo.getTemp() - tempSnapshot[i];
                        if(tinc > tmpInc)
                        {
                            tmpInc = tinc;
                            maxTmpIncFreeSensor = i;
                        }
                    }
                }
                debugOutputln("");
                if(maxTmpIncFreeSensor >= 0)
                {
                    TempSensorInfo& sensInfo = sensorsData[maxTmpIncFreeSensor];
                    sensInfo.setDeviceId(device->getDeviceId());
                    debugOutputts("Link success: sensor = ", ptmSysTime->Parsed);
                    debugOutput(maxTmpIncFreeSensor);
                    debugOutput(", device id = ");
                    debugOutputln(sensInfo.getDeviceId());
                }
                setTesting(false);
                testingDeviceNo++;
                if(testingDeviceNo < devicesToLinkCount)
                {
                    // make pause between testing
                    debugOutputts("Wait ", ptmSysTime->Parsed);
                    debugOutput(testPauseSec);
                    debugOutputln(" sec for next device test...");
                    startTestingTime = ptmSysTime->NotParsed + testPauseSec;
                }
            }
        }
        else
        {
            if(ptmSysTime->NotParsed >= startTestingTime && testingDeviceNo < devicesToLinkCount)
            {
                debugOutputts("Start testing device ", ptmSysTime->Parsed);
                debugOutput(testingDeviceNo);
                debugOutput(", id = ");
                debugOutputln(devicesToLink[testingDeviceNo]->getDeviceId());
                startTestingTime = ptmSysTime->NotParsed;
                // begin test device
                debugOutputts("Get sensors begin data: ", ptmSysTime->Parsed);
                for(uint8_t i=0;i < sensorsCount;i++)
                {
                    TempSensorInfo& sensInfo = sensorsData[i];
                    debugOutput(i);
                    debugOutput(") ");
                    tempSnapshot[i] = sensInfo.getTemp();
                    debugOutput(tempSnapshot[i]);
                    debugOutput("(dev=");
                    debugOutput(sensInfo.getDeviceId());
                    debugOutput(") ");
                    
                }
                debugOutputln("");
                debugOutputts("Switch on device and wait for ", ptmSysTime->Parsed);
                debugOutput(testRunSec);
                debugOutputln(" sec...");
                for(uint8_t i = 0;i < devicesToLinkCount;i++)
                {
                    devicesToLink[i]->setMode(i == testingDeviceNo ? dmOn : dmOff);
                }
                setTesting(true);
            }
        }
        if(testingDeviceNo >= devicesToLinkCount)
        {
            // finish link
            writeLinks();
            setNeedLink(false);
            stopLinking();
        }
    }
}

void DallasTemperatureSensors::linkToDevices(GScheduleDevice** devices, uint8_t count)
{
    if(isLinking())
    {
        stopLinking();
    }
    debugOutputts("Start linking devices. Devices count = ", ptmSysTime->Parsed);
    debugOutput(count);
    debugOutput(". Sensors count = ");
    debugOutput(sensorsCount);
    debugOutputln(".");
    setLinking(true);
    debugOutputts("Cooldowning devices (", ptmSysTime->Parsed);
    debugOutput(coolDownSec);
    debugOutputln(" sec)...");
    for(uint8_t i = 0;i < count;i++)
    {
        devicesToLink[i] = devices[i];
        devicesToLink[i]->setMode(dmOff);
    }
    for(uint8_t i = 0;i < sensorsCount;i++)
    {
        sensorsData[i].setDeviceId(0);
    }
    devicesToLinkCount = count;
    startTestingTime = ptmSysTime->NotParsed + coolDownSec;
    maxLinkingTime = startTestingTime + (sensorsCount+1)*(testRunSec + testPauseSec);
    testingDeviceNo = 0;
}

void DallasTemperatureSensors::stopLinking()
{
    setLinking(false);
    setTesting(false);
    debugOutputlnts("Return devices to auto mode...", ptmSysTime->Parsed);
    for(uint8_t i = 0;i < devicesToLinkCount;i++)
    {
        devicesToLink[i]->setMode(dmAuto);
    }
    debugOutputts("Linking is finished: ", ptmSysTime->Parsed);
    for(uint8_t i=0;i < sensorsCount;i++)
    {
        TempSensorInfo& sensInfo = sensorsData[i];
        debugOutput(i);
        debugOutput("(dev=");
        debugOutput(sensInfo.getDeviceId());
        debugOutput(") ");
    }
    debugOutputln("");
    devicesToLinkCount = 0;
    testingDeviceNo = 0;
}

void DallasTemperatureSensors::setLinking(bool value)
{
    if(value)
        nFlags |= 0x01;
    else
        nFlags &= 0xFE;
}

bool DallasTemperatureSensors::isLinking()
{
    return (nFlags & 0x01) > 0;
}

void DallasTemperatureSensors::setTesting(bool value)
{
    if(value)
        nFlags |= 0x02;
    else
        nFlags &= 0xFD;
}

bool DallasTemperatureSensors::getTesting()
{
    return (nFlags & 0x02) > 0;
}

void DallasTemperatureSensors::setNeedLink(bool value)
{
    if(value)
        nFlags |= 0x04;
    else
        nFlags &= 0xFB;
}

bool DallasTemperatureSensors::getNeedLink()
{
    return (nFlags & 0x04) > 0;
}

int DallasTemperatureSensors::getSensorsCount()
{
    return sensorsCount;
}

TempSensorInfo& DallasTemperatureSensors::getSensorInfo(int idx)
{
    return sensorsData[idx];
}

float DallasTemperatureSensors::getMaxTemp()
{
    float result = NAN;
    for(int i=0;i < sensorsCount;i++)
    {
        TempSensorInfo& sInfo = sensorsData[i];
        if(sInfo.getHasTemp())
        {
            float tmp = sInfo.getTemp();
            if(isnan(result))
            {
                result = tmp;
            }
            else if(result < tmp)
            {
                result = tmp;
            }
        }
    }
    return result;
}

float DallasTemperatureSensors::getMinTemp()
{
    bool fromAll = true;
    for(int i=0;i < sensorsCount;i++)
    {
        if(sensorsData[i].getDeviceId() > 0)
        {
            fromAll = false;
            break;
        }
    }
    float res = NAN;
    for(int i=0;i < sensorsCount;i++)
    {
        TempSensorInfo& sInfo = getSensorInfo(i);
        if(sInfo.getHasTemp() && (fromAll || sInfo.getDeviceId() > 0))
        {
            float tmp = sInfo.getTemp();
            if(isnan(res))
                res = tmp;
            else if(res > tmp)
                res = tmp;
        }
    }
    return res;
}

void DallasTemperatureSensors::writeLinks()
{
    uint8_t buff[10];
    buff[0] = TEMPSENSORS_SAVE_BEGIN_MARKER;
    buff[1] = (uint8_t)sensorsCount;
    RTC_MEM.eepromWriteBuffer(nMemAddress, buff, 2);
    for(int i=0;i < sensorsCount;i++)
    {
        delay(50);
        TempSensorInfo& sInfo = getSensorInfo(i);
        sInfo.getAddress(buff);
        buff[8] = sInfo.getDeviceId();
        buff[9] = TEMPSENSOR_SAVE_END_MARKER;
        RTC_MEM.eepromWriteBuffer(nMemAddress + (i+1)*16, buff, 10);
    }
}

void DallasTemperatureSensors::readLinks()
{
    uint8_t buff[10];
    uint8_t length = 2;
    if(!RTC_MEM.eepromReadBuffer(nMemAddress, buff, &length) || length < 2)
        return;
    if(buff[0] != TEMPSENSORS_SAVE_BEGIN_MARKER)
        return;
    uint8_t scount = buff[1];
    if(scount != sensorsCount)
        return;
    uint8_t ids[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for(int i=0;i < scount;i++)
    {
        delay(50);
        length = 10;
        if(!RTC_MEM.eepromReadBuffer(nMemAddress + (i+1)*16, buff, &length) || length < 10)
            return;
        if(buff[9] != TEMPSENSOR_SAVE_END_MARKER)
            return;
        TempSensorInfo& sInfo = getSensorInfo(i);
        if(!sInfo.compareAddress(buff))
            return;
        ids[i] = buff[8];
    }
    for(int i=0;i < scount;i++)
    {
        getSensorInfo(i).setDeviceId(ids[i]);
    }
    setNeedLink(false);
}

float DallasTemperatureSensors::getDeviceTemp(uint8_t id)
{
    for(int i=0;i < sensorsCount;i++)
    {
        TempSensorInfo& sInfo = sensorsData[i];
        if(sInfo.getDeviceId() == id)
        {
            return sInfo.getTemp();
        }
    }
    return NAN;
}

void DallasTemperatureSensors::setSensorDeviceLink(int sensidx, uint8_t dev_id)
{
    debugOutput("setSensorDeviceLink:sensidx=");
    debugOutput(sensidx);
    debugOutput(";dev_id=");
    debugOutputln(dev_id);
    for(int i=0;i < MAX_SENSORS_COUNT;i++)
    {
        TempSensorInfo& sens_info = sensorsData[i];
        if(i == sensidx)
        {
            sens_info.setDeviceId(dev_id);
        }
        else
        {
            if(sens_info.getDeviceId() == dev_id)
            {
                sens_info.setDeviceId(0);
            }
        }
    }
    writeLinks();
}

// RunSleepSettings

RunSleepSettings::RunSleepSettings()
{
    RunItv = 0;
    SleepItv = 0;
}

RunSleepSettings::RunSleepSettings(uint16_t runItv, uint16_t sleepItv)
{
    RunItv = runItv;
    SleepItv = sleepItv;
}

RunSleepSettings::RunSleepSettings(String& str)
{
    int pos = str.indexOf(":");
    if(pos > 0 && pos < str.length()-1){
        RunItv = (uint16_t)str.substring(0, pos).toInt();
        SleepItv = (uint16_t)str.substring(pos+1, str.length()).toInt();
    }
    else{
        RunItv = SleepItv = 0;
    }
}

String RunSleepSettings::toString()
{
    return String(RunItv) + ":" + String(SleepItv);
}

#define RUNSLEEP_PRESETS_COUNT 8

RunSleepSettings g_runSleepPresets[RUNSLEEP_PRESETS_COUNT] = { RunSleepSettings(5, 25),
    RunSleepSettings(5, 20), RunSleepSettings(10, 20), RunSleepSettings(10, 15),
    RunSleepSettings(15, 15), RunSleepSettings(15, 10), RunSleepSettings(20, 10), RunSleepSettings(30, 0) };

// GScheduleDevice

GScheduleDevice::GScheduleDevice(uint16_t memAddress)
{
    nType = dtUnknown;
    dwFlags = 0;
    tmStartTime.Hour = 0;
    tmStartTime.Minute = 0;
    tmEndTime.Hour = 0;
    tmEndTime.Minute = 0;
    runSleepItv.RunItv = 0;
    runSleepItv.SleepItv = 0;
    ptmSysTime = NULL;
    nMemAddress = memAddress;
}

GDeviceType GScheduleDevice::getType()
{
    return nType;
}

bool GScheduleDevice::getConnected()
{
    return (dwFlags & 0x00000001) > 0;
}

void GScheduleDevice::setConnected(bool value)
{
    if(value)
        dwFlags |= 0x000000001;
    else
        dwFlags &= 0xFFFFFFFE;
}

bool GScheduleDevice::getScheduled()
{
    return (dwFlags & 0x00000002) > 0;
}

void GScheduleDevice::setScheduled(bool value)
{
    if(value)
        dwFlags |= 0x000000002;
    else
        dwFlags &= 0xFFFFFFFD;
}

void GScheduleDevice::setMode(GDeviceMode value)
{
    dwFlags &= 0xFFFFFFF3;
    dwFlags |= (((uint32_t)value) & 0x00000003) << 2;
}

GDeviceMode GScheduleDevice::getMode()
{
    return (GDeviceMode)((dwFlags & 0x0000000C) >> 2);
}

void GScheduleDevice::setIsDelayShutdown(bool value)
{
    if(value)
        dwFlags |= 0x10000000;
    else
        dwFlags &= 0xEFFFFFFF;
}

bool GScheduleDevice::getIsDelayShutdown()
{
    return (dwFlags & 0x10000000) > 0;
}

void GScheduleDevice::setIsDelayRun(bool value)
{
    if(value)
        dwFlags |= 0x20000000;
    else
        dwFlags &= 0xDFFFFFFF;
}

bool GScheduleDevice::getIsDelayRun()
{
    return (dwFlags & 0x20000000) > 0;
}

void GScheduleDevice::init(GTimePtr sysTime, uint8_t id)
{
    ptmSysTime = sysTime;
    setDeviceId(id);
    tmRunSleepStartTime = TimeOfDay(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute);
}

void GScheduleDevice::setDeviceId(uint8_t value)
{
    dwFlags &= 0xFFFFFF0F;
    dwFlags |= (value & 0x0F) << 4;
}

uint8_t GScheduleDevice::getDeviceId()
{
    return (uint8_t)((dwFlags & 0x000000F0) >> 4);
}

void GScheduleDevice::setSchedule(TimeOfDay tmStartTime, TimeOfDay tmEndTime)
{
    this->tmStartTime = tmStartTime;
    this->tmEndTime = tmEndTime;
    setScheduled(true);
    tmRunSleepStartTime = tmStartTime;
}

TimeOfDay GScheduleDevice::getScheduleStart()
{
    return tmStartTime;
}

TimeOfDay GScheduleDevice::getScheduleEnd()
{
    return tmEndTime;
}

RunSleepSettings GScheduleDevice::getRunSleepItv()
{
    return runSleepItv;
}

void GScheduleDevice::setRunSleepItv(RunSleepSettings value)
{
    runSleepItv = value;
}

bool GScheduleDevice::isWithinTimeInterval()
{
    if(ptmSysTime == NULL)
        return false;
    if(getIsDelayShutdown() && ptmSysTime->NotParsed < tmDelayEndTime)
        return false;
    if(getIsDelayRun() && ptmSysTime->NotParsed < tmDelayEndTime)
        return true;
    if(!getScheduled())
        return false;
    int cmp = tmStartTime.compareTo(tmEndTime);
    if(cmp == 0)
        return false;
    TimeOfDay cur(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute);
    if(cmp < 0)
        return tmStartTime.compareTo(cur) <= 0 && tmEndTime.compareTo(cur) > 0;
    else
        return !(tmEndTime.compareTo(cur) <= 0 && tmStartTime.compareTo(cur) > 0);
}

int GScheduleDevice::getMinutesPassedFromStart(const TimeOfDay& start)
{
    int startMinutes = start.Hour*60 + start.Minute;
    int curMinutes = ptmSysTime->Parsed.Hour*60 + ptmSysTime->Parsed.Minute;
    int result = curMinutes - startMinutes;
    if(result < 0)
        result += 1440; // 24*60
    return result;
}

bool GScheduleDevice::matchFirstInterval(const TimeOfDay& start, int firstItv, int secondItv)
{
    int totalItvLength = firstItv + secondItv;
    if(totalItvLength <= 0)
        return false;
    if(secondItv == 0)
        return true;
    int minPassed = getMinutesPassedFromStart(start);
    return minPassed < ((minPassed/totalItvLength)*totalItvLength + firstItv);
}

void GScheduleDevice::saveFlagsToBuffer(uint8_t* buff)
{
    buff[0] = (uint8_t)(dwFlags & 0x000000FF);
    buff[1] = (uint8_t)((dwFlags & 0x0000FF00) >> 8);
    buff[2] = (uint8_t)((dwFlags & 0x00FF0000) >> 16);
    buff[3] = (uint8_t)((dwFlags & 0x0F000000) >> 24);
}

void GScheduleDevice::saveFlags()
{
    uint8_t buff[4];
    saveFlagsToBuffer(buff);
    RTC_MEM.eepromWriteBuffer(nMemAddress + 4, buff, 4);
}

void GScheduleDevice::saveScheduleToBuffer(uint8_t* buff)
{
    buff[0] = tmStartTime.Hour;
    buff[1] = tmStartTime.Minute;
    buff[2] = tmEndTime.Hour;
    buff[3] = tmEndTime.Minute;
}

void GScheduleDevice::saveSchedule()
{
    uint8_t buff[4];
    saveScheduleToBuffer(buff);
    RTC_MEM.eepromWriteBuffer(nMemAddress + 8, buff, 4);
}

void GScheduleDevice::saveRunSleepSettinsToBuffer(uint8_t* buff)
{
    buff[0] = (uint8_t)(runSleepItv.RunItv & 0x00FF);
    buff[1] = (uint8_t)((runSleepItv.RunItv & 0xFF00) >> 8);
    buff[2] = (uint8_t)(runSleepItv.SleepItv & 0x00FF);
    buff[3] = (uint8_t)((runSleepItv.SleepItv & 0xFF00) >> 8);
}

void GScheduleDevice::saveRunSleepSettins()
{
    uint8_t buff[4];
    saveRunSleepSettinsToBuffer(buff);
    RTC_MEM.eepromWriteBuffer(nMemAddress + 12, buff, 4);
}

bool GScheduleDevice::readSettings()
{
    uint8_t buff[16];
    uint8_t length = 16;
    if(!RTC_MEM.eepromReadBuffer(nMemAddress, buff, &length) || length < 16)
        return false;
    if(buff[0] != DEVICE_SAVE_BEGIN_MARKER)
        return false;
    int readedId = ((buff[4] & 0xF0) >> 4);
    if(readedId != getDeviceId())
        return false;
    dwFlags = buff[4] | (((uint32_t)buff[5]) << 8) | (((uint32_t)buff[6]) << 16) |
        (((uint32_t)buff[7]) << 24);
    tmStartTime.Hour = buff[8];
    tmStartTime.Minute = buff[9];
    tmEndTime.Hour = buff[10];
    tmEndTime.Minute = buff[11];
    runSleepItv.RunItv = buff[12] | (((uint32_t)buff[13]) << 8);
    runSleepItv.SleepItv = buff[14] | (((uint32_t)buff[15]) << 8);
    return true;
}

void GScheduleDevice::writeSettings()
{
    uint8_t buff[16];
    for(int i=0;i < 16;i++)
        buff[i] = 0;
    buff[0] = DEVICE_SAVE_BEGIN_MARKER;
    saveFlagsToBuffer((uint8_t*)(buff+4));
    saveScheduleToBuffer((uint8_t*)(buff+8));
    saveRunSleepSettinsToBuffer((uint8_t*)(buff+12));
    RTC_MEM.eepromWriteBuffer(nMemAddress, buff, 16);
}

bool GScheduleDevice::isRunning()
{
    return false;
}

void GScheduleDevice::debugInfo()
{
    debugOutput("device ");
    debugOutput(getDeviceId());
    debugOutput(", ");
    debugOutput(nMemAddress);
    debugOutput(": flags=");
    debugOutput(dwFlags);
    debugOutput("; start=");
    debugOutput(tmStartTime.Hour);
    debugOutput(":");
    debugOutput(tmStartTime.Minute);
    debugOutput("; end=");
    debugOutput(tmEndTime.Hour);
    debugOutput(":");
    debugOutput(tmEndTime.Minute);
    debugOutput("; run=");
    debugOutput(runSleepItv.RunItv);
    debugOutput("; sleep=");
    debugOutputln(runSleepItv.SleepItv);
}

void GScheduleDevice::delayShutdown(uint16_t delay)
{
    setIsDelayRun(false);
    if(ptmSysTime != NULL)
    {
        tmDelayEndTime = ptmSysTime->NotParsed + delay;
        setIsDelayShutdown(true);
    }
}

void GScheduleDevice::delayRun(uint16_t delay)
{
    debugOutput("GScheduleDevice::delayRun()");
    setIsDelayShutdown(false);
    if(ptmSysTime != NULL)
    {
        tmDelayEndTime = ptmSysTime->NotParsed + delay;
        debugOutput(";setIsDelayRun(true)");
        setIsDelayRun(true);
        debugOutput(";getIsDelayRun()=");
        debugOutput(getIsDelayRun());
    }
    debugOutputln("");
}

void GScheduleDevice::checkDelay()
{
    if(ptmSysTime == NULL || ptmSysTime->NotParsed >= tmDelayEndTime)
    {
        setIsDelayShutdown(false);
        setIsDelayRun(false);
    }
}

void GScheduleDevice::setRunSleepPreset(int num, bool toSleepItv)
{
    if(num >= 0 && num < RUNSLEEP_PRESETS_COUNT)
    {
        RunSleepSettings preset = g_runSleepPresets[num];
        runSleepItv.RunItv = preset.RunItv;
        runSleepItv.SleepItv = preset.SleepItv;
        resetRunSleepStartTime(toSleepItv);
    }
}

bool GScheduleDevice::canIncreaseDutyRatio()
{
    if((runSleepItv.RunItv + runSleepItv.SleepItv) > 0)
    {
        RunSleepSettings maxRatioPreset = g_runSleepPresets[RUNSLEEP_PRESETS_COUNT - 1];
        if(((float)(runSleepItv.RunItv))/(runSleepItv.RunItv + runSleepItv.SleepItv) <
           ((float)maxRatioPreset.RunItv)/(maxRatioPreset.RunItv + maxRatioPreset.SleepItv))
        return true;
    }
    return false;
}

bool GScheduleDevice::canDecreaseDutyRatio()
{
    if((runSleepItv.RunItv + runSleepItv.SleepItv) > 0)
    {
        RunSleepSettings minRatioPreset = g_runSleepPresets[0];
        if(((float)(runSleepItv.RunItv))/(runSleepItv.RunItv + runSleepItv.SleepItv) >
           ((float)minRatioPreset.RunItv)/(minRatioPreset.RunItv + minRatioPreset.SleepItv))
        return true;
    }
    else
    {
        // ratio == 1.0 and we can decrease it
        return true;
    }
    return false;
}

void GScheduleDevice::increaseDutyRatio()
{
    if((runSleepItv.RunItv + runSleepItv.SleepItv) > 0)
    {
        float ratio = ((float)runSleepItv.RunItv)/(runSleepItv.RunItv + runSleepItv.SleepItv);
        for(int i = 0;i < RUNSLEEP_PRESETS_COUNT;i++)
        {
            RunSleepSettings ratioPreset = g_runSleepPresets[i];
            if(ratio < ((float)ratioPreset.RunItv)/(ratioPreset.RunItv + ratioPreset.SleepItv))
            {
                setRunSleepPreset(i, false);
                return;
            }
        }
    }
}

void GScheduleDevice::decreaseDutyRatio()
{
    if((runSleepItv.RunItv + runSleepItv.SleepItv) > 0)
    {
        float ratio = ((float)runSleepItv.RunItv)/(runSleepItv.RunItv + runSleepItv.SleepItv);
        for(int i = RUNSLEEP_PRESETS_COUNT-1;i >= 0;i--)
        {
            RunSleepSettings ratioPreset = g_runSleepPresets[i];
            if(ratio > ((float)ratioPreset.RunItv)/(ratioPreset.RunItv + ratioPreset.SleepItv))
            {
                setRunSleepPreset(i, true);
                return;
            }
        }
    }
}

void GScheduleDevice::resetRunSleepStartTime(bool toSleepItv)
{
    if(ptmSysTime != NULL)
    {
        TimeOfDay tm(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute);
        if(toSleepItv)
        tm.decrease(runSleepItv.RunItv);
        tmRunSleepStartTime = tm;
    }
}

float GScheduleDevice::getDutyRatio()
{
    int sum = runSleepItv.RunItv + runSleepItv.SleepItv;
    return sum == 0 ? 1.0f : ((float)runSleepItv.RunItv)/sum;
}

int GScheduleDevice::calcStartInSec()
{
    int res = 0;
    if(runSleepItv.RunItv > 0 && runSleepItv.SleepItv > 0 && isWithinTimeInterval())
    {
        int minFromStart = getMinutesPassedFromStart(tmRunSleepStartTime);
        int itv_len = runSleepItv.RunItv + runSleepItv.SleepItv;
        int curFromStart = minFromStart - (minFromStart/itv_len)*itv_len;
        if(curFromStart >= runSleepItv.RunItv)
        {
            // sleep
            res = (itv_len - curFromStart)*60;
            if(ptmSysTime != NULL)
            res -= ptmSysTime->Parsed.Second;
        }
        else
        {
            // run
            res = (runSleepItv.RunItv - curFromStart)*60;
            if(ptmSysTime != NULL)
            res -= ptmSysTime->Parsed.Second;
            res = -res;
        }
    }
    return res;
}

// GLedLamp

GLedLamp::GLedLamp(uint16_t memAddress):GScheduleDevice(memAddress)
{
    nType = dtLed;
}

void GLedLamp::init(uint8_t pin1, uint8_t pin2, GTimePtr sysTime, uint8_t id)
{
    this->pin1 = pin1;
    this->pin2 = pin2;
    ptmSysTime = sysTime;
    setDeviceId(id);
    pinMode(pin1, OUTPUT);
    digitalWrite(pin1, HIGH);
    pinMode(pin2, OUTPUT);
    digitalWrite(pin2, HIGH);
}

void GLedLamp::run(bool bStealth)
{
    bool firstOn = false;
    bool secondOn = false;
    checkDelay();
    if(getIsDelayShutdownHalf() && (ptmSysTime == NULL || ptmSysTime->NotParsed >= tmDelayHalfModeEndTime))
    {
        setIsDelayShutdownHalf(false);
    }
    if(!bStealth && getConnected())
    {
        switch (getMode()) {
            case dmOn:
                firstOn = secondOn = true;
                break;
            case dmOff:
                break;
            default: // auto
                {
                    if(isWithinTimeInterval())
                    {
                        firstOn = true;
                        if(!getIsDelayRun() && runSleepItv.RunItv > 0 && runSleepItv.SleepItv > 0)
                        {
                            if(!matchFirstInterval(tmRunSleepStartTime, runSleepItv.RunItv, runSleepItv.SleepItv))
                                firstOn = false;
                        }
                    }
                    secondOn = firstOn;
                }
                break;
        }
    }
    if(firstOn)
    {
        if(getModeEx() == lmeHalfPower || getIsDelayShutdownHalf())
        {
            if(matchFirstInterval(tmStartTime, 120, 120))
            {
                secondOn = false;
            }
            else{
                firstOn = false;
            }
        }
    }
    setIsOnFirst(firstOn);
    setIsOnSecond(secondOn);
}

void GLedLamp::setIsOnFirst(bool value)
{
    if(value)
        dwFlags |= 0x000000100;
    else
        dwFlags &= 0xFFFFFEFF;
    digitalWrite(pin1, value ? LOW : HIGH);
}

bool GLedLamp::getIsOnFirst()
{
    return (dwFlags & 0x00000100) > 0;
}

void GLedLamp::setIsOnSecond(bool value)
{
    if(value)
        dwFlags |= 0x000000200;
    else
        dwFlags &= 0xFFFFFDFF;
    digitalWrite(pin2, value ? LOW : HIGH);
}

bool GLedLamp::getIsOnSecond()
{
    return (dwFlags & 0x00000200) > 0;
}

void GLedLamp::setModeEx(LedModeEx value)
{
    dwFlags &= 0xFFFFFBFF;
    dwFlags |= (((int)value) & 0x00000001) << 10;
}

LedModeEx GLedLamp::getModeEx()
{
    return (LedModeEx)((dwFlags & 0x00000400) >> 10);
}

void GLedLamp::setIsDelayShutdownHalf(bool value)
{
    if(value)
        dwFlags |= 0x40000000;
    else
        dwFlags &= 0xBFFFFFFF;
}

bool GLedLamp::getIsDelayShutdownHalf()
{
    return (dwFlags & 0x40000000) > 0;
}

void GLedLamp::delayShutdownHalf(uint16_t delay)
{
    if(ptmSysTime != NULL)
    {
        tmDelayHalfModeEndTime = ptmSysTime->NotParsed + delay;
        setIsDelayShutdownHalf(true);
    }
}

bool GLedLamp::isRunning()
{
    return getIsOnFirst() || getIsOnSecond();
}

void GLedLamp::reset()
{
    if(ptmSysTime != NULL)
        tmRunSleepStartTime = TimeOfDay(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute);
    digitalWrite(pin1, getIsOnFirst() ? LOW : HIGH);
    digitalWrite(pin2, getIsOnSecond() ? LOW : HIGH);
}

// GDnatLamp

GDnatLamp::GDnatLamp(uint16_t memAddress):GScheduleDevice(memAddress)
{
    nType = dtDnat;
}

void GDnatLamp::init(uint8_t powerPin, uint8_t coolPin, GTimePtr sysTime, uint8_t id)
{
    this->powerPin = powerPin;
    this->coolPin = coolPin;
    ptmSysTime = sysTime;
    setDeviceId(id);
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, HIGH);
    pinMode(coolPin, OUTPUT);
    digitalWrite(coolPin, HIGH);
    tmStartCoolingTime = ptmSysTime->NotParsed;
}

void GDnatLamp::setPowerOn(bool value)
{
    if(value)
        dwFlags |= 0x000000100;
    else
        dwFlags &= 0xFFFFFEFF;
    digitalWrite(powerPin, value ? LOW : HIGH);
}

bool GDnatLamp::getPowerOn()
{
    return ((dwFlags & 0x000000100) > 0);
}

void GDnatLamp::setCoolOn(bool value)
{
    if(value)
        dwFlags |= 0x000000200;
    else
        dwFlags &= 0xFFFFFDFF;
    digitalWrite(coolPin, value ? LOW : HIGH);
}

bool GDnatLamp::getCoolOn()
{
    return ((dwFlags & 0x000000200) > 0);
}

void GDnatLamp::run(bool bStealth)
{
    checkDelay();
    bool on = false;
    bool coolOn = false;
    if(!bStealth && getConnected())
    {
        switch (getMode()) {
            case dmOn:
                on = true;
                break;
            case dmOff:
                break;
            default: // auto
                {
                    if(isWithinTimeInterval())
                    {
                        on = true;
                        if(!getIsDelayRun() && runSleepItv.RunItv > 0 && runSleepItv.SleepItv > 0)
                        {
                            if(!matchFirstInterval(tmRunSleepStartTime, runSleepItv.RunItv, runSleepItv.SleepItv))
                                on = false;
                        }
                    }
                }
                break;
        }
        if(getPowerOn() && !on)
        {
            tmStartCoolingTime = ptmSysTime->NotParsed;
            setCoolOn(true);
        }
        coolOn = getCoolOn();
        if(coolOn && ptmSysTime->NotParsed > tmStartCoolingTime + DNAT_COOLING_TIME)
            coolOn = false;
    }
    setPowerOn(on);
    setCoolOn(coolOn);
}

void GDnatLamp::startCooling()
{
    if(getConnected())
    {
        tmStartCoolingTime = ptmSysTime->NotParsed;
        setCoolOn(true);
    }
}

void GDnatLamp::stopCooling()
{
    if(getConnected())
        setCoolOn(false);
}

int GDnatLamp::getLostCoolingTime()
{
    if(getConnected() && getCoolOn())
    {
        int delta = ptmSysTime->NotParsed - tmStartCoolingTime;
        if(delta >= 0 && delta < DNAT_COOLING_TIME)
        {
            return DNAT_COOLING_TIME - delta;
        }
    }
    return 0;
}

bool GDnatLamp::isRunning()
{
    return getPowerOn();
}

void GDnatLamp::reset()
{
    if(ptmSysTime != NULL)
        tmRunSleepStartTime = TimeOfDay(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute);
    digitalWrite(powerPin, getPowerOn() ? LOW : HIGH);
    digitalWrite(coolPin, getCoolOn() ? LOW : HIGH);
}

// GEmergencyLamp

GEmergencyLamp::GEmergencyLamp(uint16_t memAddress):GScheduleDevice(memAddress)
{
    nType = dtEmergencyLamp;
}

void GEmergencyLamp::init(uint8_t powerPin, GTimePtr sysTime, uint8_t id)
{
    this->powerPin = powerPin;
    ptmSysTime = sysTime;
    startTime = sysTime->NotParsed;
    setDeviceId(id);
    if(powerPin > 0)
        pinMode(powerPin, OUTPUT);
    setIsOn(true);
}

void GEmergencyLamp::setIsOn(bool value)
{
    if(value)
        dwFlags |= 0x000000100;
    else
        dwFlags &= 0xFFFFFEFF;
    if(powerPin > 0)
        digitalWrite(powerPin, value ? HIGH : LOW);
}

bool GEmergencyLamp::isRunning()
{
    return (dwFlags & 0x000000100) > 0;
}

void GEmergencyLamp::reset()
{
    if(ptmSysTime != NULL)
        tmRunSleepStartTime = TimeOfDay(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute);
    if(powerPin > 0)
        digitalWrite(powerPin, isRunning() ? HIGH : LOW);
}

void GEmergencyLamp::run(bool emergencyOn)
{
    GDeviceMode mode = getMode();
    bool on = false;
    if(getConnected())
    {
        if(mode == dmOn || mode == dmAuto && emergencyOn && (ptmSysTime->NotParsed > startTime + 60))
        {
            on = true;
        }
    }
    setIsOn(on);
}

// GTubeFan

GTubeFan::GTubeFan(uint16_t memAddress):GScheduleDevice(memAddress)
{
    nType = dtTubeFan;
}

void GTubeFan::init(uint8_t powerPin, uint8_t speedSwitchPin, GTimePtr sysTime, uint8_t id)
{
    GScheduleDevice::init(sysTime, id);
    this->powerPin = powerPin;
    this->speedSwitchPin = speedSwitchPin;
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, HIGH);
    pinMode(speedSwitchPin, OUTPUT);
    digitalWrite(speedSwitchPin, HIGH);
}

void GTubeFan::setPowerOn(bool value)
{
    if(value)
        dwFlags |= 0x000000100;
    else
        dwFlags &= 0xFFFFFEFF;
    digitalWrite(powerPin, value ? LOW : HIGH);
}

bool GTubeFan::getPowerOn()
{
    return (dwFlags & 0x00000100) > 0;
}

void GTubeFan::setModeEx(TubeFanModeEx value)
{
    dwFlags &= 0xFFFFFDFF;
    dwFlags |= (((int)value) & 0x00000001) << 9;
}

TubeFanModeEx GTubeFan::getModeEx()
{
    return (TubeFanModeEx)((dwFlags & 0x00000200) >> 9);
}

void GTubeFan::run(bool bStealth)
{
    bool on = false;
    checkDelay();
    if(getIsDelayHighPower() && (ptmSysTime == NULL || ptmSysTime->NotParsed >= tmDelayHighPowerEndTime))
    {
        setIsDelayHighPower(false);
    }
    if(!bStealth && getConnected())
    {
        switch (getMode()) {
            case dmOn:
                on = true;
                break;
            case dmOff:
                break;
            default: // auto
                if(isWithinTimeInterval())
                {
                    on = true;
                    if(!getIsDelayRun() && runSleepItv.RunItv > 0 && runSleepItv.SleepItv > 0)
                    {
                        if(!matchFirstInterval(tmRunSleepStartTime, runSleepItv.RunItv, runSleepItv.SleepItv))
                            on = false;
                    }
                }
                break;
        }
        setHighPower(getIsDelayHighPower() || getModeEx() == tfmeHighPower);
    }
    setPowerOn(on);
}

void GTubeFan::reset()
{
    if(ptmSysTime != NULL)
        tmRunSleepStartTime = TimeOfDay(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute);
    digitalWrite(speedSwitchPin, getHighPower() ? LOW : HIGH);
    digitalWrite(powerPin, getPowerOn() ? LOW : HIGH);
}

bool GTubeFan::isRunning()
{
    return getPowerOn();
}

void GTubeFan::setIsDelayHighPower(bool value)
{
    if(value)
        dwFlags |= 0x40000000;
    else
        dwFlags &= 0xBFFFFFFF;
}

bool GTubeFan::getIsDelayHighPower()
{
    return (dwFlags & 0x40000000) > 0;
}

void GTubeFan::delayHighPower(uint16_t delay)
{
    if(ptmSysTime != NULL)
    {
        tmDelayHighPowerEndTime = ptmSysTime->NotParsed + delay;
        setIsDelayHighPower(true);
    }
}

void GTubeFan::setHighPower(bool value)
{
    if(value)
        dwFlags |= 0x80000000;
    else
        dwFlags &= 0x7FFFFFFF;
    digitalWrite(speedSwitchPin, value ? LOW : HIGH);
}

bool GTubeFan::getHighPower()
{
    return (dwFlags & 0x80000000) > 0;
}

// GBlowFan

GBlowFan::GBlowFan(uint16_t memAddress):GScheduleDevice(memAddress)
{
    nType = dtBlowFan;
}

void GBlowFan::init(uint8_t powerPinLeft, uint8_t powerPinRight, GTimePtr sysTime, uint8_t id)
{
    GScheduleDevice::init(sysTime, id);
    this->powerPinLeft = powerPinLeft;
    this->powerPinRight = powerPinRight;
    pinMode(powerPinLeft, OUTPUT);
    digitalWrite(powerPinLeft, HIGH);
    if(powerPinLeft != powerPinRight)
    {
        pinMode(powerPinRight, OUTPUT);
        digitalWrite(powerPinRight, HIGH);
    }
}

void GBlowFan::run(bool bStealth)
{
    bool on = false;
    checkDelay();
    if(!bStealth && getConnected())
    {
        switch (getMode()) {
            case dmOn:
                on = true;
                break;
            case dmOff:
                break;
            default: // auto
                if(isWithinTimeInterval())
                {
                    on = true;
                    if(!getIsDelayRun() && runSleepItv.RunItv > 0 && runSleepItv.SleepItv > 0)
                    {
                        if(!matchFirstInterval(tmRunSleepStartTime, runSleepItv.RunItv, runSleepItv.SleepItv))
                            on = false;
                    }
                }
                break;
        }
    }
    setIsOn(on);
}

void GBlowFan::reset()
{
    if(ptmSysTime != NULL)
        tmRunSleepStartTime = TimeOfDay(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute);
    digitalWrite(powerPinLeft, getIsOn() ? LOW : HIGH);
    if(powerPinLeft != powerPinRight)
        digitalWrite(powerPinRight, getIsOn() ? LOW : HIGH);
}

void GBlowFan::setIsOn(bool value)
{
    if(value)
        dwFlags |= 0x000000100;
    else
        dwFlags &= 0xFFFFFEFF;
    digitalWrite(powerPinLeft, value ? LOW : HIGH);
    if(powerPinLeft != powerPinRight)
        digitalWrite(powerPinRight, value ? LOW : HIGH);
}

bool GBlowFan::getIsOn()
{
    return (dwFlags & 0x00000100) > 0;
}

bool GBlowFan::isRunning()
{
    return getIsOn();
}

// GWaterPump

GWaterPump::GWaterPump(uint16_t memAddress):GScheduleDevice(memAddress)
{
    nType = dtWaterPump;
    tank1PortionMlValue = 0;
    tank2PortionMlValue = 0;
    tank1PortionMlLost = 0;
    tank2PortionMlLost = 0;
}

void GWaterPump::init(uint8_t tank1PowerPin, uint8_t tank2PowerPin, GTimePtr sysTime, uint8_t id)
{
    this->tank1PowerPin = tank1PowerPin;
    this->tank2PowerPin = tank2PowerPin;
    ptmSysTime = sysTime;
    setDeviceId(id);
    pinMode(tank1PowerPin, OUTPUT);
    digitalWrite(tank1PowerPin, HIGH);
    if(tank2PowerPin != tank1PowerPin)
    {
        pinMode(tank2PowerPin, OUTPUT);
        digitalWrite(tank2PowerPin, HIGH);
    }
    lastTime = ptmSysTime->NotParsed;
    referenceDate.year = ptmSysTime->Parsed.Year;
    referenceDate.month = ptmSysTime->Parsed.Month;
    referenceDate.day = ptmSysTime->Parsed.Day;
    lastWateringDate = referenceDate;
    lastWateringDate.decDay();
}

bool GWaterPump::readSettings()
{
    uint8_t buff[32];
    uint8_t length = 32;
    if(!RTC_MEM.eepromReadBuffer(nMemAddress, buff, &length) || length < 32)
        return false;
    if(buff[0] != WATERPUMP_SAVE_BEGIN_MARKER)
        return false;
    int readedId = ((buff[4] & 0xF0) >> 4);
    if(readedId != getDeviceId())
        return false;
    dwFlags = buff[4] | (((uint32_t)buff[5]) << 8) | (((uint32_t)buff[6]) << 16) |
    (((uint32_t)buff[7]) << 24);
    tmStartTime.Hour = buff[8];
    tmStartTime.Minute = buff[9];
    tmEndTime.Hour = buff[10];
    tmEndTime.Minute = buff[11];
    runSleepItv.RunItv = buff[12] | (((uint32_t)buff[13]) << 8);
    runSleepItv.SleepItv = buff[14] | (((uint32_t)buff[15]) << 8);
    tank1PortionMlValue = buff[16] | (((uint32_t)buff[17]) << 8);
    tank2PortionMlValue = buff[18] | (((uint32_t)buff[19]) << 8);
    tank1PortionMlLost = buff[20] | (((uint32_t)buff[21]) << 8);
    tank2PortionMlLost = buff[22] | (((uint32_t)buff[23]) << 8);
    referenceDate.day = buff[24];
    referenceDate.month = buff[25];
    referenceDate.year = buff[26];
    lastWateringDate.day = buff[27];
    lastWateringDate.month = buff[28];
    lastWateringDate.year = buff[29];
    return true;
}

void GWaterPump::writeSettings()
{
    uint8_t buff[32];
    for(int i=0;i < 32;i++)
        buff[i] = 0;
    buff[0] = WATERPUMP_SAVE_BEGIN_MARKER;
    saveFlagsToBuffer((uint8_t*)(buff+4));
    saveScheduleToBuffer((uint8_t*)(buff+8));
    saveRunSleepSettinsToBuffer((uint8_t*)(buff+12));
    savePortionMlValuesToBuffer((uint8_t*)(buff+16));
    saveWaterLostPortionToBuffer((uint8_t*)(buff+20));
    saveReferenceDateToBuffer((uint8_t*)(buff+24));
    saveLastWateringDateToBuffer((uint8_t*)(buff+27));
    RTC_MEM.eepromWriteBuffer(nMemAddress, buff, 32);
}

void GWaterPump::savePortionMlValuesToBuffer(uint8_t* buff)
{
    buff[0] = (uint8_t)(tank1PortionMlValue & 0x00FF);
    buff[1] = (uint8_t)((tank1PortionMlValue & 0xFF00) >> 8);
    buff[2] = (uint8_t)(tank2PortionMlValue & 0x00FF);
    buff[3] = (uint8_t)((tank2PortionMlValue & 0xFF00) >> 8);
}

void GWaterPump::saveWaterLostPortionToBuffer(uint8_t* buff)
{
    buff[0] = (uint8_t)(tank1PortionMlLost & 0x00FF);
    buff[1] = (uint8_t)((tank1PortionMlLost & 0xFF00) >> 8);
    buff[2] = (uint8_t)(tank2PortionMlLost & 0x00FF);
    buff[3] = (uint8_t)((tank2PortionMlLost & 0xFF00) >> 8);
}

void GWaterPump::saveReferenceDateToBuffer(uint8_t* buff)
{
    buff[0] = referenceDate.day;
    buff[1] = referenceDate.month;
    buff[2] = referenceDate.year;
}

void GWaterPump::saveLastWateringDateToBuffer(uint8_t* buff)
{
    buff[0] = lastWateringDate.day;
    buff[1] = lastWateringDate.month;
    buff[2] = lastWateringDate.year;
}

void GWaterPump::writeLostPortion()
{
    uint8_t buff[4];
    saveWaterLostPortionToBuffer(buff);
    RTC_MEM.eepromWriteBuffer(nMemAddress + 20, buff, 4);
}

void GWaterPump::writeLastWateringDate()
{
    uint8_t buff[3];
    saveWaterLostPortionToBuffer(buff);
    RTC_MEM.eepromWriteBuffer(nMemAddress + 27, buff, 3);
}

void GWaterPump::setIsOn1(bool value)
{
    if(value)
        dwFlags |= 0x000000100;
    else
        dwFlags &= 0xFFFFFEFF;
    digitalWrite(tank1PowerPin, value ? LOW : HIGH);
}

bool GWaterPump::getIsOn1()
{
    return (dwFlags & 0x00000100) > 0;
}

void GWaterPump::setIsOn2(bool value)
{
    if(value)
        dwFlags |= 0x000000200;
    else
        dwFlags &= 0xFFFFFDFF;
    digitalWrite(tank2PowerPin, value ? LOW : HIGH);
}

bool GWaterPump::getIsOn2()
{
    return (dwFlags & 0x00000200) > 0;
}

bool GWaterPump::isOnePin()
{
    return (tank1PowerPin == tank2PowerPin);
}

void GWaterPump::run()
{
    bool on1 = getIsOn1();
    bool on2 = getIsOn2();
    uint16_t wasTank1PortionMlLost = tank1PortionMlLost;
    uint16_t wasTank2PortionMlLost = tank2PortionMlLost;
    if(getConnected())
    {
        int secPassed = (int)(ptmSysTime->NotParsed - lastTime);
        if(secPassed > 0)
        {
            uint32_t waterConsumption = secPassed*PUMP_FLOWSPEED_MLPERSEC;
            if(on1)
            {
                if(waterConsumption >= tank1PortionMlLost)
                {
                    tank1PortionMlLost = 0;
                    on1 = false;
                }
                else
                {
                    tank1PortionMlLost -= waterConsumption;
                }
            }
            else{
                if(tank1PortionMlLost > 0)
                    on1 = true;
            }
            if(on2)
            {
                if(waterConsumption >= tank2PortionMlLost)
                {
                    tank2PortionMlLost = 0;
                    on2 = false;
                }
                else
                {
                    tank2PortionMlLost -= waterConsumption;
                }
            }
            else{
                if(tank2PortionMlLost > 0)
                    on2 = true;
            }
        }
    }
    else
    {
        on1 = on2 = false;
        tank1PortionMlLost = 0;
        tank2PortionMlLost = 0;
    }
    lastTime = ptmSysTime->NotParsed;
    setIsOn1(on1);
    setIsOn2(isOnePin() ? on1 : on2);
    if(wasTank1PortionMlLost != tank1PortionMlLost && wasTank2PortionMlLost != tank2PortionMlLost)
    {
        writeLostPortion();
    }
}

void GWaterPump::runSchedule()
{
    if(getConnected())
    {
        if(getMode() == dmAuto && !isWatering())
        {
            GDate curDate(ptmSysTime->Parsed.Day, ptmSysTime->Parsed.Month, ptmSysTime->Parsed.Year);
            if(lastWateringDate != curDate)
            {
                if(isWithinTimeInterval())
                {
                    bool doWater = true;
                    if(runSleepItv.RunItv && runSleepItv.SleepItv > 0)
                    {
                        uint32_t delta = GDate::daysDelta(referenceDate, curDate);
                        uint16_t totalItvLength = runSleepItv.RunItv + runSleepItv.SleepItv;
                        if(delta >= ((delta/totalItvLength)*totalItvLength + runSleepItv.RunItv))
                            doWater = false;
                    }
                    if(doWater)
                    {
                        lastWateringDate = curDate;
                        writeLastWateringDate();
                        delay(50);
                        startWatering();
                    }
                }
            }
        }
    }
}

void GWaterPump::startWatering()
{
    if(isWatering() || getMode() == dmOff)
        return;
    if(getConnected() && getMode() != dmOff)
    {
        tank1PortionMlLost = getTank1PortionMl();
        tank2PortionMlLost = getTank2PortionMl();
        writeLostPortion();
    }
}

void GWaterPump::startWatering(uint16_t portion1, uint16_t portion2)
{
    if(isWatering() || getMode() == dmOff)
        return;
    if(getConnected())
    {
        tank1PortionMlLost = portion1;
        tank2PortionMlLost = portion2;
        writeLostPortion();
    }
}

void GWaterPump::stopWatering()
{
    tank1PortionMlLost = 0;
    setIsOn1(false);
    
    tank2PortionMlLost = 0;
    setIsOn2(false);
    writeLostPortion();
}

bool GWaterPump::isWatering()
{
    return getIsOn1() || getIsOn2();
}

void GWaterPump::setTank1PortionMl(uint16_t value)
{
    tank1PortionMlValue = value;
}

void GWaterPump::setTank2PortionMl(uint16_t value)
{
    tank2PortionMlValue = value;
}

uint16_t GWaterPump::getTank1PortionMl()
{
    return tank1PortionMlValue;
}

uint16_t GWaterPump::getTank2PortionMl()
{
    return tank2PortionMlValue;
}

uint16_t GWaterPump::getLostWater()
{
    return tank1PortionMlLost + tank2PortionMlLost;
}

GDate GWaterPump::getReferenceDate()
{
    return referenceDate;
}

void GWaterPump::setReferenceDate(GDate& value)
{
    referenceDate = value;
}

GDate GWaterPump::getNextWateringDate()
{
    GDate res(ptmSysTime->Parsed.Day, ptmSysTime->Parsed.Month, ptmSysTime->Parsed.Year);
    bool bCheckSchedule = true;
    if(runSleepItv.RunItv > 0 && runSleepItv.SleepItv > 0)
    {
        int32_t delta = GDate::daysDelta(referenceDate, res);
        if(delta >= 0)
        {
            uint16_t total = runSleepItv.RunItv + runSleepItv.SleepItv;
            uint16_t startRun = (delta/total)*total;
            if (delta >= startRun + runSleepItv.RunItv) {
                delta = startRun + total;
                tmElements_t tm;
                tm.Second = 0;
                tm.Minute = 0;
                tm.Hour = 0;
                tm.Day = referenceDate.day;
                tm.Month = referenceDate.month;
                tm.Year = referenceDate.year;
                time_t newTime = makeTime(tm) + delta*86400;    //24*60*60
                breakTime(newTime, tm);
                res.year = tm.Year;
                res.month = tm.Month;
                res.day = tm.Day;
                bCheckSchedule = false;
            }
        }
    }
    if(bCheckSchedule)
    {
        if(TimeOfDay(ptmSysTime->Parsed.Hour, ptmSysTime->Parsed.Minute).compareTo(tmStartTime) > 0)
            res.incDay();
    }
    return res;
    
}

// GHumidifier

GHumidifier::GHumidifier(uint16_t memAddress):GScheduleDevice(memAddress)
{
    nType = dtHumidifier;
}

void GHumidifier::init(uint8_t powerPin, GTimePtr sysTime, uint8_t id)
{
    GScheduleDevice::init(sysTime, id);
    this->powerPin = powerPin;
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, HIGH);
}

void GHumidifier::setIsOn(bool value)
{
    if(value)
        dwFlags |= 0x000000100;
    else
        dwFlags &= 0xFFFFFEFF;
    digitalWrite(powerPin, value ? LOW : HIGH);
}

bool GHumidifier::getIsOn()
{
    return (dwFlags & 0x00000100) > 0;
}

void GHumidifier::run(bool bStealth)
{
    bool on = false;
    checkDelay();
    if(!bStealth && getConnected())
    {
        switch (getMode()) {
            case dmOn:
            on = true;
            break;
            case dmOff:
            break;
            default: // auto
            if(isWithinTimeInterval())
            {
                on = true;
                if(!getIsDelayRun() && runSleepItv.RunItv > 0 && runSleepItv.SleepItv > 0)
                {
                    if(!matchFirstInterval(tmRunSleepStartTime, runSleepItv.RunItv, runSleepItv.SleepItv))
                    on = false;
                }
            }
            break;
        }
    }
    setIsOn(on);
}

bool GHumidifier::isRunning()
{
    return getIsOn();
}

// GWarnLed

GWarnLed::GWarnLed()
{
    dwFlags = 0;
    counter = 0;
}

void GWarnLed::init(uint8_t pin)
{
    this->pin = pin;
    pinMode(pin, OUTPUT);
}

bool GWarnLed::getLedOn()
{
    return (dwFlags & 0x0001) > 0;
}

void GWarnLed::setLedOn(bool value)
{
    if(value)
        dwFlags |= 0x0001;
    else
        dwFlags &= 0xFFFE;
    digitalWrite(pin, value ? HIGH : LOW);
}

void GWarnLed::setMode(GWarnLedMode value)
{
    dwFlags &= 0xFFF9;
    dwFlags |= (((int)value) & 0x0003) << 1;
}

GWarnLedMode GWarnLed::getMode()
{
    return (GWarnLedMode)((dwFlags & 0x0006) >> 1);
}

int GWarnLed::getDashCount()
{
    return ((dwFlags & 0x00F0) >> 4);
}

void GWarnLed::setDashCount(int value)
{
    dwFlags &= 0xFF0F;
    dwFlags |= (value & 0x000F) << 4;
}

int GWarnLed::getDotCount()
{
    return ((dwFlags & 0x0F00) >> 8);
}

void GWarnLed::setDotCount(int value)
{
    dwFlags &= 0xF0FF;
    dwFlags |= (value & 0x000F) << 8;
}

bool GWarnLed::checkBlinkOn()
{
    if(counter <= 0)
        return false;
    int dashCnt = getDashCount();
    int lastDashEnd = 0;
    int itv = 0;
    if(dashCnt > 0)
    {
        lastDashEnd = dashCnt*DASH_TICK_COUNT + (dashCnt-1)*PAUSE_TICK_COUNT;
        if(counter <= lastDashEnd)
        {
            itv = DASH_TICK_COUNT + PAUSE_TICK_COUNT;
            return (counter-1) < ((counter-1)/itv)*itv + DASH_TICK_COUNT;
        }
    }
    int dotCnt = getDotCount();
    if(dotCnt > 0)
    {
        int dotsStart = 0;
        if(lastDashEnd > 0)
        {
            dotsStart = lastDashEnd + PAUSE_TICK_COUNT;
            if(counter <= dotsStart)
                return false;
        }
        if(counter <= dotsStart + dotCnt*DOT_TICK_COUNT + (dotCnt-1)*PAUSE_TICK_COUNT)
        {
            itv = DOT_TICK_COUNT + PAUSE_TICK_COUNT;
            return (counter - dotsStart - 1) < ((counter - dotsStart - 1)/itv)*itv + DOT_TICK_COUNT;
        }
    }
    return false;
}

void GWarnLed::checkOn()
{
    bool on = false;
    switch (getMode()) {
        case wlmOn:
            on = true;
            break;
        case wlmBlink:
            on = checkBlinkOn();
            break;
        default:
            break;
    }
    if(on)
    {
        if(!getLedOn())
            setLedOn(true);
    }
    else
    {
        if(getLedOn())
            setLedOn(false);
    }
}

void GWarnLed::check()
{
    checkOn();
    if(getMode() != wlmBlink)
    {
        return;
    }
    int dashcnt = getDashCount();
    int dotcnt = getDotCount();
    if(dashcnt + dotcnt > 0)
    {
        if(++counter >= dashcnt*DASH_TICK_COUNT + dotcnt*DOT_TICK_COUNT +
           (dashcnt+dotcnt-1)*PAUSE_TICK_COUNT + END_TICK_COUNT)
            counter = 0;
    }
}

void GWarnLed::switchOn()
{
    setMode(wlmOn);
    checkOn();
}

void GWarnLed::switchOff()
{
    setMode(wlmOff);
    checkOn();
}

void GWarnLed::setBlink(uint8_t dashCnt, uint8_t dotCnt)
{
    setDashCount(dashCnt);
    setDotCount(dotCnt);
    counter = 0;
    setMode(wlmBlink);
    checkOn();
}

// Greenhouse

// --------- eeprom mem usage: --------------

// 0..15 - sys settings: // добавилось еще 32 бита
// 0..47 - sys settings
// 0 - GREENHOUSE_SAVE_START_MARKER
// 1 - greenhouse program num
// 2..15 - reserved
// 16..19 - sys flags
// 20..23 - sysWarnTemp
// 24..27 - systemShutdownTemp
// 28..31 - sysHumidityExtremeTop
// 32..35 - devicesWarnTemp
// 36..39 - devicesExtremeTemp
// 40..43 - devicesOverheatTemp
// 44..47 - devicesShutdownTemp


// лень переписывать. все что ниже +32 бита (расширились собственные настройки теплицы)

// 16..31 - ledLampLeft1 GScheduleDevice settings
// 0 byte - DEVICE_SAVE_BEGIN_MARKER
// 1..3 - reserved
// 4..7 - flags
// 8 - start time hour
// 9 - start time minute
// 10 - end time hour
// 11 - end time minute
// 12..13 - run itv
// 14..15 - sleep itv

// 32..47 - ledLampLeft2 settings
// 48..63 - ledLampRight1 settings
// 64..79 - ledLampRight2 settings
// 80..95 - dnatLamp settings
// 96..111 - tubeFan settings
// 112..127 - blowFan settings
// 128..143 - emergencyLamp settings

// 144..287 (16 + 8*16 byte) Temp sensors settings
// 144..159 common settings
// 144 - TEMPSENSORS_SAVE_BEGIN_MARKER
// 145 - sensors count
// 146..159 - reserved
// 160..175 - first sensor info
// 176..191 - second sensor info
// 192..207 - third sensor info
// 208..223 - 4th sensor info
// 224..239 - 5th sensor info
// 240..255 - 6th sensor info
// 256..271 - 7th sensor info
// 272..287 - 8th sensor info

// temp sensor info
// 0..15
// 0..7 - device address
// 8 - device id
// TEMPSENSOR_SAVE_END_MARKER

// 288..319 - waterPump

// >= 320 - stat

Greenhouse::Greenhouse(GreenhouseCreatePins& createPins):
    gsm(createPins.GsmRX, createPins.GsmTX)
    , dht(createPins.TempHumiditySensor, DHT11)
    , tempSensors(createPins.DallasSensors, 176)
    , ledLampLeft1(48)
    , ledLampLeft2(64)
    , ledLampRight1(80)
    , ledLampRight2(96)
    , dnatLamp(112)
    , tubeFan(128)
    , blowFan(144)
    , emergencyLamp(160)
    , waterPump(320)
    , humidifier(352)
{
    dwFlags = 0;
    programNum = 0;
    sysHumidity = 40.0f;
    sysTemperature = 25.0f;
    sysWarnTempDelta = 0;
    devicesWarnTempDelta = 0;
}

void Greenhouse::setupStartTime()
{
    tmStart = tmCurrent;
    setClockStartIsOk(getClockIsOk());
    if(getClockStartIsOk())
    {
        tmTimer1LastTick = tmStart.NotParsed;
        tmTimer2LastTick = tmTimer1LastTick + 1;
        tmTimer3LastTick = tmTimer2LastTick + 1;
        tmTimer4LastTick = tmTimer3LastTick + 1;
        tmTimer5MinPeriodStart = tmStart.NotParsed;
        tmTimerShutdown = tmStart.NotParsed;
        tmTimerPowerOn = tmStart.NotParsed - ((getAutoSwitchOnItv() > 0 ? getAutoSwitchOnItv()*60 : 1800) - 30);
    }
    
}

void Greenhouse::init(GreenhouseInitPins &controlPins)
{
    gsm.init(controlPins.GsmResetPin);
    // relays2
    pinMode(controlPins.SystemSwitchOff, OUTPUT);
    digitalWrite(controlPins.SystemSwitchOff, HIGH);
    pinMode(controlPins.SystemSwitchOn, OUTPUT);
    digitalWrite(controlPins.SystemSwitchOn, HIGH);
    pinMode(controlPins.DCAdapterCooler, OUTPUT);
    digitalWrite(controlPins.DCAdapterCooler, HIGH);
    if(controlPins.ArduinoReset > 0)
    {
        pinMode(controlPins.ArduinoReset, OUTPUT);
        digitalWrite(controlPins.ArduinoReset, HIGH);
    }
    
    // init warn leds
    greenWarnLed.init(controlPins.GreenWarnLed);
    yellowWarnLed.init(controlPins.YellowWarnLed);
    redWarnLed.init(controlPins.RedWarnLed);
    
    // shutdown pin
    shutDownPin = controlPins.SystemSwitchOff;
    systemStartPin = controlPins.SystemSwitchOn;
    
    // soil moisture sensor pin
    soilMoistureAnalogPin = controlPins.SoilMoistureAnalog;
    
    dcAdapterCoolerPin = controlPins.DCAdapterCooler;
    
    arduinoResetPin = controlPins.ArduinoReset;
    
    //setup start time
    readTime();
    setupStartTime();
    
    // init devices
    ledLampLeft1.init(controlPins.LedLampLeft1_1, controlPins.LedLampLeft1_2, &tmCurrent, 1);
    ledLampLeft2.init(controlPins.LedLampLeft2_1, controlPins.LedLampLeft2_2, &tmCurrent, 2);
    ledLampRight1.init(controlPins.LedLampRight1_1, controlPins.LedLampRight1_2, &tmCurrent, 3);
    ledLampRight2.init(controlPins.LedLampRight2_1, controlPins.LedLampRight2_2, &tmCurrent, 4);
    dnatLamp.init(controlPins.DnatPower, controlPins.DnatCool, &tmCurrent, 5);
    tubeFan.init(controlPins.TubeFanPower, controlPins.TubeFanSwitch, &tmCurrent, 6);
    blowFan.init(controlPins.BlowFanPowerLeft, controlPins.BlowFanPowerRight, &tmCurrent, 7);
    emergencyLamp.init(controlPins.EmergencyLamp, &tmCurrent, 8);
    waterPump.init(controlPins.WaterPumpTank1Pin, controlPins.WaterPumpTank2Pin, &tmCurrent, 9);
    humidifier.init(controlPins.Humidifier, &tmCurrent, 10);
    
    // init sensors
    dht.begin();
    tempSensors.init(&tmCurrent);
    
    uint8_t memProgNum = 0;
    bool rmem = readProgramNumFromMem(&memProgNum);
    delay(50);
    if(!rmem)
    {
        debugOutputln("failed to read program num!");
    }
    else if(memProgNum != GREENHOUSE_CURRENT_PROGRAM)
    {
        debugOutputln("memProgNum != GREENHOUSE_CURRENT_PROGRAM");
    }
    if(rmem && memProgNum == GREENHOUSE_CURRENT_PROGRAM && readFromMem())
    {
        //ledLampLeft1.reset();
        //ledLampLeft2.reset();
        //ledLampRight1.reset();
        //ledLampRight2.reset();
        //dnatLamp.reset();
        //tubeFan.reset();
        //blowFan.reset();
        //emergencyLamp.reset();
    }
    else
    {
        debugOutputln("failed read from mem!");
        setProgram(GREENHOUSE_CURRENT_PROGRAM);
        writeToMem();
        //tempSensors.setNeedLink(true);
    }
    
    // GO!
    setIsRunning(true);
    greenWarnLed.switchOn();
}

bool Greenhouse::readSelfSettings()
{
    uint8_t buff[36];
    uint8_t length = 4;
    if(!RTC_MEM.eepromReadBuffer(12, buff, &length) || length < 4)
        return false;
    delay(50);
    length = 32;
    if(!RTC_MEM.eepromReadBuffer(16, (uint8_t*)(buff+4), &length) || length < 32)
        return false;
    nAutoSwitchOnItv = buff[0] | (((uint32_t)buff[1]) << 8);
    tmAutoResetTime.Hour = buff[2];
    tmAutoResetTime.Minute = buff[3];
    if(tmAutoResetTime.Hour > 23)
        tmAutoResetTime.Hour = 0;
    if(tmAutoResetTime.Minute > 59)
        tmAutoResetTime.Minute = 0;
    uint32_t sysflags = buff[4] | (((uint32_t)buff[5]) << 8) | (((uint32_t)buff[6]) << 16) | (((uint32_t)buff[7]) << 24);
    setStealthModeOn((sysflags & 0x00000001) > 0);
    setAutoSwitchOn((sysflags & 0x00000002) > 0);
    setAutoReset((sysflags & 0x00000004) > 0);
    sysWarnTemp = readFloatFromMemBuff((uint8_t*)(buff + 8));
    systemShutdownTemp = readFloatFromMemBuff((uint8_t*)(buff + 12));
    sysHumidityExtremeTop = readFloatFromMemBuff((uint8_t*)(buff + 16));
    devicesWarnTemp = readFloatFromMemBuff((uint8_t*)(buff + 20));
    devicesExtremeTemp = readFloatFromMemBuff((uint8_t*)(buff + 24));
    devicesOverheatTemp = readFloatFromMemBuff((uint8_t*)(buff + 28));
    devicesShutdownTemp = readFloatFromMemBuff((uint8_t*)(buff + 32));
    return true;
}

void Greenhouse::writeSelfSettings()
{
    uint8_t buff[36];
    for(int i=0;i < 36;i++)
        buff[i] = 0;
    uint32_t sysflags = 0;
    if(getStealthModeOn())
        sysflags |= 0x00000001;
    if(getAutoSwitchOn())
        sysflags |= 0x00000002;
    if(getAutoReset())
        sysflags |= 0x00000004;
    buff[0] = (uint8_t)(nAutoSwitchOnItv & 0x00FF);
    buff[1] = (uint8_t)((nAutoSwitchOnItv & 0xFF00) >> 8);
    buff[2] = tmAutoResetTime.Hour;
    buff[3] = tmAutoResetTime.Minute;
    for(int i=0;i < 4;i++)
    {
        buff[i+4] = (uint8_t)((sysflags & (((uint32_t)0x000000FF) << (8*i))) >> (8*i));
    }
    getFloatAsMemBuff(sysWarnTemp, (uint8_t*)(buff + 8));
    getFloatAsMemBuff(systemShutdownTemp, (uint8_t*)(buff + 12));
    getFloatAsMemBuff(sysHumidityExtremeTop, (uint8_t*)(buff + 16));
    getFloatAsMemBuff(devicesWarnTemp, (uint8_t*)(buff + 20));
    getFloatAsMemBuff(devicesExtremeTemp, (uint8_t*)(buff + 24));
    getFloatAsMemBuff(devicesOverheatTemp, (uint8_t*)(buff + 28));
    getFloatAsMemBuff(devicesShutdownTemp, (uint8_t*)(buff + 32));
    RTC_MEM.eepromWriteBuffer(12, buff, 36);
}

bool Greenhouse::readProgramNumFromMem(uint8_t* outval)
{
    uint8_t buff[2];
    uint8_t length = 2;
    if(!RTC_MEM.eepromReadBuffer(0, buff, &length) || length < 2)
        return false;
    if(buff[0] != GREENHOUSE_SAVE_START_MARKER)
        return false;
    *outval = buff[1];
    return true;
}

bool Greenhouse::readFromMem()
{
    uint8_t buff[2];
    uint8_t length = 2;
    programNum = 0;
    if(!RTC_MEM.eepromReadBuffer(0, buff, &length) || length < 2)
        return false;
    if(buff[0] != GREENHOUSE_SAVE_START_MARKER)
        return false;
    delay(50);
    if(!readSelfSettings())
        return false;
    delay(50);
    if(!ledLampLeft1.readSettings())
        return false;
    delay(50);
    if(!ledLampLeft2.readSettings())
        return false;
    delay(50);
    if(!ledLampRight1.readSettings())
        return false;
    delay(50);
    if(!ledLampRight2.readSettings())
        return false;
    delay(50);
    if(!dnatLamp.readSettings())
        return false;
    delay(50);
    if(!tubeFan.readSettings())
        return false;
    delay(50);
    if(!blowFan.readSettings())
        return false;
    delay(50);
    if(!emergencyLamp.readSettings())
        return false;
    delay(50);
    if(!waterPump.readSettings())
        return false;
    delay(50);
    if(!humidifier.readSettings())
        return false;
    programNum = buff[1];
    return true;
}

void Greenhouse::writeToMem()
{
    uint8_t buff[2];
    buff[0] = GREENHOUSE_SAVE_START_MARKER;
    buff[1] = programNum;
    RTC_MEM.eepromWriteBuffer(0, buff, 2);
    delay(50);
    writeSelfSettings();
    delay(50);
    ledLampLeft1.writeSettings();
    delay(50);
    ledLampLeft2.writeSettings();
    delay(50);
    ledLampRight1.writeSettings();
    delay(50);
    ledLampRight2.writeSettings();
    delay(50);
    dnatLamp.writeSettings();
    delay(50);
    tubeFan.writeSettings();
    delay(50);
    blowFan.writeSettings();
    delay(50);
    emergencyLamp.writeSettings();
    delay(50);
    waterPump.writeSettings();
    delay(50);
    humidifier.writeSettings();
}

void Greenhouse::readTime()
{
    setClockIsOk(RTC_MEM.read(tmCurrent.Parsed));
    if(getClockIsOk())
        tmCurrent.NotParsed = makeTime(tmCurrent.Parsed);
}

void Greenhouse::setTime(tmElements_t &tm)
{
    RTC_MEM.write(tm);
    readTime();
    setupStartTime();
}

void Greenhouse::setTime(TimeOfDay &tm)
{
    tmElements_t t;
    t.Year = CalendarYrToTm(2015);
    t.Month = 8;
    t.Day = 26;
    if(getClockIsOk())
    {
        t.Year = tmCurrent.Parsed.Year;
        t.Month = tmCurrent.Parsed.Month;
        t.Day = tmCurrent.Parsed.Day;
    }
    t.Hour = tm.Hour;
    t.Minute = tm.Minute;
    t.Second = 0;
    setTime(t);
}

tmElements_t Greenhouse::getStartTime()
{
    return tmStart.Parsed;
}

tmElements_t Greenhouse::getCurrentTime()
{
    return tmCurrent.Parsed;
}

void Greenhouse::setCurrentTime(tmElements_t& value)
{
    setTime(value);
}

void Greenhouse::setProgram(uint8_t programNum)
{
    this->programNum = programNum;
    switch (programNum) {
        case 1:
            setupProgram01();
            break;
        case 2:
            setupProgram02();
            break;
            
        default:
            break;
    }
}

void Greenhouse::setupProgram01()
{
    // setup connected
    ledLampLeft1.setConnected(true);
    ledLampLeft2.setConnected(true);
    ledLampRight1.setConnected(true);
    ledLampRight2.setConnected(false);
    dnatLamp.setConnected(false);
    tubeFan.setConnected(true);
    blowFan.setConnected(false);
    
    // setup schedule
    TimeOfDay start(8, 0);
    TimeOfDay end(2, 0);
    ledLampLeft1.setSchedule(start, end);
    ledLampLeft2.setSchedule(start, end);
    ledLampRight1.setSchedule(start, end);
    ledLampRight2.setSchedule(start, end);
    dnatLamp.setSchedule(start, end);
    tubeFan.setSchedule(start, end);
    tubeFan.setRunSleepItv(RunSleepSettings(30, 0));
    blowFan.setSchedule(start, end);
    
    waterPump.setSchedule(start, TimeOfDay(10, 0));
    
    // setup ex properties
    //tubeFan.setModeEx(tfmeHighPower);
    
    // switchOn
    ledLampLeft1.setMode(dmAuto);
    ledLampLeft2.setMode(dmAuto);
    ledLampRight1.setMode(dmAuto);
    tubeFan.setMode(dmAuto);
}

void Greenhouse::setupProgram02()
{
    // for test
    
    // setup connected
    ledLampLeft1.setConnected(true);
    ledLampLeft2.setConnected(false);
    ledLampRight1.setConnected(true);
    ledLampRight2.setConnected(false);
    dnatLamp.setConnected(false);
    tubeFan.setConnected(true);
    blowFan.setConnected(false);
    
    // setup schedule
    TimeOfDay start(6, 0);
    TimeOfDay end(0, 0);
    ledLampLeft1.setSchedule(start, end);
    ledLampLeft2.setSchedule(start, end);
    ledLampRight1.setSchedule(start, end);
    ledLampRight2.setSchedule(start, end);
    dnatLamp.setSchedule(start, end);
    tubeFan.setSchedule(start, end);
    tubeFan.setRunSleepItv(RunSleepSettings(5, 25));
    blowFan.setSchedule(start, end);
    
    // setup ex properties
    //tubeFan.setModeEx(tfmeHighPower);
    
    // switchOn
    ledLampLeft1.setMode(dmAuto);
    ledLampRight1.setMode(dmAuto);
    tubeFan.setMode(dmAuto);
}

bool Greenhouse::getClockIsOk()
{
    return (dwFlags & 0x00000001) > 0;
}

void Greenhouse::setClockIsOk(bool val)
{
    if(val)
        dwFlags |= 0x000000001;
    else
        dwFlags &= 0xFFFFFFFE;
}

void Greenhouse::setClockStartIsOk(bool val)
{
    if(val)
        dwFlags |= 0x000000002;
    else
        dwFlags &= 0xFFFFFFFD;
}

bool Greenhouse::getClockStartIsOk()
{
    return (dwFlags & 0x00000002) > 0;
}

bool Greenhouse::getIsRunnig()
{
    return (dwFlags & 0x00000008) > 0;
}

void Greenhouse::setIsRunning(bool value)
{
    if(value)
        dwFlags |= 0x000000008;
    else
        dwFlags &= 0xFFFFFFF7;
}

bool Greenhouse::getStealthModeOn()
{
    return (dwFlags & 0x00000010) > 0;
}

void Greenhouse::setStealthModeOn(bool value)
{
    if(value)
        dwFlags |= 0x000000010;
    else
        dwFlags &= 0xFFFFFFEF;
}

bool Greenhouse::getIsShutdowning()
{
    return (dwFlags & 0x00000020) > 0;
}

void Greenhouse::setIsShutdowning(bool value)
{
    if(value)
        dwFlags |= 0x000000020;
    else
        dwFlags &= 0xFFFFFFDF;
}

bool Greenhouse::getIsSwitchingOn()
{
    return (dwFlags & 0x00000004) > 0;
}

void Greenhouse::setIsSwitchingOn(bool value)
{
    if(value)
        dwFlags |= 0x00000004;
    else
        dwFlags &= 0xFFFFFFFB;
}

bool Greenhouse::getIsAdapterCoolerOn()
{
    return (dwFlags & 0x00000040) > 0;
}

void Greenhouse::setIsAdapterCoolerOn(bool value)
{
    if(value)
        dwFlags |= 0x00000040;
    else
        dwFlags &= 0xFFFFFFBF;
    digitalWrite(dcAdapterCoolerPin, value ? LOW : HIGH);
}

bool Greenhouse::getAutoSwitchOn()
{
    return (dwFlags & 0x00000080) > 0;
}

void Greenhouse::setAutoSwitchOn(bool value)
{
    if(value)
    {
        dwFlags |= 0x000000080;
        tmTimerPowerOn = tmCurrent.NotParsed;
    }
    else
    {
        dwFlags &= 0xFFFFFF7F;
    }
}

uint16_t Greenhouse::getAutoSwitchOnItv()
{
    return nAutoSwitchOnItv;
}

void Greenhouse::setAutoSwitchOnItv(uint16_t val)
{
    nAutoSwitchOnItv = val;
    tmTimerPowerOn = tmCurrent.NotParsed;
}

bool Greenhouse::getAutoReset()
{
    return (dwFlags & 0x00000100) > 0;
}

void Greenhouse::setAutoReset(bool value)
{
    if(value)
        dwFlags |= 0x000000100;
    else
        dwFlags &= 0xFFFFFEFF;
}

TimeOfDay Greenhouse::getAutoResetTime()
{
    return tmAutoResetTime;
}

void Greenhouse::setAutoResetTime(TimeOfDay value)
{
    tmAutoResetTime = value;
}

void Greenhouse::systemShutdown()
{
    setIsRunning(false);
    setIsShutdowning(true);
    if(getClockIsOk())
    {
        tmTimerShutdown = tmCurrent.NotParsed;
        tmTimerPowerOn = tmCurrent.NotParsed;
    }
    digitalWrite(shutDownPin, LOW);
    // shutdown devices
    ledLampLeft1.run(true);
    ledLampLeft2.run(true);
    ledLampRight1.run(true);
    ledLampRight2.run(true);
    dnatLamp.run(true);
    dnatLamp.startCooling();
    tubeFan.run(true);
    blowFan.run(true);
    humidifier.run(true);
    waterPump.stopWatering();
    checkDCAdapterCooler();
}

void Greenhouse::systemPowerOn()
{
    if(!getIsSwitchingOn())
    {
        setIsSwitchingOn(true);
        digitalWrite(systemStartPin, LOW);
        if(getClockIsOk())
        {
            tmTimerPowerOn = tmCurrent.NotParsed - (getAutoSwitchOnItv() > 0 ? getAutoSwitchOnItv()*60 : 1800);
        }
    }
}

String Greenhouse::getDeviceName(int id)
{
    switch (id) {
        case 1:
            return "LED светильник левый 1";
        case 2:
            return "LED светильник левый 2";
        case 3:
            return "LED светильник правый 1";
        case 4:
            return "LED светильник правый 2";
        case 5:
            return "ДНАТ";
        case 6:
            return "Канальник";
        case 7:
            return "Обдув";
        case 8:
            return "Экстренная ЭСЛ";
        case 9:
            return "Полив";
        case 10:
            return "Увлажнитель";
        default:
            return "Неизвестное устройство";
    }
}

uint8_t Greenhouse::getConnectedLamps(GScheduleDevice** devices)
{
    uint8_t cnt = 0;
    if(ledLampLeft1.getConnected())
        devices[cnt++] = &ledLampLeft1;
    if(ledLampLeft2.getConnected())
        devices[cnt++] = &ledLampLeft2;
    if(ledLampRight1.getConnected())
        devices[cnt++] = &ledLampRight1;
    if(ledLampRight2.getConnected())
        devices[cnt++] = &ledLampRight2;
    if(dnatLamp.getConnected())
        devices[cnt++] = &dnatLamp;
    return cnt;
}

void Greenhouse::linkTempSensors()
{
    GScheduleDevice* devices[8];
    uint8_t cnt = getConnectedLamps(devices);
    if(cnt > 0)
        tempSensors.linkToDevices(devices, cnt);
}

void Greenhouse::doArduinoReset()
{
    if(arduinoResetPin > 0)
        digitalWrite(arduinoResetPin, LOW);
}

void Greenhouse::loop()
{
    if(programNum == 0)
    {
        if(greenWarnLed.getMode() != wlmBlink || greenWarnLed.getDashCount() != 0 ||
           greenWarnLed.getDotCount() != 1)
            greenWarnLed.setBlink(0, 1);
        return;
    }
    
    if(!getClockStartIsOk())
    {
        if(greenWarnLed.getMode() != wlmBlink || greenWarnLed.getDashCount() != 0 ||
           greenWarnLed.getDotCount() != 2)
            greenWarnLed.setBlink(0, 2);
        return;
    }
    
    readTime();
    
    if(!getClockIsOk())
    {
        if(greenWarnLed.getMode() != wlmBlink || greenWarnLed.getDashCount() != 0 ||
           greenWarnLed.getDotCount() != 3)
            greenWarnLed.setBlink(0, 3);
        return;
    }
    
    if(tmCurrent.NotParsed < tmStart.NotParsed + 30)
    {
        return;
    }
    
    if(getAutoReset())
    {
        if(tmCurrent.NotParsed - tmStart.NotParsed > 3600)  // от момента последнего рестарта прошло больше часа
        {
            // если текущее время в интервале [время_презагрузки, время_перезагрузки+1час] то делаем перезагрузку
            TimeOfDay t1 = tmAutoResetTime;
            t1.decrease(60);
            int cmp = tmAutoResetTime.compareTo(t1);
            TimeOfDay cur(tmCurrent.Parsed.Hour, tmCurrent.Parsed.Minute);
            cur.decrease(60);
            bool bDo = false;
            if(cmp < 0)
                bDo = t1.compareTo(cur) <= 0 && tmAutoResetTime.compareTo(cur) > 0;
            else
                bDo = !(tmAutoResetTime.compareTo(cur) <= 0 && t1.compareTo(cur) > 0);
            if(bDo)
                digitalWrite(arduinoResetPin, LOW);
        }
    }
    
    if(getIsShutdowning() && tmCurrent.NotParsed > tmTimerShutdown + 5)
    {
        setIsShutdowning(false);
        digitalWrite(shutDownPin, HIGH);
    }
    
    if(getAutoSwitchOn() && !getIsSwitchingOn() &&
       tmCurrent.NotParsed > tmTimerPowerOn + (getAutoSwitchOnItv() > 0 ? getAutoSwitchOnItv()*60 : 1800))
    {
        setIsSwitchingOn(true);
        digitalWrite(systemStartPin, LOW);
    }
    
    if(getIsSwitchingOn())
    {
        if(tmCurrent.NotParsed > tmTimerPowerOn + ((getAutoSwitchOnItv() > 0 ? getAutoSwitchOnItv()*60 : 1800) + 5))
        {
            setIsSwitchingOn(false);
            digitalWrite(systemStartPin, HIGH);
            tmTimerPowerOn = tmCurrent.NotParsed;
            setIsRunning(true);
        }
    }
    
    if(!getIsRunnig())
        return;
    
    sysHumidity = dht.readHumidity();
    float tmpSysTemp = dht.readTemperature();
    if(!isnan(tmpSysTemp) && abs(tmpSysTemp - sysTemperature) < 50.0f)
        sysTemperature = tmpSysTemp;
    tempSensors.read();
    float devicesMaxTemp = tempSensors.getMaxTemp();
    if(sysTemperature >= systemShutdownTemp || !isnan(devicesMaxTemp) && devicesMaxTemp >= devicesShutdownTemp)
    {
        debugOutput("system shutdown: systmp=");
        debugOutput(sysTemperature);
        debugOutput("; devMaxTmp=");
        debugOutputln(devicesMaxTemp);
        systemShutdown();
        return;
    }
    
    bool bStealthOn = getStealthModeOn();
    
    if(tmCurrent.NotParsed >= tmTimer1LastTick)
    {
        // run first block
        // run devices
        waterPump.runSchedule();
        waterPump.run();
        ledLampLeft1.run(bStealthOn);
        ledLampLeft2.run(bStealthOn);
        ledLampRight1.run(bStealthOn);
        ledLampRight2.run(bStealthOn);
        checkDCAdapterCooler();
        // run or link temp sensors
        if(tempSensors.getNeedLink() && !tempSensors.isLinking())
        {
            linkTempSensors();
        }
        else
        {
            tempSensors.run();
        }
        soilMoisture = analogRead(soilMoistureAnalogPin);
        checkSystemTemp();
        
        tmTimer1LastTick += 5;
    }
    
    if(tmCurrent.NotParsed >= tmTimer2LastTick)
    {
        // run second block
        // run devices
        waterPump.run();
        dnatLamp.run(false);
        // check climate
        checkHumidity();
        tmTimer2LastTick += 5;
    }
    
    if(tmCurrent.NotParsed >= tmTimer3LastTick)
    {
        // run third block
        waterPump.run();
        emergencyLamp.run(tempSensors.isLinking() || !(ledLampLeft1.isRunning() || ledLampLeft2.isRunning() ||
            ledLampRight1.isRunning() || ledLampRight2.isRunning() || dnatLamp.isRunning()));
        tempSensors.run();
        humidifier.run(bStealthOn);
        checkLedTemp(&ledLampLeft1);
        checkLedTemp(&ledLampLeft2);
        checkLedTemp(&ledLampRight1);
        checkLedTemp(&ledLampRight2);
        tmTimer3LastTick += 5;
    }
    if(tmCurrent.NotParsed >= tmTimer4LastTick)
    {
        // run 4th block
        waterPump.run();
        tubeFan.run(bStealthOn);
        blowFan.run(bStealthOn);
        // check red warn led
        updateWarnLeds();
        
        tmTimer4LastTick += 5;
        tmTimer3LastTick = tmTimer4LastTick - 1;
        tmTimer2LastTick = tmTimer3LastTick - 1;
        tmTimer1LastTick = tmTimer2LastTick - 1;
    }
}

void Greenhouse::checkDCAdapterCooler()
{
    bool on = ledLampLeft1.isRunning() || ledLampLeft2.isRunning() || ledLampRight1.isRunning() || ledLampRight2.isRunning() || blowFan.isRunning();
    if(on != getIsAdapterCoolerOn())
        setIsAdapterCoolerOn(on);
}

void Greenhouse::checkHumidity()
{
    if(sysHumidity > sysHumidityExtremeTop)
    {
        if(humidifier.isRunning() && !humidifier.getIsDelayShutdown())
        {
            humidifier.delayShutdown(180);
        }
        if(!tubeFan.isRunning() && !tubeFan.getIsDelayRun())
        {
            tubeFan.delayRun(180);
        }
    }
}

void Greenhouse::checkLedTemp(GLedLamp* ledLamp)
{
    if (ledLamp->isRunning()) {
        float tmp = tempSensors.getDeviceTemp(ledLamp->getDeviceId());
        if(!isnan(tmp) && tmp > devicesExtremeTemp)
        {
            if(!ledLamp->getIsDelayShutdownHalf())
            {
                uint16_t delay = 600;
                float overTemp = sysTemperature - (sysWarnTemp - 5.0f);
                if(overTemp > 0)
                    delay += (int)floor(overTemp*120.0f);
                ledLamp->delayShutdownHalf(delay);
            }
        }
    }
}

void Greenhouse::checkSystemTemp()
{
    if(sysTemperature >= sysWarnTemp)
    {
        if(!tubeFan.isRunning() && !tubeFan.getIsDelayRun())
        {
            tubeFan.delayRun(1800);
        }
        if(!tubeFan.getIsDelayHighPower())
        {
            tubeFan.delayHighPower(1800);
        }
    }
}

void Greenhouse::updateWarnLeds()
{
    int delta = 0;
    if(!isnan(sysTemperature) && sysTemperature >= sysWarnTemp)
    {
        delta = (int)floor(sysTemperature - sysWarnTemp + 1.0f);
        if(delta != sysWarnTempDelta)
        {
            redWarnLed.setBlink(delta / 5, delta % 5);
            sysWarnTempDelta = delta;
        }
    }
    else
    {
        sysWarnTempDelta = 0;
        if(redWarnLed.getMode() != wlmOff)
            redWarnLed.switchOff();
    }
    // check yellow warn led
    float maxDeviceTemp = tempSensors.getMaxTemp();
    if(!isnan(maxDeviceTemp) && maxDeviceTemp >= devicesWarnTemp)
    {
        delta = (int)floor(maxDeviceTemp - devicesWarnTemp + 1.0f);
        if(delta != devicesWarnTempDelta)
        {
            yellowWarnLed.setBlink(delta / 5, delta % 5);
            devicesWarnTempDelta = delta;
        }
    }
    else
    {
        devicesWarnTempDelta = 0;
        if(yellowWarnLed.getMode() != wlmOff)
            yellowWarnLed.switchOff();
    }
    // check green warn led
    if(tempSensors.isLinking() || emergencyLamp.isRunning() && emergencyLamp.getMode() == dmAuto)
    {
        if(greenWarnLed.getMode() != wlmBlink || greenWarnLed.getDashCount() != 1 ||
           greenWarnLed.getDotCount() != 0)
            greenWarnLed.setBlink(1, 0);
    }
    else
    {
        if(greenWarnLed.getMode() != wlmOn)
            greenWarnLed.switchOn();
    }
}

void Greenhouse::loopWarnLeds()
{
    greenWarnLed.check();
    yellowWarnLed.check();
    redWarnLed.check();
}

void Greenhouse::loopGSM()
{
    gsm.loop();
}

String Greenhouse::executeCommand(int target, String command)
{
    ParamStringReader resultBuilder("");
    ParamStringReader commandReader(command);
    //Serial.println("Greenhouse::executeCommand begin: target=" + String(target) + ";command=" + command);
    if (target == -1) {
        ParamStringElement* param = commandReader.getCurrentParam();
        while(param){
            if(param->name.equalsIgnoreCase("cmd")){
                gsm.runcmd(param->value);
            }
            else if(param->name.equalsIgnoreCase("atcmd")){
                gsm.runatcmd(param->value);
            }
            else if(param->name.equalsIgnoreCase("state")){
                if (param->isRead) {
                    resultBuilder.addParam(param->name, String(gsm.getState()));
                }
            }
            else if(param->name.equalsIgnoreCase("balance")){
                if (param->isRead) {
                    resultBuilder.addParam(param->name, String(gsm.getBalance()));
                }
            }
            else if(param->name.equalsIgnoreCase("smspacket")){
                if (param->isRead) {
                    resultBuilder.addParam(param->name, String(gsm.getSmsPacketLost()));
                }
            }
            else if(param->name.equalsIgnoreCase("output")){
                if (param->isRead) {
                    resultBuilder.addParam(param->name, String(gsm.getOutput()));
                }
            }
            commandReader.next();
            param = commandReader.getCurrentParam();
        }
    }
    else if(target == 0)
    {
        // собственные свойства
        tmElements_t curtm;
        int dwSetDate = 0;
        String val;
        ParamStringElement* param = commandReader.getCurrentParam();
        while(param){
            if(param->name.equalsIgnoreCase("devcnt")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getDevicesCount()));
                }
            }
            else if(param->name.equalsIgnoreCase("systemp")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getSystemTemp()));
                }
            }
            else if(param->name.equalsIgnoreCase("devtemp")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(tempSensors.getMaxTemp()));
                }
            }
            else if(param->name.equalsIgnoreCase("humidity")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getHumidity()));
                }
            }
            else if(param->name.equalsIgnoreCase("soil")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getSoilMoisture()));
                }
            }
            else if(param->name.equalsIgnoreCase("curdate")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, tmElements2String(getCurrentTime()));
                }
            }
            else if(param->name.equalsIgnoreCase("startdate")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, tmElements2String(getStartTime()));
                }
            }
            else if(param->name.equalsIgnoreCase("date")){
                dwSetDate |= 1;
                GDate dt(param->value);
                curtm.Year = dt.year;
                curtm.Month = dt.month;
                curtm.Day = dt.day;
            }
            else if(param->name.equalsIgnoreCase("time")){
                dwSetDate |= 2;
                TimeOfDay tm(param->value);
                curtm.Hour = tm.Hour;
                curtm.Minute = tm.Minute;
                curtm.Second = 0;
            }
            else if(param->name.equalsIgnoreCase("condevices")){
                if(param->isRead){
                    val = "";
                    for(int i=0;i < TOTAL_DEVICES_COUNT;i++){
                        if(allDevices[i]->getConnected()){
                            if(val.length() > 0)
                                val += ",";
                            val += String(allDevices[i]->getDeviceId());
                        }
                    }
                    resultBuilder.addParam(param->name, val);
                }
            }
            else if(param->name.equalsIgnoreCase("tempsensorlink")){
                if(param->isRead){
                    val = "";
                    for (int i=0; i < tempSensors.getSensorsCount(); i++) {
                        if(val.length() > 0)
                            val += ",";
                        val += String(i) + ":" + String(tempSensors.getSensorInfo(i).getDeviceId());
                    }
                    resultBuilder.addParam(param->name, val);
                }
                else{
                    ParamArrayReader arrayReader(param->value);
                    while (arrayReader.hasParam()) {
                        PairFromString pair(arrayReader.getParam());
                        tempSensors.setSensorDeviceLink(pair.First.toInt(), (uint8_t)pair.Second.toInt());
                        arrayReader.next();
                    }
                }
            }
            else if(param->name.equalsIgnoreCase("tempsensortemp")){
                if(param->isRead){
                    val = "";
                    for (int i=0; i < tempSensors.getSensorsCount(); i++) {
                        if(val.length() > 0)
                            val += ",";
                        val += String(i) + ":" + String(tempSensors.getSensorInfo(i).getTemp());
                    }
                    resultBuilder.addParam(param->name, val);
                }
            }
            else if(param->name.equalsIgnoreCase("stealth")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, getStealthModeOn() ? "1" : "0");
                }
                else {
                    setStealthModeOn(param->value == "1");
                }
            }
            else if(param->name.equalsIgnoreCase("devwarntemp")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getDevWarnTemp()));
                }
                else {
                    setDevWarnTemp(param->value.toFloat());
                }
            }
            else if(param->name.equalsIgnoreCase("devextremetemp")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getDevExtremeTemp()));
                }
                else {
                    setDevExtremeTemp(param->value.toFloat());
                }
            }
            else if(param->name.equalsIgnoreCase("devoverheattemp")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getDevOverheatTemp()));
                }
                else {
                    setDevOverheatTemp(param->value.toFloat());
                }
            }
            else if(param->name.equalsIgnoreCase("devshutdowntemp")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getDevShutdownTemp()));
                }
                else {
                    setDevShutdownTemp(param->value.toFloat());
                }
            }
            else if(param->name.equalsIgnoreCase("syswarntemp")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getSysWarnTemp()));
                }
                else {
                    setSysWarnTemp(param->value.toFloat());
                }
            }
            else if(param->name.equalsIgnoreCase("sysshutdowntemp")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getSysShutdownTemp()));
                }
                else {
                    setSysShutdownTemp(param->value.toFloat());
                }
            }
            else if(param->name.equalsIgnoreCase("sysmaxhum")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getSysHumidityExtremeTop()));
                }
                else {
                    setSysHumidityExtremeTop(param->value.toFloat());
                }
            }
            else if(param->name.equalsIgnoreCase("autolinkstart")){
                linkTempSensors();
            }
            else if(param->name.equalsIgnoreCase("autolinkstop")){
                tempSensors.setNeedLink(false);
                tempSensors.stopLinking();
            }
            else if(param->name.equalsIgnoreCase("syson")){
                systemPowerOn();
            }
            else if(param->name.equalsIgnoreCase("sysoff")){
                systemShutdown();
            }
            else if(param->name.equalsIgnoreCase("sysreset")){
                doArduinoReset();
            }
            else if(param->name.equalsIgnoreCase("autolinking")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, tempSensors.isLinking() ? "1" : "0");
                }
            }
            else if(param->name.equalsIgnoreCase("autoswitchon")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, getAutoSwitchOn() ? "1" : "0");
                }
                else {
                    setAutoSwitchOn(param->value == "1");
                }
            }
            else if(param->name.equalsIgnoreCase("autoswitchonitv")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, String(getAutoSwitchOnItv()));
                }
                else {
                    setAutoSwitchOnItv((uint16_t)param->value.toInt());
                }
            }
            else if(param->name.equalsIgnoreCase("autoreset")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, getAutoReset() ? "1" : "0");
                }
                else {
                    setAutoReset(param->value == "1");
                }
            }
            else if(param->name.equalsIgnoreCase("autoresettime")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, getAutoResetTime().toString());
                }
                else {
                    setAutoResetTime(TimeOfDay(param->value));
                }
            }
            else if(param->name.equalsIgnoreCase("savetomem")){
                if(param->value == "self"){
                    writeSelfSettings();
                }
            }
            //Serial.println(resultBuilder.getParamString());
            //Serial.println("length: " + String(resultBuilder.getParamString().length()));
            commandReader.next();
            param = commandReader.getCurrentParam();
        }
        if(dwSetDate != 0){
            if(dwSetDate != 3){
                tmElements_t gtm = getCurrentTime();
                if (dwSetDate == 1) {
                    curtm.Hour = gtm.Hour;
                    curtm.Minute = gtm.Minute;
                    curtm.Second = gtm.Second;
                }
                else{
                    curtm.Year = gtm.Year;
                    curtm.Month = gtm.Month;
                    curtm.Day = gtm.Day;
                }
            }
            setCurrentTime(curtm);
        }
    }
    else{
        GScheduleDevice* device = getDevice(target);
        ParamStringElement* param = commandReader.getCurrentParam();
        while(param){
            if(param->name.equalsIgnoreCase("exists")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, device ? "1" : "0");
                }
            }
            else if(param->name.equalsIgnoreCase("name")){
                if(param->isRead){
                    resultBuilder.addParam(param->name, getDeviceName(target));
                }
            }
            else if(param->name.equalsIgnoreCase("temp")){
                if(param->isRead){
                    float temp = getDeviceTemp(target);
                    resultBuilder.addParam(param->name, isnan(temp) ? "nan" : String(temp));
                }
            }
            else {
                if(device){
                    TimeOfDay schStart = device->getScheduleStart();
                    TimeOfDay schEnd = device->getScheduleEnd();
                    bool bSetSchedule = false;
                    if(param->name.equalsIgnoreCase("type")){
                        if(param->isRead){
                            resultBuilder.addParam(param->name, devTypeToString(device->getType()));
                        }
                    }
                    else if(param->name.equalsIgnoreCase("connected")){
                        if(param->isRead){
                            resultBuilder.addParam(param->name, device->getConnected() ? "1" : "0");
                        }
                        else{
                            device->setConnected(param->value == "1");
                        }
                    }
                    else if(param->name.equalsIgnoreCase("isrunning")){
                        if(param->isRead){
                            resultBuilder.addParam(param->name, device->isRunning() ? "1" : "0");
                        }
                    }
                    else if(param->name.equalsIgnoreCase("mode")){
                        if(param->isRead){
                            resultBuilder.addParam(param->name, devMode2String(device->getMode()));
                        }
                        else{
                            device->setMode(devModeFromString(param->value));
                        }
                    }
                    else if(param->name.equalsIgnoreCase("modeex")){
                        if(device->getType() == dtLed){
                            if(param->isRead){
                                resultBuilder.addParam(param->name, ((GLedLamp*)device)->getModeEx() == lmeHalfPower ? "ledhalf" : "ledfull");
                            }
                            else {
                                ((GLedLamp*)device)->setModeEx(param->value == "ledhalf" ? lmeHalfPower : lmeFullPower);
                            }
                        }
                        else if(device->getType() == dtTubeFan){
                            if(param->isRead){
                                resultBuilder.addParam(param->name, ((GTubeFan*)device)->getModeEx() == tfmeHighPower ? "tubehigh" : "tubelow");
                            }
                            else {
                                ((GTubeFan*)device)->setModeEx(param->value == "tubehigh" ? tfmeHighPower : tfmeLowPower);
                            }
                        }
                    }
                    else if(param->name.equalsIgnoreCase("shedulestart")){
                        if(param->isRead){
                            resultBuilder.addParam(param->name, device->getScheduleStart().toString());
                        }
                        else {
                            schStart = TimeOfDay(param->value);
                            bSetSchedule = true;
                        }
                    }
                    else if(param->name.equalsIgnoreCase("sheduleend")){
                        if(param->isRead){
                            resultBuilder.addParam(param->name, device->getScheduleEnd().toString());
                        }
                        else {
                            schEnd = TimeOfDay(param->value);
                            bSetSchedule = true;
                        }
                    }
                    else if(param->name.equalsIgnoreCase("runsleepitv")){
                        if(param->isRead){
                            resultBuilder.addParam(param->name, device->getRunSleepItv().toString());
                        }
                        else {
                            device->setRunSleepItv(RunSleepSettings(param->value));
                        }
                    }
                    else if(param->name.equalsIgnoreCase("savetomem")){
                        device->writeSettings();
                    }
                    else if(param->name.equalsIgnoreCase("startinsec")){
                        if(param->isRead){
                            resultBuilder.addParam(param->name, String(device->calcStartInSec()));
                        }
                    }
                    else if(param->name.equalsIgnoreCase("dnatcoollost")){
                        if(param->isRead){
                            if(device->getType() == dtDnat){
                                resultBuilder.addParam(param->name, String(((GDnatLamp*)device)->getLostCoolingTime()));
                            }
                        }
                    }
                    else if(param->name.equalsIgnoreCase("waterlost")){
                        if(param->isRead){
                            if(device->getType() == dtWaterPump){
                                resultBuilder.addParam(param->name, String(((GWaterPump*)device)->getLostWater()));
                            }
                        }
                    }
                    else if(param->name.equalsIgnoreCase("incdutyratio")){
                        device->increaseDutyRatio();
                    }
                    else if(param->name.equalsIgnoreCase("decdutyratio")){
                        device->decreaseDutyRatio();
                    }
                    else if(param->name.equalsIgnoreCase("startwork")){
                        device->resetRunSleepStartTime(false);
                    }
                    else if(param->name.equalsIgnoreCase("startsleep")){
                        device->resetRunSleepStartTime(true);
                    }
                    else if(param->name.equalsIgnoreCase("forcewateringstart")){
                        if(device->getType() == dtWaterPump){
                            ((GWaterPump*)device)->startWatering();
                        }
                    }
                    else if(param->name.equalsIgnoreCase("forcewateringstop")){
                        if(device->getType() == dtWaterPump){
                            ((GWaterPump*)device)->stopWatering();
                        }
                    }
                    else if(param->name.equalsIgnoreCase("dnatstartcooling")){
                        if(device->getType() == dtDnat){
                            ((GDnatLamp*)device)->startCooling();
                        }
                    }
                    else if(param->name.equalsIgnoreCase("dnatstopcooling")){
                        if(device->getType() == dtDnat){
                            ((GDnatLamp*)device)->stopCooling();
                        }
                    }
                    else if(param->name.equalsIgnoreCase("waterreferencedate")){
                        if(device->getType() == dtWaterPump){
                            if(param->isRead){
                                resultBuilder.addParam(param->name, ((GWaterPump*)device)->getReferenceDate().toString());
                            }
                            else{
                                GDate dt(param->value);
                                ((GWaterPump*)device)->setReferenceDate(dt);
                            }
                        }
                    }
                    else if(param->name.equalsIgnoreCase("nextwateringdate")){
                        if(device->getType() == dtWaterPump){
                            if(param->isRead){
                                resultBuilder.addParam(param->name, ((GWaterPump*)device)->getNextWateringDate().toString());
                            }
                        }
                    }
                    else if(param->name.equalsIgnoreCase("waterportion")){
                        if(device->getType() == dtWaterPump){
                            GWaterPump* pump = (GWaterPump*)device;
                            if(param->isRead){
                                resultBuilder.addParam(param->name, String(pump->getTank1PortionMl()+pump->getTank2PortionMl()));
                            }
                            else{
                                int portion = param->value.toInt();
                                pump->setTank1PortionMl(portion/2);
                                pump->setTank2PortionMl(portion/2);
                            }
                        }
                    }

                    if(bSetSchedule)
                    {
                        device->setSchedule(schStart, schEnd);
                    }
                }
            }
            commandReader.next();
            param = commandReader.getCurrentParam();
        }
    }
    //Serial.print("Greenhouse::executeCommand end. result=");
    //Serial.println(resultBuilder.getParamString());
    return resultBuilder.getParamString();
}

float Greenhouse::getSystemTemp()
{
    return sysTemperature;
}

void Greenhouse::getDevicesTemp(float* temp, String* devNames, int* devcount)
{
    int cnt = tempSensors.getSensorsCount();
    for(int i=0;i < cnt;i++)
    {
        TempSensorInfo& sInfo = tempSensors.getSensorInfo(i);
        temp[i] = sInfo.getTemp();
        devNames[i] = getDeviceName(sInfo.getDeviceId());
    }
    *devcount = cnt;
}

float Greenhouse::getDeviceTemp(int devid)
{
    float res = NAN;
    for(int i=0;i < tempSensors.getSensorsCount();i++)
    {
        TempSensorInfo& sInfo = tempSensors.getSensorInfo(i);
        if(sInfo.getDeviceId() == devid)
        {
            if(sInfo.getHasTemp())
                res = sInfo.getTemp();
        }
    }
    return res;
}
                                            
float Greenhouse::getHumidity()
{
    return sysHumidity;
}

uint16_t Greenhouse::getSoilMoisture()
{
    return soilMoisture;
}

DallasTemperatureSensors& Greenhouse::getTempSensors()
{
    return tempSensors;
}

int Greenhouse::getDevicesCount()
{
    return TOTAL_DEVICES_COUNT;
}

GScheduleDevice* Greenhouse::getDevice(int id)
{
    for(int i=0;i < TOTAL_DEVICES_COUNT;i++)
    {
        GScheduleDevice* tmp = allDevices[i];
        if(tmp->getDeviceId() == id)
            return tmp;
    }
    return NULL;
}

String Greenhouse::devTypeToString(GDeviceType val)
{
    switch (val) {
        case dtLed:
            return "led";
        case dtDnat:
            return "dnat";
        case dtTubeFan:
            return "tubefan";
        case dtBlowFan:
            return "blowfan";
        case dtEmergencyLamp:
            return "emergencyLamp";
        case dtWaterPump:
            return "waterPump";
        case dtHumidifier:
            return "humidifier";
        default:
            return "unknown";
    }
}

void Greenhouse::getConnectedDevices(GScheduleDevice** devices, int* count)
{
    int resCnt = 0;
    for(int i=0;i < TOTAL_DEVICES_COUNT;i++)
    {
        GScheduleDevice* tmp = allDevices[i];
        if(tmp->getConnected())
        {
            devices[resCnt++] = tmp;
        }
    }
    *count = resCnt;
}

float Greenhouse::getDevWarnTemp()
{
    return devicesWarnTemp;
}
void Greenhouse::setDevWarnTemp(float value)
{
    devicesWarnTemp = value;
}

float Greenhouse::getDevExtremeTemp()
{
    return devicesExtremeTemp;
}
void Greenhouse::setDevExtremeTemp(float value)
{
    devicesExtremeTemp = value;
}

float Greenhouse::getDevOverheatTemp()
{
    return devicesOverheatTemp;
}
void Greenhouse::setDevOverheatTemp(float value)
{
    devicesOverheatTemp = value;
}

float Greenhouse::getDevShutdownTemp()
{
    return devicesShutdownTemp;
}
void Greenhouse::setDevShutdownTemp(float value)
{
    devicesShutdownTemp = value;
}

float Greenhouse::getSysWarnTemp()
{
    return sysWarnTemp;
}
void Greenhouse::setSysWarnTemp(float value)
{
    sysWarnTemp = value;
}

float Greenhouse::getSysShutdownTemp()
{
    return systemShutdownTemp;
}
void Greenhouse::setSysShutdownTemp(float value)
{
    systemShutdownTemp = value;
}

float Greenhouse::getSysHumidityExtremeTop()
{
    return sysHumidityExtremeTop;
}
void Greenhouse::setSysHumidityExtremeTop(float value)
{
    sysHumidityExtremeTop = value;
}

void Greenhouse::getFloatAsMemBuff(float num, uint8_t* buff)
{
    uint32_t asInt = *((uint32_t*)(&num));
    for(int i=0;i < 4;i++)
        buff[i] = (uint8_t)((asInt & (((uint32_t)0x000000FF) << (i*8))) >> (i*8));
}

float Greenhouse::readFloatFromMemBuff(uint8_t* buff)
{
    uint32_t asInt = 0;
    for(int i=0;i < 4;i++)
        asInt |= ((uint32_t)buff[i]) << (i*8);
    return *((float*)((uint32_t*)(&asInt)));
}

void Greenhouse::executeCommand(String command, String* params, int paramCount)
{
    if(command.equalsIgnoreCase("getsystemtemp"))
    {
        Serial.print("t = ");
        Serial.print(sysTemperature);
        Serial.print("*C; h = ");
        Serial.print(sysHumidity);
        Serial.println("%");
    }
    else if(command.equalsIgnoreCase("getdevicestemp"))
    {
        Serial.print("Devices temp: ");
        for(int i=0;i < tempSensors.getSensorsCount();i++)
        {
            TempSensorInfo& sinfo = tempSensors.getSensorInfo(i);
            Serial.print(i);
            Serial.print(") ");
            if(sinfo.getHasTemp())
                Serial.print(sinfo.getTemp());
            else
                Serial.print("NAN");
            Serial.print(" *C (");
            Serial.print(getDeviceName(sinfo.getDeviceId()));
            Serial.print("); ");
        }
        Serial.println("");
    }
    else if(command.equalsIgnoreCase("linktempsensors"))
    {
        linkTempSensors();
    }
    else if(command.equalsIgnoreCase("settime"))
    {
        if(paramCount > 0)
        {
            String param = params[0];
            int pos = param.indexOf(':');
            if(pos > 0 && pos < param.length() - 1)
            {
                TimeOfDay tm(param.substring(0, pos).toInt(), param.substring(pos+1, param.length()).toInt());
                if(tm.Hour < 24 && tm.Minute < 60)
                {
                    setTime(tm);
                    return;
                }
            }
        }
        Serial.println("Invalid params!");
    }
    else if(command.equalsIgnoreCase("tubefan"))
    {
        if(paramCount > 0)
        {
            String param = params[0];
            if(param.equalsIgnoreCase("lowpower"))
            {
                tubeFan.setModeEx(tfmeLowPower);
            }
            else if(param.equalsIgnoreCase("highpower"))
            {
                tubeFan.setModeEx(tfmeHighPower);
            }
            else if(param.equalsIgnoreCase("decreasedutyratio"))
            {
                tubeFan.decreaseDutyRatio();
            }
            else if(param.equalsIgnoreCase("increasedutyratio"))
            {
                tubeFan.increaseDutyRatio();
            }
            else
            {
                Serial.println("Invalid params!");
            }
        }
    }
    else
    {
        Serial.println("Invalid command!");
    }
}
