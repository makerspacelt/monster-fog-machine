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
extern bool heater1state;
extern bool heater2state;
extern bool heater3state;
extern bool fluidTankState;

#endif