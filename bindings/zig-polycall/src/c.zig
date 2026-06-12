//! c.zig — raw extern declarations for libpolycall-v2
//! Every public symbol from the C headers is declared here.
//! Higher-level modules (core.zig, protocol.zig, …) import from this file;
//! consumers should prefer those wrappers over calling these directly.

const types = @import("types.zig");

// ---------------------------------------------------------------------------
// Opaque handle for the core context
// ---------------------------------------------------------------------------
/// Opaque polycall context — never dereference.
pub const Context = opaque {};

// ---------------------------------------------------------------------------
// State machine opaque/extern structs
// ---------------------------------------------------------------------------

/// State action callback: fn(*Context) callconv(.C) void
pub const StateAction = ?*const fn (?*Context) callconv(.C) void;

/// Guard condition callback: fn(*State, *State) callconv(.C) bool
pub const GuardCondition = ?*const fn (*const State, *const State) callconv(.C) bool;

/// Integrity check callback: fn(*State) callconv(.C) bool
pub const StateIntegrityCheck = ?*const fn (*const State) callconv(.C) bool;

pub const State = extern struct {
    name: [types.max_name_length]u8,
    on_enter: StateAction,
    on_exit: StateAction,
    is_final: bool,
    id: c_uint,
    checksum: u32,
    timestamp: u64,
    version: c_uint,
    is_locked: bool,
};

pub const Transition = extern struct {
    name: [types.max_name_length]u8,
    from_state: c_uint,
    to_state: c_uint,
    action: StateAction,
    is_valid: bool,
    guard_condition: GuardCondition,
    guard_checksum: u32,
};

pub const SmDiagnostics = extern struct {
    failed_transitions: c_uint,
    integrity_violations: c_uint,
    last_verification: u64,
};

pub const StateMachine = extern struct {
    states: [types.max_states]State,
    transitions: [types.max_transitions]Transition,
    current_state: c_uint,
    num_states: c_uint,
    num_transitions: c_uint,
    ctx: ?*Context,
    is_initialized: bool,
    integrity_check: StateIntegrityCheck,
    machine_checksum: u32,
    diagnostics: SmDiagnostics,
};

pub const StateSnapshot = extern struct {
    state: State,
    timestamp: u64,
    checksum: u32,
};

pub const StateDiagnostics = extern struct {
    state_id: c_uint,
    creation_time: u64,
    last_modified: u64,
    transition_count: c_uint,
    integrity_check_count: c_uint,
    is_locked: bool,
    current_checksum: u32,
};

// ---------------------------------------------------------------------------
// Network extern structs
// ---------------------------------------------------------------------------

pub const PhantomDaemon = opaque {};

pub const ClientState = extern struct {
    lock: PthreadMutex,
    is_active: bool,
    socket_fd: c_int,
    addr: SockaddrIn,
};

pub const NetworkEndpoint = extern struct {
    lock: PthreadMutex,
    address: [16]u8, // INET_ADDRSTRLEN
    port: u16,
    protocol: types.NetworkProtocol,
    role: types.NetworkRole,
    socket_fd: c_int,
    addr: SockaddrIn,
    phantom: ?*PhantomDaemon,
    user_data: ?*anyopaque,
};

pub const NetworkPacket = extern struct {
    data: ?*anyopaque,
    size: usize,
    flags: u32,
};

pub const NetworkProgram = extern struct {
    endpoints: ?[*]NetworkEndpoint,
    count: usize,
    clients: [10]ClientState, // NET_MAX_CLIENTS
    clients_lock: PthreadMutex,
    running: bool,
    handlers: NetworkHandlers,
    phantom: ?*PhantomDaemon,
};

pub const NetworkHandlers = extern struct {
    on_receive: ?*const fn (*NetworkEndpoint, *NetworkPacket) callconv(.C) void,
    on_connect: ?*const fn (*NetworkEndpoint) callconv(.C) void,
    on_disconnect: ?*const fn (*NetworkEndpoint) callconv(.C) void,
};

// POSIX primitives — forward-declared for ABI layout.
// The actual mutex is opaque; we only care about its size (40 bytes on Linux).
pub const PthreadMutex = extern struct {
    data: [40]u8 align(8) = [_]u8{0} ** 40,
};
pub const SockaddrIn = extern struct {
    data: [16]u8 = [_]u8{0} ** 16,
};

