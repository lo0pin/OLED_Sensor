#include "bme_sensor.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include <Arduino.h>
#include <EEPROM.h>




int16_t T = 0;
int16_t H = 0;
int16_t P = 0;

int16_t temp_messungen[array_len] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};
int16_t humid_messungen[array_len] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};
int16_t baro_messungen[array_len] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};

uint8_t old_hour =      old_hour_default;
int8_t old_day =       -1;
bool sommerzeit =   false;

uint8_t currentMeassurementCounter = 0;
int16_t tempsforMittelwert[numberOfMeassurements] = {}; //5
int16_t humidsforMittelwert[numberOfMeassurements] = {};
int16_t pressuresforMittelwert[numberOfMeassurements] = {};

uint16_t MeassurementTimerMittelwert = 0;

uint8_t displaymode =   0;

unsigned long timer =           0;
unsigned long button_timer =    0;
unsigned long meassure_timer =  0;

//String time_now_string =        "";
//String date_now_string =        "";
char timeBuf[buffLen];
char dateBuf[buffLen];

int32_t    hourlyMittelwertTemp      = 0;
int32_t    hourlyMittelwertHygro     = 0;
int32_t    hourlyMittelwertBaro      = 0;
uint8_t    hourlyMittelwertCounter   = 0;
uint8_t    old_minute                = 99;


