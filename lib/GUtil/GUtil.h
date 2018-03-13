//
//  GreenhouseControl.h
//  Greenhouse
//
//  Created by Bugslayer on 10.11.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#ifndef Greenhouse_GreenhouseControl_h
#define Greenhouse_GreenhouseControl_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class GreenhouseControl
{
public:
    virtual String executeCommand(int target, String command)=0;
};

struct ParamStringElement
{
    bool isRead = false;
    String name;
    String value;
};

class ParamStringReader
{
private:
    String paramString;
    ParamStringElement currentParam;
    bool hasParam = false;
    int curPos = 0;
    
    void readParam();
public:
    ParamStringReader(String paramString);
    ParamStringElement* getCurrentParam();
    void next();
    void addParam(String name, String val = "");
    void addQueryParam(String name);
    String getParamString();
    void init(String paramString);
    String getParamValue(String& paramName);
};

class ParamArrayReader
{
private:
    String delimiter;
    String paramString;
    String curParam;
    bool _hasParam = false;
    int curPos = 0;
public:
    ParamArrayReader(String& paramString, String delimiter = ",", bool back = false);
    void next();
    void prev();
    bool hasParam();
    String getParam();
    void flush(bool back = false);
};

struct PairFromString
{
    String First;
    String Second;
    
    PairFromString(String str);
    void init(String str);
};

#endif
