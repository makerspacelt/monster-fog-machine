#include <Arduino.h>
#include <control.h>

const int HEATER_1_VOLTAGE_IND_PIN = 33;
const int HEATER_2_VOLTAGE_IND_PIN = 25;
const int HEATER_3_VOLTAGE_IND_PIN = 26;
const int FLUID_TANK_FLOAT_PIN = 27;
const int MACHINE_CONTROL_RELAY_PIN = 4;

int controlMode = CONTROL_MODE_NONE;

void control_manual();
void control_auto();

// AUTOMATIC CONTROL SETTINGS
float waitingTime = 5 * 60 * 1000;
float activationTime = 10 * 1000; 

// CURRENT MACHINE STATE VARIABLES
bool heater_1_ready = false;
bool heater_2_ready = false;
bool heater_3_ready = false;
bool fluid_not_empty = false;

long controlTimer;

bool all_heaters_ready = false;
bool any_heaters_ready = false;

int controlState = -1; // 0 -> machine is stopped, waiting for next activation (waiting stage), 1 -> machine is started (activation stage)

void setup_control_module() {
    pinMode(MACHINE_CONTROL_RELAY_PIN, OUTPUT);
    pinMode(HEATER_1_VOLTAGE_IND_PIN, INPUT);
    pinMode(HEATER_2_VOLTAGE_IND_PIN, INPUT);
    pinMode(HEATER_3_VOLTAGE_IND_PIN, INPUT);
    pinMode(FLUID_TANK_FLOAT_PIN, INPUT);
}

void updateFogMachineState() {
    heater_1_ready = digitalRead(HEATER_1_VOLTAGE_IND_PIN) == 0;
    heater_2_ready = digitalRead(HEATER_2_VOLTAGE_IND_PIN) == 0;
    heater_3_ready = digitalRead(HEATER_3_VOLTAGE_IND_PIN) == 0;
    fluid_not_empty = digitalRead(FLUID_TANK_FLOAT_PIN) == 1;

    all_heaters_ready = heater_1_ready && heater_2_ready && heater_3_ready;
    any_heaters_ready = heater_1_ready || heater_2_ready || heater_3_ready;
}

void start_machine() {
    digitalWrite(MACHINE_CONTROL_RELAY_PIN, HIGH);
}

void stop_machine() {
    digitalWrite(MACHINE_CONTROL_RELAY_PIN, LOW);
}

void control_loop() {
    // Updates the physical state of machine parameters
    updateFogMachineState();
    
    // Checking if timer control is activated and all machine requirements are met
    if(controlMode == CONTROL_MODE_NONE) {
        // Stops the automatic and manual control of the machine and resets control variables to undefined
        
        stop_machine();
        controlState = -1;
        controlTimer = -1;
        controlMode = CONTROL_MODE_NONE;

        return;
    } else if (controlMode == CONTROL_MODE_MANUAL) {
        control_manual();
    } else if(controlMode == CONTROL_MODE_AUTO) {
        control_auto();
    }

}


void control_auto() {
    if(controlTimer == -1 || controlState == -1) {
        // Initial automatic control variables
        controlTimer = millis();
        controlState = 0;
    }
    bool machineReady = any_heaters_ready && fluid_not_empty;

    // Machine activation stage
    if (controlState == 1) {
        if (millis() - controlTimer < activationTime) {
            if (machineReady) {
                // Activates machine
                start_machine();
            } else {
                stop_machine();
            }
        } else {
            // Initiates waiting stage and adds to the counter
            controlState = 0;
            controlTimer = millis();
        }
    }

    // Waiting for next activation stage
    if (controlState == 0) {
        if (millis() - controlTimer < waitingTime) {
            // Stops machine
            stop_machine();
        } else {
            // Waits for machine to be ready and only then starts the activation stage
            if (machineReady) {
                // Initiates activation stage
                controlState = 1;
                controlTimer = millis();
            }
        }
    }
}

void control_manual() {
    // Activates machine
    // State validation is already performed at this point
    if (any_heaters_ready && fluid_not_empty) {
        start_machine();
    } else {
        // In the next cycle machine is stopped by the main method
        controlMode == CONTROL_MODE_NONE;
    }
}
