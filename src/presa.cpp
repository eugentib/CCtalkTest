#include "presa.h"

Machine_State machine_state;
Machine_Config machine_config;
Balot balot;

JsonDocument myJsonDoc;
int8_t process_lcd(char *lcd)
{
    if (IS_SCROLL(lcd))
    {
        add2scroll(&lcd[16]);
        if (IS_NR_BAL(&lcd[16]))
        {
            machine_config.numar_baloti = atoi(&lcd[25]);
            balot.nr = machine_config.numar_baloti + 1;
            return -1;
        }
        if (IS_VSW(&lcd[16]))
        {
            memcpy(machine_config.vsw, &lcd[21], 6);
            machine_config.vsw[6] = 0;
            return -101;
        }
        if (IS_MODEL(&lcd[16]))
        {
            memcpy(machine_config.model, &lcd[25], 5);
            machine_config.model[5] = 0;
            return -102;
        }
        return -2;
    }
    if (IS_INAINTE(lcd))
    {
        if (machine_state.presa_inapoi)
        {
            balot.presari++;
            machine_state.presa_inainte = true;
            machine_state.presa_inapoi = false;
            if (balot.inceput == 0)
            {
                time(&balot.inceput);
                if (balot.inceput < 1000000000)
                    balot.inceput = 0;
            }
            return 2;
        }
        else
            return 22;
    }
    if (IS_INAPOI(lcd))
    {
        if (machine_state.presa_inainte)
        {
            machine_state.presa_inainte = false;
            machine_state.presa_inapoi = true;
            saveBalotData("balot_last.data", myJsonDoc);
            return 3;
        }
    }
    if (IS_INCHIDERE_USA(lcd))
    {
        machine_state.presa_inainte = true;
        return 7;
    }
    if (IS_BALOT_GATA(lcd))
    {
        if (machine_state.presa_inainte)
        {
            machine_state.balotul_e_gata = true;
            machine_state.presa_inainte = false;
            time(&balot.sfarsit);
            if (balot.sfarsit < 1000000000)
            {
                if (balot.inceput != 0)
                    balot.sfarsit = balot.inceput + 36000;
                else
                    balot.sfarsit = 0;
            }
            if (balot.nr == 0)
                balot.nr = machine_config.numar_baloti + 1;
            char nume[30];
            sprintf(nume, "/detrimis/balot_%d.data", balot.nr);
            saveBalotData(nume, myJsonDoc);
            saveMachineConfig();
            balot.nr++;
            balot.presari = 0;
            balot.inceput = 0;
            balot.sfarsit = 0;
            return 4;
        }
        return 5;
    }
    return 0;
}

void readAtStartup(void)
{
    readMachineConfig();
    readMachineState();

    // Read last balot data if exists
    readJson("balot_last.data", myJsonDoc);
    if (myJsonDoc.size() > 0)
    {
        balot.nr = myJsonDoc["NumarBalot"];
        balot.presari = myJsonDoc["NumarPresari"];
        balot.inceput = myJsonDoc["StartAt"];
        balot.sfarsit = myJsonDoc["EndAt"];
    }
}

void readMachineConfig()
{
    myJsonDoc.clear();
    // Read file
    readJson("machine.cfg", myJsonDoc);

    // Copy values from the JsonDocument to the Config
    machine_config.numar_baloti = myJsonDoc["numar_baloti"] | 0;
    machine_config.data_interventie = myJsonDoc["data_interventie"] | 1697482780;
    machine_config.data_revizie = myJsonDoc["data_revizie"] | 1697482780;
    strlcpy(machine_config.seria,              // <- destination
            myJsonDoc["seria"] | "A123456789", // <- source
            sizeof(machine_config.seria));     // <- destination's capacity
    strlcpy(machine_config.model,              // <- destination
            myJsonDoc["model"] | "???",        // <- source
            sizeof(machine_config.model));     // <- destination's capacity
    strlcpy(machine_config.vsw,                // <- destination
            myJsonDoc["vsw"] | "???",          // <- source
            sizeof(machine_config.vsw));       // <- destination's capacity
}

void saveMachineConfig()
{
    myJsonDoc.clear();
    myJsonDoc["numar_baloti"] = machine_config.numar_baloti;
    myJsonDoc["data_interventie"] = machine_config.data_interventie;
    myJsonDoc["data_revizie"] = machine_config.data_revizie;
    myJsonDoc["seria"] = machine_config.seria;
    myJsonDoc["model"] = machine_config.model;
    myJsonDoc["vsw"] = machine_config.vsw;
    saveJson("machine.cfg", myJsonDoc);
}

