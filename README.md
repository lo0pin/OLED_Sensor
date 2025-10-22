
## Weather Display with OLED and BME280

**Project:** Arduino-based mini weather station displaying sensor data and real-time clock information on an SSD1306 OLED display

---

### **Hardware Components**

* **Arduino** (e.g. Uno, Nano, or Pro Mini)
* **BME280 Sensor** (temperature, humidity, pressure – I²C address `0x76` or `0x77`)
* **SSD1306 OLED Display** (128×32 px – I²C address `0x3C` or `0x3D`)
* **DS3231 RTC Module** (real-time clock, temperature-compensated, high accuracy)
* **Two push buttons** for switching between display modes

---

### 🔌 **Wiring (I²C + Buttons)**

| Component         | Signal | Arduino Pin |
| ----------------- | ------ | ----------- |
| BME280 SDA        | SDA    | A4          |
| BME280 SCL        | SCL    | A5          |
| SSD1306 SDA       | SDA    | A4          |
| SSD1306 SCL       | SCL    | A5          |
| DS3231 SDA        | SDA    | A4          |
| DS3231 SCL        | SCL    | A5          |
| **Button “Up”**   | to GND | D3          |
| **Button “Down”** | to GND | D9          |

Both buttons use `INPUT_PULLUP` mode — no external resistors required.

---

### **Used Libraries**

* [`Adafruit_BME280`](https://github.com/adafruit/Adafruit_BME280_Library)
* [`Adafruit_Sensor`](https://github.com/adafruit/Adafruit_Sensor)
* [`Adafruit_SSD1306`](https://github.com/adafruit/Adafruit_SSD1306)
* [`RTClib`](https://github.com/adafruit/RTClib)
* `Wire` (included by default with the Arduino IDE)

---

### **Features**

* Displays **temperature**, **humidity**, **air pressure**, and **time**
* Real-time clock (DS3231) keeps accurate time even when powered off
* **Menu navigation** via two buttons:

  * **Up button** → next display mode
  * **Down button** → previous display mode
* **Software debouncing** for clean button handling
* OLED output with automatically refreshed display

---

### **How It Works**

1. On startup, the OLED initializes and shows `Hello BUBU!`.
2. The display cycles through:

   * Temperature (°C)
   * Humidity (%)
   * Pressure (hPa)
   * Time (HH:MM)
3. Use the buttons to navigate between the different screens.

---

### **Important Notes**

* I²C addresses depend on the specific modules used:

  * **BME280:** `0x76` or `0x77`
  * **SSD1306:** `0x3C` or `0x3D`
    If the sensor or display is not detected, run an **I²C scanner** to check which addresses are active.
* All I²C devices share the same SDA and SCL lines — this is normal as long as addresses differ.
* The RTC is automatically initialized with the compilation time on upload using `__DATE__` and `__TIME__`.

---

### **Possible Extensions**

* Store and visualize a **pressure history** (barograph)
* Auto-cycling through display modes
* Display **pressure trends** (rising/falling)
* Add a **battery indicator**
* Use a **128×64 display** for additional data

---

### 📷 **Example Display Output (OLED 128×32)**

```
Temperature
22.6 °C

Mode: 0
```

