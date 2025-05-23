#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <string.h>
#include <FS.h>
#include <Preferences.h>

#define WIFI_SSID_LEN 32
#define WIFI_PASSWD_LEN 64

#define EEPROM_SIZE 512
#define CONFIG_FILE "/SETUP.INI"

typedef struct config_type
{
  unsigned char flag; // Was saved before?
  char ssid[WIFI_SSID_LEN];
  char psw[WIFI_PASSWD_LEN];
}CONFIG_TYPE;

class Config  {
public:
  int readINI(bool bSD);
  unsigned char load();
  char* ssid();
  void ssid(char* ssid);
  char* password();
  void password(char* password);
  void save(const char*ssid,const char*password);
  void save();
  int save_ip(const char *ip);
  void clear();

protected:
  Preferences prefs;

  CONFIG_TYPE data;
};

extern Config config;

#endif
