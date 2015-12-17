//
//  GWebServer.cpp
//  Greenhouse
//
//  Created by Bugslayer on 10.11.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#include "GWebServer.h"

typedef enum{
    dsrTemp = 1,
    dsrConnected = 2,
    dsrMode = 4,
    dsrModeEx = 8,
    dsrSchedule = 16,
    dsrRunSleep = 32,
    dsrStartInSec = 64,
    dsrWaterPortion = 128,
    dsrWaterLost = 256,
    dsrDnatCoolLost = 512
} DeviceSettingRead;

typedef enum{
    msrDates = 1,
    msrConnected = 2,
    msrConnectedTmp = 4
} MainSummaryRead;

typedef enum{
    mcrStealth = 1,
    mcrDate = 2,
    mcrTime = 4,
    mcrDevicesTempControl = 8,
    mcrSystemTempControl = 16,
    mcrTempSensors = 32,
    mcrTempSensorsLinks = 64,
    mcrTempSensorsTemp = 128,
    mcrAutoLinking = 256,
    mcrAutoSwitchOn = 512,
    mcrAutoSwitchOnItv = 1024,
    mcrAutoReset = 2048,
    mcrAutoResetTime = 4096
} MainControlRead;

GWebServer::GWebServer(uint16_t port):server(port)
{
    startPage = "index.htm";
}

void GWebServer::writeOkHeader(EthernetClient& client, String contentType)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: " + contentType);
    client.println("Connection: keep-alive");
    client.println();
}

void GWebServer::setStartPage(String pg)
{
    startPage = pg;
}

void GWebServer::begin(uint8_t* mac, IPAddress ip, GreenhouseControl* greenhouse)
{
    this->greenhouse = greenhouse;
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    sd_file_ok = false;
    if (SD.begin(4))
    {
        Serial.println("SUCCESS - SD card initialized.");
        if (SD.exists(const_cast<char*>(startPage.c_str())))
        {
            Serial.println("SUCCESS - Found index.html file.");
            sd_file_ok = true;
        }
        else
        {
            Serial.println("ERROR - Can't find index.html file!");
        }
    }
    else
    {
        Serial.println("ERROR - SD card initialization failed!");
    }
    Ethernet.begin(mac, ip);
    server.begin();
}

