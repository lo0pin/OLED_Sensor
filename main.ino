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
  for (uint8_t i = 0; i < numberOfMeassurements; ++i) {
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
  DateTime setup_now = rtc.now();
  temp_messungen[setup_now.hour()] = T;
  humid_messungen[setup_now.hour()] = H;
  baro_messungen[setup_now.hour()] = P;
}



void loop() {
  DateTime right_now = rtc.now();
  CheckZeitumstellung(rtc, right_now);
  doMeasurements(bme);

  if (old_minute != right_now.minute()){
    old_minute = right_now.minute();
    hourlyMittelwertTemp  += T;
    hourlyMittelwertHygro += H; 
    hourlyMittelwertBaro  += P;
    hourlyMittelwertCounter++;
  }

  // Nur zur vollen Stunde ins Array (deine Vorgabe bleibt)
  if ((int)right_now.hour() != old_hour) {
    if (old_hour == old_hour_default) {
      // 1) Beim allerersten Tick nach dem Boot: Baseline auf aktuelle Stunde setzen...
      old_hour = (int)right_now.hour();
      //    ...und jetzt gezielt den *aktuellen* Slot primen:
      saveHourlyMeasurements(old_hour, right_now,
                             temp_messungen, humid_messungen, baro_messungen,
                             hourlyMittelwertTemp, hourlyMittelwertHygro, hourlyMittelwertBaro);
      // old_hour bleibt = aktuelle Stunde (saveHourlyMeasurements setzt old_hour ohnehin auf right_now.hour())
    } else {
      // 2) Später, beim echten Stundenwechsel:
      //    Vorige Stunde finalisieren (schreibt in Index = alter Wert von old_hour)
      saveHourlyMeasurements(old_hour, right_now,
                             temp_messungen, humid_messungen, baro_messungen,
                             hourlyMittelwertTemp, hourlyMittelwertHygro, hourlyMittelwertBaro);
      // saveHourlyMeasurements setzt old_hour danach auf die *neue* aktuelle Stunde
    }
  }

  //getTimeAndDateString(time_now_string, date_now_string, right_now);
  getTimeAndDateString(timeBuf, sizeof(timeBuf), dateBuf, sizeof(dateBuf), right_now);
  
  display.clearDisplay();
  display.setCursor(0, 0);

  uint8_t start = (right_now.hour() + 1) % array_len;
  if (start < 0) start += array_len; 

  switch (displaymode) {
    case 0: 
      {
        printTimeDateMeasurements(display, timeBuf, dateBuf, T, H, P);
        break;
      }

    case 1: {
        display.print(F("T"));
        //DateTime now = rtc.now();
        const int16_t T_MIN = -10;
        const int16_t T_MAX =  40;
        drawAxeY(SCREEN_HEIGHT - 1 - (int)round((0 - T_MIN) * (SCREEN_HEIGHT - 1) / (T_MAX - T_MIN)),display);
        //drawGraph(temp_messungen, display, start, F(" C"), T_MIN, T_MAX);


        for (uint8_t i = 0; i < array_len; ++i) {
          int idx = (start + i) % array_len;
          if (idx < 0) idx += array_len; 
          int16_t raw = temp_messungen[idx];
          if (raw == -1) continue; // nur echte Messwerte plotten
          float v = int16_tToFloat(raw);
          

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
          int16_t raw_dog = temp_messungen[i];
          if (raw_dog == -1) continue;
          float current_temp_mess = int16_tToFloat(raw_dog);
          durchschnitt += current_temp_mess;
          divisor++;
          if (current_temp_mess > max_temp) max_temp = current_temp_mess;
          if (current_temp_mess < min_temp) min_temp = current_temp_mess;
        }
        if (divisor>0){
          durchschnitt /= divisor;
        } 
        
        // Für die Ausgabe in ×10 umrechnen:
        int16_t max_temp10 = (int16_t)round(max_temp * 10.0f);
        int16_t avg_temp10 = (int16_t)round(durchschnitt * 10.0f);
        int16_t min_temp10 = (int16_t)round(min_temp * 10.0f);
        
        display.setCursor(xMax + 5, 0);
        printFixed10(display, max_temp10);
        display.print(F(" C"));
        
        display.setCursor(xMax + 5, 12);
        printFixed10(display, avg_temp10);
        display.println(F(" C"));
        
        display.setCursor(xMax + 5, 25);
        printFixed10(display, min_temp10);
        display.println(F(" C"));
        break;
      }

    case 2:
      {
        display.println("H");
        drawAxeY(SCREEN_HEIGHT / 2, display); //Nulllinie
        drawGraph(humid_messungen, display, start, F(" %"), 32000, -32000);
        break;
      }
    case 3: {
        display.print(F("P"));
        drawAxeY(SCREEN_HEIGHT / 2, display); //Nulllinie
        drawGraph(baro_messungen, display, start, F("hPa"), 32000, -32000);
        break;
      }
  }

  display.display();

  if (millis() - button_timer > WAIT_TIME_BUTTON) {
    handleButtonInput();
  }



  if (millis() - timer > WAIT_TIME) {
    displaymode = displaymode < numberofdisplaymodes ? displaymode + 1 : 0;
    timer = millis();
  }
}
