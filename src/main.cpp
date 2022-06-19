#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>

#define SSR_PIN D3
#define BTN_PIN D7
#define THERMO_DO D6
#define THERMO_CS D8
#define THERMO_CLK D5

MAX6675 thermo(THERMO_CLK, THERMO_CS, THERMO_DO);

void setup() {
    Serial.begin(115200);

    pinMode(SSR_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    lcd.init();
    lcd.clear();
    lcd.backlight();
}

void loop() {
    
}
