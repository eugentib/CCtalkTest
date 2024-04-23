
#include "IntelHexParse.h"

 #define PAGE_INC 0x40

//#define PAGE_SIZE 64
//#define PAGE_INC 0x20

IntelHexParse::IntelHexParse(byte pageSize, byte pageInc)
{
  _loadAddress[0] = 0x00;
  _loadAddress[1] = 0x00;
  _pageSize = pageSize;
  _pageInc = pageInc;
}

void IntelHexParse::ParseLine(byte *hexline)
{
  _recordType = GetRecordType(hexline);

  if (_recordType == 0)
  {
    _address = GetAddress(hexline);
    _length = GetLength(hexline);

    GetData(hexline, _length);

    if (_memIdx >= _pageSize)
    {
      if (!_firstRun)
      {
        _loadAddress[1] += _pageInc;

        if (_loadAddress[1] == 0)
        {
          _loadAddress[0] += 0x1;
        }
      }
      _firstRun = false;
      _pageReady = true;
      _memIdxTotal = _memIdx;
      _memIdx = 0;
    }
  }

  if (_recordType == 1)
  {
    EndOfFile();
    _pageReady = true;
  }
}

bool IntelHexParse::IsPageReady()
{
  return _pageReady;
}

byte *IntelHexParse::GetMemoryPage()
{
  return _memoryPage;
}

void IntelHexParse::shiftmemorypage()
{
  _pageReady = false;
  for (int i = 0; i < _pageSize; i++)
  {
    _memoryPage[i] = _memoryPage[i + _pageSize];
  }
  _memIdx = _memIdxTotal - _pageSize;
}

byte *IntelHexParse::GetLoadAddress()
{
  return _loadAddress;
}

void IntelHexParse::GetData(byte *hexline, int len)
{

  int start = 9;
  int end = (len * 2) + start;
  char buff[3];
  buff[2] = '\0';

  for (int x = start; x < end; x = x + 2)
  {
    buff[0] = hexline[x];
    buff[1] = hexline[x + 1];
    _memoryPage[_memIdx] = strtol(buff, 0, 16);
    _memIdx++;
  }
}

void IntelHexParse::EndOfFile()
{

  _loadAddress[1] += _pageInc;
  if (_loadAddress[1] == 0)
  {
    _loadAddress[0] += 0x1;
  }

  while (_memIdx < _pageSize)
  {
    _memoryPage[_memIdx] = 0xFF;
    _memIdx++;
  }
}

int IntelHexParse::GetAddress(byte *hexline)
{

  char buff[5];
  buff[0] = hexline[3];
  buff[1] = hexline[4];
  buff[2] = hexline[5];
  buff[3] = hexline[6];
  buff[4] = '\0';

  return strtol(buff, 0, 16);
}

int IntelHexParse::GetLength(byte *hexline)
{

  char buff[3];
  buff[0] = hexline[1];
  buff[1] = hexline[2];
  buff[2] = '\0';

  return strtol(buff, 0, 16);
}

int IntelHexParse::GetRecordType(byte *hexline)
{

  char buff[3];
  buff[0] = hexline[7];
  buff[1] = hexline[8];
  buff[2] = '\0';

  return strtol(buff, 0, 16);
}
