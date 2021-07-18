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

// information used from http://www.airspayce.com/mikem/arduino/Roomba/

#ifndef _ROOMBA_h_
#define _ROOMBA_h_

#include <SoftwareSerial.h>

/// Masks for LEDs in leds()
#define ROOMBA_MASK_LED_NONE    0
#define ROOMBA_MASK_LED_PLAY    0x2
#define ROOMBA_MASK_LED_ADVANCE 0x8

/// Masks for digitalOut()
#define ROOMBA_MASK_DIGITAL_OUT_0 0x1
#define ROOMBA_MASK_DIGITAL_OUT_1 0x2
#define ROOMBA_MASK_DIGITAL_OUT_2 0x4

/// Masks for drivers()
#define ROOMBA_MASK_DRIVER_0 0x1
#define ROOMBA_MASK_DRIVER_1 0x2
#define ROOMBA_MASK_DRIVER_2 0x4
/// Roomba only:
#define ROOMBA_MASK_SIDE_BRUSH 0x1
#define ROOMBA_MASK_VACUUM     0x2
#define ROOMBA_MASK_MAIN_BRUSH 0x4

/// Masks for bumps and wheedrops sensor packet id 7
#define ROOMBA_MASK_BUMP_RIGHT       0x1
#define ROOMBA_MASK_BUMP_LEFT        0x2
#define ROOMBA_MASK_WHEELDROP_RIGHT  0x4
#define ROOMBA_MASK_WHEELDROP_LEFT   0x8
#define ROOMBA_MASK_WHEELDROP_CASTER 0x10

/// Masks for driver overcurrents Packet ID 13
#define ROOMBA_MASK_LD1              0x1
#define ROOMBA_MASK_LD0              0x2
#define ROOMBA_MASK_LD2              0x4
#define ROOMBA_MASK_RIGHT_WHEEL      0x8
#define ROOMBA_MASK_LEFT_WHEEL       0x10
// Roomba, use ROOMBA_MASK_SIDE_BRUSH,  ROOMBA_MASK_VACUUM, ROOMBA_MASK_MAIN_BRUSH

/// Masks for buttons sensor packet ID 18
/// Create
#define ROOMBA_MASK_BUTTON_PLAY      0x1
#define ROOMBA_MASK_BUTTON_ADVANCE   0x4
/// Roomba
#define ROOMBA_MASK_BUTTON_MAX       0x1
#define ROOMBA_MASK_BUTTON_CLEAN     0x2
#define ROOMBA_MASK_BUTTON_SPOT      0x4
#define ROOMBA_MASK_BUTTON_POWER     0x8

/// Masks for digital inputs sensor packet ID 32
#define ROOMBA_MASK_DIGITAL_IN_0      0x1
#define ROOMBA_MASK_DIGITAL_IN_1      0x2
#define ROOMBA_MASK_DIGITAL_IN_2      0x4
#define ROOMBA_MASK_DIGITAL_IN_3      0x8
#define ROOMBA_MASK_DIGITAL_IN_DEVICE_DETECT      0x10

/// Masks for charging sources sensor packet ID 34
#define ROOMBA_MASK_INTERNAL_CHARGER  0x1
#define ROOMBA_MASK_HOME_BASE         0x2

/// \def ROOMBA_READ_TIMEOUT
/// Read timeout in milliseconds.
/// If we have to wait more than this to read a char when we are expecting one, then something is wrong.
#define ROOMBA_READ_TIMEOUT 200

class Roomba {

  public:
    /// \enum Baud
    /// Demo types to pass to Roomba::baud()
    typedef enum
    {
      Baud300    = 0,
      Baud600    = 1,
      Baud1200   = 2,
      Baud2400   = 3,
      Baud4800   = 4,
      Baud9600   = 5,
      Baud14400  = 6,
      Baud19200  = 7,
      Baud28800  = 8,
      Baud38400  = 9,
      Baud57600  = 10,
      Baud115200 = 11,
    } Baud;
    
    /// \enum Drive
    /// Special values for radius in Roomba::drive()
    typedef enum
    {
      DriveStraight                = 0x8000,
      DriveInPlaceClockwise        = 0xFFFF,
      DriveInPlaceCounterClockwise = 0x0001,
    } Drive;
  
