#ifndef POLYCALL_PROTOCOL_H
#define POLYCALL_PROTOCOL_H

#include "polycall.h"
#include <stdint.h>

typedef enum {
    POLYCALL_MSG_HANDSHAKE = 0x01,
    POLYCALL_MSG_COMMAND = 0x03,
    POLYCALL_MSG_RESPONSE = 0x04
} polycall_message_type_t;

typedef struct {
    uint8_t version;
    uint8_t type;
    uint32_t sequence;
    uint32_t payload_length;
} polycall_message_header_t;

int polycall_protocol_init(void);
void polycall_protocol_cleanup(void);

#endif // POLYCALL_PROTOCOL_H
