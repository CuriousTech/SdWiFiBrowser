#include <Arduino.h>
#include "sdControl.h"
#include "config.h"
#include "serial.h"
#include "network.h"
#include "FSWebServer.h"

void setup() {
  SERIAL_INIT(115200);
  sdcontrol.setup();
  network.start();
  server.begin();
}

void loop() {
  network.loop();
  server.loop();
}
