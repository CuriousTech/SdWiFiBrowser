#ifndef _SD_CONTROL_H_
#define _SD_CONTROL_H_

#define SPI_BLOCKOUT_PERIOD	10UL // Second

class SDControl {
public:
  SDControl() { }
  static void setup();
  static void takeControl();
  static void relinquishControl();
  static int canWeTakeControl();
  static bool wehaveControl();
  static bool printerRequest();
  bool deleteFile(String path, bool bDir);
  uint32_t getDiskFree();
  bool createDir(char *pszName);
private:
  static volatile long _spiBlockoutTime;
  static bool _weTookBus;
  static volatile bool _printerRequest;
};

extern SDControl sdcontrol;

#endif
