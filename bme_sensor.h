#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include <Adafruit_BME280.h>

/* * * * * * * * * * * * * * * *
 * Globale Konstanten          *
 * für #if-Anweisungen         *
 * * * * * * * * * * * * * * * */

#define INDICATOR_LED   1   //Blinken der LED: zur Anzeige ob der uC läuft
#define DEBUG           0   //Debugmode: Ausgabe von Indikatoren über Serial-Schnittstelle
#define FIX_TIME_ONCE   0   //beim Bootvorgang die Kompilierzeit in die RTC schreiben


/* * * * * * * * * * * * * * * *
 * Konstanten                  *
 * * * * * * * * * * * * * * * */
 
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

#define upperbuttonpowersource  12
#define lowerbuttonpowersource  6
#define upperbuttonsensor       3
#define lowerbuttonsensor       9

#define numberofdisplaymodes    3

#define minimumstepfordrawing   0.3

constexpr uint8_t    array_len =               24;
constexpr uint8_t    numberOfMeassurements =   5;
constexpr uint16_t   WAIT_TIME =               10000;
constexpr uint16_t   WAIT_TIME_BUTTON =        300;
constexpr uint8_t    WAIT_TIME_MEASSURE =      200;
constexpr uint16_t   WAIT_TIME_MITTELWERT =    3000;
constexpr uint8_t    xMax = (array_len * 3 < SCREEN_WIDTH - 1) ? array_len * 3 : (SCREEN_WIDTH - 1);
constexpr uint8_t    buffLen =                 16;

#define old_hour_default 99

/* * * * * * * * * * * * * * * *
 * Globale Variablen -         *
 * nur deklariert!             *
 * * * * * * * * * * * * * * * */

//Temperatur, Luftfeuchte, Luftdruck
extern int16_t T, H, P;

//Aufnahme in 24h Chronologie
extern int16_t  temp_messungen[array_len];
extern int16_t  humid_messungen[array_len];
extern int16_t  baro_messungen[array_len];
extern uint8_t  old_hour;
extern int8_t  old_day;

//Mittelwertbildung
extern uint8_t    currentMeassurementCounter; //0...5
extern int16_t    tempsforMittelwert[numberOfMeassurements];
extern int16_t    humidsforMittelwert[numberOfMeassurements];
extern int16_t    pressuresforMittelwert[numberOfMeassurements];

extern uint16_t   MeassurementTimerMittelwert;

extern int32_t    hourlyMittelwertTemp;
extern int32_t    hourlyMittelwertHygro;
extern int32_t    hourlyMittelwertBaro;
extern uint8_t    hourlyMittelwertCounter;
extern uint8_t    old_minute;

extern bool   sommerzeit;

//Anzeigestatus "Page"
extern uint8_t    displaymode;

//diverse Timer
extern unsigned long  timer;
extern unsigned long  button_timer;
extern unsigned long  meassure_timer;

//Strings für Darstellung von Zeit und Datum
//extern String time_now_string;
//extern String date_now_string;
extern char timeBuf[buffLen];
extern char dateBuf[buffLen];


/* * * * * * * * * * * * * * * *
 * Funktionsdeklarationen      *
 * * * * * * * * * * * * * * * */

// --- EEPROM Persistenz ---
bool loadMeasurementsFromEEPROM();   // gibt true zurück, wenn gültige Daten geladen wurden
bool saveMeasurementsToEEPROM();     // schreibt aktuelle Arrays in den EEPROM

//Bereitstellung von Zeit und Datum als String
void getTimeAndDateString(char* timeString, size_t timeSize, char* dateString, size_t dateSize, const DateTime& dt);

//stündliches Befüllen und mittelwertbildung der Messwerte
void fill_arrays(Adafruit_BME280& bme_ref, int16_t temp[], int16_t humi[], int16_t pressur[], uint8_t& meassure);
void mittelwerte_berechnen(int16_t& te, int16_t& hy, int16_t& ba, int16_t temp[], int16_t hygro[], int16_t baro[], const uint8_t& measure);

//Setup der Peripherie und Pins des uC
void setupPins();
void setupPeripherie(Adafruit_SSD1306& display_ref, RTC_DS3231& rtc_ref, Adafruit_BME280& bme_ref);

//Durchführen der Messungen
void doMeasurements(Adafruit_BME280& bme_var);

//Auswerten von Hardware-Eingaben
void handleButtonInput();

//Funktionen zur Zeitumstellung
bool isLastSundayOfOctober(const DateTime& dt);
bool isPastLastSundayOfOctober(const DateTime& dt);
bool isLastSundayOfMarch(const DateTime& dt);
void CheckZeitumstellung(RTC_DS3231& rtc_ref, const DateTime& dt);

//Grafische Ausgabe 
void printTimeDateMeasurements(Adafruit_SSD1306& dis, char* tns, char* dns, int16_t& T, int16_t& H, int16_t& P);
//void printTimeAndDate(Adafruit_SSD1306& dis, String& tns, String& dns);
//void printTempHygroBaro(Adafruit_SSD1306& dis, int16_t& T, int16_t& H, int16_t& P);

//schreiben in die 24h-Messarrays und ins Eeprom
void saveHourlyMeasurements(uint8_t& oldhour_var, const DateTime& right_now_var, int16_t temp_messungen_var[], int16_t humid_messungen_var[], int16_t baro_messungen_var[], int32_t& T_var, int32_t& H_var, int32_t& P_var);
void drawAxeY(int y, Adafruit_SSD1306& dis);
void drawGraph(int16_t the_array[], Adafruit_SSD1306& dis, const uint8_t start_val, const __FlashStringHelper* unit, int16_t min_value, int16_t max_value);

void printFixed10(Adafruit_SSD1306& dis, int16_t valueTimes10);

float int16_tToFloat (int16_t num);
int16_t FloatToInt16_t (float num);
