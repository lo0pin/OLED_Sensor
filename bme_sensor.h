#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include <Adafruit_BME280.h>

/* * * * * * * * * * * * * * * *
 * Globale Konstanten          *
 * für #if-Anweisungen         *
 * * * * * * * * * * * * * * * */

#define INDICATOR_LED   1   //Blinken der LED: zur Anzeige ob der uC läuft
#define DEBUG           0   //Debugmode: Ausgabe von Indikatoren über Serial-Schnittstelle
#define FIX_TIME_ONCE   0   //beim Bootvorgang die Kompilierzeit in die RTC schreiben


/* * * * * * * * * * * * * * * *
 * Konstanten                  *
 * * * * * * * * * * * * * * * */
 
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

#define upperbuttonpowersource  12
#define lowerbuttonpowersource  6
#define upperbuttonsensor       3
#define lowerbuttonsensor       9

#define numberofdisplaymodes    3

#define minimumstepfordrawing   0.3

constexpr uint8_t    array_len =               24;
constexpr uint8_t    numberOfMeassurements =   5;
constexpr uint16_t   WAIT_TIME =               10000;
constexpr uint16_t   WAIT_TIME_BUTTON =        300;
constexpr uint8_t    WAIT_TIME_MEASSURE =      200;
constexpr uint16_t   WAIT_TIME_MITTELWERT =    3000;
constexpr uint8_t    xMax = (array_len * 3 < SCREEN_WIDTH - 1) ? array_len * 3 : (SCREEN_WIDTH - 1);
constexpr uint8_t    buffLen =                 16;

#define old_hour_default 99

/* * * * * * * * * * * * * * * *
 * Globale Variablen -         *
 * nur deklariert!             *
 * * * * * * * * * * * * * * * */

//Temperatur, Luftfeuchte, Luftdruck
extern int16_t T, H, P;

//Aufnahme in 24h Chronologie
extern int16_t  temp_messungen[array_len];
extern int16_t  humid_messungen[array_len];
extern int16_t  baro_messungen[array_len];
extern uint8_t  old_hour;
extern int8_t  old_day;

//Mittelwertbildung
extern uint8_t    currentMeassurementCounter; //0...5
extern int16_t    tempsforMittelwert[numberOfMeassurements];
extern int16_t    humidsforMittelwert[numberOfMeassurements];
extern int16_t    pressuresforMittelwert[numberOfMeassurements];

extern uint16_t   MeassurementTimerMittelwert;

extern int32_t    hourlyMittelwertTemp;
extern int32_t    hourlyMittelwertHygro;
extern int32_t    hourlyMittelwertBaro;
extern uint8_t    hourlyMittelwertCounter;
extern uint8_t    old_minute;

extern bool   sommerzeit;

//Anzeigestatus "Page"
extern uint8_t    displaymode;

//diverse Timer
extern unsigned long  timer;
extern unsigned long  button_timer;
extern unsigned long  meassure_timer;

//Strings für Darstellung von Zeit und Datum
//extern String time_now_string;
//extern String date_now_string;
extern char timeBuf[buffLen];
extern char dateBuf[buffLen];


/* * * * * * * * * * * * * * * *
 * Funktionsdeklarationen      *
 * * * * * * * * * * * * * * * */

// --- EEPROM Persistenz ---


/*
 * Lädt die 24h-Messarrays und relevante Persistenzdaten aus dem EEPROM.
 *
 * Beschreibung:
 * - Liest aus dem EEPROM zuvor gespeicherte Messdaten (z.B. temp_messungen,
 *   humid_messungen, baro_messungen, evtl. Stunden-/Timestamps oder Prüfsummen).
 * - Validiert die geladenen Daten (Checksumme, Plausibilitätsprüfung o.Ä.)
 * - Setzt die globalen Arrays/Variablen, wenn die Daten gültig sind.
 *
 * Parameter:
 * - (keine)
 *
 * Rückgabewert:
 * - true  => gültige, vollständige Messdaten wurden aus dem EEPROM geladen
 * - false => keine gültigen Daten gefunden (z.B. erster Start oder EEPROM ungültig)
 */
bool loadMeasurementsFromEEPROM();   // gibt true zurück, wenn gültige Daten geladen wurden


