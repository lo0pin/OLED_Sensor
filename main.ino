#include "bme_sensor.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;
Adafruit_BME280 bme;

void setup() {
  #if DEBUG
  Serial.begin(9600);
  #endif
  Wire.begin();
  setupPins();
  setupPeripherie(display, rtc, bme);
  #if FIX_TIME_ONCE
  rtc_ref.adjust(DateTime(F(__DATE__), F(__TIME__)));
  #endif
  for (int i = 0; i < numberOfMeassurements; ++i) {
    fill_arrays(bme, tempsforMittelwert, humidsforMittelwert, pressuresforMittelwert, i);
  }
  mittelwerte_berechnen(T, H, P, tempsforMittelwert, humidsforMittelwert, pressuresforMittelwert, numberOfMeassurements);

  timer = button_timer = meassure_timer = millis();

  if (loadMeasurementsFromEEPROM()) {
    #if DEBUG
    Serial.println(F("EEPROM: Messdaten geladen."));
    #endif
  } else {
    #if DEBUG
    Serial.println(F("EEPROM: Keine gültigen Messdaten – Arrays bleiben wie initialisiert."));
    #endif
  }
}



void loop() {
  DateTime right_now = rtc.now();
  CheckZeitumstellung(rtc, right_now);
  doMeasurements(bme);


  // Nur zur vollen Stunde ins Array (deine Vorgabe bleibt)
  if ((int)right_now.hour() != old_hour) {
    saveHourlyMeasurements(old_hour, right_now, temp_messungen, humid_messungen, baro_messungen, hourlyMittelwertTemp, hourlyMittelwertHygro, hourlyMittelwertBaro);
  }

  getTimeAndDateString(time_now_string, date_now_string, right_now);

  display.clearDisplay();
  display.setCursor(0, 0);

  int start = (right_now.hour() + 1) % array_len;
  if (start < 0) start += array_len; 

  switch (displaymode) {
    case 0: 
      {
        printTimeDateMeasurements(display, time_now_string, date_now_string, T, H, P);
        break;
      }

    case 1: {
        display.print(F("T"));
        //DateTime now = rtc.now();
        const float T_MIN = -10.0;
        const float T_MAX =  40.0;
        drawAxeY(SCREEN_HEIGHT - 1 - (int)round((0 - T_MIN) * (SCREEN_HEIGHT - 1) / (T_MAX - T_MIN)),display);
        //drawGraph(temp_messungen, display, start, F(" C"), T_MIN, T_MAX);


        for (int i = 0; i < array_len; ++i) {
          int idx = (start + i) % array_len;
          if (idx < 0) idx += array_len; 
          float v = temp_messungen[idx];
          if (v == -1) continue; // nur echte Messwerte plotten

          if (v < T_MIN) v = T_MIN;
          if (v > T_MAX) v = T_MAX;

          int y = SCREEN_HEIGHT - 1 - (int)round((v - T_MIN) * (SCREEN_HEIGHT - 1) / (T_MAX - T_MIN));
          int x = i * 3;        
          if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            display.drawPixel(x, y, SSD1306_INVERSE);
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
        if (divisor>0){
          durchschnitt /= divisor;
        } 
        
        display.setCursor(xMax + 5, 0);
        display.print(max_temp, 1);
        display.print(F(" C"));

        display.setCursor(xMax + 5, 12);
        display.print(durchschnitt, 1);
        display.println(F(" C"));

        display.setCursor(xMax + 5, 25);
        display.print(min_temp, 1);
        display.println(F(" C"));



        break;
      }

    case 2:
      {
        display.println("H");
        drawAxeY(SCREEN_HEIGHT / 2, display); //Nulllinie
        drawGraph(humid_messungen, display, start, F(" %"), 9999.9f, -9999.9f);
        break;
      }
    case 3: {
        display.print(F("P"));
        drawAxeY(SCREEN_HEIGHT / 2, display); //Nulllinie
        drawGraph(baro_messungen, display, start, F("hPa"), 9999.9f, -9999.9f);
        break;
      }
  }

  display.display();

  if (millis() - button_timer > WAIT_TIME_BUTTON) {
    handleButtonInput();
  }

  if (old_minute != right_now.minute()){
    old_minute = right_now.minute();
    hourlyMittelwertTemp  += T;
    hourlyMittelwertHygro += H; 
    hourlyMittelwertBaro  += P;
    hourlyMittelwertCounter++;
  }

  if (millis() - timer > WAIT_TIME) {
    displaymode = displaymode < numberofdisplaymodes ? displaymode + 1 : 0;
    timer = millis();
  }
}
