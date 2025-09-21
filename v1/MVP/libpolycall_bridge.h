// libpolycall_bridge.h
typedef struct {
    void* (*marshal)(void* native_data);
    void* (*unmarshal)(void* wire_data);
    const char* source_lang;
    const char* target_lang;
} polycall_bridge_t;

// Register bidirectional bridges
int polycall_register_bridge(const char* name, polycall_bridge_t* bridge);
