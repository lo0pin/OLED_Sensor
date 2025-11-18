# 🌦️ Weather Display with OLED, BME280 & DS3231

**Arduino-based mini weather station** displaying real-time temperature, humidity, air pressure, and date/time on an SSD1306 OLED display — with **24-hour history** stored persistently in EEPROM.

---

## 🧩 Hardware Components

| Component            | Function                                | Notes / Pins                                                           |
| -------------------- | --------------------------------------- | ---------------------------------------------------------------------- |
| **Arduino**          | Main controller                         | Compatible with Uno / Nano / Pro Mini                                  |
| **BME280**           | Temperature, humidity, pressure sensor  | I²C address `0x76` (default)                                          |
| **SSD1306 OLED**     | 128×32 px display                       | I²C address `0x3C`                                                    |
| **DS3231 RTC**       | Real-time clock module                  | Temperature-compensated, keeps time when powered off                   |
| **2 × Push buttons** | Navigation controls                     | Up button on **D3**, Down button on **D9**; powered via **D12** / **D6** |
| **LED_BUILTIN**      | Status indicator (optional)             | Blinks during sensor sampling                                          |

**I²C Wiring (Arduino Uno/Nano)**: SDA → **A4**, SCL → **A5**  
All I²C devices share the same bus with different addresses.

---

## 📚 Required Libraries

* `Adafruit_BME280`
* `Adafruit_Sensor`
* `Adafruit_SSD1306`
* `RTClib`
* `Wire`
* `EEPROM` (built-in for AVR)

Install via Arduino Library Manager or as Git submodules.

---

## 🚀 Features

* **Live display** of Temperature (°C), Humidity (%), Pressure (hPa), Time & Date
* **Minute-by-minute averaging** accumulated into hourly data points
  * Every minute: current sensor readings are collected
  * On hour change: the hourly **mean** is saved to a 24-slot ring buffer
* **24-hour history** persistently stored in EEPROM
  * Robust storage with **magic number**, **version**, and **Fletcher-16 checksum**
* **Button navigation** to switch between display pages (debounced)
* **Automatic EU DST adjustment** (last Sunday in March/October)
* Optional: **Set RTC to compile time** on first upload (`FIX_TIME_ONCE`)
* Optional: **LED blinks** during measurements for visual feedback

---

## 🖥️ Display Modes

Navigate through modes using the buttons or wait for automatic rotation:

0. **Current readings**: Time, Date, Temperature, Humidity, Pressure
1. **Temperature graph**: 24-hour history with max/avg/min labels
2. **Humidity graph**: 24-hour history
3. **Pressure graph**: 24-hour history

> The display cycles through modes 0–3 automatically every 10 seconds.

---

## ⚙️ Configuration

Edit settings in `bme_sensor.h`:

```cpp
#define INDICATOR_LED   1   // Enable LED blinking during sampling
#define DEBUG           0   // Enable Serial debug output
#define FIX_TIME_ONCE   0   // Set RTC to compile time on boot (use once)

// Display settings
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

// Button configuration
#define upperbuttonpowersource  12  // Power pin for upper button
#define lowerbuttonpowersource  6   // Power pin for lower button
#define upperbuttonsensor       3   // Up button input
#define lowerbuttonsensor       9   // Down button input

// Timing & history
constexpr int array_len               = 24;    // 24 hourly slots
constexpr int numberOfMeassurements   = 5;     // Samples per averaging window
constexpr int WAIT_TIME               = 10000; // Auto-rotate delay (ms)
constexpr int WAIT_TIME_BUTTON        = 300;   // Button debounce delay (ms)
constexpr int WAIT_TIME_MEASSURE      = 200;   // Sampling interval (ms)
constexpr int WAIT_TIME_MITTELWERT    = 3000;  // Averaging window (ms)
```

---

## 🧠 How It Works

### Sampling & Averaging

* `doMeasurements()` reads the BME280 sensor every 200ms and maintains a rolling average
* Every ~3 seconds, the system calculates mean values for Temperature, Humidity, and Pressure
* These values are displayed in real-time on the OLED

### Hourly History & EEPROM Storage

* When the hour changes, `saveHourlyMeasurements()` stores the previous hour's average values
* Data is saved to three 24-slot arrays in EEPROM: `temp_messungen`, `humid_messungen`, `baro_messungen`
* On boot, `loadMeasurementsFromEEPROM()` restores the history if the data is valid
* Data integrity is ensured with:
  * Magic number `0xBEE5`
  * Version number `1`
  * Fletcher-16 checksum

**EEPROM Data Structure:**
```cpp
struct EepromPayload {
  float temp[24], humid[24], baro[24];
  int16_t old_hour_saved, old_day_saved;
};
```

