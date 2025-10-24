#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED-Breite
#define SCREEN_HEIGHT 32 // OLED-Höhe
#define OLED_RESET    -1 // Reset-Pin (nicht verwendet beim I2C-Modul)
#define OLED_ADDRESS  0x3C // Meist 0x3C oder 0x3D

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#include "RTClib.h"
// I2C-Instanz RTC:
RTC_DS3231 rtc;     // Für DS3231 (genauer & mit Temperatur)


#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// I2C-Instanz Sensor
Adafruit_BME280 bme;
// Hinweis: BME280 hat meist I2C-Adresse 0x76 ODER 0x77

float T = 0;
float H = 0;
float P = 0;

float temp_messungen[24] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};

//float temp_messungen[24] = {
//  1.3, 2.8, 3.1, 4.7, 5.4, 6.2,
//  7.0, 8.3, -9.1, 10.0, 11.8, 12.4,
//  13.0, 14.3, 15.1, 16.5, 17.7, 18.9,
//  19.8, 20.5, 21.4, 22.2, 23.3, 24.6
//};

//int temp_pixel[24] = {
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1
//};

float humid_messungen[24] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};

//int humid_pixel[24] = {
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1
//};

// -------- Barograph --------
float baro_messungen[24] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};

//int baro_pixel[24] = {
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1
//};
const int array_len = 24;
int old_hour = -1;

int meassure_counter = 0;
const int measurements = 5;
float temps[measurements];
float humids[measurements];
float pressures[measurements];

#define upperbuttonpowersource 12
#define lowerbuttonpowersource 6
#define upperbuttonsensor 3
#define lowerbuttonsensor 9

//bool lowerbutton = false;
//bool upperbutton = false;

int displaymode = 0;
unsigned long timer = 0;
unsigned long button_timer = 0;
const int WAIT_TIME = 10000;
const int WAIT_TIME_BUTTON = 300;


void setup() {
  //Serial.begin(9600);
  Wire.begin();
  if (!rtc.begin()) {
    //Serial.println("RTC not found");
    while (1);
  }
  if (!bme.begin(0x76)) {
    //Serial.println("BME280 not found");
    while (1);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  pinMode(upperbuttonpowersource, OUTPUT);
  pinMode(lowerbuttonpowersource, OUTPUT);
  pinMode(upperbuttonsensor, INPUT);
  pinMode(lowerbuttonsensor, INPUT);

  digitalWrite(upperbuttonpowersource, HIGH);
  digitalWrite(lowerbuttonpowersource, HIGH);


  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    //Serial.println(F("SSD1306 nicht gefunden!"));
    for (;;); // Stoppt hier
  }
  display.clearDisplay();
  // Löscht den gesamten Bildschirminhalt im Speicher des Displays.
  // Der Bildschirm wird erst nach display.display() tatsächlich aktualisiert.

  display.setTextSize(1);
  // Legt die Textgröße fest (1 = kleinste Standardgröße).
  // Größere Zahlen vergrößern den Text proportional.

  display.setTextColor(SSD1306_WHITE);
  // Setzt die Textfarbe auf "weiß" (bei monochromen OLEDs = leuchtende Pixel).
  // Es gibt auch SSD1306_BLACK (löscht Text) oder SSD1306_INVERSE (invertiert).

  display.setCursor(0, 10);
  // Setzt den "Schreibcursor" auf Position x=0, y=10 Pixel.
  // Dort beginnt der nächste Text, der mit println() ausgegeben wird.

  display.println(F("Hallo BUBU!"));
  // Schreibt den Text "Hallo BUBU!" in den Display-Puffer.
  // Das 'F()' sorgt dafür, dass der Text im Flash-Speicher bleibt (spart RAM).

  display.display();
  // Aktualisiert das Display – alles, was im Speicher steht, wird jetzt angezeigt.
  //

  for (int i = 0; i < 5; ++i) {
    temps[i] = bme.readTemperature();     // °C
    humids[i] = bme.readHumidity();
    pressures[i] = bme.readPressure() / 100.0F; // in hPa
  }
  for (int i = 0; i < measurements; ++i) {
    T += temps[i];
    H += humids[i];
    P += pressures[i];
  }
  T /= measurements;
  H /= measurements;
  P /= measurements;

  timer = millis();
}



