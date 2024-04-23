#include "globale.h"


char FIRMWARE_VERSION[]="0.76";

const char *PubTopic = "CCtalkWL/ESP8266_All"; // Topic to publish
char unTopic[30] = "CCtalkWL/";                // uniq topic to publish
char swTopic[30];                              // sw topic to publish
char _debugOutputBuffer[2048];
uint8_t lastSent[34] = {"New"}; // for presenting a stream of characters obtained from the LCD data bus.
char sBuff[256];                // for sprintf
char sTopic[80];                // for sprintf
char strMAC[12] = {0};

char scr[MAX_SCROLL_LENGTH], oldscr[MAX_SCROLL_LENGTH];
uint8_t scl = 0;
boolean scf = false;
boolean new_scr = false;
time_t timeDiff = 0;

boolean ck_upd = false, fs_upd = false;
boolean atmega_upd = false;

stkSrvc stkSrv;

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
Ticker SendFiles;

EspSaveCrash SaveCrash(10, 1000, false);
WiFiUDP udp;
IPAddress SendIP(255, 255, 255, 255);


void setMacAddress(const char *filePath)
{
    WiFi.mode(WIFI_AP_STA);

    // Verifică existența fișierului new.mac în LittleFS
    if (LittleFS.exists(filePath))
    {
        // Citește conținutul fișierului
        File file = LittleFS.open(filePath, "r");
        if (file)
        {
            // Citește adresa MAC din fișier
            String macString = file.readString();
            macString.trim();

            if (macString.length() == 12)
            {
                uint8_t mac[6];
                for (int i = 0; i < 6; i++)
                {
                    String octetString = macString.substring(i * 2, i * 2 + 2);
                    mac[i] = strtol(octetString.c_str(), nullptr, 16);
                }

                // Setează adresa MAC
                wifi_set_macaddr(STATION_IF, mac);
            }
            else
            {
                debugPrintln(PSTR("Invalid MAC address format"));
            }
            file.close();
        }
        else
        {
            debugPrintln(PSTR("Failed to open file"));
        }
    }
    else
    {
        debugPrintln(PSTR("Nu exista fisier, nu setam MAC diferit..."));
    }

    uint8_t currentMac[6];
    wifi_get_macaddr(STATION_IF, currentMac);

    debugPrint(PSTR("Adresa MAC curenta: "));
    for (int i = 0; i < 6; i++)
    {
        debugPrint(currentMac[i], HEX);
        if (i < 5)
        {
            debugPrint(PSTR(":"));
        }
    }
    debugPrintln();
}

void onNtpSynchronized(void)
{

    // Calculează diferența dintre timpul local și timpul NTP
    time_t ntpTimestamp = time(nullptr);
    uint32_t localTimestamp = millis() / 1000;
    timeDiff = ntpTimestamp - localTimestamp;

    debugPrintf(PSTR("Adjusting timestamps for files with timestamp_diff %u\n"), timeDiff);

    // Apelează funcția pentru a ajusta timestamp-urile fișierelor de baloți
    adjustBalotTimestamps();
}

// Funcție pentru a ajusta timestamp-urile fișierelor de baloți
void adjustFileTimestamps(const char *folderPath)
{
    Dir dir = LittleFS.openDir(folderPath);
    while (dir.next())
    {
        if (dir.fileName().startsWith("balot_"))
        {
            File file = dir.openFile("r");
            if (file)
            {
                myJsonDoc.clear();
                debugPrintf(PSTR("Adjusting timestamps for file: %s..."), dir.fileName().c_str());
                DeserializationError error = deserializeJson(myJsonDoc, file);
                file.close();

                if (!error)
                {
                    time_t timpInBatot;
                    time_t adjustedTimestamp;
                    bool save = false;

                    timpInBatot = myJsonDoc["StartAt"];
                    if (timpInBatot<1000000000)
                    {
                    time_t adjustedTimestamp = timpInBatot + timeDiff;
                    debugPrintf(PSTR(" from %lld to %lld "), timpInBatot, adjustedTimestamp);
                    myJsonDoc["StartAt"] = adjustedTimestamp;
                    save = true;
                    }

                    timpInBatot = myJsonDoc["EndAt"];
                    if (timpInBatot && timpInBatot<1000000000)
                    {
                        adjustedTimestamp = timpInBatot + timeDiff;
                        myJsonDoc["EndAt"] = adjustedTimestamp;
                        debugPrintf(PSTR("and from %lld to %lld "), timpInBatot, adjustedTimestamp);
                        save = true;
                    }
                    // Salvează balotul actualizat
                    if(save){
                        file = LittleFS.open(folderPath + dir.fileName(), "w");
                        if (file) {
                            serializeJson(myJsonDoc, file);
                            file.close();
                        debugPrintf(PSTR("Ok!\n"));
                        }
                    }else{
                        debugPrintf(PSTR("Not needed\n"));
                    }
                }
                else
                    debugPrintf(PSTR("Failed!\n"));
            }
        }
    }
}

