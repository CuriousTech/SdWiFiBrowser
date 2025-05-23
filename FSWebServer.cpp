#include "FSWebServer.h"
#include "sdControl.h"
#include <SPI.h>
#include <SD.h>
#include <StreamString.h>
#include "serial.h"
#include "network.h"
#include "jsonParse.h"
#include "jsonString.h"

FSWebServer server(80);
AsyncWebSocket ws("/ws");

FSWebServer::FSWebServer(uint16_t port) : AsyncWebServer(port) {}

JsonParse jsonParse;
String ssid;

void wsTime(int32_t nValue, char *pszValue)
{
  timeval tv;
  tv.tv_sec = nValue;
  settimeofday(&tv, NULL);
  SERIAL_ECHOLN("setTime");
}

void wsRelinquish(int32_t nValue, char *pszValue)
{
  sdcontrol.relinquishControl();
}

void wsList(int32_t nValue, char *pszValue)
{
  if(server._bDiskSwitch == false)
  {
    if(sdcontrol.canWeTakeControl() == -1){
      server.sendAlert("Printer controlling the SD card");
      return;
    }
  }

  File dir;
  if(server._bDiskSwitch)
  {
    dir = INTERNAL_FS.open(pszValue);
  }
  else
  {
    sdcontrol.takeControl();
    dir = SD.open(pszValue);
  }

  if(!dir)
  {
    String s = "Can't open ";
    s += pszValue;
    server.sendAlert(s);
    Serial.print("Can't open ");
    Serial.println(pszValue);
  }

  if (!dir.isDirectory()) {
    dir.close();
    server.sendAlert("path not directory");
    Serial.println("path not directory");
    return;
  }

  dir.rewindDirectory();

  String output = "{\"type\":\"filelist\",\"value\":[";
  for (int cnt = 0; true; ++cnt) {
    File entry = dir.openNextFile();
    if (!entry)
      break;
    if (cnt > 0)
      output += ',';
    jsonString jsList;
    jsList.Var("type", (entry.isDirectory()) ? "dir" : "file");
    jsList.Var("name", entry.name() );
    jsList.Var("size", (uint32_t)entry.size() );
    output += jsList.Close();
    entry.close();
  }
  output += "]}";
  dir.close();
  if(server._bDiskSwitch == false)
    sdcontrol.relinquishControl();
  ws.textAll( output );
}

void wsDelete(int32_t nValue, char *pszValue)
{
  SERIAL_ECHO("Delete: "); SERIAL_ECHOLN(pszValue);

  if( pszValue[0] == '/' && pszValue[1] == 0)
    return;

  File file;

  if(server._bDiskSwitch)
  {
    file = INTERNAL_FS.open(pszValue);
SERIAL_ECHOLN("INT");
  }
  else
  {
    if(sdcontrol.canWeTakeControl()== -1)
    {
      server.sendAlert("Printer controlling the SD card");
      return;
    }
    sdcontrol.takeControl();
    file = SD.open(pszValue);
  }

  if(!file) {
    DEBUG_LOG("Open file fail\n");
    return;
  }

  bool isDir = file.isDirectory();
  file.close();

  if(server._bDiskSwitch)
  {
    bool bRes;
    if(isDir)
      bRes = INTERNAL_FS.rmdir(pszValue);
    else
      bRes = INTERNAL_FS.remove(pszValue);
    if(!bRes)
      server.sendAlert("Could not delete file");
      SERIAL_ECHOLN("Deleted");
  }
  else
  {
    if(!sdcontrol.deleteFile(pszValue, isDir))
      server.sendAlert("Could not delete file");
  
    server._diskFree = sdcontrol.getDiskFree();
    sdcontrol.relinquishControl();
  }
}

void wsCreateDir(int32_t nValue, char *pszValue)
{
  Serial.print("CreateDir: ");
  Serial.println(pszValue);

  if( pszValue[0] == '/' && pszValue[1] == 0)
    return;

  File file;

  if(server._bDiskSwitch)
  {
    file = INTERNAL_FS.open(pszValue);
  }
  else
  {
    if(sdcontrol.canWeTakeControl()== -1)
    {
      server.sendAlert("Printer controlling the SD card");
      return;
    }
    sdcontrol.takeControl();
    file = SD.open(pszValue);
  }

  if(file) {
    file.close();
    DEBUG_LOG("File exists\n");
    return;
  }

  if(server._bDiskSwitch)
  {
    if(!INTERNAL_FS.mkdir(pszValue))
      server.sendAlert("Could not create directory");
  }
  else
  {
    if(!sdcontrol.createDir(pszValue))
      server.sendAlert("Could not create directory");
  
    server._diskFree = sdcontrol.getDiskFree();
    sdcontrol.relinquishControl();
  }
}

void wsStartSoftAP(int32_t nValue, char *pszValue)
{
  Serial.println("wsSetSoftAP");
  if(network.isSTAmode())
    network.startSoftAP();
}

