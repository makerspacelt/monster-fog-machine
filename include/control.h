#ifndef CONTROL
#define CONTROL

void setup_control_module();
void control_loop();

const int CONTROL_MODE_NONE = 0;
const int CONTROL_MODE_MANUAL = 1;
const int CONTROL_MODE_AUTO = 2;

extern int controlMode;

// AUTOMATIC CONTROL SETTINGS
extern float waitingTime;
extern float activationTime; 
extern int activationCountSetting;

// CURRENT MACHINE STATE VARIABLES
extern bool heater_1_ready;
extern bool heater_2_ready;
extern bool heater_3_ready;
extern bool fluid_not_empty;

#endif