void GWebServer::loop()
{
    EthernetClient client = server.available();
    if(client)
    {
        boolean currentLineIsBlank = true;
        while(client.connected())
        {
            if(client.available())
            {
                char c = client.read();
                if (HTTP_req.length() < REQ_BUF_SZ)
                {
                    HTTP_req += c;          // save HTTP request character
                }
                if(c == '\n' && currentLineIsBlank)
                {
                    HTTP_req_header = (HTTP_req.length() >= 50 ? HTTP_req.substring(0, 50) : HTTP_req);
                    // send web page
                    if(HTTP_req_header.indexOf("GET / ") >= 0 || HTTP_req_header.indexOf("GET /" + startPage) >= 0)
                    {
                        if(sd_file_ok)
                        {
                            writeOkHeader(client, "text/html");
                            File webFile = SD.open(const_cast<char*>(startPage.c_str()));        // open web page file
                            if (webFile) {
                                while(webFile.available()) {
                                    client.write(webFile.read()); // send web page to client
                                }
                                webFile.close();
                            }
                        }
                        else
                        {
                            client.println("HTTP/1.1 204 Can't load index.htm");
                        }
                    }
                    else if(HTTP_req_header.indexOf("GET /ajax_") >= 0)
                    {
                        if(HTTP_req_header.indexOf("ajax_loadpagecontent") >= 0)
                        {
                            xmlLoadPageContentResponse(client, HTTP_req);
                        }
                        else if(HTTP_req_header.indexOf("ajax_loadsyssensorvalues") >= 0)
                        {
                            xmlLoadSysSensorValuesResponse(client);
                        }
                        else if(HTTP_req_header.indexOf("ajax_loaddevicesettings") >= 0)
                        {
                            writeOkHeader(client, "text/xml");
                            client.print("<?xml version = \"1.0\" ?>");
                            client.print("<data>");
                            xmlLoadDeviceSettingsResponse(client, HTTP_req);
                            client.print("</data>");
                        }
                        else if(HTTP_req_header.indexOf("ajax_loadmainsummary") >= 0)
                        {
                            xmlLoadMainSummaryResponse(client, HTTP_req);
                        }
                        else if(HTTP_req_header.indexOf("ajax_getsystemtime") >= 0)
                        {
                            txtGetSystemTimeResponse(client);
                        }
                        else if(HTTP_req_header.indexOf("ajax_loadmaincontrol") >= 0)
                        {
                            xmlLoadMainControlResponse(client, HTTP_req);
                        }
                        else if(HTTP_req_header.indexOf("ajax_gsmcmdsend") >= 0)
                        {
                            xmlGsmCmdSendResponse(client, HTTP_req);
                        }
                        else if(HTTP_req_header.indexOf("ajax_gsmcheckoutput") >= 0)
                        {
                            xmlGsmCheckOutputResponse(client);
                        }
                        else
                        {
                            client.println("HTTP/1.1 501 Unsupportable ajax request");
                        }
                    }
                    else if(HTTP_req_header.indexOf("GET /favicon.ico") >= 0)
                    {
                        if(SD.exists("favicon.ico"))
                        {
                            writeOkHeader(client, "image/vnd.microsoft.icon");
                            File webFile = SD.open("favicon.ico");
                            if (webFile) {
                                while(webFile.available()) {
                                    client.write(webFile.read()); // send web page to client
                                }
                                webFile.close();
                            }
                        }
                        else
                        {
                            client.println("HTTP/1.1 404 File not found");
                        }
                    }
                    else if(HTTP_req_header.indexOf("GET /gauge.js") >= 0)
                    {
                        if(SD.exists("gauge.js"))
                        {
                            writeOkHeader(client, "application/javascript");
                            File webFile = SD.open("gauge.js");
                            if (webFile) {
                                while(webFile.available()) {
                                    client.write(webFile.read()); // send web page to client
                                }
                                webFile.close();
                            }
                        }
                        else
                        {
                            Serial.println("file gauge.js not found!");
                            client.println("HTTP/1.1 404 File not found");
                        }
                    }
                    else
                    {
                        client.println("HTTP/1.1 501 Unsupportable server request");
                    }
                    //Serial.println(HTTP_req);
                    HTTP_req = "";
                    break;
                }
                if (c == '\n')
                {
                    currentLineIsBlank = true;
                }
                else if (c != '\r')
                {
                    currentLineIsBlank = false;
                }
            }
        }
        delay(1);
        client.stop();
    }
}

String GWebServer::getInputParamValue(String& HTTP_req, String& param)
{
    int pos = HTTP_req.indexOf("&" + param + "=");
    if(pos >= 0)
    {
        int end_pos = HTTP_req.indexOf("&", pos + param.length() + 2);
        if(end_pos >= 0)
        {
            String s = HTTP_req.substring(pos + param.length() + 2, end_pos);
            s.replace("%22", "\"");
            s.replace("%20", " ");
            return s;
        }
    }
    return "";
}

