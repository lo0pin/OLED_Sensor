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

//float temp_messungen[24] = {
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1,
//  -1, -1, -1, -1, -1, -1
//};

float temp_messungen[24] = {
  15.3, 15.8, 16.1, 16.7, 17.4, 18.2,
  19.0, 20.3, 21.1, 22.0, 22.8, 23.4,
  24.0, 24.3, 24.1, 23.5, 22.7, 21.9,
  20.8, 19.5, 18.4, 17.2, 16.3, 15.6
};

float humid_messungen[24] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};
// -------- Barograph --------
float baro_messungen[24] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};
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
const int WAIT_TIME = 2000;



void setup() {
  Serial.begin(9600);
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC not found");
    while (1);
  }
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  pinMode(upperbuttonpowersource, OUTPUT);
  pinMode(lowerbuttonpowersource, OUTPUT);
  pinMode(upperbuttonsensor, INPUT);
  pinMode(lowerbuttonsensor, INPUT);

  digitalWrite(upperbuttonpowersource, HIGH);
  digitalWrite(lowerbuttonpowersource, HIGH);


  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 nicht gefunden!"));
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
}



void loop() {
  DateTime now = rtc.now();
  temps[meassure_counter] = bme.readTemperature();     // °C
  humids[meassure_counter] = bme.readHumidity();
  pressures[meassure_counter] = bme.readPressure() / 100.0F; // in hPa

  meassure_counter = meassure_counter < 4 ? meassure_counter + 1 : 0;







  String Tim = String(now.hour());
  Tim += ":";
  if ((int)now.minute() < 10) {
    Tim += "0";
  }
  Tim += String(now.minute());
  Tim += ":";
  if (now.second() < 10) Tim += "0";
  Tim += String(now.second());


  display.clearDisplay();
  display.setCursor(0, 0);
  switch (displaymode) {
    case 0: display.println("Time");        display.println(Tim);  break;
    case 1: display.println("Temperature"); display.print(T, 1);  display.println(" C");    break;
    case 2:
      float minimum=999;
      float maximum=-999;
      for (int i=0; i<array_len; ++i){
        minimum = temp_messungen[i]<minimum ? temp_messungen[i] : minimum;
        maximum = temp_messungen[i]>maximum ? temp_messungen[i] : maximum;
      }
      float draw_step = 0;
      display.drawLine(0, SCREEN_HEIGHT, array_len * 3, SCREEN_HEIGHT, SSD1306_WHITE);
      for (int i = 0; i < array_len; ++i) {
        int actual_index = now.hour() + i;
        
        if (temp_messungen[i] != -1) {
          display.drawPixel(i * 3, SCREEN_HEIGHT-temp_messungen[i], SSD1306_WHITE);
        }

      }

      break;
    case 3: display.println("Humidity");    display.print(H, 1);    display.println(" %");    break;
    case 4: break;
    case 5: display.println("Pressure");    display.print(P, 1);  display.println(" hPa");  break;
    case 6: break;


  }
  //display.println((String)displaymode);
  display.display();

  if (millis() - timer < WAIT_TIME) return;

  displaymode = displaymode < 6 ? displaymode + 1 : 0;

  T = 0;
  H = 0;
  P = 0;
  for (int i = 0; i < measurements; ++i) {
    T += temps[i];
    H += humids[i];
    P += pressures[i];
  }
  T /= measurements;
  H /= measurements;
  P /= measurements;

  if (now.hour() != old_hour) {
    old_hour = now.hour();
    temp_messungen[old_hour] = T;
    humid_messungen[old_hour] = H;
    baro_messungen[old_hour] = P;

  }

  timer = millis();




}
