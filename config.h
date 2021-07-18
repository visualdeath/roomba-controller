// https://github.com/NimmLor/esp8266-fastled-iot-webserver
// https://www.kriwanek.de/index.php/de/homeautomation/esp8266/364-eeprom-für-parameter-verwenden

#ifndef _CONFIG_h_
#define _CONFIG_h_

#include <EEPROM.h>

#define CONFIG_SAVE_MAX_DELAY 10            // delay in seconds when the settings are saved after last change occured
#define CONFIG_COMMIT_DELAY   200           // commit delay in ms

typedef struct {
  char hostname[33];
  char ntpServerName[65];
  long ntpGmtOffset;
  int ntpDaylightOffset;
  uint8_t MQTTEnabled;
  uint8_t MQTTHomeAssistant;
  char MQTTHost[65];
  uint16_t MQTTPort;
  char MQTTUser[33];
  char MQTTPass[65];
  char MQTTTopic[65];
  char MQTTSetTopic[65];
  char MQTTDeviceName[33];
} configData_t;

configData_t cfg;
configData_t default_cfg;

// save last "timestamp" the config has been saved
unsigned long last_config_change = 0;

void saveConfig(bool force = false) {

  if (last_config_change == 0 && force == false) {
    return;
  }

  static bool write_config = false;
  static bool write_config_done = false;
  static bool commit_config = false;

  if (force == true) {
    write_config = true;
    commit_config = true;
  }

  if (last_config_change > 0) {

    if (last_config_change + (CONFIG_SAVE_MAX_DELAY * 1000) < millis()) {

      // timer expired and config has not been written
      if (write_config_done == false) {
        write_config = true;

        // config has been written but we should wait 200ms to commit
      } else if (last_config_change + (CONFIG_SAVE_MAX_DELAY * 1000) + CONFIG_COMMIT_DELAY < millis()) {
        commit_config = true;
      }
    }
  }

  // Save configuration from RAM into EEPROM
  if (write_config == true) {
    debugD("Saving Config");
    EEPROM.begin(4095);
    EEPROM.put(0, cfg );
    write_config_done = true;
    write_config = false;
  }

  if (commit_config == true) {
    if (force == true) delay(CONFIG_COMMIT_DELAY);
    debugD("Comitting config");
    EEPROM.commit();
    EEPROM.end();

    // reset all triggers
    last_config_change = 0;
    write_config = false;
    write_config_done = false;
    commit_config = false;
  }
}

// trigger a config write/commit
void setConfigChanged() {
  // start timer
  last_config_change = millis();
}

// overwrite all config settings with "0"
void resetConfig() {

  // delete EEPROM config
  EEPROM.begin(4095);
  for (unsigned int i = 0 ; i < sizeof(cfg) ; i++) {
    EEPROM.write(i, 0);
  }
  delay(CONFIG_COMMIT_DELAY);
  EEPROM.commit();
  EEPROM.end();

  // set to default config
  cfg = default_cfg;
  saveConfig(true);
}

bool isValidHostname(char *hostname_to_check, long size) {
  for (int i = 0; i < size; i++) {
    if (hostname_to_check[i] == '-' || hostname_to_check[i] == '.')
      continue;
    else if (hostname_to_check[i] >= '0' && hostname_to_check[i] <= '9')
      continue;
    else if (hostname_to_check[i] >= 'A' && hostname_to_check[i] <= 'Z')
      continue;
    else if (hostname_to_check[i] >= 'a' && hostname_to_check[i] <= 'z')
      continue;
    else if (hostname_to_check[i] == 0 && i > 0)
      break;

    return false;
  }

  return true;
}

void loadConfig() {

  debugD("Loading config");

  // Loads configuration from EEPROM into RAM
  EEPROM.begin(4095);
  EEPROM.get(0, cfg );
  EEPROM.end();

  if (!isValidHostname(cfg.hostname, sizeof(cfg.hostname))) {
    strncpy(cfg.hostname, default_cfg.hostname, sizeof(cfg.hostname));
    debugD("Hostname: %s", cfg.hostname);
    setConfigChanged();
  }

  if (!isValidHostname(cfg.ntpServerName, sizeof(cfg.ntpServerName))) {
    strncpy(cfg.ntpServerName, default_cfg.ntpServerName, sizeof(cfg.ntpServerName));
    setConfigChanged();
  }

#ifdef ENABLE_MQTT_SUPPORT
  // fall back to default settings if hostname is invalid
  if (!isValidHostname(cfg.MQTTHost, sizeof(cfg.MQTTHost))) {
    cfg.MQTTEnabled = MQTT_ENABLED;
    strncpy(cfg.MQTTHost, MQTT_HOSTNAME, sizeof(cfg.MQTTHost));
    cfg.MQTTPort = uint16_t(MQTT_PORT);
    strncpy(cfg.MQTTUser, MQTT_USER, sizeof(cfg.MQTTUser));
    strncpy(cfg.MQTTPass, MQTT_PASS, sizeof(cfg.MQTTPass));
    strncpy(cfg.MQTTTopic, MQTT_TOPIC, sizeof(cfg.MQTTTopic));
    strncpy(cfg.MQTTSetTopic, MQTT_TOPIC_SET, sizeof(cfg.MQTTSetTopic));
    strncpy(cfg.MQTTDeviceName, MQTT_DEVICE_NAME, sizeof(cfg.MQTTDeviceName));
    setConfigChanged();
  }
#endif
}

// parse and set a new hostname to config
void setHostname(String new_hostname) {
  int j = 0;
  for (unsigned int i = 0; i < new_hostname.length() && i < sizeof(cfg.hostname); i++) {
    if (new_hostname.charAt(i) == '-' or \
        (new_hostname.charAt(i) >= '0' && new_hostname.charAt(i) <= '9') or \
        (new_hostname.charAt(i) >= 'A' && new_hostname.charAt(i) <= 'Z') or \
        (new_hostname.charAt(i) >= 'a' && new_hostname.charAt(i) <= 'z')) {

      cfg.hostname[j] = new_hostname.charAt(i);
      j++;
    }
  }
  cfg.hostname[j] = '\0';
  setConfigChanged();
}

// we can't assing wifiManager.resetSettings(); to reset, somewhow it gets called straight away.
void setWiFiConf(String ssid, String password) {
#ifdef ESP8266
  struct station_config conf;

  wifi_station_get_config(&conf);

  memset(conf.ssid, 0, sizeof(conf.ssid));
  for (int i = 0; i < ssid.length() && i < sizeof(conf.ssid); i++)
    conf.ssid[i] = ssid.charAt(i);

  memset(conf.password, 0, sizeof(conf.password));
  for (int i = 0; i < password.length() && i < sizeof(conf.password); i++)
    conf.password[i] = password.charAt(i);

  wifi_station_set_config(&conf);

  // untested due to lack of ESP32
#elif defined(ESP32)
  if (WiFiGenericClass::getMode() != WIFI_MODE_NULL) {

    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);

    memset(conf.sta.ssid, 0, sizeof(conf.sta.ssid));
    for (int i = 0; i < ssid.length() && i < sizeof(conf.sta.ssid); i++)
      conf.sta.ssid[i] = ssid.charAt(i);

    memset(conf.sta.password, 0, sizeof(conf.sta.password));
    for (int i = 0; i < password.length() && i < sizeof(conf.sta.password); i++)
      conf.sta.password[i] = password.charAt(i);

    esp_wifi_set_config(WIFI_IF_STA, &conf);
  }
#endif
}

#endif