void GWebServer::xmlLoadPageContentResponse(EthernetClient& client, String& HTTP_req)
{
    String param = "page";
    String paramValue = getInputParamValue(HTTP_req, param);
    int page = paramValue.toInt();
    writeOkHeader(client, "text/xml");
    client.print("<?xml version = \"1.0\" ?>");
    client.print("<data>");
    client.print("<" + param + ">");
    client.print(page);
    client.print("</" + param + ">");
    client.print("<sidebar><![CDATA[");
    switch (page)
    {
        case 0:
            if(SD.exists("sb1.xml"))
            {
                File webFile = SD.open("sb1.xml");
                if (webFile) {
                    while(webFile.available()) {
                        client.write(webFile.read()); // send web page to client
                    }
                    webFile.close();
                }
            }
            else
            {
                client.print("File sb1.xml not exists!");
            }
            break;
        case 1:
            xmlLoadPageContentMakeDevicesSideBar(client);
            break;
        case 2:
            if(SD.exists("sb3.xml"))
            {
                File webFile = SD.open("sb3.xml");
                if (webFile) {
                    while(webFile.available()) {
                        client.write(webFile.read()); // send web page to client
                    }
                    webFile.close();
                }
            }
            else
            {
                client.print("File sb3.xml not exists!");
            }
            break;
    }
    client.print("]]></sidebar>");
    client.print("<content><![CDATA[");
    switch (page)
    {
        case 0:
            if(SD.exists("con1.xml"))
            {
                File webFile = SD.open("con1.xml");
                if (webFile) {
                    while(webFile.available()) {
                        client.write(webFile.read()); // send web page to client
                    }
                    webFile.close();
                }
            }
            else
            {
                client.print("File con1.xml not exists!");
            }
            break;
        case 1:
            if(SD.exists("con2.xml"))
            {
                File webFile = SD.open("con2.xml");
                if (webFile) {
                    while(webFile.available()) {
                        client.write(webFile.read()); // send web page to client
                    }
                    webFile.close();
                }
            }
            else
            {
                client.print("File con2.xml not exists!");
            }
            break;
        case 2:
            if(SD.exists("con3.xml"))
            {
                File webFile = SD.open("con3.xml");
                if (webFile) {
                    while(webFile.available()) {
                        client.write(webFile.read()); // send web page to client
                    }
                    webFile.close();
                }
            }
            else
            {
                client.print("File con3.xml not exists!");
            }
            break;
    }
    client.print("]]></content>");
    client.print("</data>");
}

void GWebServer::xmlLoadPageContentMakeDevicesSideBar(EthernetClient& client)
{
    client.print("<dl>");
    String s1 = "<dt id=\"dev";
    String s2 = "\"><a href=\"javascript:\" onclick=\"deviceSideBarClick(";
    String s3 = ")\">";
    String s4 = "<a></dt>";
    ParamStringReader response(greenhouse->executeCommand(0, "devcnt?"));
    int dev_cnt = response.getCurrentParam() ? response.getCurrentParam()->value.toInt() : 0;
    for(int i=1;i <= dev_cnt;i++)
    {
        int id = i;
        client.print(s1);
        client.print(id);
        client.print(s2);
        client.print(id);
        client.print(s3);
        response.init(greenhouse->executeCommand(id, "name?"));
        client.print(response.getCurrentParam() ? response.getCurrentParam()->value : "");
        client.print(s4);
    }
    client.print("</dl>");
}

void GWebServer::xmlLoadSysSensorValuesResponse(EthernetClient& client)
{
    writeOkHeader(client, "text/xml");
    client.print("<?xml version = \"1.0\" ?>");
    client.print("<data>");
    ParamStringReader response(greenhouse->executeCommand(0, "systemp?;devtemp?;humidity?;soil?"));
    ParamStringElement* param = response.getCurrentParam();
    while (param) {
        client.print("<" + param->name + ">" + param->value + "</" + param->name + ">");
        response.next();
        param = response.getCurrentParam();
    }
    client.print("</data>");
}

