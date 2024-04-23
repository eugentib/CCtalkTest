#include "stkSrv.h"

char hexTemp[21] = {"12345678901234567890"};

void binaryToHex(byte *data, int len, char *hex)
{
  for (int i = 0; i < len; i++)
  {
    sprintf(hex + (i * 2), "%02X", data[i]);
  }
  hex[len * 2] = '\0';
}

void stkSrvc::GetDeviceSignature(byte *signature){
  //  debugPrintln("GetDeviceSignature");
  HiARESET();
  delay(100);
  LoARESET();
  delay(100);
  Stk500 stk500(_resetPin);
  stk500.setupDevice(signature);
}

void stkSrvc::WSCmdFlash(File file) // The file is closed on exit
{

  Stk500 stk500(_resetPin);
  long temp = 1;
  byte signature[3];
  if (file)
  {
    if (!stk500.setupDevice(signature))
    {
      localPointerToCallback(0);
      return;
    }
    IntelHexParse hexParse = IntelHexParse(stk500.pageSize, stk500.pageInc);

    while (file.available())
    {

      byte buff[50];
      size_t p;
      p = 0;
      //      String data = file.readStringUntil('\n');
      //      data.getBytes(buff, data.length());

      while ((p < 50) && file.available())
      {
        buff[p] = byte(file.read());
        if (buff[p] == '\n')
        {
          buff[p] = '\0';
          break;
        }
        p++;
      }

      hexParse.ParseLine(buff);

      if (hexParse.IsPageReady())
      {
        byte *page = hexParse.GetMemoryPage();
        byte *address = hexParse.GetLoadAddress();
        binaryToHex(address, 2, hexTemp);
        binaryToHex(page, 8, hexTemp + 4);
        stk500.flashPage(address, page);
        hexParse.shiftmemorypage();
      }
    }
  }

  stk500.exitProgMode();
  file.close();
  localPointerToCallback(temp);
}
