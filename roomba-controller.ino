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

#include <Time.h>
//#include <TimeLib.h>
#include <Ticker.h>
#include <SoftwareSerial.h>
#include <FS.h>
#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <SPIFFS.h>
#endif

// ################################################################################
// ############################  Begin Configuration  #############################
// ################################################################################

// ################################  Main Config  #################################
#define ENABLE_DEBUG 1

#define SERIAL_RX     D5
#define SERIAL_TX     D6
#define WAKE_PIN      D1
#define TRIGGER_PIN   0

#define DEFAULT_HOSTNAME "roomba"
#define DEFAULT_NTP_SERVER "us.pool.ntp.org"

#define ENABLE_MULTICAST_DNS
#define ENABLE_MQTT_SUPPORT
#define ENABLE_WEB_PORTAL
#define ENABLE_OTA_SUPPORT

#if ENABLE_DEBUG != 0
  #define ENABLE_REMOTE_DEBUG
#endif

#ifdef ENABLE_MQTT_SUPPORT
  #define MQTT_ENABLED 0
  #define MQTT_HOSTNAME "homeassistant.local"
  #define MQTT_PORT 1883
  #define MQTT_USER "MyUserName"
  #define MQTT_PASS ""
  #define MQTT_TOPIC_SET "/set"                                       // MQTT Topic to subscribe to for changes(Home Assistant)
  #define MQTT_TOPIC "homeassistant/vacuum/roomba"               // MQTT Topic to Publish to for state and config (Home Assistant)
  #define MQTT_DEVICE_NAME "Roomba"
  #define MQTT_UNIQUE_IDENTIFIER WiFi.macAddress()                    // A Unique Identifier for the device in Homeassistant (MAC Address used by default)
  #define MQTT_MAX_PACKET_SIZE 1024
  #define MQTT_MAX_TRANSFER_SIZE 1024

  #include <PubSubClient.h>
  #include <ArduinoJson.h>
  WiFiClient espClient;
  PubSubClient client(espClient);
#endif // ENABLE_MQTT_SUPPORT

// ################################################################################
// #############################  End Configuration  ##############################
// ################################################################################

#define VERSION "0.1"
#define VERSION_DATE "2021-07-10"

// define debugging MACROS
#if ENABLE_DEBUG != 0
  #ifdef ENABLE_REMOTE_DEBUG
    #include <RemoteDebug.h>
    RemoteDebug Debug;
  #else
    #define DEBUG_A(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
    #define DEBUG_P(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
    #define DEBUG_V(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
    #define DEBUG_D(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
    #define DEBUG_I(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
    #define DEBUG_W(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
    #define DEBUG_E(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
    
    #define debugA(fmt, ...) Serial.printf("ANY [%lu]: ", millis()); DEBUG_A(fmt, ##__VA_ARGS__) Serial.print("\n")
    #define debugP(fmt, ...) Serial.printf("PROFILER [%lu]: ", millis()); DEBUG_P(fmt, ##__VA_ARGS__) Serial.print("\n")
    #define debugV(fmt, ...) Serial.printf("VERBOSE [%lu]: ", millis()); DEBUG_V(fmt, ##__VA_ARGS__) Serial.print("\n")
    #define debugD(fmt, ...) Serial.printf("DEBUG [%lu]: ", millis()); DEBUG_D(fmt, ##__VA_ARGS__) Serial.print("\n")
    #define debugI(fmt, ...) Serial.printf("INFO [%lu]: ", millis()); DEBUG_I(fmt, ##__VA_ARGS__) Serial.print("\n")
    #define debugW(fmt, ...) Serial.printf("WARNING [%lu]: ", millis()); DEBUG_W(fmt, ##__VA_ARGS__) Serial.print("\n")
    #define debugE(fmt, ...) Serial.printf("ERROR [%lu]: ", millis()); DEBUG_E(fmt, ##__VA_ARGS__) Serial.print("\n")
  #endif
#else
  #define DEBUG_A(...)
  #define DEBUG_P(...)
  #define DEBUG_V(...)
  #define DEBUG_D(...)
  #define DEBUG_I(...)
  #define DEBUG_W(...)
  #define DEBUG_E(...)
  
  #define debugA(...)
  #define debugP(...)
  #define debugV(...)
  #define debugD(...)
  #define debugI(...)
  #define debugW(...)
  #define debugE(...)
  Ticker stateCheck;
