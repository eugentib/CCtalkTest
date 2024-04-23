#ifndef GLOBALE_H
#define GLOBALE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <EspSaveCrash.h>
#include <WiFiUdp.h>
#include <time.h>
#include <ArduinoJson.h>
#include "uptime.h"
//#include "FileStream.h"

#include "AsyncMqttClient_Generic.hpp"

#define MAX_SCROLL_LENGTH 120

#define DEBUG_SERIAL Serial

#define debugPrint(...) DEBUG_SERIAL.print(__VA_ARGS__)
#define debugPrintln(...) DEBUG_SERIAL.println(__VA_ARGS__)
#define debugPrintf(...) DEBUG_SERIAL.printf(__VA_ARGS__)

#include "stkSrv.h"
#include "presa.h"
#include "my_mqtt.h"


extern JsonDocument myJsonDoc;
extern char FIRMWARE_VERSION[];
struct Sending_File
{
  bool busy;
  bool done;
  bool move_it;
  size_t pos;
  uint8_t part;
  uint16_t packetId;
  char name[30];
};

extern Sending_File sending_file;

struct Machine_State
{
  bool presa_inainte;
  bool presa_inapoi;
  bool balotul_e_gata;
  uint16_t numar_presari;
  uint16_t numar_kipp;
  uint16_t numar_err;
  time_t start;
  time_t end;
};
extern Machine_State machine_state;

struct Machine_Config
{
  uint16_t numar_baloti;
  char seria[16];
  char model[6];
  char vsw[7];
  time_t data_revizie;
  time_t data_interventie;
};
extern Machine_Config machine_config;

struct Balot
{
  uint16_t nr;
  time_t inceput;
  time_t sfarsit;
  uint16_t presari;
};
extern Balot balot;

extern WiFiUDP udp;
extern IPAddress SendIP;
extern stkSrvc stkSrv;
extern AsyncMqttClient mqttClient;
extern Ticker mqttReconnectTimer;
extern Ticker wifiReconnectTimer;
extern Ticker SendFiles;

//extern EspSaveCrash SaveCrash;

extern char scr[MAX_SCROLL_LENGTH], oldscr[MAX_SCROLL_LENGTH];
extern uint8_t scl;
extern boolean scf;
extern boolean new_scr;
extern time_t timeDiff;

extern char _debugOutputBuffer[];
extern char sBuff[256];      // for sprintf
extern char sTopic[80];      // for sprintf
extern uint8_t lastSent[34]; // for presenting a stream of characters obtained from the LCD data bus.
extern char strMAC[12];
extern const char *PubTopic; // Topic to publish
extern char unTopic[30];     // uniq topic to publish
extern char swTopic[30];     // sw topic to publish
extern boolean ck_upd, fs_upd;
extern boolean atmega_upd;

#define LED ((*((volatile uint32_t *)(0x60000318)) & (1 << ((LED_BUILTIN) & 0xF))) != 0) // FAST Read LED pin

void setMacAddress(const char *filePath);
void adjustBalotTimestamps(void);
void onNtpSynchronized(void);
void readFile(const char* path, char* buffer, size_t maxSize);
void writeFile(const char* path, const char* content);
void changeState();
void getMAC(char *smac);
void update_error(int err);
void update_progress(int cur, int total);
void update_finished();
void update_started();
void onFlashEnd_s(int f);
void handleWiFiConnect();
void handleWiFiDisconnect();
void setTopics(void);
char * nameFromFullPath(const char * fullPath);
#endif