void loop() {
  /*
    // ---- Button Handling ----
    static bool lastButtonState = LOW;
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50; // 50 ms Entprellzeit

    int buttonState = digitalRead(lowerbuttonsensor);

    // nur weiter, wenn Signal sich ändert
    if (buttonState != lastButtonState) {
      lastDebounceTime = millis();
    }

    // wenn stabil für > 50 ms:
    if ((millis() - lastDebounceTime) > debounceDelay) {
      // steigende Flanke = Taster gedrückt
      if (buttonState == HIGH && lastButtonState == LOW) {
        displaymode = (displaymode < 6) ? displaymode + 1 : 0;
        Serial.print("Displaymode: ");
        Serial.println(displaymode);

        // optional: direkt updaten
        display.clearDisplay();
      }
    }

    lastButtonState = buttonState;
  */
  DateTime now = rtc.now();
  temps[meassure_counter] = bme.readTemperature();     // °C
  humids[meassure_counter] = bme.readHumidity();
  pressures[meassure_counter] = bme.readPressure() / 100.0F; // in hPa
  meassure_counter = meassure_counter < 4 ? meassure_counter + 1 : 0;


  //  if (millis() - timer >= WAIT_TIME) {
  //    displaymode = displaymode <6? displaymode+1 : 0;
  //    timer = millis();
  //  }





  String time_now_string = String(now.hour());
  time_now_string += ":";
  if ((int)now.minute() < 10) {
    time_now_string += "0";
  }
  time_now_string += String(now.minute());
  time_now_string += ":";
  if (now.second() < 10) time_now_string += "0";
  time_now_string += String(now.second());

  String date_now_string = String(now.day());
  date_now_string += ".";
  date_now_string += String(now.month());
  date_now_string += ".";
  date_now_string += String(now.year());



  display.clearDisplay();
  display.setCursor(0, 0);

  int xMax = min(array_len * 3, SCREEN_WIDTH - 1);
  int start = (now.hour() + 1) % array_len;

  switch (displaymode) {
    case 0: {
        display.println("Time and Date");
        display.println(time_now_string);
        display.println(date_now_string);

        break;
      }

    case 1:
      {
        display.print("Temp:  ");
        display.print(T, 1);
        display.println(" C");
        display.print("Hygr:  ");
        display.print(H, 1);
        display.println(" %");
        display.print("Baro:  ");
        display.print(P, 1);
        display.println(" hPa");
        break;
      }

    case 2: {
        display.print("T");



        //DateTime now = rtc.now();
        const float T_MIN = -10.0;
        const float T_MAX =  40.0;

        // Nullinie
        int y_achse = SCREEN_HEIGHT - 1 - (int)round((0 - T_MIN) * (SCREEN_HEIGHT - 1) / (T_MAX - T_MIN));
        if (y_achse < 0) y_achse = 0;
        if (y_achse > SCREEN_HEIGHT - 1) y_achse = SCREEN_HEIGHT - 1;
        display.drawLine(0, y_achse, xMax, y_achse, SSD1306_WHITE);

        display.drawPixel(6 * 3, y_achse - 1, SSD1306_WHITE);
        display.drawPixel(12 * 3, y_achse - 1, SSD1306_WHITE);
        display.drawPixel(18 * 3, y_achse - 1, SSD1306_WHITE);

        for (int i = 0; i < array_len; ++i) {
          int idx = (start + i) % array_len;
          float v = temp_messungen[idx];
          if (v == -1) continue; // nur echte Messwerte plotten

          if (v < T_MIN) v = T_MIN;
          if (v > T_MAX) v = T_MAX;

          int y = SCREEN_HEIGHT - 1 - (int)round((v - T_MIN) * (SCREEN_HEIGHT - 1) / (T_MAX - T_MIN));
          int x = i * 3;        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            display.drawPixel(x, y, !display.getPixel(x, y));
          }
        }

        float max_temp = 0.0f;
        float min_temp = 50.0f;
        float durchschnitt = 0.f;
        int divisor = 0;
        for (int i = 0; i < array_len; ++i) {
          float current_temp_mess = temp_messungen[i];
          if (current_temp_mess == -1) continue;
          durchschnitt += current_temp_mess;
          divisor++;
          if (current_temp_mess > max_temp) max_temp = current_temp_mess;
          if (current_temp_mess < min_temp) min_temp = current_temp_mess;
        }
        durchschnitt /= divisor;



        display.setCursor(xMax + 5, 0);
        display.print(max_temp, 1);
        display.print(" C");

        display.setCursor(xMax + 5, 12);
        display.print(durchschnitt, 1);
        display.println(" C");

        display.setCursor(xMax + 5, 25);
        display.print(min_temp, 1);
        display.println(" C");


        break;
      }

    case 3:
      {
        display.println("H");

        //Nulllinie
        display.drawLine(0, SCREEN_HEIGHT / 2, xMax, SCREEN_HEIGHT / 2, SSD1306_WHITE);
        display.drawPixel(6 * 3, SCREEN_HEIGHT / 2 - 1, SSD1306_WHITE);
        display.drawPixel(12 * 3, SCREEN_HEIGHT / 2 - 1, SSD1306_WHITE);
        display.drawPixel(18 * 3, SCREEN_HEIGHT / 2 - 1, SSD1306_WHITE);
        //      display.drawPixel(6*3, SCREEN_HEIGHT/2+1, SSD1306_WHITE);
        //      display.drawPixel(12*3, SCREEN_HEIGHT/2+1, SSD1306_WHITE);
        //      display.drawPixel(18*3, SCREEN_HEIGHT/2+1, SSD1306_WHITE);

        float hum_max = 0.0f;
        float hum_min = 100.0f;
        for (int i = 0; i < array_len; ++i) {
          float hum_value_now = humid_messungen[i];
          if (hum_value_now == -1) continue;
          if (hum_value_now > hum_max) hum_max = hum_value_now;
          if (hum_value_now < hum_min) hum_min = hum_value_now;
        }
        float hum_step = (hum_max - hum_min) / (SCREEN_HEIGHT - 1);
        if (hum_step <= 0) hum_step = 1.0f;
        for (int i = 0; i < array_len; ++i) {
          int idx = (start + i) % array_len;
          float h = humid_messungen[idx];
          if (h == -1) continue; // nur echte Messwerte plotten
          int y = SCREEN_HEIGHT - 1 - (int)round((h - hum_min) * (SCREEN_HEIGHT - 1) / (hum_max - hum_min));
          int x = i * 3;
          if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {

            display.drawPixel(x, y, !display.getPixel(x, y));
          }



        }
        display.setCursor(xMax + 5, 0);
        display.print(hum_max, 1);
        display.println(" %");

        display.setCursor(xMax + 5, 12);
        display.print((hum_max + hum_min) / 2, 1);
        display.println(" %");

        display.setCursor(xMax + 5, 25);
        display.print(hum_min, 1);
        display.println(" %");

        break;
      }
    case 4: {
        display.clearDisplay();


        //Nulllinie
        display.drawLine(0, SCREEN_HEIGHT / 2, xMax, SCREEN_HEIGHT / 2, SSD1306_WHITE);
        display.drawPixel(6 * 3, SCREEN_HEIGHT / 2 - 1, SSD1306_WHITE);
        display.drawPixel(12 * 3, SCREEN_HEIGHT / 2 - 1, SSD1306_WHITE);
        display.drawPixel(18 * 3, SCREEN_HEIGHT / 2 - 1, SSD1306_WHITE);
        //      display.drawPixel(6*3, SCREEN_HEIGHT/2+1, SSD1306_WHITE);
        //      display.drawPixel(12*3, SCREEN_HEIGHT/2+1, SSD1306_WHITE);
        //      display.drawPixel(18*3, SCREEN_HEIGHT/2+1, SSD1306_WHITE);

        display.println("P");
        float pres_max = 0.0f;
        float pres_min = 9999.0f;
        for (int i = 0; i < array_len; ++i) {
          float value_now = baro_messungen[i];
          if (value_now == -1) continue;
          if (value_now > pres_max) pres_max = value_now;
          if (value_now < pres_min) pres_min = value_now;
        }
        float baro_step = (pres_max - pres_min) / (SCREEN_HEIGHT - 1);
        if (baro_step <= 0) baro_step = 1.0f;
        if (baro_step <= 0.5) baro_step = 0.5f;

        for (int i = 0; i < array_len; ++i) {
          int idx = (start + i) % array_len;
          float p = baro_messungen[idx];
          if (p == -1) continue; // nur echte Messwerte plotten
          int y = SCREEN_HEIGHT - 1 - (int)round((p - pres_min) * (SCREEN_HEIGHT - 1) / (pres_max - pres_min));
          int x = i * 3;
          if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {

            display.drawPixel(x, y, !display.getPixel(x, y));
          }

        }
        display.setCursor(xMax + 5, 0);
        display.print(pres_max, 1);
        display.println("hPa");

        display.setCursor(xMax + 5, 12);
        display.print((pres_max + pres_min) / 2, 1);
        display.println("hPa");

        display.setCursor(xMax + 5, 25);
        display.print(pres_min, 1);
        display.println("hPa");

        break;
      }
  }

  display.display();

  if (millis() - button_timer > WAIT_TIME_BUTTON) {
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



  if (millis() - timer > WAIT_TIME) {
    displaymode = displaymode < 4 ? displaymode + 1 : 0;
    //Serial.print("Displaymode:\t");
    //Serial.println(displaymode);

    // Mittelwerte bilden
    T = H = P = 0;
    for (int i = 0; i < measurements; ++i) {
      T += temps[i];
      H += humids[i];
      P += pressures[i];
    }
    T /= measurements; H /= measurements; P /= measurements;
    meassure_counter = 0;

    // Nur zur vollen Stunde ins Array (deine Vorgabe bleibt)
    DateTime now_tmp = rtc.now();
    if ((int)now_tmp.hour() != old_hour) {
      old_hour = (int)now_tmp.hour();
      temp_messungen[old_hour]  = T;
      humid_messungen[old_hour] = H;
      baro_messungen[old_hour]  = P;
    }






    timer = millis();
  }











}
