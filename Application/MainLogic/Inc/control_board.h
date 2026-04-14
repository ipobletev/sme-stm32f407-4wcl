#ifndef __CONTROL_BOARD_H
#define __CONTROL_BOARD_H

#include "app_state_machine.h"
#include "error_codes.h"

/**
 * @brief Main structure for ControlBoard-4wcl
 */
typedef struct {
    SystemState_t current_state;
    uint32_t heartbeat_count;
    /* Add other principal variables here */
    uint64_t error_flags;
} ControlBoard_t;


extern ControlBoard_t ControlBoard_4wcl;

#endif /* __CONTROL_BOARD_H */