void GWebServer::xmlLoadDeviceSettingsResponse(EthernetClient& client, String& HTTP_req)
{
    int dev_id = -1;
    String paramValue;
    String param;
    param = "dev";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        dev_id = paramValue.toInt();
    }
    ParamStringReader getReader(greenhouse->executeCommand(dev_id, "exists?"));
    if(!getReader.getCurrentParam() || getReader.getCurrentParam()->value != "1"){
        client.print("<message>error accessing to device " + paramValue + "</message>");
        return;
    }
    
    uint32_t flag = 0;
    param = "flag";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        flag = paramValue.toInt();
    }
    client.print("<flag>");
    client.print(flag);
    client.print("</flag>");
    client.print("<id>");
    client.print(dev_id);
    client.print("</id>");
    
    ParamStringReader setReader("name?;type?");
    
    if((flag & dsrTemp) > 0){
        setReader.addQueryParam("temp");
    }
    
    param = "connected";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
    }
    if((flag & dsrConnected) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "mode";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
    }
    if((flag & dsrMode) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "modeex";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
    }
    if((flag & dsrModeEx) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "shedulestart";
    paramValue = getInputParamValue(HTTP_req, param);
    String param2 = "sheduleend";
    String param2Value = getInputParamValue(HTTP_req, param2);
    if(paramValue.length() > 0 && param2Value.length() > 0){
        setReader.addParam(param);
        setReader.addParam(param2);
    }
    if((flag & dsrSchedule) > 0){
        setReader.addQueryParam(param);
        setReader.addQueryParam(param2);
    }

    param = "runsleepitv";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
    }
    if((flag & dsrRunSleep) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "runcmd";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(paramValue);
    }

    param = "waterreferencedate";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0 ){
        setReader.addParam(param, paramValue);
    }
    if((flag & dsrRunSleep) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "nextwateringdate";
    if((flag & dsrRunSleep) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "waterportion";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
    }
    if((flag & dsrWaterPortion) > 0){
        setReader.addQueryParam(param);
    }

    param = "waterlost";
    if((flag & dsrWaterLost) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "dnatcoollost";
    if((flag & dsrDnatCoolLost) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "startinsec";
    if((flag & dsrStartInSec) > 0){
        setReader.addQueryParam(param);
    }
    
    getReader.init(greenhouse->executeCommand(dev_id, setReader.getParamString()));
    
#ifdef __SRV_DEBUG
    Serial.println("xmlLoadDeviceSettingsResponse:");
    Serial.println("input:" + setReader.getParamString());
    Serial.println("output:" + getReader.getParamString());
#endif
    
    ParamStringElement* paramEl = getReader.getCurrentParam();
    while (paramEl) {
        client.print("<" + paramEl->name + ">" + paramEl->value + "</" + paramEl->name + ">");
        getReader.next();
        paramEl = getReader.getCurrentParam();
    }
}

void GWebServer::xmlLoadMainSummaryResponse(EthernetClient& client, String& HTTP_req)
{
    writeOkHeader(client, "text/xml");
    client.print("<?xml version = \"1.0\" ?>");
    client.print("<data>");
    String paramValue;
    String param;
    param = "flag";
    paramValue = getInputParamValue(HTTP_req, param);
    int flag = 0;
    if(paramValue.length() > 0){
        flag = paramValue.toInt();
    }
    client.print("<" + param + ">");
    client.print(flag);
    client.print("</" + param + ">");
    ParamStringReader setReader("");
    if((flag & msrDates) > 0){
        setReader.addQueryParam("curdate");
        setReader.addQueryParam("startdate");
    }
    ParamStringReader getReader(greenhouse->executeCommand(0, setReader.getParamString()));
    param = "curdate";
    client.print("<" + param + ">" + getReader.getParamValue(param) + "</" + param + ">");
    param = "startdate";
    client.print("<" + param + ">" + getReader.getParamValue(param) + "</" + param + ">");
    client.print("<main_dev_cons>");
    if((flag & (msrConnected|msrConnectedTmp)) > 0)
    {
        setReader.init("");
        if((flag & msrConnectedTmp) > 0){
            setReader.addQueryParam("temp");
        }
        if((flag & msrConnected) > 0){
            setReader.addQueryParam("name");
        }
        param = "condevices";
        getReader.init(greenhouse->executeCommand(0, param + "?"));
        paramValue = getReader.getParamValue(param);
        ParamArrayReader conReader(paramValue);
        while(conReader.hasParam())
        {
            int id = conReader.getParam().toInt();
            client.print("<main_dev_con id=\"" + String(id) + "\"");
            getReader.init(greenhouse->executeCommand(id, setReader.getParamString()));
            param = "temp";
            paramValue = getReader.getParamValue(param);
            if(paramValue.length() > 0){
                client.print(" temp=\"" + paramValue + "\"");
            }
            client.print(">");
            param = "name";
            client.print(getReader.getParamValue(param));
            client.print("</main_dev_con>");
            conReader.next();
        }
    }
    client.print("</main_dev_cons>");
    client.print("</data>");
}

void GWebServer::txtGetSystemTimeResponse(EthernetClient& client)
{
    writeOkHeader(client, "text/plain");
    String param = "curdate";
    ParamStringReader getReader(greenhouse->executeCommand(0, param + "?"));
    client.print(getReader.getParamValue(param));
}

void GWebServer::xmlLoadMainControlResponse(EthernetClient& client, String& HTTP_req)
{
    writeOkHeader(client, "text/xml");
    client.print("<?xml version = \"1.0\" ?>");
    client.print("<data>");
    
    String paramValue;
    String param;
    param = "flag";
    paramValue = getInputParamValue(HTTP_req, param);
    uint32_t flag = 0;
    if(paramValue.length() > 0){
        flag = paramValue.toInt();
    }
    
    bool bSaveSelfSettings = false;
    ParamStringReader setReader("");
    param = "stealth";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrStealth) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "date";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
    }
    
    param = "time";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
    }
    
    param = "devwarntemp";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrDevicesTempControl) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "devextremetemp";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrDevicesTempControl) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "devoverheattemp";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrDevicesTempControl) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "devshutdowntemp";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrDevicesTempControl) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "syswarntemp";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrSystemTempControl) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "sysshutdowntemp";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrSystemTempControl) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "sysmaxhum";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrSystemTempControl) > 0){
        setReader.addQueryParam(param);
    }
    
    ParamStringReader getReader(greenhouse->executeCommand(0, setReader.getParamString()));