void getTimeAndDateString(char* timeString, size_t timeSize, char* dateString, size_t dateSize, const DateTime& dt) {
  snprintf(timeString, timeSize, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
  snprintf(dateString, dateSize, "%02d.%02d.%04d", dt.day(), dt.month(), dt.year());
}


void fill_arrays(Adafruit_BME280& bme_ref, int16_t temp[], int16_t humi[], int16_t pressur[], uint8_t& meassure) {
  temp[meassure] =     FloatToInt16_t(bme_ref.readTemperature());     // °C
  humi[meassure] =     FloatToInt16_t(bme_ref.readHumidity());
  pressur[meassure] =   FloatToInt16_t(bme_ref.readPressure() / 100.0F); // in hPa
  meassure = meassure < 4 ? meassure + 1 : 0;
}

void mittelwerte_berechnen(int16_t& te, int16_t& hy, int16_t& ba, int16_t temp[], int16_t hygro[], int16_t baro[], const uint8_t& measure) {
  // Mittelwerte bilden
  int32_t sumT = 0;
  int32_t sumH = 0;
  int32_t sumB = 0;
  for (uint8_t i = 0; i < measure; ++i) {
    sumT += temp[i];
    sumH += hygro[i];
    sumB += baro[i];
  }
  sumT /= measure; sumH /= measure; sumB /= measure;
  //currentMeassurementCounter = 0;
  te = (int16_t)sumT;
  hy = (int16_t)sumH;
  ba = (int16_t)sumB;
}

void setupPins() {
  pinMode(upperbuttonpowersource, OUTPUT);
  pinMode(lowerbuttonpowersource, OUTPUT);
  pinMode(upperbuttonsensor, INPUT);
  pinMode(lowerbuttonsensor, INPUT);
  digitalWrite(upperbuttonpowersource, HIGH);
  digitalWrite(lowerbuttonpowersource, HIGH);
#if INDICATOR_LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif
}

void setupPeripherie(Adafruit_SSD1306& display_ref, RTC_DS3231& rtc_ref, Adafruit_BME280& bme_ref) {
  if (!rtc_ref.begin()) {
#if DEBUG
    Serial.println(F("RTC not found"));
#endif
    while (1);
  }
#if FIX_TIME_ONCE
  rtc_ref.adjust(DateTime(F(__DATE__), F(__TIME__)));
#endif
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

float int16_tToFloat (int16_t num){
  return (float)num / 10.0f;
}

int16_t FloatToInt16_t (float num){
  return (int16_t)(num*10);
}

void doMeasurements(Adafruit_BME280& bme_var) {
  if (millis() - meassure_timer > WAIT_TIME_MEASSURE) {
#if INDICATOR_LED 
    digitalWrite(LED_BUILTIN, 0); 
#endif
    fill_arrays(bme_var, tempsforMittelwert, humidsforMittelwert, pressuresforMittelwert, currentMeassurementCounter);
    if (MeassurementTimerMittelwert >= WAIT_TIME_MITTELWERT) {
      mittelwerte_berechnen(T, H, P, tempsforMittelwert, humidsforMittelwert, pressuresforMittelwert, numberOfMeassurements);
      MeassurementTimerMittelwert = 0;
#if DEBUG
      Serial.println(F("Temp"));
      for(int i = 0; i< array_len; ++i){
        Serial.print(i); Serial.print(F(" ")); Serial.print(temp_messungen[i]); Serial.println(F(", "));
      }
      Serial.println("---------------");
      Serial.println(F("Hygro"));
      for(int i = 0; i< array_len; ++i){
        Serial.print(i); Serial.print(F(" ")); Serial.print(humid_messungen[i]); Serial.println(F(", "));
      }
      Serial.println("---------------");
      Serial.println(F("Baro"));
      for(int i = 0; i< array_len; ++i){
        //Serial.print(i); Serial.print(F(" ")); 
        Serial.println(baro_messungen[i]); 
        //Serial.println(F(", "));
      }
      Serial.println("---------------");
#endif
#if INDICATOR_LED 
      digitalWrite(LED_BUILTIN, 1); 
#endif
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

void printTimeDateMeasurements(Adafruit_SSD1306& dis, char* tns, char* dns, int16_t& T, int16_t& H, int16_t& P) {
  dis.print(tns);
  dis.print("  ");
  dis.println(dns);

  dis.print(F("Temp:  "));
  printFixed10(dis, T);
  dis.println(F(" C"));

  dis.print(F("Hygr:  "));
  printFixed10(dis, H);
  dis.println(F(" %"));

  dis.print(F("Baro:  "));
  printFixed10(dis, P);
  dis.println(F(" hPa"));  
}

void saveHourlyMeasurements(uint8_t& oldhour_var, const DateTime& right_now_var, int16_t temp_messungen_var[], int16_t humid_messungen_var[], int16_t baro_messungen_var[], int32_t& T_var, int32_t& H_var, int32_t& P_var) {
  if (hourlyMittelwertCounter > 0) {
    temp_messungen_var[oldhour_var]  = (int16_t)(T_var / hourlyMittelwertCounter);
    humid_messungen_var[oldhour_var] = (int16_t)(H_var / hourlyMittelwertCounter);
    baro_messungen_var[oldhour_var]  = (int16_t)(P_var / hourlyMittelwertCounter);
  } else {
    // Optional: Slot explizit markieren (statt "alten" Wert stehen zu lassen)
    temp_messungen_var[oldhour_var]  = -1;
    humid_messungen_var[oldhour_var] = -1;
    baro_messungen_var[oldhour_var]  = -1;
  }
  oldhour_var = (uint8_t)right_now_var.hour();
  hourlyMittelwertTemp      = 0;
  hourlyMittelwertHygro     = 0;
  hourlyMittelwertBaro      = 0;
  hourlyMittelwertCounter   = 0;

  saveMeasurementsToEEPROM(); // nicht in jeder Loop – nur bei echter Änderung!
}

void drawAxeY(int y, Adafruit_SSD1306& dis) {
  if (y < 0) y = 0;
  if (y > SCREEN_HEIGHT - 1) y = SCREEN_HEIGHT - 1;
  
  dis.drawLine(0, y, xMax, y, SSD1306_WHITE);
  dis.drawPixel(6 * 3, y - 1, SSD1306_WHITE);
  dis.drawPixel(12 * 3, y - 1, SSD1306_WHITE);
  dis.drawPixel(18 * 3, y - 1, SSD1306_WHITE);
}

void drawGraph(int16_t the_array[], Adafruit_SSD1306& dis, const uint8_t start_val, const __FlashStringHelper* unit, int16_t min_value, int16_t max_value) {
  max_value = int16_tToFloat(max_value);
  min_value = int16_tToFloat(min_value);

  for (int i = 0; i < array_len; ++i) {
    int16_t raw = the_array[i];
    if (raw == -1) continue;
    float the_value_now = int16_tToFloat(raw);
    if (the_value_now > max_value) max_value = the_value_now;
    if (the_value_now < min_value) min_value = the_value_now;
  }
  float the_step = (max_value - min_value) > 0 ? (max_value - min_value) / (SCREEN_HEIGHT - 1) : 1.0f; // Division durch 0 vermeiden
  int diff = 0;
  if (the_step < minimumstepfordrawing) {
    diff = (minimumstepfordrawing - the_step)*(60*minimumstepfordrawing);
    the_step = minimumstepfordrawing;
  }

  for (int i = 0; i < array_len; ++i) {
    int idx = (start_val + i) % array_len;
    if (idx < 0) idx += array_len;
    int16_t raw_boy = the_array[idx];
    if (raw_boy == -1) continue;
    float acual_value_now = int16_tToFloat(raw_boy);
    
    int y_value = (int)round(((acual_value_now - min_value) / the_step));
    y_value = (SCREEN_HEIGHT - 1) - y_value;
    y_value -= diff;
    if (y_value < 0) y_value = 0;
    if (y_value > (SCREEN_HEIGHT - 1)) y_value = (SCREEN_HEIGHT - 1);


    int x = i * 3;
    if (x >= 0 && x < SCREEN_WIDTH && y_value >= 0 && y_value < SCREEN_HEIGHT) {
      dis.drawPixel(x, y_value, SSD1306_INVERSE);
    }
  }
  int16_t max10 = (int16_t)round(max_value * 10.0f);
  int16_t mid10 = (int16_t)round((max_value + min_value) * 5.0f); // (/2)*10 = *5
  int16_t min10 = (int16_t)round(min_value * 10.0f);
  
  dis.setCursor(xMax + 5, 0);
  printFixed10(dis, max10);
  dis.println(unit);
  
  dis.setCursor(xMax + 5, 12);
  printFixed10(dis, mid10);
  dis.println(unit);
  
  dis.setCursor(xMax + 5, 25);
  printFixed10(dis, min10);
  dis.println(unit);
}

void printFixed10(Adafruit_SSD1306& dis, int16_t valueTimes10) {
  int16_t whole = valueTimes10 / 10;
  int16_t frac  = valueTimes10 % 10;
  if (frac < 0) frac = -frac; // falls negative Werte
  
  dis.print(whole);
  dis.print('.');
  dis.print(frac);
}

// ---------- EEPROM Persistenz für Messdaten ----------

namespace {
// Layout-Kontrolle
constexpr uint16_t EEPROM_MAGIC   = 0xBEE5;
constexpr uint8_t  EEPROM_VERSION = 3;

struct EepromHeader {
  uint16_t magic;     // 0xBEE5
  uint8_t  version;   // 1
  uint8_t  reserved;  // ausgerichtet auf 4 Bytes
  uint16_t checksum;  // Fletcher-16 über den Payload
};

struct EepromPayload {
  int16_t temp[array_len];
  int16_t humid[array_len];
  int16_t baro[array_len];
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

