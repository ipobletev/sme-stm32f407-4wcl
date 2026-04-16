#include "robot_state.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

#define IS_IN_ISR() ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0)

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
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.error_flags |= flag;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.error_flags |= flag;
        taskEXIT_CRITICAL();
    }
}

void RobotState_ClearErrorFlag(uint64_t flag) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.error_flags &= ~flag;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.error_flags &= ~flag;
        taskEXIT_CRITICAL();
    }
}

uint64_t RobotState_GetErrorFlags(void) {
    uint64_t current_flags;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        current_flags = RobotState_4wcl.Telemetry.error_flags;
    } else {
        taskENTER_CRITICAL();
        current_flags = RobotState_4wcl.Telemetry.error_flags;
        taskEXIT_CRITICAL();
    }
    return current_flags;
}

void RobotState_UpdateSystemState(SystemState_t state) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.current_state = state;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.current_state = state;
        taskEXIT_CRITICAL();
    }
}

SystemState_t RobotState_GetSystemState(void) {
    SystemState_t state;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        state = RobotState_4wcl.Telemetry.current_state;
    } else {
        taskENTER_CRITICAL();
        state = RobotState_4wcl.Telemetry.current_state;
        taskEXIT_CRITICAL();
    }
    return state;
}

void RobotState_IncrementHeartbeat(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.heartbeat_count++;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.heartbeat_count++;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetAutonomous(uint8_t is_auto) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.is_autonomous = is_auto;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.is_autonomous = is_auto;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetMobilityState(MobilityState_t state) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.current_mobility_state = state;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.current_mobility_state = state;
        taskEXIT_CRITICAL();
    }
}

MobilityState_t RobotState_GetMobilityState(void) {
    MobilityState_t state;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        state = RobotState_4wcl.Telemetry.current_mobility_state;
    } else {
        taskENTER_CRITICAL();
        state = RobotState_4wcl.Telemetry.current_mobility_state;
        taskEXIT_CRITICAL();
    }
    return state;
}

void RobotState_SetArmState(ArmState_t state) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.current_arm_state = state;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.current_arm_state = state;
        taskEXIT_CRITICAL();
    }
}

ArmState_t RobotState_GetArmState(void) {
    ArmState_t state;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        state = RobotState_4wcl.Telemetry.current_arm_state;
    } else {
        taskENTER_CRITICAL();
        state = RobotState_4wcl.Telemetry.current_arm_state;
        taskEXIT_CRITICAL();
    }
    return state;
}

void RobotState_FeedWatchdogMobility(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.mobility_watchdog++;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.mobility_watchdog++;
        taskEXIT_CRITICAL();
    }
}

uint8_t RobotState_GetWatchdogMobility(void) {
    uint8_t wdg;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        wdg = RobotState_4wcl.mobility_watchdog;
    } else {
        taskENTER_CRITICAL();
        wdg = RobotState_4wcl.mobility_watchdog;
        taskEXIT_CRITICAL();
    }
    return wdg;
}

void RobotState_FeedWatchdogArm(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.arm_watchdog++;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.arm_watchdog++;
        taskEXIT_CRITICAL();
    }
}

uint8_t RobotState_GetWatchdogArm(void) {
    uint8_t wdg;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        wdg = RobotState_4wcl.arm_watchdog;
    } else {
        taskENTER_CRITICAL();
        wdg = RobotState_4wcl.arm_watchdog;
        taskEXIT_CRITICAL();
    }
    return wdg;
}

void RobotState_SetBatteryVoltage(float voltage) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.battery_voltage = voltage;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.battery_voltage = voltage;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetBatteryCurrent(float current) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.battery_current = current;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.battery_current = current;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetUCTemperature(float temp) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.uc_temperature = temp;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.uc_temperature = temp;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetBoardTemperature(float temp) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.board_temperature = temp;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.board_temperature = temp;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetTargetVelocity(float linear_x, float angular_z) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Commands.target_linear_x = linear_x;
        RobotState_4wcl.Commands.target_angular_z = angular_z;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Commands.target_linear_x = linear_x;
        RobotState_4wcl.Commands.target_angular_z = angular_z;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetTargetArmPose(float j1, float j2, float j3) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Commands.target_arm_j1 = j1;
        RobotState_4wcl.Commands.target_arm_j2 = j2;
        RobotState_4wcl.Commands.target_arm_j3 = j3;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Commands.target_arm_j1 = j1;
        RobotState_4wcl.Commands.target_arm_j2 = j2;
        RobotState_4wcl.Commands.target_arm_j3 = j3;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetTargetMobilityMode(uint8_t mode) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Commands.target_mobility_mode = mode;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Commands.target_mobility_mode = mode;
        taskEXIT_CRITICAL();
    }
}

float RobotState_GetBatteryVoltage(void) {
    float voltage;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        voltage = RobotState_4wcl.Telemetry.battery_voltage;
    } else {
        taskENTER_CRITICAL();
        voltage = RobotState_4wcl.Telemetry.battery_voltage;
        taskEXIT_CRITICAL();
    }
    return voltage;
}

float RobotState_GetUCTemperature(void) {
    float temp;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        temp = RobotState_4wcl.Telemetry.uc_temperature;
    } else {
        taskENTER_CRITICAL();
        temp = RobotState_4wcl.Telemetry.uc_temperature;
        taskEXIT_CRITICAL();
    }
    return temp;
}
