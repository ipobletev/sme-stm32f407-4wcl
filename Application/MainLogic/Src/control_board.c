#include "control_board.h"

/**
 * @brief Global instance of the ControlBoard-4wcl device.
 */
ControlBoard_t ControlBoard_4wcl = {
    .current_state = STATE_INIT,
    .heartbeat_count = 0,
    .error_flags = 0 // 0 = No Error
};