#endif

#include <WiFiManager.h>
WiFiManager wm; // global wm instance
bool wifiManagerPortalRunning = false;
bool wifiConnected = false;

unsigned long lastHeapCheck = 0;
int heapCheckDelay = 10000;
    
#include "roomba.h"
Roomba roomba(SERIAL_RX, SERIAL_TX, WAKE_PIN);

#include "config.h"

bool fsOK = false;

#ifdef ENABLE_WEB_PORTAL
  #ifdef ESP8266
  ESP8266WebServer webServer(80);
  #elif defined(ESP32)
  WebServer webServer(80);
  #endif
  #include <ESP8266HTTPUpdateServer.h>
  #include <uri/UriBraces.h>
  //#include <uri/UriRegex.h>
  #include "webserver.h"
  ESP8266HTTPUpdateServer httpUpdateServer;
#endif // ENABLE_WEB_PORTAL

#ifdef ENABLE_OTA_SUPPORT
#include <ArduinoOTA.h>
#endif // ENABLE_OTA_SUPPORT

#ifdef ENABLE_MULTICAST_DNS
  #ifdef ESP8266
  #include <ESP8266mDNS.h>
  #elif defined(ESP32)
  #include <ESPmDNS.h>
  #endif //ESP32
#endif // ENABLE_MULTICAST_DNS

// Setup
void setup(void) {
#ifdef ESP8266
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.persistent(true);
#endif
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  
  Serial.begin(115200);
#if ENABLE_DEBUG != 0
  Serial.setDebugOutput(true);
#endif

  Serial.print(F("\n\n"));
  
  Serial.println(F("\nController: Starting"));

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT);

  // set a default config to be used on config reset
  strncpy(default_cfg.hostname, DEFAULT_HOSTNAME, sizeof(default_cfg.hostname));
  strncpy(default_cfg.ntpServerName, DEFAULT_NTP_SERVER, sizeof(default_cfg.ntpServerName));
  
  loadConfig();

  Serial.print(F("\n"));
  Serial.println(F("System Information:"));
  Serial.printf("Version: %s (%s)\n", VERSION, VERSION_DATE);
  Serial.printf("Heap: %d\n", system_get_free_heap_size());
  Serial.printf("SDK: %s\n", system_get_sdk_version());
#ifdef ESP8266
  Serial.printf("Boot Vers: %d\n", system_get_boot_version());
  Serial.printf("CPU Speed: %d MHz\n", system_get_cpu_freq());
  Serial.printf("Chip ID: %d\n", system_get_chip_id());
  Serial.printf("Flash ID: %d\n", spi_flash_get_id());
  Serial.printf("Flash Size: %dKB\n", ESP.getFlashChipRealSize());
  Serial.printf("Vcc: %d\n", ESP.getVcc());
#elif defined(ESP32)
  Serial.printf("CPU Speed: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %dKB\n", ESP.getFlashChipSize());
