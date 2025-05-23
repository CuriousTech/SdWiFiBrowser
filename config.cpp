#include "pins.h"
#include "config.h"
#include "serial.h"
#include "SD.h"
#include "sdcontrol.h"
#include "FSWebServer.h"

int Config::readINI(bool bSD) {
  SERIAL_ECHOLN("Going to load config from SDCard SETUP.INI file");

  File file;

  if(bSD)
  {
    sdcontrol.takeControl();
    file = SD.open(CONFIG_FILE, "r");
  }
  else
  {
    file = INTERNAL_FS.open(CONFIG_FILE, "r");
  }

  if (!file) {
    SERIAL_ECHOLN("Failed to open config file");
    if(bSD) sdcontrol.relinquishControl();
    return 1;
  }

  // Get SSID and PASSWORD from file
  int rst = 0,step = 0;
  String buffer,sKEY,sValue;

  while (file.available()) { // check for EOF
    buffer = file.readStringUntil('\n');
    if(buffer.length() == 0) continue; // Empty line
    buffer.replace("\r", ""); // Delete all CR
    int iS = buffer.indexOf('='); // Get the seperator
    if(iS < 0) continue; // Bad line
    sKEY = buffer.substring(0,iS);
    sValue = buffer.substring(iS+1);
    if(sKEY == "SSID") {
      SERIAL_ECHOLN("INI file : SSID found");
      if(sValue.length()>0) {
        memset(data.ssid,'\0',WIFI_SSID_LEN);
        sValue.toCharArray(data.ssid,WIFI_SSID_LEN);
        step++;
      }
      else {
        rst = -2;
        goto FAIL;
      }
    }
    else if(sKEY == "PASSWORD") {
      SERIAL_ECHOLN("INI file : PASSWORD found");
      if(sValue.length()>0) {
        memset(data.psw,'\0',WIFI_PASSWD_LEN);
        sValue.toCharArray(data.psw,WIFI_PASSWD_LEN);
        step++;
      }
      else {
        rst = -3;
        goto FAIL;
      }
    }
    else continue; // Bad line
  }
  if(step != 2) { // We miss ssid or password
    SERIAL_ECHOLN("Please check your SSDI or PASSWORD in ini file");
    rst = -4;
    goto FAIL;
  }

FAIL:
  file.close();
  if(bSD) sdcontrol.relinquishControl();

  return rst;
}

unsigned char Config::load() {

  INTERNAL_FS.begin(true);

  // Try to get the config from ini file (user override)
  if(0 == readINI(true)) // SDCard
  {
    return 1; // Return as connected before
  }

  if(0 == readINI(false)) // SPIFFS/INTERNAL_FS
  {
    return 1; // Return as connected before
  }

  SERIAL_ECHOLN("Going to load config from Preferences");

  prefs.begin("sdwifi", false);
  prefs.getBytes("Creds", &data, sizeof(data) );
  if(data.flag) {
    SERIAL_ECHOLN("Going to use the old network config");
  }

  return data.flag;
}

char* Config::ssid() {
  return data.ssid;
}

void Config::ssid(char* ssid) {
  if(ssid == NULL) return;
  strncpy(data.ssid,ssid,WIFI_SSID_LEN);
}

char* Config::password() {
  return data.psw;
}

void Config::password(char* password) {
  if(password == NULL) return;
  strncpy(data.psw,password,WIFI_PASSWD_LEN);
}

void Config::save(const char*ssid,const char*password) {
  if(ssid ==NULL || password==NULL)
    return;

  data.flag = 1;
  strncpy(data.ssid, ssid, sizeof(data.ssid) );
  strncpy(data.psw, password, sizeof(data.psw) );
  prefs.putBytes("Creds", &data, sizeof(data) );
}

void Config::save() {
  if(data.ssid == NULL || data.psw == NULL)
    return;

  data.flag = 1;
  prefs.putBytes("Creds", &data, sizeof(data) );
}

void Config::clear() {

  memset(&data, 0, sizeof(data));
  prefs.putBytes("Creds", &data, sizeof(data) );
}

Config config;
