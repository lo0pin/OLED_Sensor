#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include <Adafruit_BME280.h>

/* * * * * * * * * * * * * * * *
 * Konstanten                  *
 * * * * * * * * * * * * * * * */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C

#define upperbuttonpowersource 12
#define lowerbuttonpowersource 6
#define upperbuttonsensor 3
#define lowerbuttonsensor 9

constexpr int array_len = 24;
constexpr int numberOfMeassurements = 5;
constexpr int WAIT_TIME = 10000;
constexpr int WAIT_TIME_BUTTON = 300;
constexpr int WAIT_TIME_MEASSURE = 200;
constexpr int xMax = (array_len * 3 < SCREEN_WIDTH - 1) ? array_len * 3 : (SCREEN_WIDTH - 1);

/* * * * * * * * * * * * * * * *
 * Globale Variablen           *
 * * * * * * * * * * * * * * * */
// nur deklariert!

//Temperatur, Luftfeuchte, Luftdruck
extern float T, H, P;

//mittelwertbildung
extern float temp_messungen[array_len];
extern float humid_messungen[array_len];
extern float baro_messungen[array_len];

extern int currentMeassurementCounter;
extern float temps[numberOfMeassurements];
extern float humids[numberOfMeassurements];
extern float pressures[numberOfMeassurements];

//Aufnahme in 24h Chronologie
extern int old_hour;

//Anzeigestatus "Page"
extern int displaymode;

extern unsigned long timer;
extern unsigned long button_timer;
extern unsigned long meassure_timer;

extern String time_now_string;
extern String date_now_string;


/* * * * * * * * * * * * * * * *
 * Funktionsdeklarationen      *
 * * * * * * * * * * * * * * * */
 
void getTimeAndDateString(String& timeString, String& dateString, const DateTime& actual_datetime);

void fill_arrays(Adafruit_BME280& bme_ref, float temp[], float humi[], float pressur[], int& meassure);
void mittelwerte_berechnen(float& te, float& hy, float& ba, float temp[], float hygro[], float baro[], const int& measure);

void setupPins();
void setupPeripherie(Adafruit_SSD1306& display_ref, RTC_DS3231& rtc_ref, Adafruit_BME280& bme_ref);
