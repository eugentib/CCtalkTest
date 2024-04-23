// FileStream.cpp
#include "FileStream.h"
void FileStream::openNextFile() {
  file.close();
  currentFileSize = 0;
  fileIndex++;
  char filename[32];
  snprintf(filename, sizeof(filename), "%s%d.log", baseFilename, fileIndex);
  file = LittleFS.open(filename, "w");
  if (newFileCallback) {
    newFileCallback(filename);
  }
}

FileStream::FileStream(const char* filename, size_t maxSize, void (*callback)(const char*)) {
  baseFilename = filename;
  maxFileSize = maxSize;
  currentFileSize = 0;
  fileIndex = 0;
  newFileCallback = callback;
  openNextFile();
}

FileStream::~FileStream() {
  file.close();
}

size_t FileStream::write(uint8_t data) {
  if (currentFileSize >= maxFileSize) {
    openNextFile();
  }
  size_t bytesWritten = file.write(data);
  currentFileSize += bytesWritten;
  return bytesWritten;
}

size_t FileStream::write(const uint8_t* buffer, size_t size) {
  size_t bytesWritten = 0;
  while (size > 0) {
    if (currentFileSize >= maxFileSize) {
      openNextFile();
    }
    size_t bytesToWrite = min(size, maxFileSize - currentFileSize);
    bytesWritten += file.write(buffer, bytesToWrite);
    currentFileSize += bytesToWrite;
    buffer += bytesToWrite;
    size -= bytesToWrite;
  }
  return bytesWritten;
}

size_t FileStream::print(const String& s) {
  return file.print(s);
}

size_t FileStream::print(const char* str) {
  return file.print(str);
}

size_t FileStream::print(char c) {
  return file.print(c);
}

size_t FileStream::print(unsigned char c, int base) {
  return file.print(c, base);
}

size_t FileStream::print(int n, int base) {
  return file.print(n, base);
}

size_t FileStream::print(unsigned int n, int base) {
  return file.print(n, base);
}

size_t FileStream::print(long n, int base) {
  return file.print(n, base);
}

size_t FileStream::print(unsigned long n, int base) {
  return file.print(n, base);
}

size_t FileStream::print(double n, int digits) {
  return file.print(n, digits);
}

size_t FileStream::println() {
  return file.println();
}

size_t FileStream::println(const String& s) {
  return file.println(s);
}

size_t FileStream::println(const char* str) {
  return file.println(str);
}

size_t FileStream::println(char c) {
  return file.println(c);
}

size_t FileStream::println(unsigned char c, int base) {
  return file.println(c, base);
}

size_t FileStream::println(int n, int base) {
  return file.println(n, base);
}

size_t FileStream::println(unsigned int n, int base) {
  return file.println(n, base);
}

size_t FileStream::println(long n, int base) {
  return file.println(n, base);
}

size_t FileStream::println(unsigned long n, int base) {
  return file.println(n, base);
}

size_t FileStream::println(double n, int digits) {
  return file.println(n, digits);
}

size_t FileStream::printf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  size_t result = file.printf(format, args);
  va_end(args);
  return result;
}
