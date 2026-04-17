#include "mobility_fsm.h"
#include "supervisor_fsm.h" /* For Supervisor FSM interaction */
#include "robot_state.h"
#include "encoder_motor.h"
#include "motor_hardware.h"
#include "debug_module.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* Motor Objects */
static EncoderMotorObjectTypeDef mot1, mot2, mot3, mot4;
static EncoderMotorObjectTypeDef *motors[4] = {&mot1, &mot2, &mot3, &mot4};

/* Timing */
static uint32_t last_ctrl_tick = 0;
static uint32_t last_meas_tick = 0;

const char* Mobility_StateToStr(MobilityState_t state) {
    switch(state) {
        case MOB_DISABLED: return "DISABLED";
        case MOB_IDLE:     return "IDLE";
        case MOB_BREAK:    return "BREAK";
        case MOB_MOVING:   return "MOVING";
        case MOB_TESTING:  return "TESTING";
        case MOB_FAULT:    return "FAULT";
        default:           return "UNKNOWN";
    }
}

const char* Mobility_ModeToStr(uint8_t mode) {
    switch(mode) {
        case MOB_MODE_DIRECT:    return "DIR";
        case MOB_MODE_DIFF:      return "DIF";
        case MOB_MODE_ACKERMANN: return "ACK";
        case MOB_MODE_MECANUM:   return "MEC";
        default:                 return "UNK";
    }
}

static MobilityState_t mob_state = MOB_DISABLED;

/* Kinematic targets */
static float target_linear_x = 0.0f;
static float target_angular_z = 0.0f;

void Mobility_Init(void) {
    /* Initialize Motor Objects */
    for(int i=0; i<4; i++) {
        encoder_motor_object_init(motors[i]);
        /* Use default motor type for initialization */
        Motor_Hardware_SetType(motors[i], MOTOR_TYPE_JGB520);
    }

    /* Initialize Hardware */
    Motor_Hardware_Init(motors);
    
    uint32_t now = HAL_GetTick();
    last_ctrl_tick = now;
    last_meas_tick = now;
    mob_state = MOB_DISABLED;
    RobotState_SetMobilityState(mob_state);
    printf("MOBILITY: Initialized (Disabled)\r\n");
}

MobilityState_t Mobility_GetCurrentState(void) {
    return mob_state;
}

void Mobility_SetCommandTarget(float linear_x, float angular_z) {
    target_linear_x = linear_x;
    target_angular_z = angular_z;
}

void Mobility_SetRawMotorPulse(uint8_t id, float pulse) {
    if (id < 4) {
        /* Force state to TESTING */
        mob_state = MOB_TESTING;
        /* Bypass PID by setting pulse directly on the motor hardware */
        motors[id]->set_pulse(motors[id], (int)pulse);
        /* Keep track of it in the motor object but PID will be skipped in ProcessLogic */
        motors[id]->current_pulse = pulse;
        motors[id]->pid_controller.set_point = 0; // Ensure PID doesn't fight

        printf("MOBILITY: Raw Pulse Received - ID=%u, Pulse=%d (Testing Mode)\r\n", id, (int)pulse);
    }
}


/**
 * @brief High-frequency measurement update.
 * Called by Telemetry Task (100Hz) to ensure odometry is always active.
 */
void Mobility_UpdateMeasurements(void) {
    /* 1. Timing Calculation */
    uint32_t now = HAL_GetTick();
    float dt = (float)(now - last_meas_tick) / 1000.0f;
    if (dt <= 0.0f) dt = 0.01f; /* 100Hz fallback */
    last_meas_tick = now;

    /* 2. Hardware Measurement */
    for(int i=0; i<4; i++) {
        int64_t count = Motor_Hardware_GetEncoderCount(i);
        encoder_update(motors[i], dt, count);
    }

    /* 3. Global Telemetry Update */
    RobotState_SetEncoderCounts((int32_t)mot1.counter, (int32_t)mot2.counter, 
                               (int32_t)mot3.counter, (int32_t)mot4.counter);
    
    RobotState_SetMeasuredRPS(mot1.rps, mot2.rps, mot3.rps, mot4.rps);

    /* Calculate average measured speed (Estimation) */
    float actual_vx = (mot1.rps + mot2.rps - mot3.rps - mot4.rps) / 4.0f * (M_PI * JETAUTO_WHEEL_DIAMETER);
    RobotState_SetMeasuredVelocity(actual_vx, 0.0f);

    // /* 4. Periodic Debug Logging */
    // static uint32_t log_counter = 0;
    // if (++log_counter >= 100) { /* Log at ~1Hz from 100Hz task */
    //     log_counter = 0;
    //     int rps1_int = (int)mot1.rps;
    //     int rps1_dec = (int)(fabsf(mot1.rps - rps1_int) * 100);
        
    //     LOG_INFO("MOB_DEBUG", "Enc1: %ld | RPS1: %d.%02d | VX: %d\r\n",
    //              (long)mot1.counter, rps1_int, rps1_dec, (int)(actual_vx * 100));
    // }
}

