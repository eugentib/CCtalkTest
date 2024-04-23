/****************************************************************************************************************************
  Credentials.h
  For ESP8266 / ESP32 boards

  ESPAsync_WiFiManager_Lite (https://github.com/khoih-prog/ESPAsync_WiFiManager_Lite) is a library
  for the ESP32/ESP8266 boards to enable store Credentials in EEPROM/SPIFFS/LittleFS for easy
  configuration/reconfiguration and autoconnect/autoreconnect of WiFi and other services without Hardcoding.

  Built by Khoi Hoang https://github.com/khoih-prog/ESPAsync_WiFiManager_Lite
  Licensed under MIT license
 *****************************************************************************************************************************/

#ifndef Credentials_h
#define Credentials_h

#include "defines.h"

/// Start Default Config Data //////////////////


#define TO_LOAD_DEFAULT_CONFIG_DATA      false

#if TO_LOAD_DEFAULT_CONFIG_DATA

// This feature is primarily used in development to force a known set of values as Config Data
// It will NOT force the Config Portal to activate. Use DRD or erase Config Data with ESPAsync_WiFiManager.clearConfigData()

// Used mostly for development and debugging. FORCES default values to be loaded each run.
// Config Portal data input will be ignored and overridden by DEFAULT_CONFIG_DATA
//bool LOAD_DEFAULT_CONFIG_DATA = true;

// Used mostly once debugged. Assumes good data already saved in device.
// Config Portal data input will be override DEFAULT_CONFIG_DATA
bool LOAD_DEFAULT_CONFIG_DATA = false;


ESP_WM_LITE_Configuration defaultConfig =
{
  //char header[16], dummy, not used
#if ESP8266
  "Dixi80",
#else
  "ESP32_Async",
#endif

  // WiFi_Credentials  WiFi_Creds  [NUM_WIFI_CREDENTIALS];
  // WiFi_Credentials.wifi_ssid and WiFi_Credentials.wifi_pw
  "welcome@kaufland",  "",
  //char board_name     [24];

#if ESP8266
  "Dixi80",
#else
  "ESP32_Async-Control",
#endif

  // terminate the list
  //int  checkSum, dummy, not used
  0
  /////////// End Default Config Data /////////////
};

#else

bool LOAD_DEFAULT_CONFIG_DATA = false;


ESP_WM_LITE_Configuration defaultConfig = {
  "DIXI80",
  "welcome@kaufland",
  "",
  "",
  "",
  "Dixi80",
  0x9fe
};
//*/

//ESP_WM_LITE_Configuration defaultConfig;

#endif    // TO_LOAD_DEFAULT_CONFIG_DATA

/////////// End Default Config Data /////////////


#endif    //Credentials_h