    /// \enum StreamCommand
    /// Values to pass to Roomba::streamCommand()
    typedef enum
    {
      StreamCommandPause  = 0,  
      StreamCommandResume = 1,
    } StreamCommand;
  
    /// \enum IRCommand
    /// Values for sensor packet ID 27
    typedef enum
    {
      // Remote control:
      IRCommandLeft                      = 129,
      IRCommandForward                   = 130,
      IRCommandRight                     = 131,
      IRCommandSpot                      = 132,
      IRCommandMax                       = 133,
      IRCommandSmall                     = 134,
      IRCommandMedium                    = 135,
      IRCommandLargeClean                = 136,
      IRCommandPause                     = 137,
      IRCommandPower                     = 138,
      IRCommandArcForwardLeft            = 139,
      IRCommandArcForwardRight           = 140,
      IRCommandDriveStop                 = 141,
      // Scheduling Remote:
      IRCommandSendAll                   = 142,
      IRCommandSeekDock                  = 143,
      // Roomba 600 Home Base
      IRCommand600Reserved1              = 160,
      IRCommand600RedBuoy                = 168,
      IRCommand600GreenBuoy              = 164, 
      IRCommand600ForceField             = 161,
      IRCommand600RedGreenBuoy           = 172,
      IRCommand600RedBuoyForceField      = 169,
      IRCommand600GreenBuoyForceField    = 165,
      IRCommand600RedGreenBuoyForceField = 173,
      IRCommand600VirtualWall            = 162,
      // Home Base:
      IRCommandReserved1                 = 240,
      IRCommandRedBuoy                   = 248,
      IRCommandGreenBuoy                 = 244, 
      IRCommandForceField                = 242,
      IRCommandRedGreenBuoy              = 252,
      IRCommandRedBuoyForceField         = 250,
      IRCommandGreenBuoyForceField       = 246,
      IRCommandRedGreenBuoyForceField    = 254,
    } IRCommand;
  
    /// \enum ChargeState
    /// Values for sensor packet ID 21
    typedef enum
    {
      ChargeStateNotCharging            = 0,
      ChargeStateReconditioningCharging = 1,
      ChargeStateFullCharging           = 2,
      ChargeStateTrickleCharging        = 3,
      ChargeStateWaiting                = 4,
      ChargeStateFault                  = 5,
    } ChargeState;
  
    /// \enum Mode
    /// Values for sensor packet ID 35
    typedef enum
    {
      ModeOff     = 0,
      ModePassive = 1,
      ModeSafe    = 2,
      ModeFull    = 3,
    } Mode;
  
