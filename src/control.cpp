#include <Arduino.h>
#include <control.h>

const int HEATER_1_VOLTAGE_IND_PIN = 33;
const int HEATER_2_VOLTAGE_IND_PIN = 25;
const int HEATER_3_VOLTAGE_IND_PIN = 26;
const int FLUID_TANK_FLOAT_PIN = 27;
const int MACHINE_CONTROL_RELAY_PIN = 4;

int controlMode = CONTROL_MODE_NONE;

// AUTOMATIC CONTROL SETTINGS
float waitingTime = 5 * 60 * 1000;
float activationTime = 10 * 1000; 
int activationCountSetting = 10;

// CURRENT MACHINE STATE VARIABLES
bool heater1state = false;
bool heater2state = false;
bool heater3state = false;
bool fluidTankState = false;

long controlTimer;

bool machineReady = false;
int controlState = -1; // 0 -> machine is stopped, waiting for next activation (waiting stage), 1 -> machine is started (activation stage)
int activationCounter = 0;

void setup_control_module() {
    pinMode(MACHINE_CONTROL_RELAY_PIN, OUTPUT);
    pinMode(HEATER_1_VOLTAGE_IND_PIN, INPUT);
    pinMode(HEATER_2_VOLTAGE_IND_PIN, INPUT);
    pinMode(HEATER_3_VOLTAGE_IND_PIN, INPUT);
    pinMode(FLUID_TANK_FLOAT_PIN, INPUT);
}

void updateFogMachineState() {
    heater1state = digitalRead(HEATER_1_VOLTAGE_IND_PIN) == 1;
    heater2state = digitalRead(HEATER_2_VOLTAGE_IND_PIN) == 1;
    heater3state = digitalRead(HEATER_3_VOLTAGE_IND_PIN) == 1;
    fluidTankState = digitalRead(MACHINE_CONTROL_RELAY_PIN) == 1;

    machineReady = heater1state && heater2state && heater3state && fluidTankState;

    machineReady = true; //REMOVE AFTER DEBUG
    //fluidTankState = true;
    //heater1state = true;
    //heater2state = true;
    //heater3state = true;
}

void start_machine() {
    digitalWrite(MACHINE_CONTROL_RELAY_PIN, HIGH);
    //Serial.println("STARTED");
}

void stop_machine() {
    digitalWrite(MACHINE_CONTROL_RELAY_PIN, LOW);
    //Serial.println("STOPPED");
}

void control_loop() {
    // Updates the physical state of machine parameters
    updateFogMachineState();
    
    // Checking if timer control is activated and all machine requirements are met
    if(!machineReady || controlMode == CONTROL_MODE_NONE) {
        // Stops the automatic and manual control of the machine and resets control variables to undefined
        
        stop_machine();
        controlState = -1;
        controlTimer = -1;
        activationCounter = 0;
        controlMode = CONTROL_MODE_NONE;

        return;
    }

    if (controlMode == CONTROL_MODE_AUTO) {
        if(controlTimer == -1 || controlState == -1) {
            // Initial automatic control variables
            controlTimer = millis();
            controlState = 0;
        }

        // Machine activation stage
        if (controlState == 1) {
            if (millis() - controlTimer < activationTime) {
                // Activates machine
                start_machine();
            } else {
                // Initiates waiting stage and adds to the counter
                controlState = 0;
                controlTimer = millis();
                stop_machine();
                activationCounter = activationCounter + 1;
            }
        }

        // Waiting for next activation stage
        if (controlState == 0) {
            if (millis() - controlTimer < waitingTime) {
                // Stops machine
                stop_machine();
            } else {
                // Initiates activation stage
                controlState = 1;
                controlTimer = millis();
                start_machine();
            }
        }

        // COUNTER IS NOT IMPLEMENTED IN UI
        // if (activationCounter > activationCountSetting) {
        //     // Finishes the automatic control
        //     // Variables are resetted in the next control loop
        //     controlMode = CONTROL_MODE_NONE;
        // }
    } if (controlMode == CONTROL_MODE_MANUAL) {
        // Activates machine
        // State validation is already performed at this point
        start_machine();
    }
}