// ---------------------------------------------------------------------------
// Protocol extern structs
// ---------------------------------------------------------------------------

pub const ProtocolCallbacks = extern struct {
    on_handshake: ?*const fn (*ProtocolContext) callconv(.C) void,
    on_auth_request: ?*const fn (*ProtocolContext, ?[*:0]const u8) callconv(.C) void,
    on_command: ?*const fn (*ProtocolContext, ?[*:0]const u8, usize) callconv(.C) void,
    on_error: ?*const fn (*ProtocolContext, ?[*:0]const u8) callconv(.C) void,
    on_state_change: ?*const fn (*ProtocolContext, types.ProtocolState, types.ProtocolState) callconv(.C) void,
};

pub const ProtocolConfig = extern struct {
    callbacks: ProtocolCallbacks,
    flags: u16,
    max_message_size: usize,
    timeout_ms: u32,
    user_data: ?*anyopaque,
};

pub const ProtocolContext = extern struct {
    pc_ctx: ?*Context,
    state_machine: ?*StateMachine,
    endpoint: ?*NetworkEndpoint,
    next_sequence: u32,
    state: types.ProtocolState,
    user_data: ?*anyopaque,
};

// ---------------------------------------------------------------------------
// Micro-service extern structs
// ---------------------------------------------------------------------------

pub const MicroService = extern struct {
    service_name: ?[*:0]const u8,
    port: u16,
    context: ?*anyopaque,
};

// ---------------------------------------------------------------------------
// === CORE API ===
// ---------------------------------------------------------------------------
pub extern fn polycall_init(ctx: *?*Context) types.Status;
pub extern fn polycall_init_with_config(ctx: *?*Context, config: *const types.Config) types.Status;
pub extern fn polycall_cleanup(ctx: *?*Context) void;
pub extern fn polycall_get_version() ?[*:0]const u8;
pub extern fn polycall_get_last_error(ctx: ?*Context) ?[*:0]const u8;

pub extern fn polycall_set_flag(ctx: ?*Context, flag: c_uint) types.Status;
pub extern fn polycall_clear_flag(ctx: ?*Context, flag: c_uint) types.Status;
pub extern fn polycall_get_user_data(ctx: ?*Context) ?*anyopaque;
pub extern fn polycall_set_user_data(ctx: ?*Context, data: ?*anyopaque) types.Status;
pub extern fn polycall_is_initialized(ctx: ?*Context) bool;

// ---------------------------------------------------------------------------
// === STATE MACHINE API ===
// ---------------------------------------------------------------------------
pub extern fn polycall_sm_create_with_integrity(
    ctx: ?*Context,
    sm: **StateMachine,
    integrity_check: StateIntegrityCheck,
) types.SmStatus;

pub extern fn polycall_sm_add_state(
    sm: *StateMachine,
    name: [*:0]const u8,
    on_enter: StateAction,
    on_exit: StateAction,
    is_final: bool,
) types.SmStatus;

pub extern fn polycall_sm_add_transition(
    sm: *StateMachine,
    name: [*:0]const u8,
    from_state: c_uint,
    to_state: c_uint,
    action: StateAction,
    guard_condition: GuardCondition,
) types.SmStatus;

pub extern fn polycall_sm_execute_transition(
    sm: *StateMachine,
    transition_name: [*:0]const u8,
) types.SmStatus;

pub extern fn polycall_sm_verify_state_integrity(
    sm: *StateMachine,
    state_id: c_uint,
) types.SmStatus;

pub extern fn polycall_sm_lock_state(sm: *StateMachine, state_id: c_uint) types.SmStatus;
pub extern fn polycall_sm_unlock_state(sm: *StateMachine, state_id: c_uint) types.SmStatus;

pub extern fn polycall_sm_get_state_version(
    sm: *const StateMachine,
    state_id: c_uint,
    version: *c_uint,
) types.SmStatus;

pub extern fn polycall_sm_create_state_snapshot(
    sm: *const StateMachine,
    state_id: c_uint,
    snapshot: *StateSnapshot,
) types.SmStatus;

pub extern fn polycall_sm_restore_state_from_snapshot(
    sm: *StateMachine,
    snapshot: *const StateSnapshot,
) types.SmStatus;