    /// \enum Sensor
    /// Values for sensor packet IDs to pass to getSensors() and getSensorsList()
    typedef enum
    {
      Sensors7to26                   = 0,
      Sensors7to16                   = 1,
      Sensors17to20                  = 2,
      Sensors21to26                  = 3,
      Sensors27to34                  = 4,
      Sensors35to42                  = 5,
      Sensors7to42                   = 6,
      SensorBumpsAndWheelDrops       = 7,
      SensorWall                     = 8,
      SensorCliffLeft                = 9,
      SensorCliffFrontLeft           = 10,
      SensorCliffFrontRight          = 11,
      SensorCliffRight               = 12,
      SensorVirtualWall              = 13,
      SensorOvercurrents             = 14,
      SensorDirtDetect               = 15,
    //  SensorUnused2                  = 16,
      SensorIRByteOmni               = 17,
      SensorButtons                  = 18,
      SensorDistance                 = 19,
      SensorAngle                    = 20,
      SensorChargingState            = 21,
      SensorVoltage                  = 22,
      SensorCurrent                  = 23,
      SensorBatteryTemperature       = 24,
      SensorBatteryCharge            = 25,
      SensorBatteryCapacity          = 26,
      SensorWallSignal               = 27,
      SensoCliffLeftSignal           = 28,
      SensoCliffFrontLeftSignal      = 29,
      SensoCliffFrontRightSignal     = 30,
      SensoCliffRightSignal          = 31,
      SensorUserDigitalInputs        = 32,
      SensorUserAnalogInput          = 33,
      SensorChargingSourcesAvailable = 34,
      SensorOIMode                   = 35,
      SensorSongNumber               = 36,
      SensorSongPlaying              = 37,
      SensorNumberOfStreamPackets    = 38,
      SensorVelocity                 = 39,
      SensorRadius                   = 40,
      SensorRightVelocity            = 41,
      SensorLeftVelocity             = 42,
      SensorLeftEncoderCounts        = 43,
      SensorRightEncoderCounts       = 44,
      SensorLightBumper              = 45,
      SensorLightBumperLeft          = 46,
      SensorLightBumperFrontLeft     = 47,
      SensorLightBumperCenterLeft    = 48,
      SensorLightBumperCenterRight   = 49,
      SensorLightBumperFrontRight    = 50,
      SensorLightBumperRight         = 51,
      SensorIRByteLeft               = 52,
      SersorIRByteRight              = 53,
      SensorLeftMotorCurrent         = 54,
      SensorRightMotorCurrent        = 55,
      SensorMainBrushMotorCurrent    = 56,
      SensorSideBrushMotorCurrent    = 57,
      SensorStasisWheel              = 58,
      Sensors7to58                   = 100,
      Sensors43to58                  = 101,
      Sensors46to51                  = 106,
      Sensors54to58                  = 107,
    } Sensor;

    typedef enum {
      Sunday    = 0,
      Monday    = 1,
      Tuesday   = 2,
      Wednesday = 3,
      Thursday  = 4,
      Friday    = 5,
      Saturday  = 6,
    } Day;

    // Hours are in 24 Hour format ex 0 - 23
    typedef struct {
      Day day;
      uint8_t hour;
      uint8_t minute;
    } clock_t;

    // Hours are in 24 Hour format ex 0 - 23
    typedef struct {
      bool enable;
      uint8_t hour;
      uint8_t minute;
    } scheduleday_t;

    typedef struct {
      scheduleday_t sunday;
      scheduleday_t monday;
      scheduleday_t tuesday;
      scheduleday_t wednesday;
      scheduleday_t thursday;
      scheduleday_t friday;
      scheduleday_t saturday;
    } schedule_t;

    const uint8_t dockSensors[1] = { Sensors21to26 };
    const uint8_t cleanSensors[1] = { Sensors7to26 };
    const uint8_t driveSensors[1] = { Sensors43to58 };
    
    Roomba(byte rx, byte tx, byte wake);

    void setup();
    void loop();

    /// Wakes the Roomba
    /// Will Pulse the BRC pin to wake the roomba
    void wake();
    
    /// Resets the Roomba. 
    /// It will emit its startup message
    /// Caution, this may take several seconds to complete
    void reset();

    /// Starts the Open Interface and sets the mode to Passive. 
    /// You must send this before sending any other commands.
    /// Initialises the serial port to the baud rate given in the constructor
    void start();
    
    /// Converts the specified baud code into a baud rate in bits per second
    /// \param[in] baud Baud code, one of Roomba::Baud
    /// \return baud rate in bits per second
    uint32_t baudCodeToBaudRate(Baud baud);

    /// Changes the baud rate
    /// Baud is on of the Roomba::Baud enums
    void baud(Baud baud);

    /// Sets the OI to Safe mode.
    /// In Safe mode, the cliff and wheel drop detectors work to prevent Roomba driving off a cliff
    void safeMode();

    /// Sets the OI to Full mode.
    /// In Full mode, the cliff and wheel drop detectors do not stop the motors: you are responsible
    // for full control of the Roomba
    void fullMode();

    /// Puts a Roomba in sleep mode.
    /// Roomba only, no equivalent for Create.
    void power();

    /// Causes roomba to immediately 
    /// seek the docking station.
    /// No equivalent for Create.
    void dock();

    /// Starts the Cleaning process
    /// Changes mode to Passive
    /// \param[in] bool to set max clean
    void clean(bool max = false);

    /// Starts the Spot Cover demo
    /// Changes mode to Passive
    void spot();
  
