#include "States/arm_state_handlers.h"
#include "arm_fsm_internal.h"
#include "debug_module.h"

void ArmState_Moving_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering MOVING State\r\n");
}

void ArmState_Moving_Run(void) {
    if (target_j1 == 0.0f && target_j2 == 0.0f && target_j3 == 0.0f) {
        FSM_Arm_ProcessEvent(EVENT_ARM_IDLE);
    } else {
        /* 
           Execute IK & Servo Control Here.
           In the future, this would call the trajectory generator.
        */
    }
}

void ArmState_Moving_OnExit(void) {
    LOG_INFO(LOG_TAG, "Trajectory Execution Halted/Finished\r\n");
}
