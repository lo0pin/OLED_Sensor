#include "bme_sensor.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include "bme_sensor.h"
#include <Arduino.h>

float T = 0.0f;
float H = 0.0f;
float P = 0.0f;

float temp_messungen[array_len] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};
float humid_messungen[array_len] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};
float baro_messungen[array_len] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};

int old_hour = -1;
int old_day = -1;
bool sommerzeit = false;

int currentMeassurementCounter = 0;
float temps[numberOfMeassurements] = {};
float humids[numberOfMeassurements] = {};
float pressures[numberOfMeassurements] = {};

int MeassurementCounterMittelwert = 0;

int displaymode = 0;

unsigned long timer = 0;
unsigned long button_timer = 0;
unsigned long meassure_timer = 0;

String time_now_string = "";
String date_now_string = "";

void getTimeAndDateString(String& timeString, String& dateString, const DateTime& actual_datetime) {
  timeString = String(actual_datetime.hour());
  timeString += ":";
  if ((int)actual_datetime.minute() < 10) {
    timeString += "0";
  }
  timeString += String(actual_datetime.minute());
  timeString += ":";
  if (actual_datetime.second() < 10) timeString += "0";
  timeString += String(actual_datetime.second());

  dateString = String(actual_datetime.day());
  dateString += ".";
  dateString += String(actual_datetime.month());
  dateString += ".";
  dateString += String(actual_datetime.year());
}


void fill_arrays(Adafruit_BME280& bme_ref, float temp[], float humi[], float pressur[], int& meassure) {
  temp[meassure] = bme_ref.readTemperature();     // °C
  humi[meassure] = bme_ref.readHumidity();
  pressur[meassure] = bme_ref.readPressure() / 100.0F; // in hPa
  meassure = meassure < 4 ? meassure + 1 : 0;
}

void mittelwerte_berechnen(float& te, float& hy, float& ba, float temp[], float hygro[], float baro[], const int& measure) {
  // Mittelwerte bilden
  te = 0;
  hy = 0;
  ba = 0;
  for (int i = 0; i < measure; ++i) {
    te += temp[i];
    hy += hygro[i];
    ba += baro[i];
  }
  te /= measure; hy /= measure; ba /= measure;
  //currentMeassurementCounter = 0;
}




void setupPins() {
  pinMode(upperbuttonpowersource, OUTPUT);
  pinMode(lowerbuttonpowersource, OUTPUT);
  pinMode(upperbuttonsensor, INPUT);
  pinMode(lowerbuttonsensor, INPUT);
  digitalWrite(upperbuttonpowersource, HIGH);
  digitalWrite(lowerbuttonpowersource, HIGH);
}

void setupPeripherie(Adafruit_SSD1306& display_ref, RTC_DS3231& rtc_ref, Adafruit_BME280& bme_ref) {
  if (!rtc_ref.begin()) {
    Serial.println(F("RTC not found"));
    while (1);
  }
  if (!bme_ref.begin(0x76)) {
    Serial.println(F("BME280 not found"));
    while (1);
  }

  if (!display_ref.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 nicht gefunden!"));
    while (1); // Stoppt hier
  }
  display_ref.clearDisplay();
  // Löscht den gesamten Bildschirminhalt im Speicher des Displays.
  // Der Bildschirm wird erst nach display.display() tatsächlich aktualisiert.

  display_ref.setTextSize(1);
  // Legt die Textgröße fest (1 = kleinste Standardgröße).
  // Größere Zahlen vergrößern den Text proportional.

  display_ref.setTextColor(SSD1306_WHITE);
  // Setzt die Textfarbe auf "weiß" (bei monochromen OLEDs = leuchtende Pixel).
  // Es gibt auch SSD1306_BLACK (löscht Text) oder SSD1306_INVERSE (invertiert).

  display_ref.setCursor(0, 10);
  // Setzt den "Schreibcursor" auf Position x=0, y=10 Pixel.
  // Dort beginnt der nächste Text, der mit println() ausgegeben wird.

  //display_ref.println(F("Hallo BUBU!"));
  // Schreibt den Text "Hallo BUBU!" in den Display-Puffer.
  // Das 'F()' sorgt dafür, dass der Text im Flash-Speicher bleibt (spart RAM).

  //display_ref.display();
  // Aktualisiert das Display – alles, was im Speicher steht, wird jetzt angezeigt.

  //rtc_ref.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void doMeasurements(Adafruit_BME280& bme_var) {
  if (millis() - meassure_timer > WAIT_TIME_MEASSURE) {
    fill_arrays(bme_var, temps, humids, pressures, currentMeassurementCounter);
    if (MeassurementCounterMittelwert >= WAIT_TIME_MITTELWERT) {
      mittelwerte_berechnen(T, H, P, temps, humids, pressures, numberOfMeassurements);
      MeassurementCounterMittelwert = 0;
    } else {
      MeassurementCounterMittelwert += WAIT_TIME_MEASSURE;
    }
    meassure_timer = millis();
  }
}