void adjustBalotTimestamps(void)
{
    if (timeDiff == 0)
    {
        return;
    }

    // Ajustează timestamp-urile pentru fișierele din folderul "/"
    adjustFileTimestamps("/");

    // Ajustează timestamp-urile pentru fișierele din folderul "/detrimis"
    adjustFileTimestamps("/detrimis/");
}

// Funcții helper pentru citirea și scrierea fișierelor
void readFile(const char *path, char *buffer, size_t maxSize)
{
    File file = LittleFS.open(path, "r");
    if (!file)
    {
        buffer[0] = '\0';
        return;
    }
    size_t size = file.size();
    if (size > maxSize)
    {
        size = maxSize;
    }
    file.readBytes(buffer, size);
    buffer[size] = '\0';
    file.close();
}

void writeFile(const char *path, const char *content)
{
    File file = LittleFS.open(path, "w");
    if (file)
    {
        file.print(content);
        file.close();
    }
}
void handleWiFiConnect()
{
    debugPrintln(PSTR("Conectat la rețeaua WiFi!"));
    // Codul pentru a gestiona conectarea WiFi
}

void handleWiFiDisconnect()
{
    debugPrintln(PSTR("Conexiunea WiFi a fost pierdută!"));
}

void onFlashEnd_s(int f)
{
    Serial.flush();
    Serial.swap();
    atmega_upd = false;
    debugPrint(PSTR("Flash finshed with:"));
    debugPrintln(f);
    if (f == 55)
    {
        debugPrint(PSTR("Deleting file: first.run\n"));
        if (LittleFS.remove("/first.run"))
        {
            debugPrintln(PSTR("File deleted"));
        }
        else
        {
            debugPrintln(PSTR("Delete failed"));
        }
    }
    Serial.flush();
    Serial.swap();
}

void update_started()
{
    Pstr2char(PSTR("HTTP update process started"));
    mqttClient.publish(Topic("R_sw"), 0, false, sBuff);
}

void update_finished()
{
    Pstr2char(PSTR("HTTP update process finished"));
    mqttClient.publish(Topic("R_sw"), 0, false, sBuff);
}

void update_progress(int cur, int total)
{
    //  debugPrintf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err)
{
    //  debugPrintf("CALLBACK:  HTTP update fatal error code %d\n", err);
    snprintf_P(sBuff, sizeof(sBuff) - 1, PSTR("CALLBACK:  HTTP update fatal error code %d\n"), err);
    mqttClient.publish(Topic("R_sw"), 0, false, sBuff);
}

void getMAC(char *smac)
{
    uint8_t bmac[6];
    wifi_get_macaddr(STATION_IF, bmac);
    sprintf_P(smac, PSTR("%02X%02X%02X%02X%02X%02X"), bmac[0], bmac[1], bmac[2], bmac[3], bmac[4], bmac[5]);
}

void changeState()
{
    return;
    if (LED)
        GPOC = (1 << LED_BUILTIN);
    else
        GPOS = (1 << LED_BUILTIN);
}

void setTopics(void)
{
    getMAC(strMAC);
    strcat(unTopic, strMAC);
    strcpy(swTopic, unTopic);
    strcat(unTopic, "/lcd");
    strcat(swTopic, "/sw");
}

char * nameFromFullPath(const char * fullPath) {
    char * name = strrchr(fullPath, '/');
  if (name == NULL) {
    name = (char*)fullPath;
  } else {
    name++;
  }
  return name;
}