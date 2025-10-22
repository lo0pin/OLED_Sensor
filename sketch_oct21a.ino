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

// -------- Barograph --------
float baro_messungen[24] = {
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1
};
const int baro_len = 24;

#define upperbuttonpowersource 12
#define lowerbuttonpowersource 6
#define upperbuttonsensor 3
#define lowerbuttonsensor 9

//bool lowerbutton = false;
//bool upperbutton = false;

int displaymode = 0;
unsigned long timer = 0;
const int waittime = 300;


  
void setup() {
  Serial.begin(9600);
  Wire.begin();
  bme.begin(0x76);
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

}

void loop() {
  DateTime now = rtc.now();
  float T = bme.readTemperature();       // °C
  float H = bme.readHumidity();          // %RH
  float P = bme.readPressure() / 100.0F; // in hPa
  String Tim = String(now.hour());
  Tim += ":";
  if ((int)now.minute() < 10) {
    Tim += "0";
  }
  Tim += String(now.minute());


  
  display.clearDisplay();
  display.setCursor(0, 10);
  switch(displaymode){
    case 0:
      display.println("Temperature");
      display.println((String)T);
      break;
    case 1:
      display.println("Humidity");
      display.println((String)H);
      break;
    case 2:
      display.println("Pressure");
      display.println((String)P);
      break;
    case 3:
      display.println("Time");
      display.println((String)Tim);
      break;
  }
  display.println((String)displaymode);
  display.display();
  
  if (millis() - timer < waittime) return;

  

  
  if (digitalRead(upperbuttonsensor)) {
    if (displaymode >=4){
      displaymode=0;
    } else {
      displaymode++;
    }
  }
  if (digitalRead(lowerbuttonsensor)) {
    if (displaymode <=0){
      displaymode=4;
    } else {
      displaymode--;
    }
  }
  timer = millis();
  
  
  /*
  display.clearDisplay();
  display.setCursor(0, 10);
  lowerbutton = digitalRead(upperbuttonsensor) == 1 ? true : false;
  upperbutton = digitalRead(lowerbuttonsensor) == 1 ? true : false;
//  Serial.print("lowerbutton\t");
//  Serial.println(lowerbutton);
//  Serial.print("upperbutton\t");
//  Serial.println(upperbutton);
  if (upperbutton && lowerbutton){
    display.println(F("beide"));
  } else if (upperbutton) {
    display.println(F("oben"));
  } else if (lowerbutton){
    display.println(F("unten"));
  } else{
    display.println(F("Hallo BUBU!"));
  }
  display.display();
  //delay(200);

   */

}