void readMachineState()
{
    myJsonDoc.clear();

    // Read file
    readJson("machine.state", myJsonDoc);

    // Copy values from the JsonDocument to the State
    machine_state.presa_inainte = myJsonDoc["presa_inainte"] | false;
    machine_state.presa_inapoi = myJsonDoc["presa_inapoi"] | true;
    machine_state.balotul_e_gata = myJsonDoc["balotul_e_gata"] | false;
    machine_state.numar_presari = myJsonDoc["numar_presari"] | 0;
    machine_state.start = myJsonDoc["start"] | 0;
}

void saveMachineState()
{
    myJsonDoc.clear();

    // Copy values from the State to the JsonDocument
    myJsonDoc["presa_inainte"] = machine_state.presa_inainte;
    myJsonDoc["presa_inapoi"] = machine_state.presa_inapoi;
    myJsonDoc["balotul_e_gata"] = machine_state.balotul_e_gata;
    myJsonDoc["numar_presari"] = machine_state.numar_presari;
    myJsonDoc["start"] = machine_state.start;

    // Save file
    saveJson("machine.state", myJsonDoc);
}

void saveJson(const char *nume, JsonDocument &doc)
{
    auto file = LittleFS.open(nume, "w");
    if (file)
    {
        serializeJson(doc, file);
        file.close();
    }
}

void readJson(const char *nume, JsonDocument &doc)
{
    auto file = LittleFS.open(nume, "r");
    if (file)
    {
        deserializeJson(doc, file);
        file.close();
    }
}

void saveBalotData(const char *nume, JsonDocument &doc)
{
    doc.clear();
    doc[F("StartAt")] = balot.inceput;
    doc[F("NumarBalot")] = balot.nr;
    doc[F("NumarPresari")] = balot.presari;
    doc[F("EndAt")] = balot.sfarsit;
    saveJson(nume, doc);
}

void readBalotData(const char *nume, JsonDocument &doc)
{
    readJson(nume, doc);

    balot.inceput = doc["StartAt"];
    balot.presari = doc["NumarPresari"];
    balot.sfarsit = doc["EndAt"];
    balot.nr = doc["NumarBalot"];
    doc.clear();
}

size_t LittleFSFilesize(const char *filename)
{
    auto file = LittleFS.open(filename, "r");
    size_t filesize = file.size();
    // Don't forget to clean up!
    file.close();
    return filesize;
}

bool appendFile(fs::FS &fs, const char *path, const char *message)
{ // return true on succes
    File file = fs.open(path, "a");
    if (!file)
    {
        //        debugPrintln("- failed to open file for appending");
        return false;
    }
    if (file.print(message))
    {
        file.close();
        return true;
    }
    else
    {
        file.close();
        return false;
    }
}

void listDir(const char *dirname)
{
    debugPrintf(PSTR("Listing directory: %s\n"), dirname);

    Dir root = LittleFS.openDir(dirname);

    while (root.next())
    {
        File file = root.openFile("r");
        debugPrint(PSTR("  FILE: "));
        debugPrint(root.fileName());
        debugPrint(PSTR("  SIZE: "));
        debugPrint(file.size());
        time_t cr = file.getCreationTime();
        time_t lw = file.getLastWrite();
        file.close();
        struct tm *tmstruct = localtime(&cr);
        debugPrintf(PSTR("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n"), (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
        tmstruct = localtime(&lw);
        debugPrintf(PSTR("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n"), (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
}

void add2scroll(char *add)
{
    uint8_t num_chars = 15;
    uint8_t founAt; // = custom_strstr(scr, add, num_chars);

    if ((founAt = custom_strstr(scr, add, num_chars)) != 0xFF)
    {
        replace_text(scr, founAt, add, 16);
    }
    else if ((founAt = custom_strstr(scr, add, 14)) != 0xFF)
    {
        replace_text(scr, founAt, add, 16);
    }
    else if (!scf && (memcmp_P(add, PSTR("*** MASCHINENFAB"), 16) == 0))
    {
        replace_text(scr, 0, add, 16);
    }
}

uint8_t custom_strstr(char *text, char *fragment, uint8_t num_chars)
{
    for (uint8_t i = 0; i < MAX_SCROLL_LENGTH; i++)
    {
        uint8_t j;
        for (j = 0; j < num_chars; j++)
        {
            if (text[i + j] != fragment[j])
            {
                break;
            }
        }
        if (j == num_chars)
        {
            return i;
        }
    }
    return 0xFF; // Fragmentul nu a fost găsit
}

void replace_text(char *text, uint8_t index, const char *fragment, uint8_t num_chars)
{
    if (index + num_chars > 119)
        return;
    uint8_t i;
    for (i = 0; i < num_chars; i++)
    {
        text[index + i] = fragment[i];
    }
    if (scl < index + i)
        text[index + i] = 0;
    scl = strlen(&text[0]);

    if (memcmp(scr, oldscr, scl) != 0)
    {
        memcpy(oldscr, scr, scl);
        new_scr = true;
    }
    else
        new_scr = false;
}
