#ifndef stkSrv_h
#define stkSrv_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "IntelHexParse.h"
#include "Stk500.h"

#define ARESET 12
#define HiARESET() GPOS = (1 << ARESET)
#define LoARESET() GPOC = (1 << ARESET)

extern char hexTemp[21];

class stkSrvc
{

public:
  void WSCmdFlash(File file);//The file is closed on exit
  void GetDeviceSignature(byte *signature);
  void setCallback(void (*userDefinedCallback)(int))
  {
    localPointerToCallback = userDefinedCallback;
  }

private:
  int _resetPin = ARESET;
  void (*localPointerToCallback)(int);
};

#endif
