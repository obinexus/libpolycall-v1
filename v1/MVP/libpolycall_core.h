// libpolycall_core.h - Pure transport, NO business logic
typedef struct {
    void* data;
    size_t size;
    char* service_id;
    char* action;
} polycall_message_t;

// Pure dispatch - no decisions
int polycall_dispatch(const polycall_message_t* msg);
int polycall_receive(polycall_message_t** msg);

// Connection management - no logic
void* polycall_connect(const char* service, uint16_t port);
void polycall_disconnect(void* handle);
