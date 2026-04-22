#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"
#include <math.h>

void MobState_Break_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering BREAK State (Active Braking). Commanding 0.0 m/s.\r\n");
    
    /* Instead of hard braking, we set targets to zero to allow controlled deceleration */
    for(int i=0; i<4; i++) {
        encoder_motor_set_speed(motors[i], 0.0f);
    }
}

void MobState_Break_Run(void) {
    bool all_stopped = true;
    
    /* Run PID loop for all motors and check if they have stopped */
    for(int i=0; i<4; i++) {
        encoder_motor_control(motors[i], 0.02f);
        
        // Use a small threshold to consider the system stopped
        if (fabs(motors[i]->measured_speed) > 0.02f) {
            all_stopped = false;
        }
    }
    
    /* If targets appear, transition back to MOVING */
    if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
        FSM_Mobility_ProcessEvent(EVENT_MOB_MOVING);
        return;
    }

    /* If all wheels are stopped, we can safely transition to IDLE (Power cut) */
    if (all_stopped) {
        LOG_INFO(LOG_TAG, "Braking complete. All wheels stopped. Transitioning to IDLE.\r\n");
        FSM_Mobility_ProcessEvent(EVENT_MOB_IDLE);
    }
}

void MobState_Break_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting BREAK State.\r\n");
}
