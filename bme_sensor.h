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

constexpr int   array_len =               24;
constexpr int   numberOfMeassurements =   5;
constexpr int   WAIT_TIME =               10000;
constexpr int   WAIT_TIME_BUTTON =        300;
constexpr int   WAIT_TIME_MEASSURE =      200;
constexpr int   WAIT_TIME_MITTELWERT =    3000;
constexpr int   xMax = (array_len * 3 < SCREEN_WIDTH - 1) ? array_len * 3 : (SCREEN_WIDTH - 1);

/* * * * * * * * * * * * * * * *
 * Globale Variablen -         *
 * nur deklariert!             *
 * * * * * * * * * * * * * * * */

//Temperatur, Luftfeuchte, Luftdruck
extern float T, H, P;

//Aufnahme in 24h Chronologie
extern float  temp_messungen[array_len];
extern float  humid_messungen[array_len];
extern float  baro_messungen[array_len];
extern int  old_hour;
extern int  old_day;

//Mittelwertbildung
extern int    currentMeassurementCounter; //0...5
extern float  tempsforMittelwert[numberOfMeassurements];
extern float  humidsforMittelwert[numberOfMeassurements];
extern float  pressuresforMittelwert[numberOfMeassurements];

extern int    MeassurementTimerMittelwert;

extern float    hourlyMittelwertTemp;
extern float    hourlyMittelwertHygro;
extern float    hourlyMittelwertBaro;
extern uint8_t     hourlyMittelwertCounter;
extern int         old_minute;

extern bool   sommerzeit;

//Anzeigestatus "Page"
extern int    displaymode;

//diverse Timer
extern unsigned long  timer;
extern unsigned long  button_timer;
extern unsigned long  meassure_timer;

//Strings für Darstellung von Zeit und Datum
extern String time_now_string;
extern String date_now_string;


/* * * * * * * * * * * * * * * *
 * Funktionsdeklarationen      *
 * * * * * * * * * * * * * * * */

// --- EEPROM Persistenz ---
bool loadMeasurementsFromEEPROM();   // gibt true zurück, wenn gültige Daten geladen wurden
bool saveMeasurementsToEEPROM();     // schreibt aktuelle Arrays in den EEPROM

//Bereitstellung von Zeit und Datum als String
void getTimeAndDateString(String& timeString, String& dateString, const DateTime& actual_datetime);

//stündliches Befüllen und mittelwertbildung der Messwerte
void fill_arrays(Adafruit_BME280& bme_ref, float temp[], float humi[], float pressur[], int& meassure);
void mittelwerte_berechnen(float& te, float& hy, float& ba, float temp[], float hygro[], float baro[], const int& measure);

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
void printTimeDateMeasurements(Adafruit_SSD1306& dis, String& tns, String& dns, float& T, float& H, float& P);
//void printTimeAndDate(Adafruit_SSD1306& dis, String& tns, String& dns);
//void printTempHygroBaro(Adafruit_SSD1306& dis, float& T, float& H, float& P);

//schreiben in die 24h-Messarrays und ins Eeprom
void saveHourlyMeasurements(int& oldhour_var, const DateTime& right_now_var, float temp_messungen_var[], float humid_messungen_var[], float baro_messungen_var[],float& T_var, float& H_var, float& P_var);

void drawAxeY(int y, Adafruit_SSD1306& dis);
void drawGraph(float the_array[], Adafruit_SSD1306& dis, const int start_val, const String string_val, float min_value, float max_value);
