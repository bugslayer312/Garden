//
//  GSMService.cpp
//  Greenhouse
//
//  Created by Bugslayer on 10.11.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#include "GSMService.h"

GSMService::GSMService(uint8_t rx, uint8_t tx):serial(rx, tx)
{
    
}

bool GSMService::getResetting()
{
    return (nFlags & 0x00000001) > 0;
}

void GSMService::setResetting(bool value)
{
    if(value)
        nFlags |= 0x00000001;
    else
        nFlags &= 0xFFFFFFFE;
}

bool GSMService::getInitializedSerial(){
    return (nFlags & 0x00000002) > 0;
}

void GSMService::setInitializedSerial(bool value){
    if(value)
        nFlags |= 0x00000002;
    else
        nFlags &= 0xFFFFFFFD;
}

bool GSMService::getStartupProcessing(){
    return (nFlags & 0x00000004) > 0;
}

void GSMService::setStartupProcessing(bool value){
    if(value)
        nFlags |= 0x00000004;
    else
        nFlags &= 0xFFFFFFFB;
}

bool GSMService::getStartupReady(){
    return (nFlags & 0x00000008) > 0;
}

void GSMService::setStartupReady(bool value){
    if(value)
        nFlags |= 0x00000008;
    else
        nFlags &= 0xFFFFFFF7;
}

bool GSMService::getSignalQualityOk(){
    return (nFlags & 0x00000010) > 0;
}

void GSMService::setSignalQualityOk(bool value){
    if(value)
        nFlags |= 0x00000010;
    else
        nFlags &= 0xFFFFFFEF;
}

bool GSMService::getGsmRegistrationOk(){
    return (nFlags & 0x00000020) > 0;
}

void GSMService::setGsmRegistrationOk(bool value){
    if(value)
        nFlags |= 0x00000020;
    else
        nFlags &= 0xFFFFFFDF;
}

bool GSMService::getNetworkOk(){
    return (nFlags & 0x00000030) == 0x00000030;
}

bool GSMService::getCallIdentification(){
    return (nFlags & 0x00000040) > 0;
}

void GSMService::setCallIdentification(bool value){
    if(value)
        nFlags |= 0x00000040;
    else
        nFlags &= 0xFFFFFFBF;
}

bool GSMService::getChoseSmsService(){
    return (nFlags & 0x00000080) > 0;
}

void GSMService::setChoseSmsService(bool value){
    if(value)
        nFlags |= 0x00000080;
    else
        nFlags &= 0xFFFFFF7F;
}

bool GSMService::getSmsIndication(){
    return (nFlags & 0x00000100) > 0;
}

void GSMService::setSmsIndication(bool value){
    if(value)
        nFlags |= 0x00000100;
    else
        nFlags &= 0xFFFFFEFF;
}

bool GSMService::getSmsMode(){
    return (nFlags & 0x00000200) > 0;
}

void GSMService::setSmsMode(bool value){
    if(value)
        nFlags |= 0x00000200;
    else
        nFlags &= 0xFFFFFDFF;
}

bool GSMService::getCharacterSet(){
    return (nFlags & 0x00000400) > 0;
}

void GSMService::setCharacterSet(bool value){
    if(value)
        nFlags |= 0x00000400;
    else
        nFlags &= 0xFFFFFBFF;
}

bool GSMService::getInitParams(){
    return (nFlags & 0x000007C0) == 0x000007C0;
}

bool GSMService::getAskSmsPacketLost(){
    return (nFlags & 0x00000800) > 0;
}

void GSMService::setAskSmsPacketLost(bool value){
    if(value)
        nFlags |= 0x00000800;
    else
        nFlags &= 0xFFFFF7FF;
}

bool GSMService::getNeedAskSmsPacketLost(){
    return nAskSmsPacketLost == 0;
}

void GSMService::setNeedAskSmsPacketLost(){
    nAskSmsPacketLost = 0;
}

bool GSMService::getAskBalance(){
    return (nFlags & 0x00001000) > 0;
}

void GSMService::setAskBalance(bool value){
    if(value)
        nFlags |= 0x00001000;
    else
        nFlags &= 0xFFFFEFFF;
}

bool GSMService::getNeedAskBalance(){
    return nAskBalance == 0;
}

void GSMService::setNeedAskBalance(){
    nAskBalance = 0;
}