    /// Starts the Roomba driving with a specified wheel velocity and radius of turn
    /// \param[in] velocity Speed in mm/s (-500 to 500 mm/s)
    /// \param[in] radius Radius of the turn in mm. (-2000 to 2000 mm). 
    /// Any of the special values in enum Roomba::Drive may be used instead of a radius value
    void drive(int16_t velocity, int16_t radius);

    /// Starts the Roomba driving with a specified velocity for each wheel
    /// Create only. No equivalent on Roomba.
    /// \param[in] leftVelocity Left wheel velocity in mm/s (-500 to 500 mm/s)
    /// \param[in] rightVelocity Right wheel velocity in mm/s (-500 to 500 mm/s)
    void driveDirect(int16_t leftVelocity, int16_t rightVelocity);

    /// Controls the LEDs on the Create
    /// \param[in] leds Bitmask specifying which LEDs to activate. ORed combination of ROOMBA_MASK_LED_*
    /// \param[in] powerColour The colour of the Power LED. 0 to 255. 0 = green, 255 = red, 
    /// intermediate values are intermediate colours
    /// \param[in] powerIntensity Power LED intensity. 0 to 255. 0 = off, 255 = full intensity
    void leds(uint8_t leds, uint8_t powerColour, uint8_t powerIntensity);

    /// Sets the duty cycle for PWM outputs on the low side drivers. These can be use for PWM driving of
    /// motors, lights etc.
    /// Create only. No equivalent on Roomba.
    /// \param[in] dutyCycle0 Duty cycle for low side driver 0. 0 to 128.
    /// \param[in] dutyCycle1 Duty cycle for low side driver 1. 0 to 128.
    /// \param[in] dutyCycle2 Duty cycle for low side driver 2. 0 to 128.
    void pwmDrivers(uint8_t dutyCycle0, uint8_t dutyCycle1, uint8_t dutyCycle2); 

    /// Sets the low side drivers on or off. On the Romba, these control the 3 motors.
    /// \param[in] out Bitmask of putputs to enable. ORed value ROOMBA_MASK_DRIVER_*
    void drivers(uint8_t out);

    /// Defines a song which can later be played with playSong()
    /// \param[in] songNumber Song number for this song. 0 to 15
    /// \param[in] notes Array of note/duration pairs. See Open Interface manual for details. 2 bytes per note, 
    /// first byte is the note and the second is the duration
    /// \param[in] len Length of notes array in bytes, so this will be twice the number of notes in the song
    void song(uint8_t songNumber, const uint8_t* notes, int len);

    /// Plays a song that has previously been defined by song()
    /// \param[in] songNumber The song number to play. 0 to 15
    void playSong(uint8_t songNumber);

    /// Requests that a stream of sensor data packets be sent by the Roomba.
    /// See the Open Interface manual for details on the resutting data.
    /// The packets will be sent every 15ms.
    /// You can use pollSensors() to receive sensor data streams.
    /// Create only. No equivalent on Roomba.
    /// See the Open Interface maual for more details and limitations.
    /// \param[in] packetIDs Array specifying sensor packet IDs from Roomba::Sensor to be sent.
    /// \param[in] len Number of IDs in packetIDs
    void stream(const uint8_t* packetIDs, int len);

    /// Pause or resume a stream of sensor data packets previously requested by stream()
    /// Create only. No equivalent on Roomba.
    /// \param[in] command One of Roomba::StreamCommand
    void streamCommand(StreamCommand command);

    /// Sets the clock on the Roomba
    void setClock(Day day, uint8_t hour, uint8_t minute);
    void setClock(clock_t clock);

    /// Sets the schedule on the Roomba
    void setSchedule(schedule_t schedule);

    /// Low level funciton to read len bytes of data from the Roomba
    /// Blocks untill all len bytes are read or a read timeout occurs.
    /// \param[out] dest Destination where the read data is stored. Must have at least len bytes available.
    /// \param[in] len Number of bytes to read
    /// \return true if all len bytes were successfully read. Returns false in the case of a timeout 
    /// on reading any byte.
    bool getData(uint8_t* dest, uint8_t len);