### Daylight Saving Time (EU)

* `CheckZeitumstellung()` automatically adjusts the DS3231 clock:
  * **March**: Last Sunday at 02:00 → 03:00 (CEST)
  * **October**: Last Sunday at 03:00 → 02:00 (CET)
* Protected against multiple adjustments with a daily marker

---

## 🧪 Example Display Output

**Mode 0 (Current readings):**
```
14:37:05  02.11.2025
Temp:  22.6 C
Hygr:  45.1 %
Baro:  1008.7 hPa
```

Graph modes (1-3) show a vertical Y-axis with tick marks and plot historical values over 24 hours.

---

## 🧰 Project Structure

```
/OLED_Sensor
├── bme_sensor.h          // Constants, global declarations, function prototypes
├── bme_sensor.cpp        // Core logic: setup, sampling, DST, display, EEPROM
├── main.ino              // Arduino setup() and loop(), page navigation
├── LICENSE               // MIT License
└── README.md             // This documentation
```

---

## 🔧 Installation & Setup

1. **Install required libraries** via Arduino Library Manager
2. **Wire the components** according to the hardware table above
3. **(Optional)** Set `FIX_TIME_ONCE 1` in `bme_sensor.h` for the first upload to initialize the RTC
4. **Upload the sketch** to your Arduino
5. On first boot:
   * History arrays will show `-1` (empty) until the first hour completes
   * After the first hour, data persists across resets

---

## 🧯 Troubleshooting

| Problem | Solution |
|---------|----------|
| **No display output** | Check OLED address (`0x3C` typical) and I²C wiring (SDA/SCL) |
| **BME280 not found** | Verify I²C address (`0x76` or `0x77`), check power and connections |
| **RTC not found** | Ensure CR2032 battery is installed, verify I²C connections |
| **LED doesn't blink** | Set `INDICATOR_LED 1` in configuration |
| **History has gaps** | Device must run continuously across hour boundaries to save data |
| **EEPROM won't load** | Corrupted data; new image will be created on next save |

---

## 🧭 Future Improvements

- [ ] Enhanced graph labels and axes
- [ ] Configurable Y-axis scaling for graphs
- [ ] Battery level indicator
- [ ] Low-power sleep mode
- [ ] Support for 128×64 OLED displays
- [ ] CSV data export via Serial
- [ ] Advanced menu system

---

## 📜 License

**MIT License**

Copyright (c) 2025 Julian Kampitsch

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

## 👨‍💻 Author