void GSMService::askBalance(){
    gsmDebugOutputln("Ask balance");
    askBalanceCount++;
    fBalance = 0.0f;
    nAskBalance = millis();
    setAskBalance(true);
    serial.println("AT+CUSD=1,\"*100#\"");
}

void GSMService::askSmsPackedLost(){
    gsmDebugOutputln("Ask SMS packet lost");
    askSmsPacketLostCount++;
    nSmsPacketLost = 0;
    nAskSmsPacketLost = millis();
    setAskSmsPacketLost(true);
    serial.println("AT+CUSD=1,\"*100*2#\"");
}

void GSMService::initParams(){
    if(!getStartupReady())
        return;
    gsmDebugOutputln("Init params start...");
    String response;
    if (!getCallIdentification()) {
        gsmDebugOutputln("AT+CLIP=1 start...");
        if(executeATCommand("AT+CLIP=1", response, 2000, 2) == atOk){
            setCallIdentification(true);
            gsmDebugOutputln("AT+CLIP=1 OK");
        }
    }
    if (!getChoseSmsService()) {
        gsmDebugOutputln("AT+CSMS=1 start...");
        if(executeATCommand("AT+CSMS=1", response, 2000, 2) == atOk){
            setChoseSmsService(true);
            gsmDebugOutputln("AT+CSMS=1OK");
        }
    }
    if (!getSmsIndication()) {
        gsmDebugOutputln("AT+CNMI=2,2 start...");
        if(executeATCommand("AT+CNMI=2,2", response, 2000, 2) == atOk){
            setSmsIndication(true);
            gsmDebugOutputln("AT+CNMI=2,2 OK");
        }
    }
    if (!getSmsMode()) {
        gsmDebugOutputln("AT+CMGF=1 start...");
        if(executeATCommand("AT+CMGF=1", response, 2000, 2) == atOk){
            setSmsMode(true);
            gsmDebugOutputln("AT+CMGF=1 OK");
        }
    }
    if (!getCharacterSet()) {
        gsmDebugOutputln("AT+CSCS=\"GSM\" start...");
        if(executeATCommand("AT+CSCS=\"GSM\"", response, 2000, 2) == atOk){
            setCharacterSet(true);
            gsmDebugOutputln("AT+CSCS=\"GSM\" OK");
        }
    }
    gsmDebugOutputln("Init params end.");
}

void GSMService::diagnose(){
    if(!getStartupReady())
        return;

    gsmDebugOutputln("Diagnose start...");
    String response;
    int param;
    int value;
    if (executeATCommand("AT+CSQ", response, 2000, 3) == atOk) {
        ParamArrayReader reader(response);
        param = 0;
        while (reader.hasParam() || param < 2) {
            value = reader.getParam().toInt();
            if (param == 0) {
                if(value < 4 || value == 99)
                    break;
            }
            else {
                if(value >= 0 && value <= 7){
                    setSignalQualityOk(true);
                }
            }
            reader.next();
            param++;
        }
    }
    if (executeATCommand("AT+CREG?", response, 2000, 3) == atOk) {
        ParamArrayReader reader(response);
        param = 0;
        while (reader.hasParam() || param < 2) {
            value = reader.getParam().toInt();
            if (param == 1) {
                if(reader.getParam().toInt() == 1){
                    setGsmRegistrationOk(true);
                }
            }
            reader.next();
            param++;
        }
    }
    gsmDebugOutputln("Diagnose end: " + String(getNetworkOk()));
}

void GSMService::init(uint8_t resetPin)
{
    this->resetPin = resetPin;
    pinMode(this->resetPin, OUTPUT);
    digitalWrite(this->resetPin, HIGH);
    reset();
}

void GSMService::initSerial()
{
    serial.begin(19200);
    delay(100);
    setInitializedSerial(true);
}

void GSMService::outputPrintln(String txt){
    if (output.length() + txt.length() > 200) {
        output = "";
    }
    if(output.length() > 0){
        output += "\n";
    }
    output += txt;
}

bool GSMService::watingFor(String response, uint32_t timeout)
{
    uint32_t start = millis();
    while (!serial.available()) {
        if (millis() > start + timeout) {
            return false;
        }
    }
    String val;
    while (serial.available()) {
        val += (char)serial.read();
        if (val.indexOf(response) >= 0) {
            return true;
        }
        if (millis() > start + timeout) {
            break;
        }
    }
    return false;
}

void GSMService::clearSerial(){
    while (serial.available()) {
        serial.read();
    }
}

