#include "robot_state.h"
#include "FreeRTOS.h"
#include "debug_module.h"
#include "task.h"
#include "main.h"
#include "osal.h"
#include "app_config.h"

#define IS_IN_ISR() ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0)

/**
 * @brief Global instance of the RobotState-4wcl device.
 */
RobotState_t RobotState_4wcl = {
    .Telemetry = {
        .current_state = STATE_SUPERVISOR_INIT,
        .heartbeat_count = 0,
        .error_flags = 0 // 0 = No Error
    },
    .pid_enabled = 0 // Initialized in RobotState_Init
};

/* Actuator Instances */
static EncoderMotorObjectTypeDef motor_instances[4];
EncoderMotorObjectTypeDef *motors[4] = {
    &motor_instances[0], 
    &motor_instances[1], 
    &motor_instances[2], 
    &motor_instances[3]
};

static void RobotState_ConfigCallback(void) {
    LOG_INFO("ROBOT_STATE", "Syncing state from new configuration...\r\n");
    RobotState_4wcl.pid_enabled = (uint8_t)AppConfig->pid_enabled;
    RobotState_4wcl.Commands.target_mobility_mode = (MobilityMode_t)AppConfig->mobility_mode;
}

void RobotState_Init(void) {
    /* Initialize from current configuration shadow */
    RobotState_4wcl.pid_enabled = (uint8_t)AppConfig->pid_enabled;
    RobotState_4wcl.Commands.target_mobility_mode = (MobilityMode_t)AppConfig->mobility_mode;

    /* Register for future updates */
    AppConfig_RegisterCallback(RobotState_ConfigCallback);
}

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

void RobotState_ClearAllErrorFlags(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.error_flags = 0;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.error_flags = 0;
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

uint8_t RobotState_IsAutonomous(void) {
    uint8_t is_auto;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        is_auto = RobotState_4wcl.Telemetry.is_autonomous;
    } else {
        taskENTER_CRITICAL();
        is_auto = RobotState_4wcl.Telemetry.is_autonomous;
        taskEXIT_CRITICAL();
    }
    return is_auto;
}

void RobotState_SetPIDEnabled(uint8_t enabled) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.pid_enabled = enabled;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.pid_enabled = enabled;
        taskEXIT_CRITICAL();
    }
}

uint8_t RobotState_PIDIsEnabled(void) {
    uint8_t enabled;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        enabled = RobotState_4wcl.pid_enabled;
    } else {
        taskENTER_CRITICAL();
        enabled = RobotState_4wcl.pid_enabled;
        taskEXIT_CRITICAL();
    }
    return enabled;
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

void RobotState_UpdateMobilityHeartbeat(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.mobility_heartbeat_tick = osal_get_tick();
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.mobility_heartbeat_tick = osal_get_tick();
        taskEXIT_CRITICAL();
    }
}

uint32_t RobotState_GetMobilityHeartbeat(void) {
    uint32_t tick;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        tick = RobotState_4wcl.mobility_heartbeat_tick;
    } else {
        taskENTER_CRITICAL();
        tick = RobotState_4wcl.mobility_heartbeat_tick;
        taskEXIT_CRITICAL();
    }
    return tick;
}

void RobotState_UpdateArmHeartbeat(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.arm_heartbeat_tick = osal_get_tick();
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.arm_heartbeat_tick = osal_get_tick();
        taskEXIT_CRITICAL();
    }
}

uint32_t RobotState_GetArmHeartbeat(void) {
    uint32_t tick;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        tick = RobotState_4wcl.arm_heartbeat_tick;
    } else {
        taskENTER_CRITICAL();
        tick = RobotState_4wcl.arm_heartbeat_tick;
        taskEXIT_CRITICAL();
    }
    return tick;
}

void RobotState_UpdateSupervisorHeartbeat(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.supervisor_heartbeat_tick = osal_get_tick();
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.supervisor_heartbeat_tick = osal_get_tick();
        taskEXIT_CRITICAL();
    }
}

uint32_t RobotState_GetSupervisorHeartbeat(void) {
    uint32_t tick;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        tick = RobotState_4wcl.supervisor_heartbeat_tick;
    } else {
        taskENTER_CRITICAL();
        tick = RobotState_4wcl.supervisor_heartbeat_tick;
        taskEXIT_CRITICAL();
    }
    return tick;
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

void RobotState_ResetMobilityCommands(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Commands.target_linear_x = 0.0f;
        RobotState_4wcl.Commands.target_angular_z = 0.0f;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Commands.target_linear_x = 0.0f;
        RobotState_4wcl.Commands.target_angular_z = 0.0f;
        taskEXIT_CRITICAL();
    }
}

