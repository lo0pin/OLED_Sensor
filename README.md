# 🌦️ Weather Display with OLED and BME280

**Project:** Arduino-based mini weather station that displays temperature, humidity, air pressure, and real-time clock data on an SSD1306 OLED.

---

## 🧩 Hardware Components

| Component            | Function                                     | Notes                                   |
| -------------------- | -------------------------------------------- | --------------------------------------- |
| **Arduino**          | Main controller                              | e.g. Uno, Nano, Pro Mini                |
| **BME280**           | Measures temperature, humidity, and pressure | I²C address `0x76` or `0x77`            |
| **SSD1306 OLED**     | Display output                               | 128×32 px, I²C address `0x3C` or `0x3D` |
| **DS3231 RTC**       | Real-time clock (temperature-compensated)    | Keeps time when powered off             |
| **2 × Push buttons** | Navigation through display modes             | Use `INPUT_PULLUP`, no resistors needed |

---

## ⚡ Wiring (I²C + Buttons)

| Component         | Signal | Arduino Pin |
| ----------------- | ------ | ----------- |
| **BME280 SDA**    | SDA    | A4          |
| **BME280 SCL**    | SCL    | A5          |
| **SSD1306 SDA**   | SDA    | A4          |
| **SSD1306 SCL**   | SCL    | A5          |
| **DS3231 SDA**    | SDA    | A4          |
| **DS3231 SCL**    | SCL    | A5          |
| **Button “Up”**   | to GND | D3          |
| **Button “Down”** | to GND | D9          |

All I²C devices share the same SDA/SCL lines – as long as their addresses differ, this is perfectly fine.

---

## 📚 Used Libraries

* [Adafruit_BME280](https://github.com/adafruit/Adafruit_BME280_Library)
* [Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor)
* [Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
* [RTClib](https://github.com/adafruit/RTClib)
* **Wire** (included with Arduino IDE)

---

## 🚀 Features

✅ Displays:

* Temperature (°C)
* Humidity (%)
* Pressure (hPa)
* Real-time clock (HH:MM:SS)

✅ Smart functionality:

* Automatic time initialization on upload (`__DATE__`, `__TIME__`)
* Handles **daylight saving time changes** (EU rules)
* **Software debouncing** for clean button input
* **Averaging over multiple readings** for smooth data
* **Modular structure** (`bme_sensor.cpp` / `.h`) for easy expansion

---

## 🖥️ Display Modes

Use the two buttons to navigate between pages:

| Button        | Function      |
| ------------- | ------------- |
| **Up (D3)**   | Previous mode |
| **Down (D9)** | Next mode     |

The display cycles through:

1. **Temperature**
2. **Humidity**
3. **Pressure**
4. **Time + Date**

---

## 🧠 How It Works

1. The setup initializes **RTC**, **BME280**, and **OLED**.
2. Measurements are taken periodically and averaged.
3. Button input changes the current display mode.
4. Display updates automatically based on the selected mode.
5. Time is corrected for summer/winter time changes using the last Sundays in March and October.

---

## 🧪 Example OLED Output (128×32)

```
Temperature
22.6 °C
```

or

```
Time and Date
14:37:05
02.11.2025
```

---

## 🧰 File Structure

```
/weather-display
├── bme_sensor.h          # Global variables, constants, and declarations
├── bme_sensor.cpp        # Sensor logic, measurements, display handling
├── sketch_oct23.ino      # Main sketch (setup & loop)
└── README.md             # This file
```

---

## 🧭 Planned Extensions

* Store and plot temperature, humidity & pressure history 
* Add further buttons and menu interface
* Add battery indicator or power-saving mode
* Use a **128×64 OLED** for more information per screen
* use persistent memory

