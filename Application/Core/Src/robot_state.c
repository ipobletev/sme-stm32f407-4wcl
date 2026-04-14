#include "robot_state.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Global instance of the RobotState-4wcl device.
 */
RobotState_t RobotState_4wcl = {
    .Telemetry = {
        .current_state = STATE_INIT,
        .heartbeat_count = 0,
        .error_flags = 0 // 0 = No Error
    }
};

void RobotState_SetErrorFlag(uint64_t flag) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.Telemetry.error_flags |= flag;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.error_flags |= flag;
        taskEXIT_CRITICAL();
    }
}

void RobotState_ClearErrorFlag(uint64_t flag) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.Telemetry.error_flags &= ~flag;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.error_flags &= ~flag;
        taskEXIT_CRITICAL();
    }
}

uint64_t RobotState_GetErrorFlags(void) {
    uint64_t current_flags;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        current_flags = RobotState_4wcl.Telemetry.error_flags;
    } else {
        taskENTER_CRITICAL();
        current_flags = RobotState_4wcl.Telemetry.error_flags;
        taskEXIT_CRITICAL();
    }
    return current_flags;
}

void RobotState_UpdateSystemState(SystemState_t state) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.Telemetry.current_state = state;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.current_state = state;
        taskEXIT_CRITICAL();
    }
}

SystemState_t RobotState_GetSystemState(void) {
    SystemState_t state;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        state = RobotState_4wcl.Telemetry.current_state;
    } else {
        taskENTER_CRITICAL();
        state = RobotState_4wcl.Telemetry.current_state;
        taskEXIT_CRITICAL();
    }
    return state;
}

void RobotState_IncrementHeartbeat(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.Telemetry.heartbeat_count++;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.heartbeat_count++;
        taskEXIT_CRITICAL();
    }
}

uint32_t RobotState_GetHeartbeat(void) {
    uint32_t count;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        count = RobotState_4wcl.Telemetry.heartbeat_count;
    } else {
        taskENTER_CRITICAL();
        count = RobotState_4wcl.Telemetry.heartbeat_count;
        taskEXIT_CRITICAL();
    }
    return count;
}

void RobotState_SetAutonomous(uint8_t is_auto) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.Telemetry.is_autonomous = is_auto;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.is_autonomous = is_auto;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetMobilityState(MobilityState_t state) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.Telemetry.current_mobility_state = state;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.current_mobility_state = state;
        taskEXIT_CRITICAL();
    }
}

MobilityState_t RobotState_GetMobilityState(void) {
    MobilityState_t state;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        state = RobotState_4wcl.Telemetry.current_mobility_state;
    } else {
        taskENTER_CRITICAL();
        state = RobotState_4wcl.Telemetry.current_mobility_state;
        taskEXIT_CRITICAL();
    }
    return state;
}

void RobotState_SetArmState(ArmState_t state) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.Telemetry.current_arm_state = state;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.current_arm_state = state;
        taskEXIT_CRITICAL();
    }
}

ArmState_t RobotState_GetArmState(void) {
    ArmState_t state;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        state = RobotState_4wcl.Telemetry.current_arm_state;
    } else {
        taskENTER_CRITICAL();
        state = RobotState_4wcl.Telemetry.current_arm_state;
        taskEXIT_CRITICAL();
    }
    return state;
}

void RobotState_FeedWatchdogMobility(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.mobility_watchdog++;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.mobility_watchdog++;
        taskEXIT_CRITICAL();
    }
}

uint8_t RobotState_GetWatchdogMobility(void) {
    uint8_t wdg;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        wdg = RobotState_4wcl.mobility_watchdog;
    } else {
        taskENTER_CRITICAL();
        wdg = RobotState_4wcl.mobility_watchdog;
        taskEXIT_CRITICAL();
    }
    return wdg;
}

void RobotState_FeedWatchdogArm(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        RobotState_4wcl.arm_watchdog++;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.arm_watchdog++;
        taskEXIT_CRITICAL();
    }
}

uint8_t RobotState_GetWatchdogArm(void) {
    uint8_t wdg;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        wdg = RobotState_4wcl.arm_watchdog;
    } else {
        taskENTER_CRITICAL();
        wdg = RobotState_4wcl.arm_watchdog;
        taskEXIT_CRITICAL();
    }
    return wdg;
}