#ifdef __SRV_DEBUG
    Serial.println("xmlLoadMainControlResponse1:");
    Serial.println("input:" + setReader.getParamString());
    Serial.println("output:" + getReader.getParamString());
#endif
    
    ParamStringElement* paramEl = getReader.getCurrentParam();
    while (paramEl) {
        client.print("<" + paramEl->name + ">" + paramEl->value + "</" + paramEl->name + ">");
        getReader.next();
        paramEl = getReader.getCurrentParam();
    }
    
    setReader.init("");
    getReader.init("");
    
    param = "senslink";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam("tempsensorlink", paramValue);
    }

    param = "runcmd";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(paramValue);
    }
    
    String checkAutolinking;
    param = "autolinking";
    if((flag & mcrAutoLinking) > 0){
        setReader.addQueryParam(param);
        param = "links";
        checkAutolinking = getInputParamValue(HTTP_req, param);
        if(checkAutolinking.length() > 0)
            setReader.addQueryParam("tempsensorlink");
    }
    
    if((flag & (mcrTempSensors|mcrTempSensorsLinks|mcrTempSensorsTemp)) > 0){
        if(checkAutolinking.length() == 0)
            setReader.addQueryParam("tempsensorlink");
        if((flag & mcrTempSensorsTemp) > 0)
            setReader.addQueryParam("tempsensortemp");
    }
    
    param = "autoswitchon";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrAutoSwitchOn) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "autoswitchonitv";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrAutoSwitchOnItv) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "autoreset";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrAutoReset) > 0){
        setReader.addQueryParam(param);
    }
    
    param = "autoresettime";
    paramValue = getInputParamValue(HTTP_req, param);
    if(paramValue.length() > 0){
        setReader.addParam(param, paramValue);
        bSaveSelfSettings = true;
    }
    if((flag & mcrAutoResetTime) > 0){
        setReader.addQueryParam(param);
    }

    if(bSaveSelfSettings){
        setReader.addParam("savetomem", "self");
    }
    
    getReader.init(greenhouse->executeCommand(0, setReader.getParamString()));
    
#ifdef __SRV_DEBUG
    Serial.println("xmlLoadMainControlResponse2:");
    Serial.println("input:" + setReader.getParamString());
    Serial.println("output:" + getReader.getParamString());
