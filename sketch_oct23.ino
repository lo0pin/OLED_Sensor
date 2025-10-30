#include "bme_sensor.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;
Adafruit_BME280 bme;

void setup() {
  Wire.begin();
  /*Einmal-Setup: ohne Kommentar hochladen und dann auskommentiert nochmal hochladen, sonst wird bei 
   jedem Bootvorgang die Kompilierzeit in die RTC geschrieben*/
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  setupPins();
  setupPeripherie(Adafruit_SSD1306& display, RTC_DS3231& rtc, Adafruit_BME280& bme);

  for (int i = 0; i < numberOfMeassurements; ++i) {
    fill_arrays(bme, temps, humids, pressures, i);
  }
  mittelwerte_berechnen(T, H, P, temps, humids, pressures, numberOfMeassurements);

  timer = button_timer = meassure_timer = millis();
}



void loop() {
  DateTime right_now = rtc.now();

  if (millis() - meassure_timer > WAIT_TIME_MEASSURE) {
    fill_arrays(bme, temps, humids, pressures, currentMeassurementCounter);
    mittelwerte_berechnen(T, H, P, temps, humids, pressures, numberOfMeassurements);
    meassure_timer = millis();
  }

  getTimeAndDateString(time_now_string, date_now_string, right_now);

  display.clearDisplay();
  display.setCursor(0, 0);


  int start = (right_now.hour() + 1) % array_len;

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