void RobotState_ResetArmCommands(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Commands.target_arm_j1 = 0.0f;
        RobotState_4wcl.Commands.target_arm_j2 = 0.0f;
        RobotState_4wcl.Commands.target_arm_j3 = 0.0f;
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

void RobotState_GetTargetVelocity(float *linear_x, float *angular_z) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        *linear_x = RobotState_4wcl.Commands.target_linear_x;
        *angular_z = RobotState_4wcl.Commands.target_angular_z;
    } else {
        taskENTER_CRITICAL();
        *linear_x = RobotState_4wcl.Commands.target_linear_x;
        *angular_z = RobotState_4wcl.Commands.target_angular_z;
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

void RobotState_GetTargetArmPose(float *j1, float *j2, float *j3) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        *j1 = RobotState_4wcl.Commands.target_arm_j1;
        *j2 = RobotState_4wcl.Commands.target_arm_j2;
        *j3 = RobotState_4wcl.Commands.target_arm_j3;
    } else {
        taskENTER_CRITICAL();
        *j1 = RobotState_4wcl.Commands.target_arm_j1;
        *j2 = RobotState_4wcl.Commands.target_arm_j2;
        *j3 = RobotState_4wcl.Commands.target_arm_j3;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetTargetMobilityMode(MobilityMode_t mode) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Commands.target_mobility_mode = mode;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Commands.target_mobility_mode = mode;
        taskEXIT_CRITICAL();
    }
}

MobilityMode_t RobotState_GetTargetMobilityMode(void) {
    MobilityMode_t mode;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        mode = RobotState_4wcl.Commands.target_mobility_mode;
    } else {
        taskENTER_CRITICAL();
        mode = RobotState_4wcl.Commands.target_mobility_mode;
        taskEXIT_CRITICAL();
    }
    return mode;
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

void RobotState_SetEncoderCounts(int32_t enc1, int32_t enc2, int32_t enc3, int32_t enc4) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.enc_1 = enc1;
        RobotState_4wcl.Telemetry.enc_2 = enc2;
        RobotState_4wcl.Telemetry.enc_3 = enc3;
        RobotState_4wcl.Telemetry.enc_4 = enc4;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.enc_1 = enc1;
        RobotState_4wcl.Telemetry.enc_2 = enc2;
        RobotState_4wcl.Telemetry.enc_3 = enc3;
        RobotState_4wcl.Telemetry.enc_4 = enc4;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetMeasuredVelocity(float linear_x, float angular_z) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.measured_linear_x = linear_x;
        RobotState_4wcl.Telemetry.measured_angular_z = angular_z;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.measured_linear_x = linear_x;
        RobotState_4wcl.Telemetry.measured_angular_z = angular_z;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetMeasuredSpeed(float s1, float s2, float s3, float s4) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.measured_speed_1 = s1;
        RobotState_4wcl.Telemetry.measured_speed_2 = s2;
        RobotState_4wcl.Telemetry.measured_speed_3 = s3;
        RobotState_4wcl.Telemetry.measured_speed_4 = s4;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.measured_speed_1 = s1;
        RobotState_4wcl.Telemetry.measured_speed_2 = s2;
        RobotState_4wcl.Telemetry.measured_speed_3 = s3;
        RobotState_4wcl.Telemetry.measured_speed_4 = s4;
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetMeasuredMotorDebug(uint8_t motor_id, float target, float measured, float pwm) {
    if (motor_id < 1 || motor_id > 4) return;
    
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        switch(motor_id) {
            case 1: RobotState_4wcl.Telemetry.target_speed_1 = target; RobotState_4wcl.Telemetry.measured_speed_1 = measured; RobotState_4wcl.Telemetry.pwm_output_1 = pwm; break;
            case 2: RobotState_4wcl.Telemetry.target_speed_2 = target; RobotState_4wcl.Telemetry.measured_speed_2 = measured; RobotState_4wcl.Telemetry.pwm_output_2 = pwm; break;
            case 3: RobotState_4wcl.Telemetry.target_speed_3 = target; RobotState_4wcl.Telemetry.measured_speed_3 = measured; RobotState_4wcl.Telemetry.pwm_output_3 = pwm; break;
            case 4: RobotState_4wcl.Telemetry.target_speed_4 = target; RobotState_4wcl.Telemetry.measured_speed_4 = measured; RobotState_4wcl.Telemetry.pwm_output_4 = pwm; break;
        }
    } else {
        taskENTER_CRITICAL();
        switch(motor_id) {
            case 1: RobotState_4wcl.Telemetry.target_speed_1 = target; RobotState_4wcl.Telemetry.measured_speed_1 = measured; RobotState_4wcl.Telemetry.pwm_output_1 = pwm; break;
            case 2: RobotState_4wcl.Telemetry.target_speed_2 = target; RobotState_4wcl.Telemetry.measured_speed_2 = measured; RobotState_4wcl.Telemetry.pwm_output_2 = pwm; break;
            case 3: RobotState_4wcl.Telemetry.target_speed_3 = target; RobotState_4wcl.Telemetry.measured_speed_3 = measured; RobotState_4wcl.Telemetry.pwm_output_3 = pwm; break;
            case 4: RobotState_4wcl.Telemetry.target_speed_4 = target; RobotState_4wcl.Telemetry.measured_speed_4 = measured; RobotState_4wcl.Telemetry.pwm_output_4 = pwm; break;
        }
        taskEXIT_CRITICAL();
    }
}


