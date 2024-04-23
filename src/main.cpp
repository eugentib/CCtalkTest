#include "defines.h"
#include "Credentials.h"
#include "dynamicParams.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <AsyncMqtt_Generic.h>
#include <ESP8266httpUpdate.h>
#include "globale.h"

AsyncMqttClient mqttClient;

#define MY_TZ "EET-2EEST,M3.5.0/3,M10.5.0/4"
#define MY_NTP_SERVER "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org"

extern "C"
{
#include <user_interface.h>
}
ESPAsync_WiFiManager_Lite *ESPAsync_WiFiManager;

void newFileCallback(const char *filename)
{
  // Implementarea funcției de callback pentru noul fișier
  // ...
}

#define GPIO0c GPOC = (1 << 0);
#define GPIO0s GPOS = (1 << 0);

char LCD_data[80];
bool wifiConnected = false; // Variabilă pentru a urmări starea anterioară a conexiunii WiFi
uint16 disconctCounter = 0;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

String FILESYSTEM_VERSION = "0.0";

void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
  (void)event;

  debugPrintf(PSTR("Connected to Wi-Fi. IP address:"));
  debugPrintf(PSTR("%s"), WiFi.localIP());
  connectToMqtt();
  disconctCounter = 0;
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
  (void)event;

  debugPrintf(PSTR("Disconnect counter: %d\n"), disconctCounter++);
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

void setup()
{
  DEBUG_SERIAL.begin(115200);
  Serial.setTimeout(5);
  delay(100);
  uint16 i = 1;
  while (i--)
  {
    debugPrintln(PSTR("Starting..."));
    Serial1.flush();
    delay(100);
  }
  debugPrintln(PSTR("Starting..."));

  debugPrint(PSTR("Mounting LittleFS... "));

  //  while (1) ;
  if (!LittleFS.begin())
  {
    debugPrintln(PSTR("Failed!"));
    return;
  }
  else
  {
    debugPrintln(PSTR("Success!"));
  }

  // Fișierul wm_config.dat nu există, creați-l cu valori implicite
  if (!LittleFS.exists("/wm_config.dat"))
  {
    File file = LittleFS.open("/wm_config.dat", "w");
    if (file)
    {
      // Scrieți valorile implicite în fișier
      file.write((uint8_t *)&defaultConfig, sizeof(defaultConfig));
      file.close();
    }
  }

  ESPAsync_WiFiManager = new ESPAsync_WiFiManager_Lite();
  const char *AP_SSID = "CCtalkWL";
  const char *AP_PWD = "1234512345";

  setTopics();
  SetMQTTcallbacks();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  stkSrv.setCallback(onFlashEnd);

  machine_state.presa_inapoi = true;

  ESPAsync_WiFiManager->setConfigPortal(AP_SSID, AP_PWD);

  ESPAsync_WiFiManager->begin(HOST_NAME);
  debugPrintln(PSTR("Entering loop"));
}

void loop()
{
  static bool runAtStartUp = true, ss = false;
  static uint32_t lastLoopAtMs = 0, alive = 0;
  // Verifică starea conexiunii WiFi

  ESPAsync_WiFiManager->run();
  if (disconctCounter > 3600 * 4)
  {
    debugPrintln(PSTR("Resetting by disconection count..."));
    ESP.reset();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    if (runAtStartUp && (millis() > 20000))
    {
      debugPrintf(PSTR("Start NTP synk..."));
      runAtStartUp = false;
      configTime(MY_TZ, MY_NTP_SERVER);
      struct tm tmstruct;
      delay(2000);
      tmstruct.tm_year = 0;
      getLocalTime(&tmstruct, 5000);
      debugPrintf(PSTR("OK. Now is %s"), asctime(&tmstruct));
      onNtpSynchronized();
    }

    if (ck_upd)
    {
      ck_upd = false;

      WiFiClient client;

      // The line below is optional. It can be used to blink the LED on the board during flashing
      // The LED will be on during download of one buffer of data from the network. The LED will
      // be off during writing that buffer to flash
      // On a good connection the LED should flash regularly. On a bad connection the LED will be
      // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
      // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
      // ESPhttpUpdate.setLedPin(LED_BUILTIN, HIGH);

      // Add optional callback notifiers
      ESPhttpUpdate.onStart(update_started);
      ESPhttpUpdate.onEnd(update_finished);
      ESPhttpUpdate.onProgress(update_progress);
      ESPhttpUpdate.onError(update_error);

      ESPhttpUpdate.rebootOnUpdate(false); // remove automatic update
      t_httpUpdate_return ret;
      
      if (fs_upd)
      {
        ret = ESPhttpUpdate.updateFS(client, "http://sso.testeit.eu/OTA/updateFS", FILESYSTEM_VERSION);
      }
      else
      {
        ret = ESPhttpUpdate.update(client, "http://sso.testeit.eu/OTA/update", String(FIRMWARE_VERSION));
      }
      switch (ret)
      {
      case HTTP_UPDATE_FAILED:
        snprintf_P(sBuff, sizeof(sBuff) - 1, PSTR("HTTP_UPDATE_FAILED Error (%d): %s\n"), ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        mqttClient.publish(Topic("R_sw"), 1, false, sBuff);

        break;

      case HTTP_UPDATE_NO_UPDATES:
        Pstr2mqttSW(PSTR("NO_UPDATES"));
        break;

      case HTTP_UPDATE_OK:
        Pstr2mqttSW(PSTR("HTTP_UPDATE_OK"));
        if (fs_upd)
        {

        }
        else
        {
          delay(100); // Wait and restart
          ESP.restart();
        }
        // uint32 cip = CHIPID;
        break;
      }
      fs_upd = false;
    }


    if ((millis() - lastLoopAtMs > 150))
    {
      //    GPIO0s;
      if (memcmp(LCD_data, lastSent, 32) != 0)
      {
        GPIO0c;
        lastLoopAtMs = millis();
        memcpy(lastSent, LCD_data, 32);
        //    GPIO0s;
        mqttClient.publish(unTopic, 0, false, (char *)lastSent, 32);
        if (new_scr)
        {
          mqttClient.publish(Topic("scr"), 0, false, scr, scl);
        }
      }
      GPIO0c;

      lastLoopAtMs = millis();
    }

    static uint32_t rssitime = 0;
    if (rssitime < millis())
    {
      static uint8_t send_time = 1;
      struct tm tmstruct;
      tmstruct.tm_year = 0;
      getLocalTime(&tmstruct, 5000);

      if (send_time == 0)
      {
        send_time = 10;
        uptime::calculateUptime();
        snprintf_P(sBuff, sizeof(sBuff) - 1, PSTR("%02d:%02d:%02d in %02d.%02d.%d"), tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec, tmstruct.tm_mday, (tmstruct.tm_mon) + 1, 1900 + tmstruct.tm_year);
        mqttClient.publish(Topic("Time"), 0, false, sBuff);
      }
      send_time--;

      uptime::calculateUptime();
      snprintf_P(sBuff, sizeof(sBuff) - 1, PSTR("%dz %02d:%02d:%02d"), uptime::getDays(), uptime::getHours(), uptime::getMinutes(), uptime::getSeconds());
      mqttClient.publish(Topic("Uptime"), 0, false, sBuff);

      itoa(WiFi.RSSI(), sBuff, 10);
      mqttClient.publish(Topic("rssi"), 0, false, sBuff);
      snprintf(sBuff, sizeof(sBuff) - 1, "%u", ESP.getFreeHeap());
      mqttClient.publish(Topic("HEAP"), 0, false, sBuff);

      rssitime += 10000;
    }
  }
}