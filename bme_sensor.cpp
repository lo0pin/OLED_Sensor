#include "bme_sensor.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include "bme_sensor.h"
#include <Arduino.h>
#include <EEPROM.h>


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
float tempsforMittelwert[numberOfMeassurements] = {}; //5
float humidsforMittelwert[numberOfMeassurements] = {};
float pressuresforMittelwert[numberOfMeassurements] = {};

int MeassurementTimerMittelwert = 0;

int displaymode = 0;

unsigned long timer = 0;
unsigned long button_timer = 0;
unsigned long meassure_timer = 0;

String time_now_string = "";
String date_now_string = "";

void getTimeAndDateString(String& timeString, String& dateString, const DateTime& actual_datetime) {
  if (actual_datetime.hour() <10){
    timeString = "0";
    timeString += String(actual_datetime.hour());
  } else{
    timeString = String(actual_datetime.hour());
  }
  timeString = String(actual_datetime.hour());
  timeString += ":";
  if ((int)actual_datetime.minute() < 10) {
    timeString += "0";
  }
  timeString += String(actual_datetime.minute());
  timeString += ":";
  if (actual_datetime.second() < 10) timeString += "0";
  timeString += String(actual_datetime.second());
  
  if ((int)actual_datetime.day()<10){
    dateString = "0";
    dateString += String(actual_datetime.day());
  } else {
    dateString = String(actual_datetime.day());
  }
  dateString = String(actual_datetime.day());
  dateString += ".";
  if (actual_datetime.month()<10){
    dateString += "0";
  }
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
    #if DEBUG
    Serial.println(F("RTC not found"));
    #endif
    while (1);
  }
  if (!bme_ref.begin(0x76)) {
    #if DEBUG
    Serial.println(F("BME280 not found"));
    #endif
    while (1);
  }

  if (!display_ref.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    #if DEBUG
    Serial.println(F("SSD1306 nicht gefunden!"));
    #endif
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

  display_ref.setCursor(0, 0);
  // Setzt den "Schreibcursor" auf Position x=0, y=10 Pixel.
  // Dort beginnt der nächste Text, der mit println() ausgegeben wird.

  //display_ref.println(F("Hallo BUBU!"));
  // Schreibt den Text "Hallo BUBU!" in den Display-Puffer.
  // Das 'F()' sorgt dafür, dass der Text im Flash-Speicher bleibt (spart RAM).

  //display_ref.display();
  // Aktualisiert das Display – alles, was im Speicher steht, wird jetzt angezeigt.
}

void doMeasurements(Adafruit_BME280& bme_var) {
  if (millis() - meassure_timer > WAIT_TIME_MEASSURE) {
    fill_arrays(bme_var, tempsforMittelwert, humidsforMittelwert, pressuresforMittelwert, currentMeassurementCounter);
    if (MeassurementTimerMittelwert >= WAIT_TIME_MITTELWERT) {
      mittelwerte_berechnen(T, H, P, tempsforMittelwert, humidsforMittelwert, pressuresforMittelwert, numberOfMeassurements);
      MeassurementTimerMittelwert = 0;
    } else {
      MeassurementTimerMittelwert += WAIT_TIME_MEASSURE;
    }
    meassure_timer = millis();
  }
}


