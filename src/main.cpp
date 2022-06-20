#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include <EEPROM.h>

#define SSR_PIN D3 // Solid State Relay
#define BTN_PIN D7
#define THERMO_DO D6
#define THERMO_CS D8
#define THERMO_CLK D5
#define HEATER_CRITICAL_TEMP 350.0 // when to halt everything
#define HEATER_STOP_TEMP 300.0 // when to shut off the heaters
#define HEATER_START_TEMP 260.0 // below this number we start the heaters

MAX6675 thermo(THERMO_CLK, THERMO_CS, THERMO_DO);
float blockTemp;
bool halt;

void setup() {
    Serial.begin(115200);
    Serial.println();

    EEPROM.begin(1);

    pinMode(SSR_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);

    digitalWrite(SSR_PIN, LOW);

    LiquidCrystal_I2C lcd(0x27, 16, 2);
    lcd.init();
    lcd.clear();
    lcd.backlight();

    // reset self-destruction status
    if (digitalRead(BTN_PIN) == LOW) {
        EEPROM.write(0, 0);
        EEPROM.commit();
    } else {
        // has the block attempted self-destruction before?
        int val = EEPROM.read(0);
        Serial.print(val);
        if (val != 0) {
            lcd.setCursor(2, 0);
            lcd.print("CHECK HEALTH");
            halt = true;
            return;
        }
    }

    // check initial temperature before anything else
    // shut down if the block is nearing self-destruction
    blockTemp = thermo.readCelsius();
    if (isnan(blockTemp)) {
        // thermocoupler not attached!
        lcd.setCursor(2, 0);
        lcd.print("CHECK THRMCP");
        halt = true;
        return;
    } else if (blockTemp >= HEATER_CRITICAL_TEMP) {
        lcd.setCursor(2, 0);
        lcd.print("CHECK HEALTH");
        EEPROM.write(0, 1);
        EEPROM.commit();
        halt = true;
        return;
    }
}

void loop() {
    if (halt) return;
}