/*
 * Speichert die aktuellen 24h-Messarrays und relevante Persistenzdaten in das EEPROM.
 *
 * Beschreibung:
 * - Schreibt die aktuellen Messdaten (temp_messungen, humid_messungen, baro_messungen)
 *   sowie benötigte Metadaten (z.B. aktueller Index, Prüfsumme) in den EEPROM,
 *   damit sie nach einem Neustart wiederhergestellt werden können.
 * - Kümmert sich idealerweise um Wear-Leveling / Minimierung von EEPROM-Schreibzyklen.
 *
 * Parameter:
 * - (keine)
 *
 * Rückgabewert:
 * - true  => Speichervorgang war erfolgreich
 * - false => Schreiben in EEPROM ist fehlgeschlagen
 */
bool saveMeasurementsToEEPROM();     // schreibt aktuelle Arrays in den EEPROM


/*
 * Formatiert Zeit- und Datumsstrings aus einem DateTime-Objekt.
 *
 * Beschreibung:
 * - Erzeugt aus dem übergebenen DateTime-Objekt zwei nullterminierte C-Strings:
 *   einen Zeitstring und einen Datumsstring, passend für die Anzeige.
 * - Die Funktion achtet auf die übergebenen Puffergrößen (timeSize / dateSize)
 *   und vermeidet Pufferüberläufe.
 *
 * Parameter:
 * - char* timeString: Zeiger auf einen Puffer, in den der Zeitstring geschrieben wird.
 * - size_t timeSize: Größe des Zeitpuffers in Bytes.
 * - char* dateString: Zeiger auf einen Puffer, in den der Datumsstring geschrieben wird.
 * - size_t dateSize: Größe des Datumpuffers in Bytes.
 * - const DateTime& dt: RTC-Zeitpunkt, der formatiert werden soll.
 *
 * Rückgabewert:
 * - void: Die formatierten Strings werden in die übergebenen Puffer geschrieben.
 */
void getTimeAndDateString(char* timeString, size_t timeSize, char* dateString, size_t dateSize, const DateTime& dt);


/*
 * Füllt die stündlichen Arrays mit aktuellen Messwerten vom BME280 und
 * verwaltet die Werte für spätere Mittelwertberechnungen.
 *
 * Beschreibung:
 * - Liest (oder erhält) aktuelle Messwerte vom Sensor (über bme_ref) und
 *   speichert diese Rohwerte in die jeweiligen Arrays (temp/humi/pressur).
 * - Aktualisiert ggf. einen Index / Zähler (meassure) für die Position
 *   innerhalb der Arrays.
 * - Kann auch zur Glättung bzw. Vorverarbeitung der Rohdaten genutzt werden.
 *
 * Parameter:
 * - Adafruit_BME280& bme_ref: Referenz auf das BME280-Objekt zur Messwerterfassung.
 * - int16_t temp[]: Array, in das Temperaturwerte aufgezeichnet werden.
 * - int16_t humi[]: Array, in das Feuchtewerte aufgezeichnet werden.
 * - int16_t pressur[]: Array, in das Luftdruckwerte aufgezeichnet werden.
 * - uint8_t& meassure: Referenz auf den Zähler/Index für die aktuelle Messposition.
 *
 * Rückgabewert:
 * - void: Die Arrays und der Index werden in-place aktualisiert.
 */
void fill_arrays(Adafruit_BME280& bme_ref, int16_t temp[], int16_t humi[], int16_t pressur[], uint8_t& meassure);


/**
 * Berechnet Mittelwerte aus den gesammelten Messungen.
 *
 * Beschreibung:
 * - Berechnet aus den übergebenen Messwerten (Arrays und Sammler) gleitende
 *   oder stündliche Mittelwerte für Temperatur, Luftfeuchte und Luftdruck.
 * - Das Ergebnis wird in die übergebenen Referenzvariablen geschrieben.
 * - Nutzt numberOfMeassurements und currentMeassurementCounter für die Logik.
 *
 * Parameter:
 * - int16_t& te: Referenz, in die der berechnete Temperatur-Mittelwert geschrieben wird.
 * - int16_t& hy: Referenz, in die der berechnete Feuchte-Mittelwert geschrieben wird.
 * - int16_t& ba: Referenz, in die der berechnete Druck-Mittelwert geschrieben wird.
 * - int16_t temp[]: Array mit Temperaturmessungen für die Berechnung.
 * - int16_t hygro[]: Array mit Feuchtemessungen für die Berechnung.
 * - int16_t baro[]: Array mit Druckmessungen für die Berechnung.
 * - const uint8_t& measure: Anzahl bzw. gültige Messwerte, die in die Berechnung einfließen.
 *
 * Rückgabewert:
 * - void: Die Ergebnisse werden in te, hy und ba geschrieben.
 */