void wsSetSSID(int32_t nValue, char *pszValue)
{
  ssid = pszValue;
}

void wsSetPWD(int32_t nValue, char *pszValue)
{
  network.startConnect(ssid, pszValue);
}

void wsScan(int32_t nValue, char *pszValue)
{
  network.doScan();
}

void wsDisk(int32_t nValue, char *pszValue)
{
  bool bNewSwitch = nValue ? true:false;

  if(bNewSwitch != server._bDiskSwitch){
    server._bDiskSwitch = bNewSwitch;
    wsList(0, "/");
  }
}

jsCbFunc jsonFuncs[]{
  {"time", wsTime},
  {"relinquish", wsRelinquish},
  {"list", wsList},
  {"delete", wsDelete},
  {"startSoftAP", wsStartSoftAP},
  {"SSID", wsSetSSID},
  {"PWD", wsSetPWD},
  {"scan", wsScan},
  {"createdir", wsCreateDir},
  {"disk", wsDisk},
  {"", NULL}
};

void FSWebServer::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{ //Handle WebSocket event

  switch (type)
  {
    case WS_EVT_CONNECT:      //client connected
//      client->text( stuff );
      break;
    case WS_EVT_DISCONNECT:    //client disconnected
    case WS_EVT_ERROR:    //error was received from the other end
      _bDiskSwitch = false;
      break;
    case WS_EVT_PONG:    //pong message was received (in response to a ping request maybe)
      break;
    case WS_EVT_DATA:  //data packet
      AwsFrameInfo * info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len) {
        //the whole message is in a single frame and we got all of it's data
        if (info->opcode == WS_TEXT) {
          data[len] = 0;
          jsonParse.process((char *)data, jsonFuncs);
        }
      }
      break;
  }
}

void FSWebServer::sendAlert(String s) {
    jsonString js;
    js.Var("type", "alert");
    js.Var("value", s);
    ws.textAll(js.Close());
}

void FSWebServer::loop() {
  static uint32_t tm;

  if(millis() - tm > 1000) // 1 second keepAlive
  {
    tm = millis();
    jsonString js;
    js.Var("type", "info");
    js.Var("sdfree", _diskFree);
    js.Var("intfree", (uint32_t)((INTERNAL_FS.totalBytes() - INTERNAL_FS.usedBytes()) >> 10) );
    js.Var("wifiStatus", network.status() );
    js.Var("ip", WiFi.localIP().toString() );
    js.Var("disk", _bDiskSwitch);
    ws.textAll(js.Close());

    if(network.hasScan())
    {
      String s;
      network.getWiFiList(s);
      ws.textAll(s);
    }
  }
}

void FSWebServer::begin() {

    sdcontrol.takeControl();
    _diskFree = sdcontrol.getDiskFree();
    sdcontrol.relinquishControl();

    ws.onEvent([this](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
      this->onWsEvent(server, client, type, arg, data, len);
    });
    server.addHandler(&ws);
    AsyncWebServer::begin();

  	server.on("/download", HTTP_GET, [this](AsyncWebServerRequest *request) {
  		this->onHttpDownload(request);
  	});

  	server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) { 
  	  request->send(200, "text/plain", ""); },[this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
		  this->onHttpFileUpload(request, filename, index, data, len, final);
	  });

	  server.onNotFound([this](AsyncWebServerRequest *request) {
      this->onHttpNotFound(request);
    });
}

