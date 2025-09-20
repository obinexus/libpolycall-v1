#ifndef POLYCALL_BRIDGE_H
#define POLYCALL_BRIDGE_H

/**
 * LibPolyCall Trial v1 Java Bridge
 * Native interface for Java PolyCall binding
 */

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for polycall.exe FFI
int polycall_init(const char* host, int port);
int polycall_connect(void);
int polycall_authenticate(const char* credentials);
int polycall_execute(const char* operation, const char* params, char** result);
void polycall_disconnect(void);
void polycall_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // POLYCALL_BRIDGE_H
