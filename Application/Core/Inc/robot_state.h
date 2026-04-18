#ifndef __ROBOT_STATE_H
#define __ROBOT_STATE_H

#include "supervisor_fsm.h"
#include "error_codes.h"
#include "mobility_fsm.h"
#include "arm_fsm.h"
#include <stdint.h>

/**
 * @brief Main structure for RobotState-4wcl
 * Organized to separate ROS Commands (Rx) from Telemetry (Tx)
 */
typedef struct {
    /* 1. COMMANDS FROM ROS (Rx) */
    struct {
        /* Mobility Commands */
        float target_linear_x;
        float target_angular_z;
        uint8_t target_mobility_mode;          /* 0: Direct, 1: DiffDrive, 2: Ackermann, 3: Mecanum */

        /* Arm Commands */
        float target_arm_j1;
        float target_arm_j2;
        float target_arm_j3;
    } Commands;

    /* 2. TELEMETRY TO ROS (Tx) */
    struct {
        /* System Core */

        /* FSM States */
        SystemState_t current_state;            /* Current state of the system (Supervisor FSM) */
        MobilityState_t current_mobility_state; /* Current state of the mobility subsystem */
        ArmState_t current_arm_state;           /* Current state of the arm subsystem */

        /* Variables */
        uint64_t error_flags;           /* Bitmask for hardware and software errors */
        uint32_t heartbeat_count;       /* General system heartbeat */
        float uc_temperature;           /* Microcontroller temperature */
        float board_temperature;        /* Board temperature */
        float battery_voltage;          /* Battery voltage */
        float battery_current;          /* Total system current consumption */
        uint8_t is_autonomous;          /* 0: Manual, 1: Auto */

        /* Mobility Feedback */
        uint8_t current_mobility_mode;  /* Real active mode: 0: Direct, 1: DiffDrive, 2: Ackermann, 3: Mecanum */
        float measured_linear_x;
        float measured_angular_z;
        int32_t enc_1;
        int32_t enc_2;
        int32_t enc_3;
        int32_t enc_4;
        float measured_rps_1;
        float measured_rps_2;
        float measured_rps_3;
        float measured_rps_4;

        /* Arm Feedback */
        float arm_j1_pos;
        float arm_j2_pos;
        float arm_j3_pos;
        float arm_j1_vel;
        float arm_j2_vel;
        float arm_j3_vel;
        float arm_j1_eff;
        float arm_j2_eff;
        float arm_j3_eff;

        /* IMU 9-DOF */
        float roll;
        float pitch;
        float yaw;
        float gyro_x;
        float gyro_y;
        float gyro_z;
        float accel_x;                  /* Linear Acceleration X */
        float accel_y;                  /* Linear Acceleration Y */
        float accel_z;                  /* Linear Acceleration Z */

    } Telemetry;

    /* 3. Internal Controls (Not sent to ROS) */
    uint32_t mobility_heartbeat_tick;   /* Last tick from mobility task (Passive) */
    uint32_t arm_heartbeat_tick;        /* Last tick from arm task (Passive) */
    uint32_t supervisor_heartbeat_tick; /* Last tick from supervisor task */
    

} RobotState_t;
extern RobotState_t RobotState_4wcl;

/* Thread-safe API for Shared Global Variables */
void RobotState_SetErrorFlag(uint64_t flag);
void RobotState_ClearErrorFlag(uint64_t flag);
uint64_t RobotState_GetErrorFlags(void);

/* System Core Getters / Setters */
void RobotState_UpdateSystemState(SystemState_t state);
SystemState_t RobotState_GetSystemState(void);
void RobotState_IncrementHeartbeat(void);
uint32_t RobotState_GetHeartbeat(void);
void RobotState_SetAutonomous(uint8_t is_auto);
uint8_t RobotState_IsAutonomous(void);

/* Subsystem Getters / Setters */
void RobotState_SetMobilityState(MobilityState_t state);
MobilityState_t RobotState_GetMobilityState(void);
void RobotState_SetArmState(ArmState_t state);
ArmState_t RobotState_GetArmState(void);

/* Telemetry Setters */
void RobotState_SetBatteryVoltage(float voltage);
void RobotState_SetBatteryCurrent(float current);
void RobotState_SetUCTemperature(float temp);
void RobotState_SetBoardTemperature(float temp);
float RobotState_GetBatteryVoltage(void);
float RobotState_GetUCTemperature(void);

/* Heartbeat / Watchdog API */
void RobotState_UpdateMobilityHeartbeat(void);
uint32_t RobotState_GetMobilityHeartbeat(void);
void RobotState_UpdateArmHeartbeat(void);
uint32_t RobotState_GetArmHeartbeat(void);
void RobotState_UpdateSupervisorHeartbeat(void);
uint32_t RobotState_GetSupervisorHeartbeat(void);


/* Command Setters (Rx from ROS) */
void RobotState_SetTargetVelocity(float linear_x, float angular_z);
void RobotState_GetTargetVelocity(float *linear_x, float *angular_z);
void RobotState_SetTargetArmPose(float j1, float j2, float j3);
void RobotState_GetTargetArmPose(float *j1, float *j2, float *j3);
void RobotState_SetTargetMobilityMode(uint8_t mode);
uint8_t RobotState_GetTargetMobilityMode(void);

/* Mobility Feedback Setters */
void RobotState_SetEncoderCounts(int32_t enc1, int32_t enc2, int32_t enc3, int32_t enc4);
void RobotState_SetMeasuredVelocity(float linear_x, float angular_z);
void RobotState_SetMeasuredRPS(float rps1, float rps2, float rps3, float rps4);

#endif /* __ROBOT_STATE_H */
