/*
   Roomba Controller: https://github.com/visualdeath/roomba-controller
   Copyright (C) 2021 VisualDeath

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "roomba.h"
#include <RemoteDebug.h>

extern RemoteDebug Debug;

unsigned long sleepPulse = 0;
#define SLEEP_PULSE_TIMEOUT 30000

Roomba::Roomba(byte rx, byte tx, byte wake) : _serial(rx, tx) {
  this->_rx = rx;
  this->_tx = tx;
  this->_wake = wake;
  this->_baud = 115200;
}

void Roomba::setup() {
  debugD("Begin Roomba Setup!");
  _serial.begin(_baud);
  pinMode(_rx, INPUT);
  pinMode(_tx, OUTPUT);
  pinMode(_wake, OUTPUT);
  digitalWrite(_wake, LOW);
  this->start();
  //this->reset();
  delay(300);
  this->_updateTime();
  //this->stream(dockSensors, sizeof(dockSensors));
  debugD("End Roomba Setup");
}

void Roomba::loop() {
  // SoftwareSerial max buffer is 64 bytes
  if (_serial.available()) {
    uint8_t fByte = _serial.read();

    if (fByte == (uint8_t)19) {
      uint8_t lenByte = _serial.read();
      uint8_t buff[lenByte + 3];
      buff[0] = fByte;
      buff[1] = lenByte;
      for (int x = 2; x < lenByte; x++) {
        buff[x] = _serial.read();
      }
      debugV("Roomba: Sensor Stream Packet received with %d bytes.", fByte);
    } else {
      uint8_t buff[161];
      buff[0] = fByte;
      int x = 1;
      while (x < 161) {
        buff[x] = _serial.read();
        //debugV("Roomba Read: raw %d", buff[x]);
        if (buff[x] == (byte)10) {
          if (buff[x -1] == (byte)13) {
            buff[x - 1] = '\0';
          } else {
            buff[x] = '\0';
          }
          break;
        }
        x++;
        if (x == 161) {
          buff[x] = '\0';
        }
      }
      debugV("Roomba Read: %s", &buff);
    }

    if ( millis() >= sleepPulse || millis() < (sleepPulse - SLEEP_PULSE_TIMEOUT) ) {
      digitalWrite(_wake, HIGH);
      delay(200);
      digitalWrite(_wake, LOW);
      delay(200);
      //digitalWrite(_wake, HIGH);
      sleepPulse = millis() + SLEEP_PULSE_TIMEOUT;
      debugD("Roomba: Pulse BRC next pulse at %ul", sleepPulse);
    }
    
//    int toRead = _serial.available();
//    uint8_t buffer[toRead];
//    int x = 0;
//    while ( x < toRead ) {
//      if (x < toRead) {
//        buffer[x] = _serial.read();
//        debugV("Roomba Read: %s", (char)buffer[x]);
//        x += 1;
//      } else {
//        debugE("Roomba: Buffer overflow!");
//        break;
//      }
//    }
//    buffer[x] = '\0';
    
//    while (_serial.available()) {
//      uint8_t b = _serial.read();
//      debugV("Roomba Read: %u", b);
//    }
  }
}

// Wakes the Roomba
void Roomba::wake() {
  debugV("Roomba: Wake");
  digitalWrite(_wake, HIGH);
  delay(200);
  digitalWrite(_wake, LOW);
  //delay(200);
  this->streamCommand(StreamCommandResume);
  //this->dock();
  digitalWrite(_wake, LOW);
}

// Resets the Roomba
void Roomba::reset() {
  _serial.write(7);
  debugV("Roomba Write: 7");
}

// Start OI
// Changes mode to passive
void Roomba::start()
{
  _serial.write(128);
  debugV("Roomba Write: 128");
}

uint32_t Roomba::baudCodeToBaudRate(Baud baud)
{
    switch (baud)
    {
  case Baud300:
      return 300;
  case Baud600:
      return 600;
  case Baud1200:
      return 1200;
  case Baud2400:
      return 2400;
  case Baud4800:
      return 4800;
  case Baud9600:
      return 9600;
  case Baud14400:
      return 14400;
  case Baud19200:
      return 19200;
  case Baud28800:
      return 28800;
  case Baud38400:
      return 38400;
  case Baud57600:
      return 57600;
  case Baud115200:
      return 115200;
  default:
      return 57600;
    }
}

void Roomba::baud(Baud baud)
{
  _serial.write(129);
  _serial.write(baud);
  debugV("Roomba Write: 129 %d", baud);

  _baud = baudCodeToBaudRate(baud);
  _serial.end();
  _serial.begin(_baud);
}

void Roomba::safeMode()
{
  _serial.write(131);
  debugV("Roomba Write: 131");
}

void Roomba::fullMode()
{
  _serial.write(132);
  debugV("Roomba Write: 132");
}

void Roomba::power()
{
  _serial.write(133);
  debugV("Roomba Write: 133");
}

void Roomba::dock()
{
  _serial.write(143);
  debugV("Roomba Write: 143");
}

void Roomba::clean(bool max)
{
  if (max) {
    _serial.write(135);
    debugV("Roomba Write: 135");
    debugI("Roomba Started Cleaning");
  } else {
    _serial.write(136);
    debugV("Roomba Write: 136");
    debugI("Roomba Started Cleaning in Max Mode");
  }
}

void Roomba::spot()
{
  _serial.write(134);
  debugV("Roomba Write: 134");
  debugI("Roomba Started Cleaning in Spot Mode");
}

void Roomba::drive(int16_t velocity, int16_t radius)
{
  _serial.write(137);
  _serial.write((velocity & 0xff00) >> 8);
  _serial.write(velocity & 0xff);
  _serial.write((radius & 0xff00) >> 8);
  _serial.write(radius & 0xff);
  debugV("Roomba Write: 137 %d %d", velocity, radius);
}

void Roomba::driveDirect(int16_t leftVelocity, int16_t rightVelocity)
{
  _serial.write(145);
  _serial.write((rightVelocity & 0xff00) >> 8);
  _serial.write(rightVelocity & 0xff);
  _serial.write((leftVelocity & 0xff00) >> 8);
  _serial.write(leftVelocity & 0xff);
  debugV("Roomba Write: 145 %d %d", rightVelocity, leftVelocity);
}

void Roomba::leds(uint8_t leds, uint8_t powerColour, uint8_t powerIntensity)
{
  _serial.write(139);
  _serial.write(leds);
  _serial.write(powerColour);
  _serial.write(powerIntensity);
  debugV("Roomba Write: 139 %d %d %d", leds, powerColour, powerIntensity);
}

// Sets PWM duty cycles on low side drivers
void Roomba::pwmDrivers(uint8_t dutyCycle0, uint8_t dutyCycle1, uint8_t dutyCycle2)
{
  _serial.write(144);
  _serial.write(dutyCycle2);
  _serial.write(dutyCycle1);
  _serial.write(dutyCycle0);
  debugV("Roomba Write: 144 %d %d %d", dutyCycle2, dutyCycle1, dutyCycle0);
}

// Sets low side driver outputs on or off
void Roomba::drivers(uint8_t out)
{
  _serial.write(138);
  _serial.write(out);
  debugV("Roomba Write: 138 %d", out);
}

// Define a song
// Data is 2 bytes per note
void Roomba::song(uint8_t songNumber, const uint8_t* data, int len)
{
  _serial.write(140);
  _serial.write(songNumber);
  _serial.write(len >> 1); // 2 bytes per note
  _serial.write(data, len);
  debugV("Roomba Write: 140 %d %d %d", songNumber, len, data);
}

void Roomba::playSong(uint8_t songNumber)
{
  _serial.write(141);
  _serial.write(songNumber);
  debugV("Roomba Write: 141 %d", songNumber);
}

// Start a stream of sensor data with the specified packet IDs in it
void Roomba::stream(const uint8_t* packetIDs, int len)
{
  _serial.write(148);
  _serial.write((uint8_t)len);
  _serial.write(packetIDs, len);
}

// One of StreamCommand*
void Roomba::streamCommand(StreamCommand command)
{
  _serial.write(150);
  _serial.write(command);
}

void Roomba::setClock(Day day, uint8_t hour, uint8_t minute){
  _serial.write(168);
  _serial.write((uint8_t)day);
  _serial.write(hour);
  _serial.write(minute);
}

void Roomba::setClock(clock_t clock) {
  setClock(clock.day, clock.hour, clock.minute);
}

void Roomba::setSchedule(schedule_t schedule) {
  uint8_t days = B00000000;
  if (schedule.sunday.enable == true ) {
    days |= B00000001;
  }
  if (schedule.monday.enable == true ) {
    days |= B00000010;
  }
  if (schedule.tuesday.enable == true ) {
    days |= B00000100;
  }
  if (schedule.wednesday.enable == true ) {
    days |= B00001000;
  }
  if (schedule.thursday.enable == true ) {
    days |= B00010000;
  }
  if (schedule.friday.enable == true ) {
    days |= B00100000;
  }
  if (schedule.saturday.enable == true ) {
    days |= B01000000;
  }
  _serial.write(167);
  _serial.write(days);
  _serial.write(schedule.sunday.hour);
  _serial.write(schedule.sunday.minute);
  _serial.write(schedule.monday.hour);
  _serial.write(schedule.monday.minute);
  _serial.write(schedule.tuesday.hour);
  _serial.write(schedule.tuesday.minute);
  _serial.write(schedule.wednesday.hour);
  _serial.write(schedule.wednesday.minute);
  _serial.write(schedule.thursday.hour);
  _serial.write(schedule.thursday.minute);
  _serial.write(schedule.friday.hour);
  _serial.write(schedule.friday.minute);
  _serial.write(schedule.saturday.hour);
  _serial.write(schedule.saturday.minute);
}

// Reads at most len bytes and stores them to dest
// If successful, returns true.
// If there is a timeout, returns false
// Blocks until all bytes are read
// Caller must ensure there is sufficient space in dest
bool Roomba::getData(uint8_t* dest, uint8_t len)
{
  while (len-- > 0)
  {
    unsigned long startTime = millis();
    while (!_serial.available())
    {
      // Look for a timeout
      if (millis() > startTime + ROOMBA_READ_TIMEOUT)
        return false; // Timed out
    }
    *dest++ = _serial.read();
  }
  return true;
}

bool Roomba::getSensors(uint8_t packetID, uint8_t* dest, uint8_t len)
{
  _serial.write(142);
  _serial.write(packetID);
  return getData(dest, len);
}

bool Roomba::getSensorsList(uint8_t* packetIDs, uint8_t numPacketIDs, uint8_t* dest, uint8_t len)
{
  _serial.write(149);
  _serial.write(numPacketIDs);
  _serial.write(packetIDs, numPacketIDs);
  return getData(dest, len);
}

// Simple state machine to read sensor data and discard everything else
bool Roomba::pollSensors(uint8_t* dest, uint8_t destSize, uint8_t *packetLen)
{
  while (_serial.available())
  {
    uint8_t ch = _serial.read();
    switch (_pollState)
    {
      case PollStateIdle:
        if (ch == 19)
          _pollState = PollStateWaitCount;
        break;
  
        case PollStateWaitCount:
          _pollChecksum = _pollSize = ch;
          _pollCount = 0;
          _pollState = PollStateWaitBytes;
          break;
  
        case PollStateWaitBytes:
          _pollChecksum += ch;
          if (_pollCount < destSize)
              dest[_pollCount] = ch;
          if (_pollCount++ >= _pollSize)
              _pollState = PollStateWaitChecksum;
          break;
  
        case PollStateWaitChecksum:
          _pollChecksum += ch;
          _pollState = PollStateIdle;
          *packetLen = _pollSize;
          return (_pollChecksum == 0);
          break;
    } 
  }
  return false;
}

// Returns the number of bytes in the script, or 0 on errors
// Only saves at most len bytes to dest
// Calling with len = 0 will return the amount of space required without actually storing anything
uint8_t Roomba::getScript(uint8_t* dest, uint8_t len)
{
  _serial.write(154);

  unsigned long startTime = millis();
  while (!_serial.available())
  {
    // Look for a timeout
    if (millis() > startTime + ROOMBA_READ_TIMEOUT)
      return 0; // Timed out
  }

  int count = _serial.read();
  if (count > 100 || count < 0)
    return 0; // Something wrong. Cant have such big scripts!!

  // Get all the data, saving as much as we can
  uint8_t i;
  for (i = 0; i < count; i++)
  {
    startTime = millis();
    while (!_serial.available())
    {
      // Look for a timeout
      if (millis() > startTime + ROOMBA_READ_TIMEOUT)
        return 0; // Timed out
    }
    uint8_t data = _serial.read();
    if (i < len)
      *dest++ = data;
  }

  return count;
}

void Roomba::send(byte data) {
  _serial.write(data);
}

bool Roomba::_checkAwake() {

}

void Roomba::_updateTime() {
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  
  char timeHour[3];
  char timeMinute[3];
  char timeDay[10];
  strftime(timeHour, 3, "%H", timeinfo);
  strftime(timeMinute, 3, "%M", timeinfo);
  strftime(timeDay, 10, "%A", timeinfo);
  debugI("Time: %s at %s:%s", timeDay, timeHour, timeMinute);

  clock_t newTime;
  newTime.hour = atoi(timeHour);
  newTime.minute = atoi(timeMinute);

  if (timeDay == "Sunday") newTime.day = Sunday;
  if (timeDay == "Monday") newTime.day = Monday;
  if (timeDay == "Tuesday") newTime.day = Tuesday;
  if (timeDay == "Wednesday") newTime.day = Wednesday;
  if (timeDay == "Thursday") newTime.day = Thursday;
  if (timeDay == "Friday") newTime.day = Friday;
  if (timeDay == "Saturday") newTime.day = Saturday;

  setClock(newTime);
}