void handleButtonInput() {
  if (digitalRead(9)) {
    displaymode = displaymode < numberofdisplaymodes ? displaymode + 1 : 0;
    button_timer = millis();
    timer = millis();
  }
  if (digitalRead(3)) {
    displaymode = displaymode > 0 ? displaymode - 1 : numberofdisplaymodes;
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

void printTimeDateMeasurements(Adafruit_SSD1306& dis, String& tns, String& dns, float& T, float& H, float& P){
  dis.print(tns); 
  dis.print("  "); 
  dis.println(dns);
  dis.print(F("Temp:  "));
  dis.print(T, 1);
  dis.println(" C");
  dis.print(F("Hygr:  "));
  dis.print(H, 1);
  dis.println(" %");
  dis.print(F("Baro:  "));
  dis.print(P, 1);
  dis.print(F(" hPa"));
}

void saveHourlyMeasurements(int& oldhour_var, DateTime& right_now_var, float temp_messungen_var[], float humid_messungen_var[], float baro_messungen_var[],float& T_var, float& H_var, float& P_var){
  oldhour_var = (int)right_now_var.hour();
  temp_messungen_var[oldhour_var]  = T_var;
  humid_messungen_var[oldhour_var] = H_var;
  baro_messungen_var[oldhour_var]  = P_var;
  
  saveMeasurementsToEEPROM(); // nicht in jeder Loop – nur bei echter Änderung!
}


void drawAxeY(int y, Adafruit_SSD1306& dis){
  if (y < 0) y = 0;
  if (y > SCREEN_HEIGHT - 1) y = SCREEN_HEIGHT - 1;
  dis.drawLine(0, y, xMax, y, SSD1306_WHITE);
  dis.drawPixel(6 * 3, y - 1, SSD1306_WHITE);
  dis.drawPixel(12 * 3, y - 1, SSD1306_WHITE);
  dis.drawPixel(18 * 3, y - 1, SSD1306_WHITE);
}

void drawGraph(float the_array[], Adafruit_SSD1306& dis, const int start_val, const String string_val){
  float max_value = -9999.9f;
  float min_value = 9999.9f;
  
  for (int i = 0; i < array_len; ++i) {
    float the_value_now = the_array[i];
    if (the_value_now == -1) continue;
    if (the_value_now > max_value) max_value = the_value_now;
    if (the_value_now < min_value) min_value = the_value_now;
  }
  float the_step = (max_value-min_value)>0 ? (max_value-min_value)/(SCREEN_HEIGHT-1) : 1.0f; // Division durch 0 vermeiden

  for (int i = 0; i < array_len; ++i){
    int idx = (start_val + i) % array_len;
    if (idx < 0) idx += array_len; 
    float acual_value_now = the_array[idx];
    if (acual_value_now == -1){
      continue;
    }
    int y_value = (int)round(((acual_value_now - min_value)/the_step));
    if (y_value <0) y_value = 0;
    if (y_value > (SCREEN_HEIGHT-1)) y_value = (SCREEN_HEIGHT-1);

    int x = i * 3;
    if (x >= 0 && x < SCREEN_WIDTH && y_value >= 0 && y_value < SCREEN_HEIGHT) {
      dis.drawPixel(x, y_value, SSD1306_INVERSE);
    }
  } 
  dis.setCursor(xMax + 5, 0);
  dis.print(max_value, 1);
  dis.println((string_val));

  dis.setCursor(xMax + 5, 12);
  dis.print((max_value + min_value) / 2, 1);
  dis.println((string_val));

  dis.setCursor(xMax + 5, 25);
  dis.print(min_value, 1);
  dis.println((string_val));  
  
  //display.println(F(" %"));                   
}

// ---------- EEPROM Persistenz für Messdaten ----------

namespace {
  // Layout-Kontrolle
  constexpr uint16_t EEPROM_MAGIC   = 0xBEE5;
  constexpr uint8_t  EEPROM_VERSION = 1;

  struct EepromHeader {
    uint16_t magic;     // 0xBEE5
    uint8_t  version;   // 1
    uint8_t  reserved;  // ausgerichtet auf 4 Bytes
    uint16_t checksum;  // Fletcher-16 über den Payload
  };

  struct EepromPayload {
    float temp[array_len];
    float humid[array_len];
    float baro[array_len];
    int16_t old_hour_saved;
    int16_t old_day_saved;
  };

  struct EepromImage {
    EepromHeader  hdr;
    EepromPayload data;
  };

  // Fletcher-16 über beliebige Bytes (Payload)
  uint16_t fletcher16(const uint8_t* data, size_t len) {
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;
    for (size_t i = 0; i < len; ++i) {
      sum1 = (sum1 + data[i]) % 255;
      sum2 = (sum2 + sum1)  % 255;
    }
    return (sum2 << 8) | sum1;
  }

  // Liest komplette Struktur aus EEPROM in 'img'
  void eepromReadImage(EepromImage& img) {
    int addr = 0;
    EEPROM.get(addr, img);
  }

  // Schreibt komplette Struktur 'img' in EEPROM
  void eepromWriteImage(const EepromImage& img) {
    int addr = 0;
    EEPROM.put(addr, img);
    // Für ESP8266/ESP32 wäre ein EEPROM.commit() nötig; bei AVR nicht.
    #if defined(ESP8266) || defined(ESP32)
      EEPROM.commit();
    #endif
  }

  // Erstellt den aktuellen Payload aus globalen Variablen
  void buildCurrentPayload(EepromPayload& pld) {
    for (int i = 0; i < array_len; ++i) {
      pld.temp[i]  = temp_messungen[i];
      pld.humid[i] = humid_messungen[i];
      pld.baro[i]  = baro_messungen[i];
    }
    pld.old_hour_saved = static_cast<int16_t>(old_hour);
    pld.old_day_saved  = static_cast<int16_t>(old_day);
  }

  // Überträgt Payload zurück in die globalen Arrays/Status
  void applyPayloadToGlobals(const EepromPayload& pld) {
    for (int i = 0; i < array_len; ++i) {
      temp_messungen[i]  = pld.temp[i];
      humid_messungen[i] = pld.humid[i];
      baro_messungen[i]  = pld.baro[i];
    }
    old_hour = pld.old_hour_saved;
    old_day  = pld.old_day_saved;
  }

} // namespace


bool loadMeasurementsFromEEPROM() {
  // Sicherheitscheck: passt die Größe überhaupt in den EEPROM?
  if (EEPROM.length() < (int)sizeof(EepromImage)) {
    // Zu klein -> nicht laden
    return false;
  }

  EepromImage img{};
  eepromReadImage(img);

  if (img.hdr.magic != EEPROM_MAGIC || img.hdr.version != EEPROM_VERSION) {
    return false; // unbekanntes/ungültiges Layout
  }

  // Checksumme prüfen (nur über den Payload)
  const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(&img.data);
  uint16_t calc = fletcher16(pBytes, sizeof(EepromPayload));
  if (calc != img.hdr.checksum) {
    return false; // korrupt
  }

  // OK -> anwenden
  applyPayloadToGlobals(img.data);
  return true;
}


bool saveMeasurementsToEEPROM() {
  if (EEPROM.length() < (int)sizeof(EepromImage)) {
    return false;
  }

  EepromImage img{};
  img.hdr.magic   = EEPROM_MAGIC;
  img.hdr.version = EEPROM_VERSION;
  img.hdr.reserved = 0;

  buildCurrentPayload(img.data);

  // Checksumme bilden
  const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(&img.data);
  img.hdr.checksum = fletcher16(pBytes, sizeof(EepromPayload));

  eepromWriteImage(img);
  return true;
}