void handleButtonInput() {
  if (digitalRead(9)) {
    displaymode = displaymode < 4 ? displaymode + 1 : 0;
    button_timer = millis();
    timer = millis();
  }
  if (digitalRead(3)) {
    displaymode = displaymode > 0 ? displaymode - 1 : 4;
    button_timer = millis();
    timer = millis();
  }
}



static uint16_t lastAdjustYday = 65535; // Marker gegen Mehrfach-Adjust (unmöglicher Startwert)

void CheckZeitumstellung(RTC_DS3231& rtc_ref, const DateTime& dt) {
  // "Tag-im-Jahr" – reicht als Einmal-Sperre pro Kalendertag
  uint16_t yday = DateTime(dt.year(), dt.month(), dt.day()).unixtime() / 86400;

  // --- Umstellung auf Winterzeit (CEST -> CET): letzter Sonntag im Oktober, 03:00 -> 02:00 ---
  if (sommerzeit &&
      isLastSundayOfOctober(dt) &&
      dt.hour() == 3 && dt.minute() == 0 && dt.second() == 0 &&
      yday != lastAdjustYday) {

    rtc_ref.adjust(DateTime(dt.year(), dt.month(), dt.day(),
                            2, dt.minute(), dt.second())); // eine Stunde zurück
    sommerzeit = false;
    lastAdjustYday = yday;
    return; // defensiv: an diesem Tick nichts weiter
  }

  // --- Umstellung auf Sommerzeit (CET -> CEST): letzter Sonntag im März, 02:00 -> 03:00 ---
  if (!sommerzeit &&
      isLastSundayOfMarch(dt) &&
      dt.hour() == 2 && dt.minute() == 0 && dt.second() == 0 &&
      yday != lastAdjustYday) {

    rtc_ref.adjust(DateTime(dt.year(), dt.month(), dt.day(),
                            3, dt.minute(), dt.second())); // eine Stunde vor
    sommerzeit = true;
    lastAdjustYday = yday;
    return;
  }

  // Kein expliziter Reset nötig: am Folgetag ist yday != lastAdjustYday automatisch erfüllt.
}

bool isLastSundayOfOctober(const DateTime& dt) {
  if (dt.month() != 10) return false;
  // letzter Sonntag: wenn zum Datum innerhalb der nächsten 7 Tage der Monatswechsel käme
  // dayOfTheWeek(): 0=So,1=Mo,...6=Sa
  return dt.dayOfTheWeek() == 0 && (dt.day() + 7) > 31;
}

bool isLastSundayOfMarch(const DateTime& dt) {
  if (dt.month() != 3) return false;
  // letzter Sonntag: wenn zum Datum innerhalb der nächsten 7 Tage der Monatswechsel käme
  // dayOfTheWeek(): 0=So,1=Mo,...6=Sa
  return dt.dayOfTheWeek() == 0 && (dt.day() + 7) > 31;
}

bool isPastLastSundayOfOctober(const DateTime& dt) {
  return ((dt.month() == 10 && 31 - dt.day() < 7) || dt.month() > 10);
}



void printTimeAndDate(Adafruit_SSD1306& dis, String& tns, String& dns){
  dis.println(F("Time and Date"));
  dis.println(tns);
  dis.println(dns);
}

void printTempHygroBaro(Adafruit_SSD1306& dis, float& T, float& H, float& P){
  dis.print(F("Temp:  "));
  dis.print(T, 1);
  dis.println(" C");
  dis.print(F("Hygr:  "));
  dis.print(H, 1);
  dis.println(" %");
  dis.print(F("Baro:  "));
  dis.print(P, 1);
  dis.println(F(" hPa"));
}