#endif
  Serial.printf("MAC address: %s\n", WiFi.macAddress().c_str());
  Serial.print(F("\n"));

  // starting file system
  fsOK = LittleFS.begin();
  if (!fsOK) {
    Serial.println(F("An Error has occurred while mounting LittleFS"));
    return;
  }

  // setting up Wifi
  String macID = WiFi.macAddress().substring(12, 14) +
    WiFi.macAddress().substring(15, 17);

  String nameString = String(cfg.hostname) + String(" - ") + macID;
  nameString.toUpperCase();

  char nameChar[nameString.length() + 1];
  nameString.toCharArray(nameChar, sizeof(nameChar));

  // setup wifiManager
  wm.setHostname(cfg.hostname); // set hostname
  //wm.setConfigPortalBlocking(false); // config portal is not blocking (LEDs light up in AP mode)
  //wm.setSaveConfigCallback(handleReboot); // after the wireless settings have been saved a reboot will be performed
  #if ENABLE_DEBUG != 0
    wm.setDebugOutput(true);
  #else
    wm.setDebugOutput(false);
  #endif

  //automatically connect using saved credentials if they exist
  //If connection fails it starts an access point with the specified name
  if (!wm.autoConnect(nameChar)) {
    Serial.printf("INFO: Wi-Fi manager portal running. Connect to the Wi-Fi AP '%s' to configure your wireless connection\n", nameChar);
    wifiManagerPortalRunning = true;
  } else {
    Serial.println(F("INFO: Wi-Fi connected"));
    wifiConnected = true;
    Serial.print(F("INFO: WiFi Connected! Open http://"));
    Serial.print(WiFi.localIP());
    Serial.println(F(" in your browser"));
#ifdef ENABLE_MULTICAST_DNS
    if (!MDNS.begin(cfg.hostname, WiFi.localIP())) {
      Serial.println(F("\nERROR: problem while setting up MDNS responder! \n"));
    } else {
      Serial.printf("INFO: mDNS responder started. Try to open http://%s.local in your browser\n", cfg.hostname);
      MDNS.addService("http", "tcp", 80);
#ifdef ENABLE_REMOTE_DEBUG
      MDNS.addService("telnet", "tcp", 23);
#endif
    }
#endif
  }

  // FS debug information
  // THIS NEEDS TO BE PAST THE WIFI SETUP!! OTHERWISE WIFI SETUP WILL BE DELAYED
  #if ENABLE_DEBUG != 0
    Serial.println(F("LittleFS contents:"));
    #ifdef ESP8266
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
      Serial.printf("FS File: %s, size: %lu", dir.fileName().c_str(), dir.fileSize());
    }
    Serial.print("\n");
    FSInfo fs_info;
    LittleFS.info(fs_info);
    unsigned int totalBytes = fs_info.totalBytes;
    unsigned int usedBytes = fs_info.usedBytes;
    #elif defined(ESP32)
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
      Serial.printf("FS File: %s, size: %lu", file.name(), file.size());
      file = root.openNextFile();
    }
    Serial.println(F("\n"));
    unsigned int totalBytes = LittleFS.totalBytes();
    unsigned int usedBytes = LittleFS.usedBytes();
    #endif
    if (usedBytes == 0) {
      Serial.println(F("NO WEB SERVER FILES PRESENT! SEE: https://github.com/visualdeath/roomba-controller/blob/master/Software_Installation.md#32-sketch-data-upload\n"));
    }
    Serial.printf("FS Size: %luKB, used: %luKB, %0.2f%%", \
                      totalBytes, usedBytes, \
                      (float) 100 / totalBytes * usedBytes);
    Serial.print("\n");
  #endif

  // print setup details
  #ifdef ESP8266
  Serial.printf("Arduino Core Version: %s", ARDUINO_ESP8266_RELEASE);
  #elif defined(ESP32) && defined(ARDUINO_ESP32_RELEASE)
  Serial.printf("Arduino Core Version: %s", ARDUINO_ESP32_RELEASE);
  #endif
  Serial.println(F("Enabled Features:"));
  #ifdef ENABLE_MULTICAST_DNS
    Serial.println(F("Feature: mDNS support enabled"));
  #endif
  #ifdef ENABLE_OTA_SUPPORT
    Serial.println(F("Feature: OTA support enabled"));
  #endif
  #ifdef ENABLE_MQTT_SUPPORT
    Serial.printf("Feature: MQTT support enabled (mqtt version: %s)", String(MQTT_VERSION).c_str());
  #endif
  Serial.print("\n");

  //automatically connect using saved credentials if they exist
  //If connection fails it starts an access point with the specified name
  if (!wm.autoConnect(nameChar)) {
    Serial.printf("INFO: Wi-Fi manager portal running. Connect to the Wi-Fi AP '%s' to configure your wireless connection\n", nameChar);
    wifiManagerPortalRunning = true;
  } else {
    Serial.println(F("INFO: Wi-Fi connected"));
    wifiConnected = true;
    Serial.print(F("INFO: WiFi Connected! Open http://"));
    Serial.print(WiFi.localIP());
    Serial.println(F(" in your browser"));
#ifdef ENABLE_MULTICAST_DNS
    if (!MDNS.begin(cfg.hostname, WiFi.localIP())) {
      Serial.println(F("\nERROR: problem while setting up MDNS responder! \n"));
    } else {
      Serial.printf("INFO: mDNS responder started. Try to open http://%s.local in your browser\n", cfg.hostname);
      MDNS.addService("http", "tcp", 80);
#ifdef ENABLE_REMOTE_DEBUG
      MDNS.addService("telnet", "tcp", 23);
#endif
    }
#endif
  }