atResult GSMService::executeATCommand(String cmd, String& result, uint32_t timeout, int trycnt){
    atResult res = atTimeout;
    for (int i=0;i < trycnt; i++) {
        res = executeATCommandInt(cmd, result, timeout);
        if (res == atOk) {
            break;
        }
        result = "";
    }
    return res;
}

atResult GSMService::executeATCommandInt(String cmd, String& result, uint32_t timeout){
    outputPrintln(cmd);
    serial.println(cmd);
    uint32_t start = millis();
    delay(50);
    String response;
    while (serial.available()) {
        response += (char)serial.read();
        if(millis() > start + timeout){
            outputPrintln("AT_TIMEOUT");
            return atTimeout;
        }
    }
    outputPrintln(response);
    if (response.lastIndexOf("OK") < 0) {
        return atError;
    }
    if(cmd == "AT+CSQ"){
        parseResponseResult("CSQ", response, result);
        gsmDebugOutputln("CSQ: " + result);
    }
    else if(cmd == "AT+CREG?"){
        parseResponseResult("CREG", response, result);
        gsmDebugOutputln("CREG: " + result);
    }
    else if(cmd.indexOf("CUSD") >= 0){
        parseResponseResult("CUSD", response, result);
        gsmDebugOutputln("CUSD: " + result);
    }
    
    return atOk;
}

void GSMService::parseResponseResult(String cmd, String& response, String& result){
    int startpos = response.indexOf("+" + cmd + ":");
    if(startpos > 0){
        int findStart = startpos + cmd.length() + 2;
        int endpos = response.indexOf("\r\n", findStart) + 1;
        if (endpos <= 0) {
            endpos = response.length();
        }
        result = response.substring(findStart, endpos);
        result.trim();
    }
}

void GSMService::loop()
{
    if(getResetting()){
        if (millis() > nResetStart + 5000) {
            digitalWrite(resetPin, HIGH);
            setResetting(false);
            initSerial();
            gsmDebugOutputln("End reset.");
        }
        else{
            return;
        }
    }
    if(!getInitializedSerial()){
        return;
    }
    if(getStartupReady()){
        if (!getNetworkOk() && diagnoseCount < 3) {
            diagnose();
            diagnoseCount++;
        }
        if (getNetworkOk() && !getInitParams() && initParamsCount < 3) {
            initParams();
            initParamsCount++;
        }
        if (getInitParams()){
            if (getNeedAskSmsPacketLost() && !getAskBalance()) {
                askSmsPackedLost();
            }
            if (getAskSmsPacketLost()) {
                if (millis() > nAskSmsPacketLost + 5000) {
                    setAskSmsPacketLost(false);
                    gsmDebugOutputln("getAskSmsPacketLost timeout");
                    if (askSmsPacketLostCount < 3) {
                        setNeedAskSmsPacketLost();
                    }
                }
            }
            if(getNeedAskBalance() && !getAskSmsPacketLost()){
                askBalance();
            }
            if (getAskBalance()) {
                if (millis() > nAskBalance + 10000) {
                    setAskBalance(false);
                    gsmDebugOutputln("getAskBalance timeout");
                    if (askBalanceCount < 3) {
                        setNeedAskBalance();
                    }
                }
            }
        }
    }
    if(serial.available()){
        String val;
        while (serial.available()) {
            val += (char) serial.read();
        }
        bool bHandled = false;
        ParamArrayReader input(val, "\n");
        if(!getStartupReady()){
            if(!getStartupProcessing()){
                input.flush();
                val = "MODEM:STARTUP";
                while (input.hasParam()) {
                    if(input.getParam().indexOf(val) >= 0){
                        setStartupProcessing(true);
                        break;
                    }
                    input.next();
                }
            }
            if(getStartupProcessing()){
                input.flush();
                val = "+PBREADY";
                while (input.hasParam()) {
                    if(input.getParam().indexOf(val) >= 0){
                        setStartupReady(true);
                        break;
                    }
                    input.next();
                }
            }
            if(getStartupReady()){
                gsmDebugOutputln("GSM ready!");
                String dummy;
                executeATCommand("ATE0", dummy);
            }
            bHandled = true;
        }

        if (!bHandled && (val.indexOf("+CUSD:") >= 0)) {
            if(getAskSmsPacketLost()){
                int startPos = val.indexOf("0,\"???????: ");
                if (startPos >= 0) {
                    startPos += 12;
                    int endPos = val.indexOf("SMS");
                    if (endPos > startPos) {
                        nSmsPacketLost = val.substring(startPos, endPos).toInt();
                        gsmDebugOutputln("Ask sms packet lost result = " + String(nSmsPacketLost));
                    }
                }
                gsmDebugOutputln("Ask sms packet lost end");
                setAskSmsPacketLost(false);
                bHandled = true;
            }
            if (!bHandled && getAskBalance()) {
                int pos = val.indexOf("0,\"??????:");
                if(pos >= 0){
                    parseBalanceResponse(val, pos + 10);
                    gsmDebugOutputln("Ask balance result = " + String(fBalance));
                    gsmDebugOutputln("Ask balance end");
                    setAskBalance(false);
                }
            }
            bHandled = true;
        }

        if (!bHandled && (val.indexOf("+CMT:") >= 0)) {
            int pos = val.indexOf("\"Balance");
            if (pos >= 0) {
                if (getAskBalance()) {
                    pos = val.indexOf("\r\n?????????:");
                    if (pos >= 0) {
                        parseBalanceResponse(val, pos + 12);
                        gsmDebugOutputln("Ask balance result = " + String(fBalance));
                        gsmDebugOutputln("Ask balance end");
                        setAskBalance(false);
                    }
                }
                bHandled = true;
            }
            
            if (!bHandled) {
                pos = val.indexOf(master);
                if (pos >= 0) {
                    
                }
            }
            bHandled = true;
        }
        gsmDebugOutputln("tst:" + val);
        outputPrintln(val);
    }
}

