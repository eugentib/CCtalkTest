#include "my_mqtt.h"

const char *Topic(const char *nt)
{
  unTopic[21] = '\0';
  snprintf(sTopic, sizeof(sTopic) - 1, "%s/%s", unTopic, nt);
  unTopic[21] = '/';
  return sTopic;
}

void loose_time(uint32 ms)
{
  ms += millis();
  while (ms > millis())
    ;
}

void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos)
{
}

void onMqttUnsubscribe(const uint16_t &packetId)
{
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  (void)reason;
  debugPrintf(PSTR("Disconnected from MQTT _%d_\n"), reason);

  if (WiFi.isConnected())
  {
    mqttReconnectTimer.once(5, connectToMqtt);
  }
}

void onMqttConnect(bool sessionPresent)
{
  debugPrintf(PSTR("Connected to MQTT _%d_\n"), sessionPresent);
  sending_file.busy = false;
  unTopic[22] = 0;
  sprintf(sBuff, "%s%s", unTopic, WiFi.localIP().toString().c_str()); // Append str2 to str1
  unTopic[22] = 'l';
  mqttClient.publish(PubTopic, 0, false, sBuff);
  mqttClient.subscribe(PubTopic, 1);
  mqttClient.subscribe(swTopic, 1);
  mqttClient.subscribe(unTopic, 1);
  mqttClient.subscribe(Topic("write_file"), 1);
  mqttClient.publish(unTopic, 0, false, "ONLINE");
  mqttClient.publish(Topic("FW_ver"), 0, false, FIRMWARE_VERSION);
  debugPrintf(PSTR("Published FW version: %s\n"), FIRMWARE_VERSION);
}

void connectToMqtt()
{
  if (mqttClient.connected())
  {
    debugPrintln(PSTR("Already connected to MQTT. Skipping connect."));
    return;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    debugPrintln(PSTR("Connecting to MQTT..."));
    mqttClient.connect();
  }
  else
    debugPrintln(PSTR("Don't reconnect MQTT we are NOT connected to WiFi"));
}

struct Sending_File sending_file;