void mittelwerte_berechnen(int16_t& te, int16_t& hy, int16_t& ba, int16_t temp[], int16_t hygro[], int16_t baro[], const uint8_t& measure);


/*
 * Konfiguriert die verwendeten GPIO-Pins (Button, LED, Sensor-Power etc.).
 *
 * Beschreibung:
 * - Initialisiert Pin-Modi (INPUT_PULLUP, OUTPUT) für die verwendeten Pins,
 *   setzt initiale Zustände (z. B. LED aus) und bereitet die Hardware für
 *   den normalen Betrieb vor.
 *
 * Parameter:
 * - (keine)
 *
 * Rückgabewert:
 * - void
 */
void setupPins();


/**
 * Initialisiert Peripheriekomponenten: Display, RTC und BME280.
 *
 * Beschreibung:
 * - Führt die notwendige Initialisierung für Display, RTC und BME280 aus,
 *   überprüft ggf. die Erreichbarkeit und stellt Default-Konfigurationen ein.
 * - Diese Funktion kapselt alle Hardware-Initialisierungen, damit setup() im
 *   Hauptprogramm sauber bleibt.
 *
 * Parameter:
 * - Adafruit_SSD1306& display_ref: Referenz auf das SSD1306-Displayobjekt.
 * - RTC_DS3231& rtc_ref: Referenz auf das RTC-Objekt (DS3231).
 * - Adafruit_BME280& bme_ref: Referenz auf das BME280-Sensorobjekt.
 *
 * Rückgabewert:
 * - void
 */
void setupPeripherie(Adafruit_SSD1306& display_ref, RTC_DS3231& rtc_ref, Adafruit_BME280& bme_ref);

/**
 * Führt eine Messsequenz mit dem BME280 durch und aktualisiert globale Werte.
 *
 * Beschreibung:
 * - Liest Temperatur, Luftfeuchte und Luftdruck vom Sensor (bme_var).
 * - Führt ggf. eine Mehrfachmessung mit Mittelwertbildung durch (je nach
 *   Implementierung in der C-Datei) und schreibt die Werte in die globalen
 *   Variablen T, H, P oder entsprechende Puffer.
 *
 * Parameter:
 * - Adafruit_BME280& bme_var: Referenz auf das BME280-Objekt, das die Messung ausführt.
 *
 * Rückgabewert:
 * - void
 */
void doMeasurements(Adafruit_BME280& bme_var);

/*
 * Liest die Taster-/Knopfzustände ein und verarbeitet Ereignisse.
 *
 * Beschreibung:
 * - Ermittelt, ob Taster gedrückt, gehalten oder losgelassen wurden.
 * - Führt entsprechende Aktionen aus (z. B. Displaywechsel, Display an/aus,
 *   Messung anstoßen), dabei werden Debounce- und Zeitfenster (WAIT_TIME_BUTTON)
 *   berücksichtigt.
 *
 * Parameter:
 * - (keine) - verwendet globale Pin- und Timer-Variablen.
 *
 * Rückgabewert:
 * - void
 */
void handleButtonInput();

////////////////////////////////////////////////
//Funktionen zur Zeitumstellung
////////////////////////////////////////////////

/*
 * Prüft, ob der übergebene Zeitpunkt der letzte Sonntag im Oktober ist.
 *
 * Beschreibung:
 * - Hilfsfunktion zur Erkennung des Endes der Sommerzeit (Umstellung auf Normalzeit).
 * - Prüft Datum/Tag im DateTime-Objekt und gibt true zurück, wenn es sich um
 *   den letzten Sonntag im Oktober handelt.
 *
 * Parameter:
 * - const DateTime& dt: zu prüfender Zeitpunkt (Datum).
 *
 * Rückgabewert:
 * - true  => dt liegt auf dem letzten Sonntag im Oktober
 * - false => dt ist nicht der letzte Sonntag im Oktober
 */
bool isLastSundayOfOctober(const DateTime& dt);

/*
 * Prüft, ob ein Zeitpunkt bereits nach dem letzten Sonntag im Oktober liegt.
 *
 * Beschreibung:
 * - Nutzt isLastSundayOfOctober oder zusätzliche Logik, um festzustellen,
 *   ob der aktuelle Zeitpunkt bereits nach dem Umstellzeitpunkt im Oktober liegt.
 *
 * Parameter:
 * - const DateTime& dt: zu prüfender Zeitpunkt
 *
 * Rückgabewert:
 * - true  => dt ist nach dem letzten Sonntag im Oktober (inkl. Zeitpunkt)
 * - false => dt ist vor dem Umstellzeitpunkt
 */
