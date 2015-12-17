//
//  DS13007RTC24C32.h
//  Growbox
//
//  Created by Bugslayer on 23.08.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#ifndef __Growbox__DS13007RTC24C32__
#define __Growbox__DS13007RTC24C32__

#include <Time.h>
// include types & constants of Wiring core API
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include <WProgram.h>
#include <WConstants.h>
#endif

class DS13007RTC24C32
{
public:
    DS13007RTC24C32();
    static time_t get();
    static bool set(time_t t);
    static bool read(tmElements_t &tm);
    static bool write(tmElements_t &tm);
    static bool chipPresent() { return exists; }
    static bool eepromReadByte(unsigned int address, byte* data);
    static bool eepromReadBuffer(unsigned int address, byte* buffer, byte* length);
    static bool eepromWriteByte(unsigned int address, byte data);
    static bool eepromWriteBuffer(unsigned int address, byte* buffer, byte length);
    
private:
    static bool exists;
    static uint8_t dec2bcd(uint8_t num);
    static uint8_t bcd2dec(uint8_t num);
};

extern DS13007RTC24C32 RTC_MEM;

#endif /* defined(__Growbox__DS13007RTC24C32__) */