void RobotState_SetMotorTestCommand(uint8_t id, float value, uint8_t use_velocity) {
    if (id >= 4) return;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        if (use_velocity) {
            RobotState_4wcl.Commands.motor_test[id].velocity = value;
            RobotState_4wcl.Commands.motor_test[id].use_velocity = 1;
        } else {
            RobotState_4wcl.Commands.motor_test[id].pwm = value;
            RobotState_4wcl.Commands.motor_test[id].use_velocity = 0;
        }
    } else {
        taskENTER_CRITICAL();
        if (use_velocity) {
            RobotState_4wcl.Commands.motor_test[id].velocity = value;
            RobotState_4wcl.Commands.motor_test[id].use_velocity = 1;
        } else {
            RobotState_4wcl.Commands.motor_test[id].pwm = value;
            RobotState_4wcl.Commands.motor_test[id].use_velocity = 0;
        }
        taskEXIT_CRITICAL();
    }
}

void RobotState_GetMotorTestCommand(uint8_t id, float *value, uint8_t *use_velocity) {
    if (id >= 4) return;
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        *use_velocity = RobotState_4wcl.Commands.motor_test[id].use_velocity;
        *value = (*use_velocity) ? RobotState_4wcl.Commands.motor_test[id].velocity : RobotState_4wcl.Commands.motor_test[id].pwm;
    } else {
        taskENTER_CRITICAL();
        *use_velocity = RobotState_4wcl.Commands.motor_test[id].use_velocity;
        *value = (*use_velocity) ? RobotState_4wcl.Commands.motor_test[id].velocity : RobotState_4wcl.Commands.motor_test[id].pwm;
        taskEXIT_CRITICAL();
    }
}

void RobotState_ResetTestCommands(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        for (int i = 0; i < 4; i++) {
            RobotState_4wcl.Commands.motor_test[i].pwm = 0.0f;
            RobotState_4wcl.Commands.motor_test[i].velocity = 0.0f;
            RobotState_4wcl.Commands.motor_test[i].use_velocity = 0;
        }
    } else {
        taskENTER_CRITICAL();
        for (int i = 0; i < 4; i++) {
            RobotState_4wcl.Commands.motor_test[i].pwm = 0.0f;
            RobotState_4wcl.Commands.motor_test[i].velocity = 0.0f;
            RobotState_4wcl.Commands.motor_test[i].use_velocity = 0;
        }
        taskEXIT_CRITICAL();
    }
}

void RobotState_SetIMUOrientation(Quaternion q, EulerAngles ea) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED || IS_IN_ISR()) {
        RobotState_4wcl.Telemetry.qw = q.qw;
        RobotState_4wcl.Telemetry.qx = q.qx;
        RobotState_4wcl.Telemetry.qy = q.qy;
        RobotState_4wcl.Telemetry.qz = q.qz;
        RobotState_4wcl.Telemetry.roll = ea.roll;
        RobotState_4wcl.Telemetry.pitch = ea.pitch;
        RobotState_4wcl.Telemetry.yaw = ea.yaw;
    } else {
        taskENTER_CRITICAL();
        RobotState_4wcl.Telemetry.qw = q.qw;
        RobotState_4wcl.Telemetry.qx = q.qx;
        RobotState_4wcl.Telemetry.qy = q.qy;
        RobotState_4wcl.Telemetry.qz = q.qz;
        RobotState_4wcl.Telemetry.roll = ea.roll;
        RobotState_4wcl.Telemetry.pitch = ea.pitch;
        RobotState_4wcl.Telemetry.yaw = ea.yaw;
        taskEXIT_CRITICAL();
    }
}