bool isPastLastSundayOfOctober(const DateTime& dt);

/*
 * Prüft, ob der übergebene Zeitpunkt der letzte Sonntag im März ist.
 *
 * Beschreibung:
 * - Hilfsfunktion zur Erkennung des Beginns der Sommerzeit (Uhr vor).
 * - Prüft Datum/Tag im DateTime-Objekt und gibt true zurück, wenn es sich um
 *   den letzten Sonntag im März handelt.
 *
 * Parameter:
 * - const DateTime& dt: zu prüfender Zeitpunkt (Datum).
 *
 * Rückgabewert:
 * - true  => dt liegt auf dem letzten Sonntag im März
 * - false => dt ist nicht der letzte Sonntag im März
 */
bool isLastSundayOfMarch(const DateTime& dt);

/*
 * Prüft und führt ggf. die Zeitumstellung (Sommer-/Winterzeit) durch.
 *
 * Beschreibung:
 * - Nimmt die aktuelle Zeit (dt) und das RTC-Objekt (rtc_ref) entgegen.
 * - Bestimmt, ob eine Umstellung auf Sommer- oder Normalzeit erforderlich ist
 *   und passt die RTC entsprechend an (z. B. +1h / -1h).
 * - Setzt das Flag 'sommerzeit' und passt ggf. interne Zähler an.
 *
 * Parameter:
 * - RTC_DS3231& rtc_ref: Referenz auf das RTC-Modul, das eingestellt werden kann.
 * - const DateTime& dt: aktueller Zeitpunkt zur Entscheidungsfindung.
 *
 * Rückgabewert:
 * - void
 */
void CheckZeitumstellung(RTC_DS3231& rtc_ref, const DateTime& dt);

////////////////////////////////////////////////
//Grafische Ausgabe 
////////////////////////////////////////////////

/**
 * Gibt Zeit, Datum und die aktuellen Messwerte formatiert auf dem Display aus.
 *
 * Beschreibung:
 * - Zeichnet/zeigt die Zeit (tns), das Datum (dns) und die Messwerte T, H, P
 *   auf dem OLED-Display (dis) in einer geeigneten Layout-Anordnung.
 * - Diese Funktion kümmert sich um die grafische Darstellung (Schriftgrößen,
 *   Einheiten, Positionierung).
 *
 * Parameter:
 * - Adafruit_SSD1306& dis: Referenz auf das Displayobjekt.
 * - char* tns: String/Puffer mit dem formatierten Zeitstring.
 * - char* dns: String/Puffer mit dem formatierten Datumsstring.
 * - int16_t& T: aktuelle Temperatur (in Zehntel-Grad oder Rohwert, abhängig von Implementierung).
 * - int16_t& H: aktuelle Luftfeuchte (in resp. Einheiten).
 * - int16_t& P: aktueller Luftdruck.
 *
 * Rückgabewert:
 * - void
 */
void printTimeDateMeasurements(Adafruit_SSD1306& dis, char* tns, char* dns, int16_t& T, int16_t& H, int16_t& P);


////////////////////////////////////////////////
//schreiben in die 24h-Messarrays und ins Eeprom
////////////////////////////////////////////////

/**
 * Speichert stündliche Messungen in die 24h-Arrays und in das EEPROM.
 *
 * Beschreibung:
 * - Wenn sich die Stunde geändert hat, werden die aktuellen (ggf. gemittelten)
 *   Messwerte in die ringförmigen 24-Stunden-Arrays geschrieben.
 * - Aktualisiert oldhour_var, schreibt ggf. in EEPROM (oder markiert für späteres Speichern).
 * - Diese Funktion konsolidiert die Logik, wann und wie stündliche Werte persistiert werden.
 *
 * Parameter:
 * - uint8_t& oldhour_var: Referenz auf die letzte bekannte Stunde (wird aktualisiert).
 * - const DateTime& right_now_var: Aktueller Zeitpunkt zur Entscheidung, ob eine neue Stunde begonnen hat.
 * - int16_t temp_messungen_var[]: Array mit stündlichen Temperaturwerten (24 Einträge).
 * - int16_t humid_messungen_var[]: Array mit stündlichen Feuchtewerten.
 * - int16_t baro_messungen_var[]: Array mit stündlichen Druckwerten.
 * - int16_t& T_var: Aktueller Temperaturwert, der ggf. gespeichert werden soll.
 * - int16_t& H_var: Aktueller Feuchtewert, der ggf. gespeichert werden soll.
 * - int16_t& P_var: Aktueller Druckwert, der ggf. gespeichert werden soll.
 *
 * Rückgabewert:
 * - void
 */
