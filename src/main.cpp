#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SSR_PIN D3 // Solid State Relay
#define ERR_LED_PIN D0
#define READY_LED_PIN 3 // RX pin
#define TEMP_PIN D7 // DS18B20 sensor
#define THERMO_SO D6 // for Thermocoupler reading
#define THERMO_CS D8 // for Thermocoupler reading
#define THERMO_SCK D5 // for Thermocoupler reading
#define HEATER_CRITICAL_TEMP 400.0 // when to halt everything
#define HEATER_STOP_TEMP 300.0 // when to shut off the heaters
#define HEATER_START_TEMP 290.0 // below this number we start the heaters
#define HEATER_NOT_READY_TEMP 250.0 // below this number we notify the user to stop
#define HEATER_POLL_TIME 4000 // how often to check temp

MAX6675 thermo(THERMO_SCK, THERMO_CS, THERMO_SO);
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire tempOw(TEMP_PIN);
DallasTemperature dtSensor(&tempOw);
bool halt;
unsigned long lastMillis;
unsigned long errLastMillis;

void updateTempOnScreen(float blockTemp, float caseTemp) {
    String res;
    res += (int)blockTemp;
    res += (char)223;
    res += "C";
    if (res.length() < 5) res += " ";
    lcd.setCursor(0, 0);
    lcd.print(res.c_str());
    res = "";
    res += (int)caseTemp;
    res += (char)223;
    res += "C";
    if (res.length() < 5) res += " ";
    lcd.setCursor(11, 0);
    lcd.print(res.c_str());
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(SSR_PIN, OUTPUT);
    pinMode(ERR_LED_PIN, OUTPUT);
    pinMode(READY_LED_PIN, OUTPUT);

    digitalWrite(SSR_PIN, LOW);
    digitalWrite(READY_LED_PIN, HIGH);
    dtSensor.begin();

    lcd.init();
    lcd.clear();
    lcd.backlight();

    // check initial temperature before anything else
    // shut down if the block is nearing self-destruction
    float blockTemp = thermo.readCelsius();
    dtSensor.requestTemperatures();
    float caseTemp = dtSensor.getTempCByIndex(0);
    if (isnan(blockTemp)) {
        // thermocoupler not attached!
        lcd.setCursor(2, 1);
        lcd.print("CHECK THRMCP");
        Serial.println("CHECK THRMCP");
        halt = true;
        return;
    } else if (caseTemp == DEVICE_DISCONNECTED_C) {
        // case temp sensor not attached!
        lcd.setCursor(2, 1);
        lcd.print("CHECK SENSOR");
        Serial.println("CHECK SENSOR");
        halt = true;
        return;
    } else if (blockTemp >= HEATER_CRITICAL_TEMP) {
        lcd.setCursor(3, 1);
        lcd.print("OVER-HEAT");
        Serial.println("OVER-HEAT");
        halt = true;
        return;
    } else if (blockTemp >= HEATER_START_TEMP) {
        lcd.setCursor(4, 1);
        lcd.print("Ready  ");
        Serial.println("Ready  ");
    }

    if (!halt && (blockTemp < HEATER_NOT_READY_TEMP)) {
        digitalWrite(READY_LED_PIN, HIGH);
    } else if (!halt && (blockTemp >= HEATER_STOP_TEMP)) {
        digitalWrite(READY_LED_PIN, LOW);
    }

    if (!halt) updateTempOnScreen(blockTemp, caseTemp);
}

void loop() {
    if (halt) {
        if (digitalRead(SSR_PIN) != LOW) digitalWrite(SSR_PIN, LOW);
        if ((millis() - errLastMillis) >= 1000) {
            errLastMillis = millis();
            digitalWrite(ERR_LED_PIN, !digitalRead(ERR_LED_PIN));
        }
    }

    if ((millis() - lastMillis) >= HEATER_POLL_TIME) {
        lastMillis = millis();
        float blockTemp = thermo.readCelsius();
        dtSensor.requestTemperatures();
        float caseTemp = dtSensor.getTempCByIndex(0);
        // Serial.println(caseTemp);

        if (caseTemp == DEVICE_DISCONNECTED_C) {
            // case temp sensor not attached!
            lcd.setCursor(2, 1);
            lcd.print("CHECK SENSOR");
            digitalWrite(READY_LED_PIN, HIGH);
            Serial.println("CHECK SENSOR");
            halt = true;
        }

        if (!isnan(blockTemp)) {
            // Serial.print(blockTemp);
            updateTempOnScreen(blockTemp, caseTemp);
            if (!halt && (blockTemp >= HEATER_CRITICAL_TEMP)) {
                lcd.setCursor(3, 1);
                lcd.print("OVER-HEAT");
                digitalWrite(READY_LED_PIN, HIGH);
                Serial.println("OVER-HEAT");
                halt = true;
                return;
            } else if (!halt && (blockTemp >= HEATER_STOP_TEMP)) {
                digitalWrite(SSR_PIN, LOW);
                digitalWrite(READY_LED_PIN, LOW);
                lcd.setCursor(4, 1);
                lcd.print("Ready  ");
                Serial.println("Ready  ");
            } else if (!halt && (blockTemp <= HEATER_START_TEMP)) {
                digitalWrite(SSR_PIN, HIGH);
                lcd.setCursor(4, 1);
                lcd.print("Heating");
                Serial.println("Heating");
            }

            if (!halt && (blockTemp < HEATER_NOT_READY_TEMP)) {
                digitalWrite(READY_LED_PIN, HIGH);
            }
        } else {
            // thermocoupler not attached!
            lcd.clear();
            lcd.setCursor(2, 1);
            lcd.print("CHECK THRMCP");
            digitalWrite(READY_LED_PIN, HIGH);
            Serial.println("CHECK THRMCP");
            halt = true;
            return;
        }
    }
}
