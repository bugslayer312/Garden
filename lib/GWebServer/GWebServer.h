//
//  GWebServer.h
//  Greenhouse
//
//  Created by Bugslayer on 10.11.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#ifndef __Greenhouse__GWebServer__
#define __Greenhouse__GWebServer__

#include <Ethernet.h>
#include <SD.h>
#include <GUtil.h>

#define REQ_BUF_SZ 1024

//#define __SRV_DEBUG

class GWebServer
{
    EthernetServer server;
    String startPage;
    bool sd_file_ok = false;
    String HTTP_req;
    String HTTP_req_header;
    GreenhouseControl* greenhouse;

private:
    void writeOkHeader(EthernetClient& client, String contentType);
    String getInputParamValue(String& HTTP_req, String& param);
    void xmlLoadPageContentResponse(EthernetClient& client, String& HTTP_req);
    void xmlLoadPageContentMakeDevicesSideBar(EthernetClient& client);
    void xmlLoadSysSensorValuesResponse(EthernetClient& client);
    void xmlLoadDeviceSettingsResponse(EthernetClient& client, String& HTTP_req);
    void xmlLoadMainSummaryResponse(EthernetClient& client, String& HTTP_req);
    void txtGetSystemTimeResponse(EthernetClient& client);
    void xmlLoadMainControlResponse(EthernetClient& client, String& HTTP_req);
    void xmlGsmCmdSendResponse(EthernetClient& client, String& HTTP_req);
    void xmlGsmCheckOutputResponse(EthernetClient& client);
public:
    GWebServer(uint16_t port);
    void setStartPage(String pg);
    void begin(uint8_t* mac, IPAddress ip, GardenControl* garden);
    void loop();
};

#endif /* defined(__Greenhouse__GWebServer__) */