#endif
    
    paramEl = getReader.getCurrentParam();
    while (paramEl) {
        if(!(paramEl->name == "tempsensorlink" || paramEl->name == "tempsensortemp"))
            client.print("<" + paramEl->name + ">" + paramEl->value + "</" + paramEl->name + ">");
        getReader.next();
        paramEl = getReader.getCurrentParam();
    }
    
    String param2;
    String param2Value;
    
    if((flag & (mcrTempSensors|mcrTempSensorsLinks|mcrTempSensorsTemp)) > 0){
        param = "tempsensorlink";
        paramValue = getReader.getParamValue(param);
        ParamArrayReader linkReader(paramValue);
        if((flag & mcrTempSensorsTemp) > 0){
            param2 = "tempsensortemp";
            param2Value = getReader.getParamValue(param2);
        }
        ParamArrayReader tempReader(param2Value);
        
        while (linkReader.hasParam()) {
            PairFromString link(linkReader.getParam());
            client.print("<" + param + " idx=\"" + link.First + "\"");
            if((flag & mcrTempSensorsTemp) > 0){
                tempReader.flush();
                while (tempReader.hasParam()) {
                    PairFromString temp(tempReader.getParam());
                    if(temp.First == link.First){
                        client.print(" tmp=\"" + temp.Second + "\"");
                        break;
                    }
                    tempReader.next();
                }
            }
            client.print(">");
            if((flag & mcrTempSensorsLinks) > 0){
                client.print(link.Second);
            }
            client.print("</" + param + ">");
            linkReader.next();
        }
    }
    if(checkAutolinking.length() > 0){
        bool bLinksChanged = false;
        param = "tempsensorlink";
        paramValue = getReader.getParamValue(param);
        ParamArrayReader linkReader(paramValue);
        ParamArrayReader checkReader(checkAutolinking);
        while (linkReader.hasParam()) {
            PairFromString linkPair(linkReader.getParam());
            checkReader.flush();
            while (checkReader.hasParam()) {
                PairFromString checkPair(checkReader.getParam());
                if(linkPair.First == checkPair.First){
                    if(linkPair.Second != checkPair.Second){
                        bLinksChanged = true;
                        client.print("<" + param + " idx=\"" + linkPair.First + "\">");
                        client.print(linkPair.Second);
                        client.print("</" + param + ">");
                    }
                    break;
                }
            }
            linkReader.next();
        }
        if(bLinksChanged)
            flag |= mcrTempSensorsLinks;
    }
    param = "flag";
    client.print("<" + param + ">");
    client.print(flag);
    client.print("</" + param + ">");
    client.print("</data>");
}

void GWebServer::xmlGsmCmdSendResponse(EthernetClient& client, String& HTTP_req){
    writeOkHeader(client, "text/plain");
    String param("cmd");
    String paramValue = getInputParamValue(HTTP_req, param);
    ParamStringReader setReader("");
    if (paramValue.length() > 0) {
        setReader.addParam(param, paramValue);
    }
    param = "atcmd";
    paramValue = getInputParamValue(HTTP_req, param);
    if (paramValue.length() > 0) {
        setReader.addParam(param, paramValue);
    }
    greenhouse->executeCommand(-1, setReader.getParamString());
}

void GWebServer::xmlGsmCheckOutputResponse(EthernetClient& client){
    writeOkHeader(client, "text/xml");
    client.print("<?xml version = \"1.0\" ?>");
    client.print("<data>");
    ParamStringReader getReader(greenhouse->executeCommand(-1, "state?;balance?;smspacket?"));
    ParamStringElement* paramEl = getReader.getCurrentParam();
    while (paramEl) {
        client.print("<" + paramEl->name + ">" + paramEl->value + "</" + paramEl->name + ">");
        getReader.next();
        paramEl = getReader.getCurrentParam();
    }
    client.print("<output>");
    String res = greenhouse->executeCommand(-1, "output?");
    int pos = res.indexOf("output=");
    if (pos >= 0) {
        client.print(res.substring(pos + 7, res.length()));
    }
    client.print("</output>");
    client.print("</data>");
}