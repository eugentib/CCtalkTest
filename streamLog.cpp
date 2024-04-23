#include <LittleFS.h>

class FileStream : public Stream {
private:
  File file;
  const char* baseFilename;
  size_t maxFileSize;
  size_t currentFileSize;
  int fileIndex;
  void (*newFileCallback)(const char* filename);

  void openNextFile() {
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

public:
  FileStream(const char* filename, size_t maxSize, void (*callback)(const char*) = nullptr) {
    baseFilename = filename;
    maxFileSize = maxSize;
    currentFileSize = 0;
    fileIndex = 0;
    newFileCallback = callback;
    openNextFile();
  }

  ~FileStream() {
    file.close();
  }

  virtual size_t write(uint8_t data) {
    if (currentFileSize >= maxFileSize) {
      openNextFile();
    }
    size_t bytesWritten = file.write(data);
    currentFileSize += bytesWritten;
    return bytesWritten;
  }

  virtual size_t write(const uint8_t* buffer, size_t size) {
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

  // Restul metodelor (print, println, printf) rămân neschimbate
};

  virtual void flush() {
    file.flush();
  }

  size_t print(const String& s) {
    return file.print(s);
  }

  size_t print(const char* str) {
    return file.print(str);
  }

  size_t print(char c) {
    return file.print(c);
  }

  size_t print(unsigned char c, int base = DEC) {
    return file.print(c, base);
  }

  size_t print(int n, int base = DEC) {
    return file.print(n, base);
  }

  size_t print(unsigned int n, int base = DEC) {
    return file.print(n, base);
  }

  size_t print(long n, int base = DEC) {
    return file.print(n, base);
  }

  size_t print(unsigned long n, int base = DEC) {
    return file.print(n, base);
  }

  size_t print(double n, int digits = 2) {
    return file.print(n, digits);
  }

  size_t println() {
    return file.println();
  }

  size_t println(const String& s) {
    return file.println(s);
  }

  size_t println(const char* str) {
    return file.println(str);
  }

  size_t println(char c) {
    return file.println(c);
  }

  size_t println(unsigned char c, int base = DEC) {
    return file.println(c, base);
  }

  size_t println(int n, int base = DEC) {
    return file.println(n, base);
  }

  size_t println(unsigned int n, int base = DEC) {
    return file.println(n, base);
  }

  size_t println(long n, int base = DEC) {
    return file.println(n, base);
  }

  size_t println(unsigned long n, int base = DEC) {
    return file.println(n, base);
  }

  size_t println(double n, int digits = 2) {
    return file.println(n, digits);
  }

  size_t printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    size_t result = file.printf(format, args);
    va_end(args);
    return result;
  }
};
