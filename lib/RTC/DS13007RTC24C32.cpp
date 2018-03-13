//
//  DS13007RTC24C32.cpp
//  Growbox
//
//  Created by Bugslayer on 23.08.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#include <Wire.h>
#include "DS13007RTC24C32.h"

#define DS1307_CTRL_ID 0x68
#define AT24C32_CTRL_ID 0x50
#define PAGE_SIZE 32

DS13007RTC24C32::DS13007RTC24C32()
{
    Wire.begin();
}

// PUBLIC FUNCTIONS
time_t DS13007RTC24C32::get()   // Aquire data from buffer and convert to time_t
{
    tmElements_t tm;
    if (read(tm) == false) return 0;
    return(makeTime(tm));
}

bool DS13007RTC24C32::set(time_t t)
{
    tmElements_t tm;
    breakTime(t, tm);
    tm.Second |= 0x80;  // stop the clock
    write(tm);
    tm.Second &= 0x7f;  // start the clock
    write(tm);
}

// Aquire data from the RTC chip in BCD format
bool DS13007RTC24C32::read(tmElements_t &tm)
{
    uint8_t sec;
    Wire.beginTransmission(DS1307_CTRL_ID);
#if ARDUINO >= 100
    Wire.write((uint8_t)0x00);
#else
    Wire.send(0x00);
#endif
    if (Wire.endTransmission() != 0) {
        exists = false;
        return false;
    }
    exists = true;
    
    // request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
    Wire.requestFrom(DS1307_CTRL_ID, tmNbrFields);
    if (Wire.available() < tmNbrFields) return false;
#if ARDUINO >= 100
    sec = Wire.read();
    tm.Second = bcd2dec(sec & 0x7f);
    tm.Minute = bcd2dec(Wire.read() );
    tm.Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
    tm.Wday = bcd2dec(Wire.read() );
    tm.Day = bcd2dec(Wire.read() );
    tm.Month = bcd2dec(Wire.read() );
    tm.Year = y2kYearToTm((bcd2dec(Wire.read())));
#else
    sec = Wire.receive();
    tm.Second = bcd2dec(sec & 0x7f);
    tm.Minute = bcd2dec(Wire.receive() );
    tm.Hour =   bcd2dec(Wire.receive() & 0x3f);  // mask assumes 24hr clock
    tm.Wday = bcd2dec(Wire.receive() );
    tm.Day = bcd2dec(Wire.receive() );
    tm.Month = bcd2dec(Wire.receive() );
    tm.Year = y2kYearToTm((bcd2dec(Wire.receive())));
#endif
    if (sec & 0x80) return false; // clock is halted
    return true;
}

bool DS13007RTC24C32::write(tmElements_t &tm)
{
    Wire.beginTransmission(DS1307_CTRL_ID);
#if ARDUINO >= 100
    Wire.write((uint8_t)0x00); // reset register pointer
    Wire.write(dec2bcd(tm.Second)) ;
    Wire.write(dec2bcd(tm.Minute));
    Wire.write(dec2bcd(tm.Hour));      // sets 24 hour format
    Wire.write(dec2bcd(tm.Wday));
    Wire.write(dec2bcd(tm.Day));
    Wire.write(dec2bcd(tm.Month));
    Wire.write(dec2bcd(tmYearToY2k(tm.Year)));
#else
    Wire.send(0x00); // reset register pointer
    Wire.send(dec2bcd(tm.Second)) ;
    Wire.send(dec2bcd(tm.Minute));
    Wire.send(dec2bcd(tm.Hour));      // sets 24 hour format
    Wire.send(dec2bcd(tm.Wday));
    Wire.send(dec2bcd(tm.Day));
    Wire.send(dec2bcd(tm.Month));
    Wire.send(dec2bcd(tmYearToY2k(tm.Year)));
#endif
    if (Wire.endTransmission() != 0) {
        exists = false;
        return false;
    }
    exists = true;
    return true;
}

bool DS13007RTC24C32::eepromReadByte(unsigned int address, byte* data)
{
    byte rdata = 0xFF;
    bool res = false;
    Wire.beginTransmission(AT24C32_CTRL_ID);
    Wire.write((int)(address >> 8)); // MSB
    Wire.write((int)(address & 0xFF)); // LSB
    if(Wire.endTransmission() == 0)
    {
        Wire.requestFrom(AT24C32_CTRL_ID, 1);
        if (Wire.available())
        {
            rdata = Wire.read();
            res = true;
        }
    }
    *data = rdata;
    return res;
}

bool DS13007RTC24C32::eepromReadBuffer(unsigned int address, byte* buffer, byte* length)
{
    bool res = false;
    Wire.beginTransmission(AT24C32_CTRL_ID);
    Wire.write((int)(address >> 8)); // MSB
    Wire.write((int)(address & 0xFF)); // LSB
    byte obtained = 0;
    if(Wire.endTransmission() == 0)
    {
        res = true;
        byte toRead = *length;
        Wire.requestFrom(AT24C32_CTRL_ID, toRead);
        for(int i = 0;i < toRead;i++)
        {
            if(Wire.available())
            {
                buffer[i] = Wire.read();
                obtained++;
            }
            else
            {
                break;
            }
        }
    }
    *length = obtained;
    return res;
}

bool DS13007RTC24C32::eepromWriteByte(unsigned int address, byte data)
{
    Wire.beginTransmission(AT24C32_CTRL_ID);
    Wire.write((int)(address >> 8)); // MSB
    Wire.write((int)(address & 0xFF)); // LSB
    Wire.write((int)data);
    return (Wire.endTransmission() == 0);
}

bool DS13007RTC24C32::eepromWriteBuffer(unsigned int address, byte* buffer, byte length)
{
    uint16_t endAddr = address + length - 1;
    uint16_t startPage = address/PAGE_SIZE;
    uint16_t endPage = endAddr/PAGE_SIZE;
    uint8_t pageBuff[PAGE_SIZE];
    uint16_t blockAddr = address;
    uint16_t blockLen = length;
    uint16_t buffShift = 0;
    for(uint16_t page = startPage;page <= endPage;page++)
    {
        if(page == startPage)
        {
            blockAddr = address;
            blockLen = (endPage > startPage) ? PAGE_SIZE*(startPage+1) - blockAddr : length;
            buffShift = 0;
        }
        else
        {
            blockAddr = PAGE_SIZE*page;
            blockLen = (page == endPage) ? address + length - blockAddr : PAGE_SIZE;
            buffShift = blockAddr - address;
        }
        for(int i=0;i < blockLen;i++)
        {
            pageBuff[i] = buffer[i + buffShift];
        }
        Wire.beginTransmission(AT24C32_CTRL_ID);
        Wire.write((int)(blockAddr >> 8)); // MSB
        Wire.write((int)(blockAddr & 0xFF)); // LSB
        for(int i=0;i < blockLen;i++)
        {
            Wire.write((int)pageBuff[i]);
        }
        if(Wire.endTransmission() != 0)
            return false;
        if(page < endPage)
            delay(50);
    }
    return true;
}

// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t DS13007RTC24C32::dec2bcd(uint8_t num)
{
    return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t DS13007RTC24C32::bcd2dec(uint8_t num)
{
    return ((num/16 * 10) + (num % 16));
}

bool DS13007RTC24C32::exists = false;

DS13007RTC24C32 RTC_MEM = DS13007RTC24C32(); // create an instance for the user
