#ifndef __SERIAL_ROS_H
#define __SERIAL_ROS_H

#include <stdint.h>
#include <stdbool.h>
#include "serial_ros_protocol.h"

#define SERIAL_ROS_COMMS_TIMEOUT_MS  1000

/**
 * @brief Initialize the SerialRos module
 */
void SerialRos_Init(void);

/**
 * @brief Check if the PC (ROS) is currently connected
 * @return true if a valid packet was received within the timeout period
 */
bool SerialRos_IsConnected(void);

/**
 * @brief Process an incoming raw byte buffer
 * parses the packet, verifies CRC, and updates global state
 * @param buffer Raw data buffer
 * @param size Buffer size
 */
void SerialRos_ProcessPacket(uint8_t *buffer, uint16_t size);

/**
 * @brief Build and return a telemetry packet buffer
 * @param out_buffer Buffer to store the resulting packet
 * @param max_size Maximum buffer size
 * @return actual size of the built packet
 */
uint16_t SerialRos_BuildTelemetryPacket(uint8_t *out_buffer, uint16_t max_size);

/**
 * @brief Enqueue a message for transmission. 
 * This is thread-safe and can be called from any task.
 * @param topic_id Protocol topic ID
 * @param msg Pointer to the message structure (Topic-specific)
 * @param len Length of the message payload
 * @return true if enqueued successfully
 */
bool SerialRos_EnqueueTx(uint8_t topic_id, void* msg, uint8_t len);

/**
 * @brief Dequeue a processed packet.
 * @param out_packet Pointer to store the dequeued packet
 * @param timeout_ms Timeout for waiting
 * @return true if a packet was retrieved
 */
bool SerialRos_DequeueRx(SerialRos_Packet_t* out_packet, uint32_t timeout_ms);

#endif /* __SERIAL_ROS_H */
