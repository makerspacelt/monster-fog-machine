#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>

#define SSR_PIN D3 // Solid State Relay
#define ERR_LED_PIN D0
#define THERMO_SO D6
#define THERMO_CS D8
#define THERMO_SCK D5
#define HEATER_CRITICAL_TEMP 150.0 // when to halt everything
#define HEATER_STOP_TEMP 100.0 // when to shut off the heaters
#define HEATER_START_TEMP 60.0 // below this number we start the heaters
#define HEATER_POLL_TIME 1000 // how often to check block temp

MAX6675 thermo(THERMO_SCK, THERMO_CS, THERMO_SO);
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool halt;
unsigned long lastMillis;

void updateTempOnScreen(float blockTemp) {
    lcd.setCursor(0, 0);
    lcd.print((int)blockTemp);
    lcd.print((char)223);
    lcd.print("C");
    lcd.print("      ");
    lcd.print((int)blockTemp);
    lcd.print((char)223);
    lcd.print("C");
    lcd.print("  ");
    
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(SSR_PIN, OUTPUT);
    pinMode(ERR_LED_PIN, OUTPUT);

    digitalWrite(SSR_PIN, LOW);

    lcd.init();
    lcd.clear();
    lcd.backlight();

    // check initial temperature before anything else
    // shut down if the block is nearing self-destruction
    float blockTemp = thermo.readCelsius();
    if (isnan(blockTemp)) {
        // thermocoupler not attached!
        lcd.setCursor(2, 0);
        lcd.print("CHECK THRMCP");
        halt = true;
        return;
    } else if (blockTemp >= HEATER_CRITICAL_TEMP) {
        lcd.setCursor(3, 0);
        lcd.print("OVER-HEAT");
        halt = true;
        return;
    }
}

void loop() {
    if (halt) {
        if (digitalRead(SSR_PIN) != LOW) digitalWrite(SSR_PIN, LOW);
        if ((millis() - lastMillis) >= 1000) {
            lastMillis = millis();
            digitalWrite(ERR_LED_PIN, !digitalRead(ERR_LED_PIN));
        }
        return;
    }

    if ((millis() - lastMillis) >= HEATER_POLL_TIME) {
        lastMillis = millis();
        float blockTemp = thermo.readCelsius();
        if (!isnan(blockTemp)) {
            // Serial.print(blockTemp);
            updateTempOnScreen(blockTemp);
            if (blockTemp >= HEATER_CRITICAL_TEMP) {
                lcd.clear();
                lcd.setCursor(3, 0);
                lcd.print("OVER-HEAT");
                halt = true;
                return;
            } else if (blockTemp >= HEATER_STOP_TEMP) {
                digitalWrite(SSR_PIN, LOW);
                lcd.setCursor(4, 1);
                lcd.print("Ready");
            } else if (blockTemp <= HEATER_START_TEMP) {
                digitalWrite(SSR_PIN, HIGH);
                lcd.setCursor(4, 1);
                lcd.print("Heating");
            }
        } else {
            // thermocoupler not attached!
            lcd.clear();
            lcd.setCursor(2, 0);
            lcd.print("CHECK THRMCP");
            halt = true;
            return;
        }
    }

}
