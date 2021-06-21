#include <Arduino.h>

const int HEATER_1_VOLTAGE_IND_PIN = 5;
const int HEATER_2_VOLTAGE_IND_PIN = 5;
const int HEATER_3_VOLTAGE_IND_PIN = 5;
const int FLUID_TANK_FLOAT_PIN = 5;
const int MACHINE_CONTROL_RELAY_PIN = 2;

bool timerControlActive = false;

float activationFrequency = 5 * 1000;
float activationLength = 10 * 1000; 

long controlTimer;

bool heater1state = false;
bool heater2state = false;
bool heater3state = false;
bool fluidTankState = false;

bool machineReady = false;
int controlState = -1; // 0 -> machine is stopped, waiting for next activation (waiting stage), 1 -> machine is started (activation stage)

void setup_control_module() {
    pinMode(MACHINE_CONTROL_RELAY_PIN, OUTPUT);
    pinMode(HEATER_1_VOLTAGE_IND_PIN, INPUT);
    pinMode(HEATER_2_VOLTAGE_IND_PIN, INPUT);
    pinMode(HEATER_3_VOLTAGE_IND_PIN, INPUT);
    pinMode(FLUID_TANK_FLOAT_PIN, INPUT);

    timerControlActive = true; //REMOVE AFTER DEBUG
}

void updateFogMachineState() {
    heater1state = digitalRead(HEATER_1_VOLTAGE_IND_PIN) == 1;
    heater2state = digitalRead(HEATER_2_VOLTAGE_IND_PIN) == 1;
    heater3state = digitalRead(HEATER_3_VOLTAGE_IND_PIN) == 1;
    fluidTankState = digitalRead(MACHINE_CONTROL_RELAY_PIN) == 1;

    machineReady = heater1state && heater2state && heater3state && fluidTankState;
    machineReady = true; //REMOVE AFTER DEBUG
}

void start_machine() {
    digitalWrite(MACHINE_CONTROL_RELAY_PIN, HIGH);
    Serial.println("STARTED");
}

void stop_machine() {
    digitalWrite(MACHINE_CONTROL_RELAY_PIN, LOW);
    Serial.println("STOPPED");
}

void control_loop() {
    // Updates the physical state of machine parameters
    updateFogMachineState();
    
    // Checking if timer control is activated and all machine requirements are met
    if(!timerControlActive || !machineReady) {
        // Stops the automatic and manual control of the machine and resets control variables to undefined
        
        stop_machine();
        controlState = -1;
        controlTimer = -1;
        return;
    }

    if(controlTimer == -1 || controlState == -1) {
        // Initial automatic control variables
        controlTimer = millis();
        controlState = 1;
    }

    // Machine activation stage
    if (controlState == 1) {
        if (millis() - controlTimer < activationLength) {
            // Activates machine
            start_machine();
        } else {
            // Initiates waiting stage
            controlState = 0;
            controlTimer = millis();
            stop_machine();
        }
    }

    // Waiting for next activation stage
    if (controlState == 0) {
        if (millis() - controlTimer < activationFrequency) {
            // Stops machine
            stop_machine();
        } else {
            // Initiates activation stage
            controlState = 1;
            controlTimer = millis();
            start_machine();
        }
    }

}
