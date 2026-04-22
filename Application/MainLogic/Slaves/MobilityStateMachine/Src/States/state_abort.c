#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"

void MobState_Abort_OnEnter(void) {
    LOG_WARNING(LOG_TAG, "Entering ABORT State (Emergency Stop). Applying active braking.\r\n");
    
    /* Ensure all targets are zero but keep PID active for deceleration */
    for(int i=0; i<4; i++) {
        encoder_motor_set_speed(motors[i], 0.0f);
    }
}

void MobState_Abort_Run(void) {
    /* Abort state runs the PID loop for all motors until they are stopped.
       Once stopped, encoder_motor_control handles the shift to PWM 0 via encoder_motor_brake. */
    for(int i=0; i<4; i++) {
        encoder_motor_control(motors[i], 0.02f);
    }
}

void MobState_Abort_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting ABORT State.\r\n");
}