#ifdef ENABLE_WEB_PORTAL
  httpUpdateServer.setup(&webServer);
  webServer.on(UriBraces("/api/{}/{}"), handleApi);
  webServer.on(UriBraces("/api/{}/{}/{}"), handleApi);
  webServer.on(UriBraces("/api/{}/{}/{}/{}"), handleApi);
  webServer.on(UriBraces("/api/{}/{}/{}/{}/{}"), handleApi);
  webServer.on(UriBraces("/api/{}/{}/{}/{}/{}/{}"), handleApi);
  webServer.on(UriBraces("/api/{}/{}/{}/{}/{}/{}/{}"), handleApi);
  webServer.on(UriBraces("/api/{}/{}/{}/{}/{}/{}/{}/{}"), handleApi);
  //webServer.on(UriRegex("^\\/api(?:[\\/]([\\w]+))?(?:[\\/]([\\w]+))?(?:[\\/]([\\w]+))?(?:[\\/]([\\w]+))?(?:[\\/]([\\w]+))?(?:[\\/]([\\w]+))?(?:[\\/]([\\w]+))?(?:[\\/]([\\w]+))?$"), handleApi);
  //called when the url is not defined here
  //use it to load content from LittleFS
  webServer.serveStatic("/", LittleFS, "/", "max-age=86400");
  webServer.onNotFound([]() {
    //f (!handleFileRead(webServer.uri()))
    webServer.send(404, "text/plain", "FileNotFound");
  });
  webServer.begin();
#endif // ENABLE_WEB_PORTAL

#ifdef ENABLE_REMOTE_DEBUG
  Debug.begin(cfg.hostname);
  Debug.setResetCmdEnabled(true);
  Debug.showProfiler(true);
  Debug.showColors(true);
  Debug.setCallBackProjectCmds(&processRemoteDebugCmd);
#endif

#ifdef ENABLE_OTA_SUPPORT
  ArduinoOTA.setHostname(cfg.hostname);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("OTA: Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println(F("OTA: Auth Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println(F("OTA: Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println(F("OTA: Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println(F("OTA: Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      Serial.println(F("OTA: End Failed"));
    }
  });
  ArduinoOTA.begin();
#endif

  Serial.printf("Getting time from %s", cfg.ntpServerName);
  configTime(cfg.ntpGmtOffset, cfg.ntpDaylightOffset, cfg.ntpServerName);
  //configTime(gmtOffset_sec, daylightOffset_sec, cfg.ntpServerName);

  // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  setenv("TZ","EST5EDT,M3.2.0,M11.1.0", 1);

  roomba.setup();
}

// LOOP
void loop(void) {
  roomba.loop();

#ifdef ENABLE_WEB_PORTAL
  webServer.handleClient();
#endif // ENABLE_WEB_PORTAL

#ifdef ENABLE_OTA_SUPPORT
  ArduinoOTA.handle();
#endif
  
#if defined(ENABLE_MULTICAST_DNS) && defined(ESP8266)
  MDNS.update();
#endif // ENABLE_MULTICAST_DNS

  if ( millis() >= (lastHeapCheck + heapCheckDelay) ) {
    debugD("Heap: %d", system_get_free_heap_size());
    lastHeapCheck = millis();
  }

#ifdef ENABLE_REMOTE_DEBUG
  Debug.handle();
  yield();
#endif

}

void reboot() {
    ESP.restart();
}

void updateLocalTime() {
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
}

#if ENABLE_DEBUG == 1 && defined(ENABLE_REMOTE_DEBUG)

void processRemoteDebugCmd() {
  String lastCmd = Debug.getLastCommand();

  if (lastCmd.substring(0,7) == "roomba " || lastCmd.substring(0,2) == "r ") {
    int x = 7;
    if ( lastCmd.substring(0,2) == "r " ) x = 2;
    String cmd = lastCmd.substring(x);
    int cmdI = cmd.toInt();
    roomba.send((byte)cmdI);
  }
}
 
#endif
