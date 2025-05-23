#ifndef _FSWEBSERVER_h_
#define _FSWEBSERVER_h_

#include "Arduino.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#endif
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

#include <SPIFFS.h>
#define INTERNAL_FS SPIFFS

// Comment the 2 above, and uncomment these 2 for FFat (enabled write access and folders)
//#include <FFat.h>
//#define INTERNAL_FS FFat

class FSWebServer : public AsyncWebServer {
public:
  FSWebServer(uint16_t port);
  void begin();
  void loop();
  void sendAlert(String s);
  
  uint32_t _diskFree;
  bool _bDiskSwitch;

protected:
  void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
  
  bool onHttpNotFound(AsyncWebServerRequest *request);
  void onHttpDownload(AsyncWebServerRequest *request);
  bool handleFileRead(String path, AsyncWebServerRequest *request);
  void onHttpFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
};

extern FSWebServer server;

#endif // _FSWEBSERVER_h_
