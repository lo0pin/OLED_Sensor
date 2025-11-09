Hier ist dein README – aktualisiert, präzise auf deinen aktuellen Code zugeschnitten und direkt für GitHub bereit.

---

# 🌦️ Weather Display with OLED, BME280 & DS3231

**Arduino-based mini weather station** with an SSD1306 OLED, showing live temperature, humidity, air pressure and time/date — plus **hourly history** stored persistently in EEPROM.

---

## 🧩 Hardware

| Component            | Function                                | Notes / Pins                                                           |
| -------------------- | --------------------------------------- | ---------------------------------------------------------------------- |
| **Arduino**          | Main controller                         | e.g. Uno / Nano / Pro Mini                                             |
| **BME280**           | Temperature, humidity, pressure         | I²C `0x76` (default)                                                   |
| **SSD1306 OLED**     | 128×32 px display                       | I²C `0x3C`                                                             |
| **DS3231 RTC**       | Real-time clock (temp-compensated)      | Keeps time when powered off                                            |
| **2 × Push buttons** | Page navigation                         | Sensors on **D3** (Up) and **D9** (Down); powered via **D12** / **D6** |
| **LED_BUILTIN**      | Status blink during sampling (optional) | Controlled via `INDICATOR_LED`                                         |

**I²C wiring (Uno/Nano)**: SDA → **A4**, SCL → **A5**
All I²C devices share the bus; addresses differ.

---

## 📚 Libraries

* `Adafruit_BME280`
* `Adafruit_Sensor`
* `Adafruit_SSD1306`
* `RTClib`
* `Wire`
* `EEPROM` (AVR on-board)

Install via Library Manager or as Git submodules.

---

## 🚀 What it does

* **Live display** of Temp (°C), Humidity (%), Pressure (hPa), Time/Date
* **Minute-wise averaging** into hourly slots

  * Per minute: current `T/H/P` are accumulated
  * On hour change: the hour’s **mean** is written to a 24-slot ring buffer
* **24-hour history** persists in EEPROM

  * robust image with **magic**, **version**, **Fletcher-16 checksum**
* **Buttons** to switch display pages (debounced by polling)
* **EU DST auto-adjust** (last Sunday in March/October) at the exact switch time
* Optional: **compile time → RTC** once (`FIX_TIME_ONCE`)
* Optional: **LED blinks** during measurements

---

## 🖥️ Display Modes

`displaymode` cycles automatically and via buttons:

0. **Time + Date + current T/H/P**
1. **Temperature graph** (24 h) with **max / avg / min** labels
2. **Humidity graph** (24 h)
3. **Pressure graph** (24 h)

> `#define numberofdisplaymodes 3` keeps the 0–3 cycle intact in the logic.

---

## ⚙️ Configuration (in `bme_sensor.h`)

```cpp
#define INDICATOR_LED   1   // Blink LED_BUILTIN during sampling
#define DEBUG           0   // Serial prints
#define FIX_TIME_ONCE   0   // Set RTC to compile time once on boot

// OLED / I²C
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

// Buttons (powered by pins to avoid external resistors)
#define upperbuttonpowersource  12
#define lowerbuttonpowersource  6
#define upperbuttonsensor       3   // Up
#define lowerbuttonsensor       9   // Down

// History / timing
constexpr int array_len               = 24; // 24 hourly slots
constexpr int numberOfMeassurements   = 5;  // samples per averaging window
constexpr int WAIT_TIME               = 10000; // auto-rotate pages
constexpr int WAIT_TIME_BUTTON        = 300;   // button poll debounce
constexpr int WAIT_TIME_MEASSURE      = 200;   // sampling interval (ms)
constexpr int WAIT_TIME_MITTELWERT    = 3000;  // averaging window (ms)

#define old_hour_default 99 // sentinel for first post-boot hour
```

---

## 🧠 How the logic works

### Sampling & Averaging

* `doMeasurements()` samples the BME280 every `WAIT_TIME_MEASSURE` ms and fills ring buffers for a **short averaging window** (`numberOfMeassurements`, default 5).
* Every ~`WAIT_TIME_MITTELWERT` ms the code computes mean **T/H/P** → `T, H, P`.

### Hourly History & EEPROM

* Once the **hour changes**, `saveHourlyMeasurements()` writes the previous hour’s **mean** into the history arrays (`temp_messungen`, `humid_messungen`, `baro_messungen`) and **persists** them via `saveMeasurementsToEEPROM()`.
* On boot, `loadMeasurementsFromEEPROM()` restores the arrays if the **image is valid** (header **magic** `0xBEE5`, **version** `1`, **Fletcher-16 checksum** over payload).
* Stored payload:

  ```cpp
  struct EepromPayload {
    float temp[24], humid[24], baro[24];
    int16_t old_hour_saved, old_day_saved;
  };
  ```

### DST (EU)

* `CheckZeitumstellung()` adjusts the DS3231:

  * **March** last Sunday at **02:00 → 03:00** (CEST)
  * **October** last Sunday at **03:00 → 02:00** (CET)
* Guarded by a per-day marker to prevent multiple adjusts.

---

## 🧪 Example Output (128×32)

```
14:37:05  02.11.2025
Temp:  22.6 C
Hygr:  45.1 %
Baro:  1008.7 hPa
```

Graph pages show a vertical Y-axis with tick marks and plot **real values only** (skips `-1`).

---

## 🧰 File Structure

```
/weather-display
├── bme_sensor.h          // Constants, globals (extern), declarations
├── bme_sensor.cpp        // Logic: setup, sampling, DST, drawing, EEPROM
├── main.ino              // setup(), loop(), page control
└── README.md             // This file
```

> If your sketch file is named differently (e.g., `sketch_oct23.ino`), keep the includes consistent.

---

## 🔧 Build & Run

1. Install libraries (Library Manager).
2. Wire I²C and buttons as listed above.
3. (Optional) Set `FIX_TIME_ONCE 1` for one boot to initialize RTC to compile time.
4. Upload. On first boot:

   * History arrays may be `-1` (empty) until the first hour closes.
   * From then on, hourly means persist across resets.

---

## 🧯 Troubleshooting

* **No display text** → Check `OLED_ADDRESS` (`0x3C` typical) and I²C wiring.
* **BME not found** → Address `0x76`/`0x77`, power & pull-ups.
* **RTC not found** → Battery installed? SDA/SCL correct?
* **LED doesn’t blink** → Set `INDICATOR_LED 1`.
* **History gaps** → The slot is written on the **hour change**; ensure the device runs across the boundary.
* **EEPROM not loading** → Corrupt image or size mismatch; a new image will be written on the next hour save.

---

## 🧭 Roadmap

* Smoother, labeled axes for humidity/pressure pages
* Configurable scaling for graphs
* Battery indicator / low-power mode
* 128×64 OLED support for richer layouts
* CSV export via Serial
* Menu system and more pages

---

## 📜 License

MIT License — see below.

```
MIT License Copyright (c) 2025 Julian Kampitsch Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```