void saveHourlyMeasurements(uint8_t& oldhour_var, const DateTime& right_now_var, int16_t temp_messungen_var[], int16_t humid_messungen_var[], int16_t baro_messungen_var[], int32_t& T_var, int32_t& H_var, int32_t& P_var);

/**
 * Zeichnet die Y-Achse (Skalierung/Labels) für Graphendarstellungen.
 *
 * Beschreibung:
 * - Zeichnet auf dem Display (dis) die vertikale Achse bei X-Position 'y'
 *   inklusive Einteilungen und ggf. Einheitenbeschriftung.
 * - Dient als Hilfsfunktion für drawGraph.
 *
 * Parameter:
 * - int y: X-Position oder Offset an dem die Y-Achse gezeichnet werden soll.
 * - Adafruit_SSD1306& dis: Referenz auf das Displayobjekt.
 *
 * Rückgabewert:
 * - void
 */
void drawAxeY(int y, Adafruit_SSD1306& dis);

/**
 * Zeichnet ein Liniendiagramm der übergebenen Messwerte auf dem Display.
 *
 * Beschreibung:
 * - Wandelt die Werte des Arrays in Pixelkoordinaten um und zeichnet Linien/Punkte,
 *   die den Verlauf über die Zeit zeigen.
 * - Berücksichtigt Startindex (start_val), Einheitstext (unit) und vorgegebene Min/Max-Werte,
 *   um die Skalierung korrekt durchzuführen.
 *
 * Parameter:
 * - int16_t the_array[]: Array mit den darzustellenden Messwerten.
 * - Adafruit_SSD1306& dis: Referenz auf das Displayobjekt.
 * - const uint8_t start_val: Startindex oder Offset innerhalb des Arrays für die Darstellung.
 * - const __FlashStringHelper* unit: Einheit als Flash-String (z. B. F("°C")) zur Anzeige.
 * - int16_t min_value: Minimalwert für die Skalierung der Y-Achse.
 * - int16_t max_value: Maximalwert für die Skalierung der Y-Achse.
 *
 * Rückgabewert:
 * - void
 */
void drawGraph(int16_t the_array[], Adafruit_SSD1306& dis, const uint8_t start_val, const __FlashStringHelper* unit, int16_t min_value, int16_t max_value);

/**
 * Zeichnet einen ganzzahligen Wert, der als Wert/10 vorliegt, formatiert auf dem Display.
 *
 * Beschreibung:
 * - Erwartet einen Wert in der Form valueTimes10 (z. B. 235 -> 23.5).
 * - Formatiert und zeichnet diesen Wert mit einer festen Dezimalstelle auf das Display.
 *
 * Parameter:
 * - Adafruit_SSD1306& dis: Referenz auf das Displayobjekt.
 * - int16_t valueTimes10: Wert multipliziert mit 10 (eine Dezimalstelle implizit).
 *
 * Rückgabewert:
 * - void
 */
void printFixed10(Adafruit_SSD1306& dis, int16_t valueTimes10);

/**
 * Konvertiert einen int16_t-Wert, der intern als "Wert*10" gespeichert sein könnte,
 * in einen float-Wert mit Dezimalstelle.
 *
 * Beschreibung:
 * - Wandelt die interne Darstellung (z. B. 235 -> 23.5) in einen float um.
 * - Nützlich für Berechnungen oder für Funktionen, die Fließkommazahlen benötigen.
 *
 * Parameter:
 * - int16_t num: Ganzzahliger Eingabewert (z. B. in Zehntel-Einheiten).
 *
 * Rückgabewert:
 * - float: Konvertierter Fließkommawert.
 */
float int16_tToFloat (int16_t num);

/**
 * Konvertiert einen float-Wert in das interne Format int16_t.
 *
 * Beschreibung:
 * - Wandelt einen float (z. B. 23.5) in eine int16_t-Repräsentation um, möglicherweise
 *   multipliziert mit 10 (z. B. 235), um Speicherplatz zu sparen und Rechenaufwand zu minimieren.
 * - Achtung: Überlauf prüfen, da int16_t begrenzte Reichweite hat.
 *
 * Parameter:
 * - float num: Eingabe-Fließkommawert.
 *
 * Rückgabewert:
 * - int16_t: Konvertierter Ganzzahlwert (z. B. num * 10).
 */
int16_t FloatToInt16_t (float num);

