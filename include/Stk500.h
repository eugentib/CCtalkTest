#ifndef Stk500_h
#define Stk500_h

#include <Arduino.h>
extern char hexTemp[21];

class Stk500 {

  public:
    Stk500(int resPin);
    void resetMCU();
    int getSync();
    int enterProgMode();
    int exitProgMode();
    int loadAddress(byte adrHi, byte adrLo);
    int setProgParams();
    int setExtProgParams();
    byte getSignature(byte* signature);
    byte pageSize;
    byte pageInc;

    bool setupDevice(byte* signature);
    void flashPage(byte* loadAddress, byte* data);
    
  private:
    byte execCmd(byte cmd);
    byte execParam(byte cmd, byte* params, int count);
    byte sendBytes(byte* bytes, int count);
    int waitForSerialData(int dataCount, int timeout);
    int getFlashPageCount(byte flashData[][131]);
    
    int _resetPin;
};

#endif