**lo0pin** - [GitHub Profile](https://github.com/lo0pin)

⭐ If you find this project useful, please consider giving it a star!


---
---


# 🌦️ Wetterstation mit OLED, BME280 & DS3231

**Arduino-basierte Mini-Wetterstation** mit Echtzeit-Anzeige von Temperatur, Luftfeuchtigkeit, Luftdruck und Uhrzeit/Datum auf einem SSD1306 OLED-Display — mit **24-Stunden-Verlauf** dauerhaft im EEPROM gespeichert.

---

## 🧩 Hardware-Komponenten

| Komponente           | Funktion                                  | Hinweise / Pins                                                        |
| -------------------- | ----------------------------------------- | ---------------------------------------------------------------------- |
| **Arduino**          | Hauptcontroller                           | Kompatibel mit Uno / Nano / Pro Mini                                   |
| **BME280**           | Temperatur-, Feuchtigkeits- und Drucksensor | I²C-Adresse `0x76` (Standard)                                        |
| **SSD1306 OLED**     | 128×32 px Display                         | I²C-Adresse `0x3C`                                                    |
| **DS3231 RTC**       | Echtzeituhr-Modul                         | Temperaturkompensiert, behält Zeit bei Stromausfall                    |
| **2 × Taster**       | Navigationssteuerung                      | Hoch-Taste an **D3**, Runter-Taste an **D9**; versorgt über **D12** / **D6** |
| **LED_BUILTIN**      | Statusanzeige (optional)                  | Blinkt während der Sensormessung                                       |

**I²C-Verkabelung (Arduino Uno/Nano)**: SDA → **A4**, SCL → **A5**  
Alle I²C-Geräte teilen sich den Bus mit unterschiedlichen Adressen.

---

## 📚 Erforderliche Bibliotheken

* `Adafruit_BME280`
* `Adafruit_Sensor`
* `Adafruit_SSD1306`
* `RTClib`
* `Wire`
* `EEPROM` (integriert für AVR)

Installation über den Arduino Library Manager oder als Git-Submodule.

---

## 🚀 Funktionen

* **Live-Anzeige** von Temperatur (°C), Luftfeuchtigkeit (%), Luftdruck (hPa), Uhrzeit & Datum
* **Minütliche Mittelwertbildung** zu stündlichen Datenpunkten
  * Jede Minute: Aktuelle Sensorwerte werden gesammelt
  * Bei Stundenwechsel: Der stündliche **Durchschnitt** wird in einem 24-Slot-Ringpuffer gespeichert
* **24-Stunden-Verlauf** dauerhaft im EEPROM gespeichert
  * Robuste Speicherung mit **Magic Number**, **Versionsnummer** und **Fletcher-16-Prüfsumme**
* **Tasten-Navigation** zum Wechseln zwischen Anzeigemodi (entprellt)
* **Automatische EU-Sommerzeitumstellung** (letzter Sonntag im März/Oktober)
* Optional: **RTC auf Kompilierzeit setzen** beim ersten Upload (`FIX_TIME_ONCE`)
* Optional: **LED blinkt** während der Messungen zur visuellen Rückmeldung

---

## 🖥️ Anzeigemodi

Navigation durch die Modi über Tasten oder automatische Rotation:

0. **Aktuelle Werte**: Uhrzeit, Datum, Temperatur, Luftfeuchtigkeit, Luftdruck
1. **Temperatur-Graph**: 24-Stunden-Verlauf mit Max/Mittel/Min-Beschriftung
2. **Luftfeuchtigkeits-Graph**: 24-Stunden-Verlauf
3. **Luftdruck-Graph**: 24-Stunden-Verlauf

> Das Display wechselt automatisch alle 10 Sekunden durch die Modi 0–3.

---

## ⚙️ Konfiguration

Einstellungen in `bme_sensor.h` bearbeiten:

```cpp
#define INDICATOR_LED   1   // LED-Blinken während der Messung aktivieren
#define DEBUG           0   // Serielle Debug-Ausgabe aktivieren
#define FIX_TIME_ONCE   0   // RTC auf Kompilierzeit beim Start setzen (einmalig)

// Display-Einstellungen
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

// Tasten-Konfiguration
#define upperbuttonpowersource  12  // Strompin für obere Taste
#define lowerbuttonpowersource  6   // Strompin für untere Taste
#define upperbuttonsensor       3   // Eingang obere Taste
#define lowerbuttonsensor       9   // Eingang untere Taste

// Timing & Verlauf
constexpr int array_len               = 24;    // 24 stündliche Slots
constexpr int numberOfMeassurements   = 5;     // Messungen pro Mittelwert-Fenster
constexpr int WAIT_TIME               = 10000; // Auto-Rotation Verzögerung (ms)
constexpr int WAIT_TIME_BUTTON        = 300;   // Tasten-Entprellung (ms)
constexpr int WAIT_TIME_MEASSURE      = 200;   // Messintervall (ms)
constexpr int WAIT_TIME_MITTELWERT    = 3000;  // Mittelwert-Fenster (ms)
```

---

## 🧠 Funktionsweise

### Messung & Mittelwertbildung

* `doMeasurements()` liest den BME280-Sensor alle 200ms aus und bildet einen gleitenden Durchschnitt
* Alle ~3 Sekunden berechnet das System Mittelwerte für Temperatur, Luftfeuchtigkeit und Luftdruck
* Diese Werte werden in Echtzeit auf dem OLED angezeigt

### Stündlicher Verlauf & EEPROM-Speicherung

* Beim Stundenwechsel speichert `saveHourlyMeasurements()` die Durchschnittswerte der letzten Stunde
* Daten werden in drei 24-Slot-Arrays im EEPROM gespeichert: `temp_messungen`, `humid_messungen`, `baro_messungen`
* Beim Start stellt `loadMeasurementsFromEEPROM()` den Verlauf wieder her, wenn die Daten gültig sind
* Datenintegrität wird sichergestellt durch:
  * Magic Number `0xBEE5`
  * Versionsnummer `1`
  * Fletcher-16-Prüfsumme

**EEPROM-Datenstruktur:**
```cpp
struct EepromPayload {
  float temp[24], humid[24], baro[24];
  int16_t old_hour_saved, old_day_saved;
};
```

### Sommerzeit-Umstellung (EU)

* `CheckZeitumstellung()` passt die DS3231-Uhr automatisch an:
  * **März**: Letzter Sonntag um 02:00 → 03:00 (MESZ)
  * **Oktober**: Letzter Sonntag um 03:00 → 02:00 (MEZ)
* Geschützt gegen mehrfache Anpassungen durch tägliche Markierung

---

## 🧪 Beispiel Display-Ausgabe

**Modus 0 (Aktuelle Werte):**
```
14:37:05  02.11.2025
Temp:  22.6 C
Hygr:  45.1 %
Baro:  1008.7 hPa
```

Graph-Modi (1-3) zeigen eine vertikale Y-Achse mit Markierungen und stellen historische Werte über 24 Stunden dar.

---

## 🧰 Projektstruktur

```
/OLED_Sensor
├── bme_sensor.h          // Konstanten, globale Deklarationen, Funktionsprototypen
├── bme_sensor.cpp        // Kernlogik: Setup, Messung, Zeitumstellung, Display, EEPROM
├── main.ino              // Arduino setup() und loop(), Seiten-Navigation
├── LICENSE               // MIT-Lizenz
└── README.md             // Diese Dokumentation
```

---

## 🔧 Installation & Einrichtung

1. **Bibliotheken installieren** über den Arduino Library Manager
2. **Komponenten verkabeln** gemäß der Hardware-Tabelle oben
3. **(Optional)** `FIX_TIME_ONCE 1` in `bme_sensor.h` setzen für den ersten Upload zur RTC-Initialisierung
4. **Sketch hochladen** auf deinen Arduino
5. Beim ersten Start:
   * Verlaufs-Arrays zeigen `-1` (leer) bis die erste Stunde abgeschlossen ist
   * Nach der ersten Stunde bleiben Daten über Resets hinweg erhalten

---

## 🧯 Fehlerbehebung

| Problem | Lösung |
|---------|--------|
| **Keine Display-Ausgabe** | OLED-Adresse überprüfen (`0x3C` typisch) und I²C-Verkabelung (SDA/SCL) |
| **BME280 nicht gefunden** | I²C-Adresse verifizieren (`0x76` oder `0x77`), Stromversorgung und Verbindungen prüfen |
| **RTC nicht gefunden** | CR2032-Batterie installiert? I²C-Verbindungen verifizieren |
| **LED blinkt nicht** | `INDICATOR_LED 1` in der Konfiguration setzen |
| **Verlauf hat Lücken** | Gerät muss kontinuierlich über Stundengrenzen laufen, um Daten zu speichern |
| **EEPROM lädt nicht** | Beschädigte Daten; neues Image wird bei nächstem Speichern erstellt |

---

## 🧭 Zukünftige Verbesserungen

- [ ] Verbesserte Graph-Beschriftung und Achsen
- [ ] Konfigurierbare Y-Achsen-Skalierung für Graphen
- [ ] Batteriestands-Anzeige
- [ ] Stromspar-Schlafmodus
- [ ] Unterstützung für 128×64 OLED-Displays
- [ ] CSV-Datenexport über Serial
- [ ] Erweitertes Menüsystem

---

## 📜 Lizenz

**MIT-Lizenz**

Copyright (c) 2025 Julian Kampitsch

Hiermit wird unentgeltlich jeder Person, die eine Kopie der Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, die Erlaubnis erteilt, sie uneingeschränkt zu nutzen, inklusive und ohne Beschränkung der Rechte zur Nutzung, Vervielfältigung, Änderung, Zusammenführung, Veröffentlichung, Verbreitung, Unterlizenzierung und/oder zum Verkauf von Kopien der Software, und Personen, denen diese Software überlassen wird, dies unter den folgenden Bedingungen zu gestatten:

Der obige Urheberrechtsvermerk und dieser Erlaubnishinweis sind in allen Kopien oder wesentlichen Teilen der Software beizulegen.

DIE SOFTWARE WIRD OHNE JEDE AUSDRÜCKLICHE ODER IMPLIZIERTE GARANTIE BEREITGESTELLT, EINSCHLIESSLICH DER GARANTIE ZUR BENUTZUNG FÜR DEN VORGESEHENEN ODER EINEM BESTIMMTEN ZWECK SOWIE JEGLICHER RECHTSVERLETZUNG, JEDOCH NICHT DARAUF BESCHRÄNKT. IN KEINEM FALL SIND DIE AUTOREN ODER COPYRIGHTINHABER FÜR JEGLICHEN SCHADEN ODER SONSTIGE ANSPRÜCHE HAFTBAR ZU MACHEN, OB INFOLGE DER ERFÜLLUNG EINES VERTRAGES, EINES DELIKTES ODER ANDERS IM ZUSAMMENHANG MIT DER SOFTWARE ODER SONSTIGER VERWENDUNG DER SOFTWARE ENTSTANDEN.

---

## 👨‍💻 Autor

**lo0pin** - [GitHub Profil](https://github.com/lo0pin)

⭐ Wenn du dieses Projekt nützlich findest, gib ihm gerne einen Stern!