    /// Reads the sensor data for the specified sensor packet ID. Note that different sensor packets have 
    /// different lengths, and it is the callers responsibilty to make sure len agrees with the expected 
    /// length of the sensor data. See the Open Interface manual for details on sensor packet lengths.
    /// Roomba.h defines various enums and defines for decoding sensor data.
    /// Blocks untill all len bytes are read or a read timeout occurs.
    /// \param[in] packetID The ID of the sensor packet to read from Roomba::Sensor
    /// \param[out] dest Destination where the read data is stored. Must have at least len bytes available.
    /// \param[in] len Number of sensor data bytes to read
    /// \return true if all len bytes were successfully read. Returns false in the case of a timeout 
    /// on reading any byte.
    bool getSensors(uint8_t packetID, uint8_t* dest, uint8_t len);

    /// Reads the sensor data for the specified set of sensor packet IDs. Note that different sensor packets have 
    /// different lengths, and it is the callers responsibilty to make sure len agrees with the expected 
    /// length of the sensor data. See the Open Interface manual for details on sensor packet lengths.
    /// Blocks until all len bytes are read or a read timeout occurs.
    /// Create only. No equivalent on Roomba.
    /// \param[in] packetIDs Array of IDs  from Roomba::Sensor of the sensor packets to read
    /// \param[in] numPacketIDs number of IDs in the packetIDs array
    /// \param[out] dest Destination where the read data is stored. Must have at least len bytes available.
    /// \param[in] len Number of sensor data bytes to read and store to dest.
    /// \return true if all len bytes were successfully read. Returns false in the case of a timeout 
    /// on reading any byte.
    bool getSensorsList(uint8_t* packetIDs, uint8_t numPacketIDs, uint8_t* dest, uint8_t len);

    /// Polls the serial input for data belonging to a sensor data stream previously requested with stream().
    /// As sensor data is read it is appended to dest until at most len bytes are stored there. 
    /// When a complete sensor stream has been read with a correct checksum, returns true. 
    /// See the Open Interface manual for details on how the sensor data will be encoded in dest.
    /// Discards characters that are not part of a stream, such as the messages the Roomba 
    /// sends at startup and while charging.
    /// Create only. No equivalent on Roomba.
    /// \param[out] dest Destination where the read data is stored. Must have at least len bytes available.
    /// \param[out] packetLen Lenth of the read packet
    /// \param[in] len Max number of sensor data bytes to store to dest
    /// \return true when a complete stream has been read, and the checksum is correct. The sensor data
    /// (at most len bytes) will have been stored into dest, ready for the caller to decode.
    bool pollSensors(uint8_t* dest, uint8_t destSize, uint8_t *packetLen);

    /// Reads a the contents of the script most recently specified by a call to script().
    /// Create only. No equivalent on Roomba.
    /// \param[out] dest Destination where the read data is stored. Must have at least len bytes available.
    /// \param[in] len The maximum number of bytes to place in dest. If the script is actually longer than len
    /// only len bytes will be written
    /// \return The actual number of bytes in the script, even if this is more than len. By calling 
    /// getScript(NULL, 0), you can determine how many bytes would be required to store the script.
    uint8_t getScript(uint8_t* dest, uint8_t len);

    void send(byte data);
    

  private:
    /// \enum PollState
    /// Values for _pollState
    typedef enum
    {
      PollStateIdle         = 0,
      PollStateWaitCount    = 1,
      PollStateWaitBytes    = 2,
      PollStateWaitChecksum = 3,
    } PollState;

    /// The baud rate to use for the serial port
    uint32_t        _baud;
  
    SoftwareSerial _serial;
    byte _rx;
    byte _tx;
    byte _wake;

    /// Variables for keeping track of polling of data streams
    uint8_t         _pollState; /// Current state of polling, one of Roomba::PollState
    uint8_t         _pollSize;  /// Expected size of the data stream in bytes
    uint8_t         _pollCount; /// Num of bytes read so far
    uint8_t         _pollChecksum; /// Running checksum counter of data bytes + count

    bool _checkAwake();
    void _updateTime();
};

#endif
