// FileStream.h
#ifndef FILESTREAM_H
#define FILESTREAM_H

#include <Stream.h>
#include <LittleFS.h>

class FileStream : public Stream
{
private:
  File file;
  const char *baseFilename;
  size_t maxFileSize;
  size_t currentFileSize;
  int fileIndex;
  void (*newFileCallback)(const char *filename);

  void openNextFile();

public:
  FileStream(const char *filename, size_t maxSize, void (*callback)(const char *) = nullptr);
  ~FileStream();

  virtual size_t write(uint8_t data);
  virtual size_t write(const uint8_t *buffer, size_t size);

  size_t print(const String &s);
  size_t print(const char *str);
  size_t print(char c);
  size_t print(unsigned char c, int base = DEC);
  size_t print(int n, int base = DEC);
  size_t print(unsigned int n, int base = DEC);
  size_t print(long n, int base = DEC);
  size_t print(unsigned long n, int base = DEC);
  size_t print(double n, int digits = 2);

  size_t println();
  size_t println(const String &s);
  size_t println(const char *str);
  size_t println(char c);
  size_t println(unsigned char c, int base = DEC);
  size_t println(int n, int base = DEC);
  size_t println(unsigned int n, int base = DEC);
  size_t println(long n, int base = DEC);
  size_t println(unsigned long n, int base = DEC);
  size_t println(double n, int digits = 2);

  size_t printf(const char *format, ...);
  // Implementări stub pentru funcțiile virtuale pure din clasa Stream
  virtual int available() { return 0; }
  virtual int read() { return -1; }
  virtual int peek() { return -1; }
};

#endif