void mqttListDir(const char *dirname)
{
  size_t p = 0;
  p += sprintf(_debugOutputBuffer, PSTR("Listing directory: %s\n"), dirname);

  Dir root = LittleFS.openDir(dirname);

  while ((p < 1900) && root.next())
  {
    File file = root.openFile("r");
    p += sprintf(_debugOutputBuffer + p, PSTR("%s  \tSize: %d"), root.fileName().c_str(), file.size());
    time_t lw = file.getLastWrite();
    file.close();
    struct tm *tmstruct = localtime(&lw);
    p += sprintf(_debugOutputBuffer + p, PSTR("  \tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n"), (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
  mqttClient.publish(Topic("R_sw"), 1, false, _debugOutputBuffer);
}

// Functie care trimite catre server fisirele facuta in perioada offline
bool sendOffFiles()
{
  char full_filename[40];
  Dir detrimis = LittleFS.openDir("/detrimis");
  //  debugPrintf(PSTR("Check if there are files to send...\n"));
  while (detrimis.next())
  {
    debugPrintf(PSTR("System is offline or busy, will retry later to send file %s\n"), detrimis.fileName().c_str());
    if (strncmp_P(detrimis.fileName().c_str(), PSTR("balot_"), 6) == 0)
    {
      strcat(full_filename, "/detrimis/");
      strcat(full_filename, detrimis.fileName().c_str());
      if (!sending_file.busy && mqttClient.connected())
      {
        debugPrintf(PSTR("Scheduled send %s via MQTT\n"), full_filename);
        sending_file.move_it = true;
        async_send_file2mqtt(full_filename);
        return true;
      }
      else
      {
        debugPrintf(PSTR("System is offline or busy, will retry later to send file %s\n"), full_filename);
        return false;
      }
      return true;
    }
  }
  return false;
}

void sendOfflineFiles(void)
{
  if (!sendOffFiles())
    return;
}

bool async_send_file2mqtt(const char *name)
{
  if (sending_file.busy)
  {
    if (strcmp(name, sending_file.name))
      return false;
    else
    { //  open file and continue sending it
      File file = LittleFS.open(name, "r");
      if (!file)
      {
        snprintf_P(sBuff, sizeof(sBuff) - 1, PSTR("Could not reopen %s file for part %d"), name, sending_file.part);
        mqttClient.publish(Topic("R_sw"), 1, false, sBuff);
        return false;
      }
      else
      {
        file.seek(sending_file.pos);
        size_t p = 0;
        // Adauga numele fisierului la buffer urmat de ':' si numarul partii
        p += sprintf(_debugOutputBuffer + p, "%s\npart_%d_\n", sending_file.name, sending_file.part);
        while ((p < 2048) && file.available())
        {
          _debugOutputBuffer[p++] = char(file.read());
        }
        if (file.available()) // more to send
        {
          sending_file.pos += p - 10;
          sending_file.part++;
          sending_file.packetId = mqttClient.publish(Topic("file_contentp"), 1, false, _debugOutputBuffer, p);
          file.close();
          return false;
        }
        else // done sending
        {
          sending_file.pos = 0;
          sending_file.done = true;
          strcpy(sending_file.name, "");
          sending_file.packetId = mqttClient.publish(Topic("file_contentp"), 1, false, _debugOutputBuffer, p);
          file.close();
          return false;
        }
      }
    }
  }
  File file = LittleFS.open(name, "r");
  if (!file)
  {
    snprintf_P(sBuff, sizeof(sBuff) - 1, PSTR("Could not open \'%s\' file"), name);
    mqttClient.publish(Topic("R_sw"), 1, false, sBuff);
    return false;
  }
  else
  {
    sending_file.done = false;
    sending_file.busy = true;
    strcpy(sending_file.name, name);
    size_t filesize = file.size();
    if (filesize > 2017) // File is greater than 2017b, send in parts
    {
      size_t p = 0;
      sending_file.part = 1;
      // Adauga numele fisierului la buffer urmat de ':' si numarul partii
      p += sprintf(_debugOutputBuffer + p, "%s\npart_%d_\n", sending_file.name, sending_file.part);

      while ((p < 2048) && file.available())
      {
        _debugOutputBuffer[p++] = char(file.read());
      }
      sending_file.pos += p - 10;
      sending_file.part++;
      sending_file.packetId = mqttClient.publish(Topic("file_contentp"), 1, false, _debugOutputBuffer, p);
      file.close();
      return false;
    }
    else
    {
      size_t p = 0;
      // Adauga numele fisierului la buffer urmat de ':'
      p += sprintf(_debugOutputBuffer + p, "%s:", name);
      while ((p < 2048) && file.available())
      {
        _debugOutputBuffer[p++] = char(file.read());
      }
      sending_file.packetId = mqttClient.publish(Topic("file_content"), 1, false, _debugOutputBuffer, p);
      file.close();
      sending_file.done = true;
      return true;
    }
  }
}

void myreset()
{
  ESP.restart();
}

void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                   const size_t &len, const size_t &index, const size_t &total)
{
  (void)payload;
  if (len == 0)
    return;
  if (strcmp(swTopic, topic) == 0)
  {
    if (memcmp_P(payload, PSTR("reset"), len) == 0)
    {
      Pstr2char(PSTR("Reseting in 3 seconds by MQTT command..."));
      mqttClient.publish(Topic("R_sw"), 1, false, sBuff);
      mqttReconnectTimer.once(3, ESP.restart);
    }
    else if (memcmp_P(payload, PSTR("fs_update"), len) == 0)
    {
      ck_upd = true;
      fs_upd = true;
    }
    else if (memcmp_P(payload, PSTR("update"), len) == 0)
      ck_upd = true;
    else if (memcmp_P(payload, PSTR("simulate"), 8) == 0)
    {
      if (len > 8 && payload[8] > 0x2F && payload[8] < 0x3A)
      {
        Serial.print("Sim");
        Serial.print(payload[8]);
      }
      else
        Serial.print("Simu");
    }
    else if (memcmp_P(payload, PSTR("stop"), len) == 0)
      Serial.print("stop");
    else if (memcmp_P(payload, PSTR("test"), len) == 0)
      Serial.print("TeSt");
    else if (memcmp_P(payload, PSTR("readCrash"), len) == 0)
    {
      // "clear" the buffer before serving a request
      //  strcpy(_debugOutputBuffer, "");
      //      size_t l = SaveCrash.print(_debugOutputBuffer, 2048);
      //    mqttClient.publish(Topic("crash"), 0, false, _debugOutputBuffer, l - 1);
      return;
    }
    else if (memcmp_P(payload, PSTR("clearCrash"), len) == 0)
    {
      //      SaveCrash.clear();
      //    mqttClient.publish(Topic("R_sw"), 0, false, "Crash cleared");
      return;
    }
    else if (memcmp_P(payload, PSTR("fread:"), 6) == 0)
    {
      unsigned char fn[30];
      memccpy(fn, &payload[6], len - 6, 30);
      fn[29] = len - 6;
      if (fn[29] < 30)
        fn[fn[29]] = 0;
      else
        fn[29] = 0;
      async_send_file2mqtt((const char *)fn);
      return;
    }
    else if (memcmp_P(payload, PSTR("fdir:"), 5) == 0)
    {
      unsigned char fn[30];
      memccpy(fn, &payload[5], len - 5, 30);
      fn[29] = len - 5;
      if (fn[29] < 30)
        fn[fn[29]] = 0;
      else
        fn[29] = 0;
      mqttListDir((const char *)fn);
      return;
    }
    else if (memcmp_P(payload, PSTR("areset"), len) == 0)
    {
      HiARESET();
      delay(50);
      LoARESET();
      return;
    }
    else if (memcmp_P(payload, PSTR("astop"), len) == 0)
    {
      HiARESET();
      return;
    }
    else if (memcmp_P(payload, PSTR("clearAllBaloti"), len) == 0)
    {
      Pstr2char(PSTR("Clearing all baloti data"));
      mqttClient.publish(Topic("R_sw"), 1, false, sBuff);
      stergeFisiereBaloti();
      return;
    }
    else if (memcmp_P(payload, PSTR("MoveAllBaloti"), len) == 0)
    {
      Pstr2char(PSTR("Moveing all baloti data"));
      mqttClient.publish(Topic("R_sw"), 1, false, sBuff);
      MoveFisiereBaloti();
      return;
    }
    else if (memcmp_P(payload, PSTR("fdelete:"), 8) == 0)
    {
      unsigned char fn[30];
      memccpy(fn, &payload[8], len - 8, 30);
      fn[29] = len - 8;
      if (fn[29] < 30)
        fn[fn[29]] = 0;
      else
        fn[29] = 0;
      if (LittleFS.remove((char *)fn))
      {
        Pstr2char2(PSTR("File \'%s\'deleted"), (char *)fn);
        mqttClient.publish(Topic("R_sw"), 0, false, sBuff);
      }
      return;
    }
    else if (memcmp_P(payload, PSTR("aflash"), len) == 0)
    {
      Pstr2char(PSTR("Start ALCD update"));
      mqttClient.publish(Topic("R_sw"), 1, false, sBuff);
      atmega_upd = true;
      return;
    }
    else if (memcmp_P(payload, PSTR("force"), len) == 0)
      memcpy_P(lastSent, PSTR("valori Corecte"), 14);
  }
  else if (strcmp(Topic("write_file"), topic) == 0)
  {
    File file;
    static char nume[30];
    if (index)
    {
      file = LittleFS.open(nume, "a");
      file.write(payload, len);
      file.close();
    }
    else
    {
      char *data;
      data = (char *)memchr(payload, ':', len);
      if (data != NULL)
      {
        memcpy(nume, payload, data - payload);
        nume[data - payload] = 0;
        file = LittleFS.open(nume, "w");
        data += 1;
        file.write(data, len - (data - payload));
        file.close();
      }
      else
        nume[0] = 0; //
    }
    if (index + len == total)
    {
      async_send_file2mqtt(nume);
      nume[0] = 0;
    }
  }
  else if (strcmp(PubTopic, topic) == 0)
  {
    if (memcmp_P(payload, PSTR("update"), len) == 0)
      ck_upd = true;
  }
}

void ltohex(unsigned long n, char *s)
{
  char c;
  for (int i = 0; i < 8; i++)
  {
    c = n & 0xF;
    if (c > 9)
      c += 0x37;
    else
      c += 0x30;
    *s++ = c;
    n >>= 4;
  }
  *s = 0;
}

void onFlashEnd(int f)
{
  switch (f)
  {
  case 0:
    Pstr2char(PSTR("Falsh error, NO SYNK"));
    mqttClient.publish(Topic("R_sw"), 0, false, sBuff);
    break;
  case -1:
    Pstr2char(PSTR("Flash success"));
    mqttClient.publish(Topic("R_sw"), 0, false, sBuff);
    break;
  default:
    Pstr2char2(PSTR("Flash end with %s%%"), hexTemp);
    mqttClient.publish(Topic("R_sw"), 0, false, sBuff);
    break;
  }
}

void onMqttPublish(const uint16_t &packetId)
{
  //  GPIO0c;
  if (sending_file.packetId == packetId)
  {
    if (!sending_file.done)
      async_send_file2mqtt(sending_file.name);
    else
    { // Fisierul s-a trimis complet
      if (sending_file.move_it)
      {
        sending_file.move_it = false;
        // muta fisierul in folderul trimise
        char newPath[40];
        strcpy(newPath, "/trimise/");
        strcat(newPath, nameFromFullPath(sending_file.name));
        if (LittleFS.rename(sending_file.name, newPath))
          debugPrintf(PSTR("Renamed %s to %s ...\n"), sending_file.name, newPath);
        else
          debugPrintf(PSTR("Error renaming %s to %s\n"), sending_file.name, newPath);
      }
      sending_file.busy = false;
    }
  }
}
void SetMQTTcallbacks()
{
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials("eumqtt", "mqttp");
  mqttClient.setWill("LWT", 0, false, strMAC, 12);
}

// Sterge toate fisierele balot*.data din folderul root al filesystem-ului LittleFS
void stergeFisiereBaloti(void)
{
  Dir root = LittleFS.openDir("/");

  while (root.next())
  {
    String fileName = root.fileName();
    if (fileName.startsWith("balot") && fileName.endsWith(".data"))
      LittleFS.remove(fileName);
  }

  Dir subdir = LittleFS.openDir("/trimise/");
  while (subdir.next())
  {
    String fileName = subdir.fileName();
    if (fileName.startsWith("balot") && fileName.endsWith(".data"))
      LittleFS.remove("/trimise/" + fileName);
  }
  subdir = LittleFS.openDir("/detrimis/");
  while (subdir.next())
  {
    String fileName = subdir.fileName();
    if (fileName.startsWith("balot") && fileName.endsWith(".data"))
      LittleFS.remove("/detrimis/" + fileName);
  }
}
// Muta toate fisierele balot*.data din folderul trimise in detrimis al filesystem-ului LittleFS
void MoveFisiereBaloti(void)
{
  Dir subdir = LittleFS.openDir("/trimise/");
  while (subdir.next())
  {
    String fileName = subdir.fileName();
    if (LittleFS.rename("/trimise/" + fileName, "/detrimis/" + fileName))
      debugPrintf(PSTR("Renamed %s to %s ...\n"), ("/trimise/" + fileName).c_str(), ("/detrimis/" + fileName).c_str());
    else
      debugPrintf(PSTR("Error renaming %s to %s\n"), ("/trimise/" + fileName).c_str(), ("/detrimis/" + fileName).c_str());
  }
}