String getContentType(String filename, AsyncWebServerRequest *request) {
  if (request->hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool FSWebServer::onHttpNotFound(AsyncWebServerRequest *request) {
  String path = request->url();
	DEBUG_LOG("handleNotFound: %s\r\n", path.c_str());

	if (path.endsWith("/"))
		path += "index.htm";

	String contentType = getContentType(path, request);
	String pathWithGz = path + ".gz";
	if (INTERNAL_FS.exists(pathWithGz) || INTERNAL_FS.exists(path)) {
		if (INTERNAL_FS.exists(pathWithGz)) {
			path += ".gz";
		}
		DEBUG_LOG("Content type: %s\r\n", contentType.c_str());
		AsyncWebServerResponse *response = request->beginResponse(INTERNAL_FS, path, contentType);
		if (path.endsWith(".gz"))
			response->addHeader("Content-Encoding", "gzip");
		DEBUG_LOG("File %s exist\r\n", path.c_str());
		request->send(response);
		DEBUG_LOG("File %s Sent\r\n", path.c_str());

		return true;
	}
	else
		DEBUG_LOG("Cannot find %s\n", path.c_str());
	return false;
}

bool FSWebServer::handleFileRead(String path, AsyncWebServerRequest *request) {
	DEBUG_LOG("handleFileRead: %s\r\n", path.c_str());

	if (path.endsWith("/"))
		path += "index.htm";

	String contentType = getContentType(path, request);
	String pathWithGz = path + ".gz";

  if(_bDiskSwitch == false)
  {
  	sdcontrol.takeControl();
  	if (SD.exists(pathWithGz) || SD.exists(path)) {
  		if (SD.exists(pathWithGz)) {
  			path += ".gz";
  		}
  		DEBUG_LOG("Content type: %s\r\n", contentType.c_str());
  		AsyncWebServerResponse *response = request->beginResponse(SD, path, contentType);
  		if (path.endsWith(".gz"))
  			response->addHeader("Content-Encoding", "gzip");
  		DEBUG_LOG("File %s exist\r\n", path.c_str());
  		request->send(response);
  		DEBUG_LOG("File %s Sent\r\n", path.c_str());
  
  		return true;
  	}
  	else
  		DEBUG_LOG("Cannot find %s\n", path.c_str());
  	sdcontrol.relinquishControl();
  	return false;
  }
  else
  {
    if (INTERNAL_FS.exists(pathWithGz) || INTERNAL_FS.exists(path)) {
      if (INTERNAL_FS.exists(pathWithGz)) {
        path += ".gz";
      }
      DEBUG_LOG("Content type: %s\r\n", contentType.c_str());
      AsyncWebServerResponse *response = request->beginResponse(INTERNAL_FS, path, contentType);
      if (path.endsWith(".gz"))
        response->addHeader("Content-Encoding", "gzip");
      DEBUG_LOG("File %s exist\r\n", path.c_str());
      request->send(response);
      DEBUG_LOG("File %s Sent\r\n", path.c_str());
  
      return true;
    }
    else
      DEBUG_LOG("Cannot find %s\n", path.c_str());
    return false;
  }
}

void FSWebServer::onHttpDownload(AsyncWebServerRequest *request) {
    DEBUG_LOG("onHttpDownload");

    if(_bDiskSwitch == false)
      if(sdcontrol.canWeTakeControl() == -1)
      {
        DEBUG_LOG("Printer controlling the SD card"); 
        request->send(500, "text/plain","DOWNLOAD:SDBUSY");
        return;
      }
  
    int params = request->params();
    if (params == 0) {
      DEBUG_LOG("No params");
      request->send(500, "text/plain","DOWNLOAD:BADARGS");
      return;
    }
    const AsyncWebParameter* p = request->getParam((uint8_t)0);
    String path = p->value();

    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    if (!this->handleFileRead(path, request))
      request->send(404, "text/plain", "DOWNLOAD:FileNotFound");
    delete response; // Free up memory!
}

void FSWebServer::onHttpFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){

  if(_bDiskSwitch)
  {
    if (!index) { // start
      if(request->_tempFile){
          request->_tempFile.close();
      }
  
      if (INTERNAL_FS.exists((char *)filename.c_str())) {
        INTERNAL_FS.remove((char *)filename.c_str());
      }
  
      request->_tempFile = INTERNAL_FS.open(filename.c_str(), FILE_WRITE);
      if(!request->_tempFile) {
        request->send(500, "text/plain", "UPLOAD:OPENFAILED");
        DEBUG_LOG("Upload: Open file failed: %s \n",filename.c_str());
      } else {
        DEBUG_LOG("Upload: First upload part: %s \n",filename.c_str());
      }
    } 
  
    if (len) { // Continue
      if(len != request->_tempFile.write(data, len)){
        DEBUG_LOG("Upload: write error\n");  
      }
      DEBUG_LOG("Upload: written: %d bytes\n",len);
    }
  
    if (final) {  // End
      if (request->_tempFile) {
        request->_tempFile.close();
      }
      DEBUG_LOG("Upload End\n");
    }
  }
  else // SD
  {
    if(sdcontrol.canWeTakeControl() == -1)
    {
      DEBUG_LOG("Printer controlling the SD card\n"); 
      request->send(500, "text/plain","UPLOAD:SDBUSY");
      return;
    }
  
    if (!index) { // start
      sdcontrol.takeControl();
      if(request->_tempFile){
          request->_tempFile.close();
      }
  
      if (SD.exists((char *)filename.c_str())) {
        SD.remove((char *)filename.c_str());
      }
  
      request->_tempFile = SD.open(filename.c_str(), FILE_WRITE);
      if(!request->_tempFile) {
        request->send(500, "text/plain", "UPLOAD:OPENFAILED");
        sdcontrol.relinquishControl();
        DEBUG_LOG("Upload: Open file failed: %s \n",filename.c_str());
      } else {
        DEBUG_LOG("Upload: First upload part: %s \n",filename.c_str());
      }
    } 
  
    if (len) { // Continue
      if(len != request->_tempFile.write(data, len)){
        DEBUG_LOG("Upload: write error\n");  
      }
      DEBUG_LOG("Upload: written: %d bytes\n",len);
    }
  
    if (final) {  // End
      if (request->_tempFile) {
        request->_tempFile.close();
      }
      DEBUG_LOG("Upload End\n");
      _diskFree = sdcontrol.getDiskFree();
      sdcontrol.relinquishControl();
    }
  } // SD
}
