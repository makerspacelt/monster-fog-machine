#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//===================================================
#define SSR_PIN D3 // Solid State Relay
#define ERR_LED_PIN D0
#define STATUS_LED_PIN 3 // RX pin, pull HIGH for red, LOW for green
//------
#define TEMP_PIN D7 // DS18B20 sensor
#define TEMP_POLL_DELAY 1000 // how often to check the inside temp
#define TEMP_POLL_RETRY_COUNT 5 // how many times to try and read the sensor before giving up
//------
// for Thermocoupler reading
#define THERMO_SO D6
#define THERMO_CS D8
#define THERMO_SCK D5
//------
#define HEATER_CRITICAL_TEMP 300.0 // when to halt everything for emergency shutdown
#define HEATER_STOP_TEMP 200.0 // when to shut off the heaters
#define HEATER_START_TEMP 190.0 // below this number we start the heaters
#define HEATER_NOT_READY_TEMP 150.0 // below this number we notify the user to stop
#define HEATER_POLL_DELAY 1000 // how often to check the block temp
//===================================================

MAX6675 thermo(THERMO_SCK, THERMO_CS, THERMO_SO);
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire tempOw(TEMP_PIN);
DallasTemperature dtSensor(&tempOw);
bool halt;
int pollReads;
unsigned long lastHeaterPollMillis;
unsigned long lastCasePollMillis;
unsigned long errLastMillis;

void showSplashScreen() {
    lcd.setCursor(5, 0);
    lcd.print("MFM v2");
    lcd.setCursor(1, 1);
    lcd.print("makerspace.lt");
    delay(1500);
}

String formatTemp(float temp) {
    String res;
    res += (int)temp;
    res += (char)223;
    res += "C";
    if (res.length() < 5) res += " ";
    return res;
}

void updateBlockTempOnScreen(float blockTemp) {
    String res = formatTemp(blockTemp);
    lcd.setCursor(0, 0);
    lcd.print(res.c_str());
}

void updateCaseTempOnScreen(float caseTemp) {
    String res = formatTemp(caseTemp);
    lcd.setCursor(11, 0);
    lcd.print(res.c_str());
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(SSR_PIN, OUTPUT);
    pinMode(ERR_LED_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);

    digitalWrite(SSR_PIN, LOW);
    digitalWrite(STATUS_LED_PIN, HIGH);
    dtSensor.begin();

    lcd.init();
    lcd.clear();
    lcd.backlight();
    showSplashScreen();

    // check initial temperature before anything else
    float blockTemp = thermo.readCelsius();
    pollReads = 0;
    float caseTemp;
    do {
        dtSensor.requestTemperatures();
        caseTemp = dtSensor.getTempCByIndex(0);
        pollReads++;
    } while ((pollReads < TEMP_POLL_RETRY_COUNT) || (caseTemp != DEVICE_DISCONNECTED_C));
    if (isnan(blockTemp)) {
        // thermocoupler not attached!
        lcd.clear();
        lcd.setCursor(2, 1);
        lcd.print("CHECK THRMCP");
        Serial.println("CHECK THRMCP");
        halt = true;
        return;
    } else if (caseTemp == DEVICE_DISCONNECTED_C) {
        // case temp sensor not attached!
        lcd.clear();
        lcd.setCursor(2, 1);
        lcd.print("CHECK SENSOR");
        Serial.println("CHECK SENSOR");
        halt = true;
        return;
    } else if (blockTemp >= HEATER_CRITICAL_TEMP) {
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("OVER-HEAT");
        Serial.println("OVER-HEAT");
        halt = true;
        return;
    } else if (blockTemp >= HEATER_START_TEMP) {
        lcd.clear();
        lcd.setCursor(4, 1);
        lcd.print("Ready");
        Serial.println("Ready");
    }

    if (blockTemp < HEATER_NOT_READY_TEMP) {
        digitalWrite(STATUS_LED_PIN, HIGH);
    } else if (blockTemp >= HEATER_STOP_TEMP) {
        digitalWrite(STATUS_LED_PIN, LOW);
    }

    dtSensor.setWaitForConversion(false);
    dtSensor.requestTemperatures();
    lastCasePollMillis = millis();
}

void loop() {
    if (halt) {
        if (digitalRead(SSR_PIN) != LOW) digitalWrite(SSR_PIN, LOW);
        if ((millis() - errLastMillis) >= 1000) {
            errLastMillis = millis();
            digitalWrite(ERR_LED_PIN, !digitalRead(ERR_LED_PIN));
        }
    }

    // process case temperature
    if ((millis() - lastCasePollMillis) >= TEMP_POLL_DELAY) {
        float caseTemp = dtSensor.getTempCByIndex(0);
        Serial.print("Case temp: ");
        Serial.println(caseTemp);
        if (caseTemp == DEVICE_DISCONNECTED_C) {
            pollReads++;
            if (pollReads == TEMP_POLL_RETRY_COUNT) {
                // case temp sensor not attached!
                lcd.setCursor(2, 1);
                lcd.print("CHECK SENSOR");
                digitalWrite(STATUS_LED_PIN, HIGH);
                Serial.println("CHECK SENSOR");
                halt = true;
            }
        } else {
            pollReads = 0;
            updateBlockTempOnScreen(caseTemp);
        }
        dtSensor.requestTemperatures();
        lastCasePollMillis = millis();
    }

    // process block temperature
    if ((millis() - lastHeaterPollMillis) >= HEATER_POLL_DELAY) {
        lastHeaterPollMillis = millis();
        float blockTemp = thermo.readCelsius();
        
        if (!isnan(blockTemp)) {
            Serial.print("Block temp: ");
            Serial.println(blockTemp);
            updateBlockTempOnScreen(blockTemp);
            if (!halt && (blockTemp >= HEATER_CRITICAL_TEMP)) {
                lcd.setCursor(3, 1);
                lcd.print("OVER-HEAT");
                digitalWrite(STATUS_LED_PIN, HIGH);
                Serial.println("OVER-HEAT");
                halt = true;
                return;
            } else if (!halt && (blockTemp >= HEATER_STOP_TEMP)) {
                digitalWrite(SSR_PIN, LOW);
                digitalWrite(STATUS_LED_PIN, LOW);
                lcd.setCursor(4, 1);
                lcd.print("Ready  ");
            } else if (!halt && (blockTemp <= HEATER_START_TEMP)) {
                digitalWrite(SSR_PIN, HIGH);
                lcd.setCursor(4, 1);
                lcd.print("Heating");
            }

            if (!halt && (blockTemp < HEATER_NOT_READY_TEMP)) {
                digitalWrite(STATUS_LED_PIN, HIGH);
            }
        } else {
            // thermocoupler not attached!
            lcd.clear();
            lcd.setCursor(2, 1);
            lcd.print("CHECK THRMCP");
            digitalWrite(STATUS_LED_PIN, HIGH);
            Serial.println("CHECK THRMCP");
            halt = true;
            return;
        }
    }
}
