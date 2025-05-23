/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <HardwareSerial.h>

#define MYSERIAL0 Serial

#define RELEASE  // Comment to enable debug output
#ifndef RELEASE
#define DEBUG_LOG(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif
#define LOG(...) Serial.printf(__VA_ARGS__)

#define SERIAL_INIT(...)		{ MYSERIAL0.begin(__VA_ARGS__); }

#define SERIAL_CHAR(x) ((void)MYSERIAL0.write(x))
#define SERIAL_EOL() SERIAL_CHAR('\r\n')

#define SERIAL_PRINT(x,b)      MYSERIAL0.print(x,b)
#define SERIAL_PRINTLN(x,b)    MYSERIAL0.println(x,b)
#define SERIAL_PRINTF(args...) MYSERIAL0.printf(args)

#define SERIAL_PROTOCOLCHAR(x)              SERIAL_CHAR(x)
#define SERIAL_PROTOCOL(x)                  MYSERIAL0.print(x)
#define SERIAL_PROTOCOL_F(x,y)              MYSERIAL0.print(x,y)
#define SERIAL_PROTOCOLPGM(x)               serialprintPGM(PSTR(x))
#define SERIAL_PROTOCOLLN(x)                do{ MYSERIAL0.print(x); SERIAL_EOL(); }while(0)
#define SERIAL_PROTOCOLLNPGM(x)             serialprintPGM(PSTR(x "\r\n"))
#define SERIAL_PROTOCOLPAIR(name, value)    serial_echopair_PGM(PSTR(name),(value))
#define SERIAL_PROTOCOLLNPAIR(name, value)  do{ SERIAL_PROTOCOLPAIR(name, value); SERIAL_EOL(); }while(0)

#define SERIAL_ECHO(x)                 MYSERIAL0.print(x)
#define SERIAL_ECHOPGM(x)              SERIAL_PROTOCOLPGM(x)
#define SERIAL_ECHOLN(x)               SERIAL_PROTOCOLLN(x)
#define SERIAL_ECHOLNPGM(x)            SERIAL_PROTOCOLLNPGM(x)
#define SERIAL_ECHOPAIR(pre,value)     SERIAL_PROTOCOLPAIR(pre, value)
#define SERIAL_ECHOLNPAIR(pre,value)   SERIAL_PROTOCOLLNPAIR(pre, value)
#define SERIAL_ECHO_F(x,y)             SERIAL_PROTOCOL_F(x,y)

// These macros compensate for float imprecision
#define SERIAL_PROTOCOLPAIR_F(pre, value)    SERIAL_PROTOCOLPAIR(pre, FIXFLOAT(value))
#define SERIAL_PROTOCOLLNPAIR_F(pre, value)  SERIAL_PROTOCOLLNPAIR(pre, FIXFLOAT(value))
#define SERIAL_ECHOPAIR_F(pre,value)         SERIAL_ECHOPAIR(pre, FIXFLOAT(value))
#define SERIAL_ECHOLNPAIR_F(pre, value)      SERIAL_ECHOLNPAIR(pre, FIXFLOAT(value))

//
// Functions for serial printing from PROGMEM. (Saves loads of SRAM.)
//

 __attribute__((always_inline)) inline
 void serialprintPGM(const char* str) {
  while (char ch = pgm_read_byte(str++)) SERIAL_CHAR(ch);
}

void serial_echopair_PGM(const char* s_P, const char *v);
void serial_echopair_PGM(const char* s_P, char v);
void serial_echopair_PGM(const char* s_P, int v);
void serial_echopair_PGM(const char* s_P, long v);
void serial_echopair_PGM(const char* s_P, float v);
void serial_echopair_PGM(const char* s_P, double v);
void serial_echopair_PGM(const char* s_P, unsigned int v);
void serial_echopair_PGM(const char* s_P, unsigned long v);

 __attribute__((always_inline)) inline
 void serial_echopair_PGM(const char* s_P, uint8_t v) { serial_echopair_PGM(s_P, (int)v); }

 __attribute__((always_inline)) inline
 void serial_echopair_PGM(const char* s_P, uint16_t v) { serial_echopair_PGM(s_P, (int)v); }

 __attribute__((always_inline)) inline
 void serial_echopair_PGM(const char* s_P, bool v) { serial_echopair_PGM(s_P, (int)v); }

 __attribute__((always_inline)) inline
 void serial_echopair_PGM(const char* s_P, void *v) { serial_echopair_PGM(s_P, (unsigned long)v); }

void serial_spaces(uint8_t count);
#define SERIAL_ECHO_SP(C)     serial_spaces(C)
#define SERIAL_ERROR_SP(C)    serial_spaces(C)
#define SERIAL_PROTOCOL_SP(C) serial_spaces(C)

#endif // __SERIAL_H__