pub extern fn polycall_sm_get_state_diagnostics(
    sm: *const StateMachine,
    state_id: c_uint,
    diagnostics: *StateDiagnostics,
) types.SmStatus;

pub extern fn polycall_sm_destroy(sm: *StateMachine) void;

// ---------------------------------------------------------------------------
// === PROTOCOL API ===
// ---------------------------------------------------------------------------
pub extern fn polycall_protocol_init(
    ctx: *ProtocolContext,
    pc_ctx: ?*Context,
    endpoint: *NetworkEndpoint,
    config: *const ProtocolConfig,
) bool;

pub extern fn polycall_protocol_cleanup(ctx: *ProtocolContext) void;

pub extern fn polycall_protocol_send(
    ctx: *ProtocolContext,
    @"type": types.MessageType,
    payload: ?*const anyopaque,
    payload_length: usize,
    flags: u16,
) bool;

pub extern fn polycall_protocol_process(
    ctx: *ProtocolContext,
    data: ?*const anyopaque,
    length: usize,
) bool;

pub extern fn polycall_protocol_update(ctx: *ProtocolContext) void;
pub extern fn polycall_protocol_get_state(ctx: *const ProtocolContext) types.ProtocolState;

pub extern fn polycall_protocol_can_transition(
    ctx: *const ProtocolContext,
    target_state: types.ProtocolState,
) bool;

pub extern fn polycall_protocol_start_handshake(ctx: *ProtocolContext) bool;
pub extern fn polycall_protocol_complete_handshake(ctx: *ProtocolContext) bool;

pub extern fn polycall_protocol_authenticate(
    ctx: *ProtocolContext,
    credentials: [*:0]const u8,
    credentials_length: usize,
) bool;

pub extern fn polycall_protocol_get_error(ctx: *const ProtocolContext) ?[*:0]const u8;
pub extern fn polycall_protocol_set_error(ctx: *ProtocolContext, err: [*:0]const u8) void;

pub extern fn polycall_protocol_calculate_checksum(data: *const anyopaque, length: usize) u32;
pub extern fn polycall_protocol_verify_checksum(
    header: *const types.MessageHeader,
    payload: *const anyopaque,
    payload_length: usize,
) bool;

pub extern fn polycall_protocol_version_compatible(remote_version: u8) bool;

pub extern fn polycall_protocol_create_header(
    @"type": types.MessageType,
    payload_length: usize,
    flags: u16,
) types.MessageHeader;

pub extern fn polycall_protocol_is_connected(ctx: *const ProtocolContext) bool;
pub extern fn polycall_protocol_is_authenticated(ctx: *const ProtocolContext) bool;
pub extern fn polycall_protocol_is_error(ctx: *const ProtocolContext) bool;

// ---------------------------------------------------------------------------
// === NETWORK API ===
// ---------------------------------------------------------------------------
pub extern fn net_init(endpoint: *NetworkEndpoint) bool;
pub extern fn net_close(endpoint: *NetworkEndpoint) void;
pub extern fn net_send(endpoint: *NetworkEndpoint, packet: *NetworkPacket) isize;
pub extern fn net_receive(endpoint: *NetworkEndpoint, packet: *NetworkPacket) isize;
pub extern fn net_run(program: *NetworkProgram) void;

pub extern fn net_is_port_in_use(port: u16) bool;
pub extern fn net_release_port(port: u16) bool;
pub extern fn net_init_client_state(state: *ClientState) void;
pub extern fn net_cleanup_client_state(state: *ClientState) void;
pub extern fn net_init_program(program: *NetworkProgram) void;
pub extern fn net_cleanup_program(program: *NetworkProgram) void;

// ---------------------------------------------------------------------------
// === MICRO SERVICE API ===
// ---------------------------------------------------------------------------
pub extern fn polycall_micro_init(service: *MicroService) c_int;
pub extern fn polycall_micro_start(service: *MicroService) c_int;
pub extern fn polycall_micro_stop(service: *MicroService) c_int;
pub extern fn polycall_micro_cleanup(service: *MicroService) void;

pub extern fn polycall_banking_init() c_int;
pub extern fn polycall_banking_process(request: ?*const anyopaque, response: ?*anyopaque) c_int;
pub extern fn polycall_banking_cleanup() void;