void GSMService::parseBalanceResponse(String& response, int pos){
    String balance;
    bool bFoundComma = false;
    for (int i=pos; i < response.length(); i++) {
        char smb = response[i];
        if (smb == ' ' && balance.length() == 0) {
            continue;
        }
        if ((smb >= '0' && smb <= '9') ||
            (smb == '-' && balance.length() == 0)){
        }
        else if(smb == ',' && !bFoundComma){
            bFoundComma = true;
            smb = '.';
        }
        else{
            break;
        }
        balance += smb;
    }
    fBalance = balance.toFloat();
}

void GSMService::reset()
{
    if(!getResetting()){
        gsmDebugOutputln("Start reset...");
        nFlags = 0;
        setResetting(true);
        if(getInitializedSerial()){
            serial.end();
        }
        diagnoseCount = 0;
        initParamsCount = 0;
        askSmsPacketLostCount = 0;
        askBalanceCount = 0;
        nResetStart = millis();
        setNeedAskBalance();
        setNeedAskSmsPacketLost();
        digitalWrite(resetPin, LOW);
    }
}

void GSMService::sendsms(String text)
{
    String cmd("AT+CMGS=\"" + master + "\"");
    outputPrintln(cmd);
    serial.println(cmd);
    if(watingFor(">", 2000)){
        outputPrintln(">\r\n" + text);
        serial.print(text);
        serial.print((char)26); // SUB
        if (nSmsPacketLost > 0) {
            nSmsPacketLost--;
        }
        nSentSmsCount++;
        if ((nSentSmsCount % 5) == 0) {
            askSmsPacketLostCount = 0;
            askBalanceCount = 0;
            setNeedAskBalance();
            setNeedAskSmsPacketLost();
        }
    }
    else{
        outputPrintln("AT_TIMEOUT");
        serial.print((char)27); // ESC
    }
}

void GSMService::runatcmd(String cmd)
{
    gsmDebugOutputln("runatcmd: " + cmd + "=>");
    outputPrintln(cmd);
    serial.println(cmd);
}

void GSMService::runcmd(String cmd)
{
    if(cmd == "reset"){
        reset();
    }
    else if(cmd == "diagnose"){
        setSignalQualityOk(false);
        setGsmRegistrationOk(false);
        diagnose();
        diagnoseCount = 1;
    }
    else if(cmd == "initparams"){
        nFlags &= 0xFFFFF83F;
        initParams();
        initParamsCount = 1;
    }
    else if(cmd == "asksmspacketlost"){
        askSmsPacketLostCount = 0;
        setNeedAskSmsPacketLost();
    }
    else if(cmd == "askbalance"){
        askBalanceCount = 0;
        setNeedAskBalance();
    }
}

String GSMService::getOutput()
{
    String res = output;
    output = "";
    return res;
}

int GSMService::getSmsPacketLost(){
    return nSmsPacketLost;
}

float GSMService::getBalance(){
    return fBalance;
}

uint32_t GSMService::getState(){
    return nFlags & gsAll;
}