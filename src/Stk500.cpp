#include "Stk500.h"

Stk500::Stk500(int resetPin)
{

  pinMode(resetPin, OUTPUT);
  // pinMode(9, OUTPUT);
  // pinMode(0, INPUT);
  // pinMode(1, OUTPUT);
  _resetPin = resetPin;
}

bool Stk500::setupDevice(byte *signature)
{
  resetMCU();
  if (getSync())
    return getSignature(signature);
  return 0;
}

void Stk500::flashPage(byte *address, byte *data)
{

  byte header[] = {0x64, 0x00, 0x40, 0x46};
  header[2] = pageSize;
  loadAddress(address[1], address[0]);

  Serial.write(header, 4);
  for (int i = 0; i < pageSize; i++)
  {
    Serial.write(data[i]);
  }
  Serial.write(0x20);

  waitForSerialData(2, 100);
  Serial.read();
  Serial.read();
}

void Stk500::resetMCU()
{

  digitalWrite(_resetPin, HIGH);
  delay(100);
  digitalWrite(_resetPin, LOW);
  delay(100);
}

int Stk500::getSync()
{

  return execCmd(0x30);
}

int Stk500::enterProgMode()
{
  byte signature[12];
  getSignature(signature);
  return 1;
  return execCmd(0x50);
}

int Stk500::exitProgMode()
{

  return execCmd(0x51);
}

int Stk500::setExtProgParams()
{
  return 1;
  byte params[] = {0x05, 0x04, 0xd7, 0xc2, 0x00};
  return execParam(0x45, params, sizeof(params));
}

int Stk500::setProgParams()
{
  return 1;
  byte params[] = {0x73, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0xff, 0xff, 0xff, 0xff, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x20, 0x00};
  return execParam(0x42, params, sizeof(params));
}

int Stk500::loadAddress(byte adrHi, byte adrLo)
{

  byte params[] = {adrHi, adrLo};
  return execParam(0x55, params, sizeof(params));
}

byte Stk500::execCmd(byte cmd)
{

  byte bytes[] = {cmd, 0x20};
  return sendBytes(bytes, 2);
}

byte Stk500::getSignature(byte *signature)
{
  byte bytes[] = {0x75, 0x20};
  Serial.write(bytes, 2);
  pageSize = 0x80, pageInc = 0x40;
  const byte atmega88[] = {0x1E, 0x93, 0x0F};
  const byte atmega88p[] = {0x1E, 0x94, 0x1F};
 
  if (waitForSerialData(5, 1000))
  {
    if (Serial.read() != 0x14)
      return 0;
    for (int i = 0; i < 3; i++)
    {
      signature[i] = Serial.read();
    }
    if (Serial.read() != 0x10)
      return 0;
    if ((memcmp_P(signature, atmega88, 3) == 0) || (memcmp_P(signature, atmega88p, 3) == 0))
    {
      pageSize = 0x40;
      pageInc = 0x20;
    }

    return 1;
  }
  else
    return 0;
}

byte Stk500::execParam(byte cmd, byte *params, int count)
{

  byte bytes[32];
  bytes[0] = cmd;

  int i = 0;
  while (i < count)
  {
    bytes[i + 1] = params[i];
    i++;
  }

  bytes[i + 1] = 0x20;

  return sendBytes(bytes, i + 2);
}

byte Stk500::sendBytes(byte *bytes, int count)
{

  Serial.write(bytes, count);
  waitForSerialData(2, 1000);

  byte sync = Serial.read();
  byte ok = Serial.read();
  if (sync == 0x14 && ok == 0x10)
  {
    return 1;
  }
  return 0;
}

int Stk500::waitForSerialData(int dataCount, int timeout)
{

  int timer = 0;

  while (timer < timeout)
  {
    if (Serial.available() >= dataCount)
    {
      return 1;
    }
    delay(1);
    timer++;
  }

  return 0;
}
