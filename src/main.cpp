//
//  main.cpp
//  Garden
//
//  Created by Bugslayer on 17.12.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#include <Time.h>
#include <Wire.h>
#include <DS13007RTC24C32.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <GUtil.h>
#include <SoftwareSerial.h>
#include <GSMService.h>
#include <Greenhouse.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <GWebServer.h>

GreenhouseCreatePins pins(42, 43, 11, 12);
Greenhouse greenhouse(pins);
int loopCounter = 0;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip (192, 168, 0, 15);

GWebServer server(80);

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("greenhouse_class_test2");
    delay(10000);
    server.begin(mac, ip, &greenhouse);
    
    GreenhouseInitPins pins;
    pins.LedLampLeft1_1 = 22;
    pins.LedLampLeft1_2 = 23;
    pins.LedLampLeft2_1 = 24;
    pins.LedLampLeft2_2 = 25;
    pins.LedLampRight1_1 = 28;
    pins.LedLampRight1_2 = 29;
    pins.LedLampRight2_1 = 26;
    pins.LedLampRight2_2 = 27;
    pins.DnatPower = 34;
    pins.DnatCool = 35;
    pins.TubeFanPower = 31;
    pins.TubeFanSwitch = 30;
    pins.BlowFanPowerLeft = 33;
    pins.BlowFanPowerRight = 33;
    pins.EmergencyLamp = 36;
    pins.SystemSwitchOff = 37;
    pins.SystemSwitchOn = 40;
    pins.WaterPumpTank1Pin = 38;
    pins.WaterPumpTank2Pin = 38;
    pins.GreenWarnLed = 44;
    pins.YellowWarnLed = 45;
    pins.RedWarnLed = 47;
    pins.Humidifier = 41;
    pins.DCAdapterCooler = 39;
    pins.SoilMoistureAnalog = 2;
    pins.ArduinoReset = 0;
    pins.GsmResetPin = 32;
    greenhouse.init(pins);
}

void checkCommandFromSerial()
{
    int l = Serial.available();
    if(l > 0)
    {
        char incomingBytes[32];
        Serial.readBytes(incomingBytes, l);
        if(incomingBytes[0] == '/')
        {
            String command;
            int k = 1;
            char smb;
            for(;k < l;k++)
            {
                smb = incomingBytes[k];
                if(smb >= 'A' && smb <= 'Z' || smb >= 'a' && smb <= 'z' || smb == '_')
                {
                    command += smb;
                }
                else
                {
                    break;
                }
            }
            const int maxParamCount = 5;
            String params[maxParamCount];
            int paramCount = 0;
            String param;
            for(;k < l;k++)
            {
                smb = incomingBytes[k];
                if(smb != ' ')
                {
                    param += smb;
                }
                else
                {
                    if(param.length() > 0)
                    {
                        if(paramCount >= maxParamCount)
                            break;
                        params[paramCount++] = param;
                        param = "";
                    }
                }
            }
            if(param.length() > 0 && paramCount < maxParamCount)
            {
                params[paramCount++] = param;
            }
            if(command.length() > 0)
                greenhouse.executeCommand(command, params, paramCount);
        }
    }
}
void loop() {
    // put your main code here, to run repeatedly:
    delay(25);
    server.loop();
    greenhouse.loopGSM();
    if((loopCounter % 4) == 0)
    {
        greenhouse.loopWarnLeds();
    }
    if(loopCounter == 19)
    {
        checkCommandFromSerial();
        greenhouse.loop();
        loopCounter = 0;
    }
    else
    {
        loopCounter++;
    }
}
