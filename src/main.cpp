#include <Arduino.h>

#include <wifi_module.h>
#include <control.h>

void setup() {
    Serial.begin(9600);

    setup_wifi_module();
    setup_control_module();
}

void loop() {
    control_loop();

    //delay(100);
}
