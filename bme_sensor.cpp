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

int currentMeassurementCounter = 0;
float temps[numberOfMeassurements] = {};
float humids[numberOfMeassurements] = {};
float pressures[numberOfMeassurements] = {};

int displaymode = 0;

unsigned long timer = 0;
unsigned long button_timer = 0;
unsigned long meassure_timer = 0;

String time_now_string = "";
String date_now_string = "";

void getTimeAndDateString(String& timeString, String& dateString, const DateTime& actual_datetime){
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


void fill_arrays(Adafruit_BME280& bme_ref, float temp[], float humi[], float pressur[], int& meassure){
  temp[meassure] = bme_ref.readTemperature();     // °C
  humi[meassure] = bme_ref.readHumidity();
  pressur[meassure] = bme_ref.readPressure() / 100.0F; // in hPa
  meassure = meassure < 4 ? meassure + 1 : 0;
}

void mittelwerte_berechnen(float& te, float& hy, float& ba, float temp[], float hygro[], float baro[], const int& measure){
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




void setupPins(){
  pinMode(upperbuttonpowersource, OUTPUT);
  pinMode(lowerbuttonpowersource, OUTPUT);
  pinMode(upperbuttonsensor, INPUT);
  pinMode(lowerbuttonsensor, INPUT);
  digitalWrite(upperbuttonpowersource, HIGH);
  digitalWrite(lowerbuttonpowersource, HIGH);
}

void setupPeripherie(Adafruit_SSD1306& display_ref, RTC_DS3231& rtc_ref, Adafruit_BME280& bme_ref){
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
}