/**
 * @brief Main logic loop for Mobility Control. Called periodically by its RTOS Task (50Hz).
 */
void Mobility_ProcessLogic(void) {
    SystemState_t master_state = Supervisor_GetCurrentState();

    /* 1. Timing Calculation for PID */
    uint32_t now = HAL_GetTick();
    float dt = (float)(now - last_ctrl_tick) / 1000.0f;
    if (dt <= 0.0f) dt = 0.02f; /* 50Hz fallback */
    last_ctrl_tick = now;

    /* 2. TOP-DOWN Override & Safety */
    if (master_state == STATE_FAULT || master_state == STATE_INIT) {
        if (mob_state != MOB_DISABLED) {
            printf("MOBILITY: Master Fault/Init -> Forcing DISABLED\r\n");
            mob_state = MOB_DISABLED;
            for(int i=0; i<4; i++) encoder_motor_brake(motors[i]);
        }
        return;
    }

    if (master_state == STATE_PAUSED || master_state == STATE_IDLE) {
        if (mob_state == MOB_MOVING || mob_state == MOB_BREAK || mob_state == MOB_IDLE) {
            /* Stop motors immediately (Hard Stop) */
            for(int i=0; i<4; i++) encoder_motor_brake(motors[i]);
            target_linear_x = 0.0f;
            target_angular_z = 0.0f;
        }
        return;
    }

    /* 3. Active Control (PID & FSM) */
    
    /* Pull latest targets from shared RobotState if in an active mode */
    RobotState_GetTargetVelocity(&target_linear_x, &target_angular_z);

    if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
        /* Optional frequent logging - use during debug only */
        // printf("MOBILITY: Target X=%.3f Z=%.3f\r\n", target_linear_x, target_angular_z);
    }

    /* Standard State Machine Logic */
    switch (mob_state) {
        case MOB_DISABLED:
            /* Enable motors if Master is active */
            mob_state = MOB_IDLE;
            printf("MOBILITY: Transitioning to IDLE (Motors Enabled)\r\n");
            break;

        case MOB_IDLE:
            if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
                mob_state = MOB_MOVING;
                printf("MOBILITY: Transitioning to MOVING\r\n");
            }
            break;

        case MOB_BREAK:
            if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
                mob_state = MOB_MOVING;
                printf("MOBILITY: Breaking interrupted, MOVING\r\n");
            } else {
                /* If speed is zero, go to IDLE after a short "purgatory" or directly */
                /* For now, let's keep it in BREAK if supervisor is still telling us to? 
                   Actually, if we got here from MOVING -> cmd=0, we could stay in BREAK until next cmd. */
            }
            break;

        case MOB_MOVING:
            if (target_linear_x == 0.0f && target_angular_z == 0.0f) {
                mob_state = MOB_IDLE;
                /* Explicitly stop motors (Hard Stop) and clear targets on transition to IDLE */
                for(int i=0; i<4; i++) encoder_motor_brake(motors[i]);
                target_linear_x = 0.0f;
                target_angular_z = 0.0f;
                printf("MOBILITY: Transitioning to IDLE (Zero Velocity)\r\n");
            } else {
                /* Execute Mecanum Kinematics */
                float vx = target_linear_x;
                float vy = 0; /* No strafing for now as only linear_x and angular_z are in RobotState */
                float az = target_angular_z;
                float l_plus_w = JETAUTO_WHEELBASE + JETAUTO_SHAFT_LENGTH;
                
                float v1 = vx - vy - l_plus_w * az;
                float v2 = vx + vy - l_plus_w * az;
                float v3 = vx + vy + l_plus_w * az;
                float v4 = vx - vy + l_plus_w * az;

                /* Convert linear velocity (m/s) to RPS: rps = v / (PI * D) */
                float rps_conv = 1.0f / (M_PI * JETAUTO_WHEEL_DIAMETER);
                
                encoder_motor_set_speed(motors[0], v1 * rps_conv);
                encoder_motor_set_speed(motors[1], v2 * rps_conv);
                encoder_motor_set_speed(motors[2], -v3 * rps_conv); 
                encoder_motor_set_speed(motors[3], -v4 * rps_conv); 
            }
            break;

        case MOB_TESTING:
            /* In testing mode, we don't do anything here. 
               Pulses are set directly via Mobility_SetRawMotorPulse.
               We stay in this state until a velocity command (0 or non-zero) 
               or a supervisor event forces us out. */
            if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
                mob_state = MOB_MOVING;
                printf("MOBILITY: Testing interrupted by CmdVel -> MOVING\r\n");
            }
            break;

        case MOB_FAULT:
            /* Hardware failure handled here. 
               Requires external reset or Master FSM reset logic. */
            break;
    }

    /* 4. Execute Motor Control (PID) on updated targets */
    if (mob_state != MOB_TESTING) {
        for(int i=0; i<4; i++) {
            encoder_motor_control(motors[i], dt);
        }
    }
    
    /* Sync local state to global RobotState */
    RobotState_SetMobilityState(mob_state);
}
