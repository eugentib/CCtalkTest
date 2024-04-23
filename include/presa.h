#ifndef PRESA_H
#define PRESA_H

#include "globale.h"

#define IS_SCROLL(X) (memcmp_P(X, PSTR("PRESA E OK     "),15) == 0)
#define IS_NR_BAL(X) (memcmp_P(X, PSTR(" BALOTI :"),9) == 0)
#define IS_MODEL(X) (memcmp_P(X, PSTR("*** DIXI "),9) == 0)
#define IS_VSW(X) (memcmp_P(X, PSTR(" *** V"),6) == 0)

//#define IS_ERR15(X) (strcmp_P(X, PSTR("EROARE 15")) == 0)

#define IS_SENSORFAULT(X) (memcmp_P(X, PSTR("PRESA E OK     KSENSOR FAULT !  "),32) == 0)
#define IS_KIPP(X) (memcmp_P(X, PSTR("PRESA E OK     KKIPPTASTER:   15"),32) == 0)
#define IS_INAINTE(X) (memcmp_P(X, PSTR("PLACA INAINTE"),13) == 0)
#define IS_INAPOI(X) (memcmp_P(X, PSTR("PLACA INAPOI"),12) == 0)  
#define IS_INCHIDERE_USA(X) (memcmp_P(X, PSTR("INCHIDERE USA"),13) == 0)  
#define IS_BALOT_GATA(X) (memcmp_P(X, PSTR("BALOTUL E GATA  GATA PT. LEGAT  "),32) == 0)

/*
            ***     structura fisierelor de baloti      ***
 nume - numarul balotului . bal
 contine
    data si ora inceput(timestamp)
    numar presari
    lista erori (err_cod,timestamp\n)
    data si ora sfarsit(timestamp)


        ***    structura fisierului de masina   ***
nume - mac.dat
contine
    seria masina
    numar baloti
    data ultima revizie
    data ultima interventie


        ***    structura fisierului de log erori masina   ***
nume - mac.log
contine
    seria masina
    lista erori(err_cod,timestamp\n)
*/


// error codes to be stored with balot data
#define ERROR10 10
#define ERROR11 11
#define ERROR12 12


int8_t process_lcd(char *lcd);
void readAtStartup(void);
void readMachineConfig();
void saveMachineConfig();
void readMachineState();
void saveMachineState();
void saveJson(const char *nume, JsonDocument &doc);
void readJson(const char *nume, JsonDocument &doc);
void saveBalotData(const char *nume, JsonDocument &doc);
void readBalotData(const char *nume, JsonDocument &doc);
size_t LittleFSFilesize(const char *filename);
bool appendFile(fs::FS &fs, const char *path, const char *message);
void listDir(const char *dirname);
void add2scroll(char *add);
uint8_t custom_strstr(char *text, char *fragment, uint8_t num_chars);
void replace_text(char *text, uint8_t index, const char *fragment, uint8_t num_chars);

#endif