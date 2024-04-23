#ifndef IntelHexParse_h
#define IntelHexParse_h

#include <Arduino.h>

class IntelHexParse {

	public:
		IntelHexParse(byte pageSize, byte pageInc);
    void ParseLine(byte* data);
    byte* GetMemoryPage();
    byte* GetLoadAddress();
    bool IsPageReady();
    void shiftmemorypage();
    
	private:
    int _address = 0;
    int _length = 0;
    int _memIdx = 0;
    int _memIdxTotal = 0;
    int _recordType = 0;
    byte _memoryPage[128+15];
    byte _loadAddress[2];
    byte _pageSize;
    byte _pageInc;
    bool _pageReady = false;
    bool _firstRun = true;
    
    int GetAddress(byte* hexline);
    int GetLength(byte* hexline);
    int GetRecordType(byte* hexline);
    void GetData(byte* hexline, int len);
    void EndOfFile();
};

#endif
