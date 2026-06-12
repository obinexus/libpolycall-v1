//! types.zig — OBINexus libpolycall-v2 Zig type system
//! Mirrors: core/include/libpolycall/core/types.h
//!          core/include/polycall_token.h
//!          core/include/libpolycall/core/polycall_state_machine.h
//! All types are ABI-compatible with the C headers.

const std = @import("std");

// ---------------------------------------------------------------------------
// Status codes  (polycall_status_t)
// ---------------------------------------------------------------------------
pub const Status = enum(c_int) {
    success = 0,
    err_invalid_parameters = -1,
    err_out_of_memory = -2,
    err_not_initialized = -3,
    err_already_initialized = -4,
    err_network = -5,
    err_timeout = -6,
    err_unknown = -999,
    _,

    pub fn toError(self: Status) Error!void {
        return switch (self) {
            .success => {},
            .err_invalid_parameters => Error.InvalidParameters,
            .err_out_of_memory => Error.OutOfMemory,
            .err_not_initialized => Error.NotInitialized,
            .err_already_initialized => Error.AlreadyInitialized,
            .err_network => Error.Network,
            .err_timeout => Error.Timeout,
            else => Error.Unknown,
        };
    }
};

/// Unified error set for all polycall operations.
pub const Error = error{
    InvalidParameters,
    OutOfMemory,
    NotInitialized,
    AlreadyInitialized,
    Network,
    Timeout,
    Unknown,
    // State machine extras
    InvalidState,
    InvalidTransition,
    MaxStatesReached,
    MaxTransitionsReached,
    InvalidContext,
    IntegrityCheckFailed,
    StateLocked,
    VersionMismatch,
};

// ---------------------------------------------------------------------------
// State machine status  (polycall_sm_status_t)
// ---------------------------------------------------------------------------
pub const SmStatus = enum(c_int) {
    success = 0,
    err_invalid_state = 1,
    err_invalid_transition = 2,
    err_max_states_reached = 3,
    err_max_transitions_reached = 4,
    err_invalid_context = 5,
    err_not_initialized = 6,
    err_integrity_check_failed = 7,
    err_state_locked = 8,
    err_version_mismatch = 9,
    _,

    pub fn toError(self: SmStatus) Error!void {
        return switch (self) {
            .success => {},
            .err_invalid_state => Error.InvalidState,
            .err_invalid_transition => Error.InvalidTransition,
            .err_max_states_reached => Error.MaxStatesReached,
            .err_max_transitions_reached => Error.MaxTransitionsReached,
            .err_invalid_context => Error.InvalidContext,
            .err_not_initialized => Error.NotInitialized,
            .err_integrity_check_failed => Error.IntegrityCheckFailed,
            .err_state_locked => Error.StateLocked,
            .err_version_mismatch => Error.VersionMismatch,
            else => Error.Unknown,
        };
    }
};

// ---------------------------------------------------------------------------
// Module init flags  (POLYCALL_FLAG_*)
// ---------------------------------------------------------------------------
pub const Flags = struct {
    pub const async_mode: c_uint = 0x01;
    pub const no_threads: c_uint = 0x02;
    pub const debug: c_uint = 0x04;
    pub const zero_copy: c_uint = 0x08;
    pub const hotwire: c_uint = 0x10;
};

// ---------------------------------------------------------------------------
// Configuration  (polycall_config_t)
// ---------------------------------------------------------------------------
pub const Config = extern struct {
    memory_pool_size: usize = 0,
    flags: c_uint = 0,
    user_data: ?*anyopaque = null,
    port: u16 = 0,
    bind_address: ?[*:0]const u8 = null,
};

// ---------------------------------------------------------------------------
// Network error codes  (NetworkError)
// ---------------------------------------------------------------------------
pub const NetworkError = enum(c_int) {
    success = 0,
    socket = -1,
    bind = -2,
    listen = -3,
    accept = -4,
    send = -5,
    receive = -6,
    memory = -7,
    invalid = -8,
    _,
};

// ---------------------------------------------------------------------------
// Network protocol / role  (NetworkProtocol / NetworkRole)
// ---------------------------------------------------------------------------
pub const NetworkProtocol = enum(c_int) {
    tcp = 0,
    udp = 1,
    raw = 2,
    _,
};

pub const NetworkRole = enum(c_int) {
    client = 0,
    server = 1,
    peer = 2,
    _,
};

// ---------------------------------------------------------------------------
// State machine constants
// ---------------------------------------------------------------------------
pub const max_name_length: usize = 64;
pub const max_states: usize = 32;
pub const max_transitions: usize = 64;

// ---------------------------------------------------------------------------
// Protocol message types  (polycall_message_type_t)
// ---------------------------------------------------------------------------
pub const MessageType = enum(u8) {
    handshake = 0x01,
    auth = 0x02,
    command = 0x03,
    response = 0x04,
    err = 0x05,
    heartbeat = 0x06,
    _,
};

// ---------------------------------------------------------------------------
// Protocol state  (polycall_protocol_state_t)
// ---------------------------------------------------------------------------
pub const ProtocolState = enum(c_int) {
    init = 0,
    handshake = 1,
    auth = 2,
    ready = 3,
    err = 4,
    closed = 5,
    _,
};

// ---------------------------------------------------------------------------
// Protocol flags  (polycall_protocol_flags_t)
// ---------------------------------------------------------------------------
pub const ProtocolFlags = struct {
    pub const none: u16 = 0x00;
    pub const encrypted: u16 = 0x01;
    pub const compressed: u16 = 0x02;
    pub const urgent: u16 = 0x04;
    pub const reliable: u16 = 0x08;
};

// ---------------------------------------------------------------------------
// Protocol message header  (polycall_message_header_t)
// ---------------------------------------------------------------------------
pub const MessageHeader = extern struct {
    version: u8,
    @"type": u8,
    flags: u16,
    sequence: u32,
    payload_length: u32,
    checksum: u32,
};

// ---------------------------------------------------------------------------
// Token types  (PolycallTokenType)
// ---------------------------------------------------------------------------
pub const TokenType = enum(c_int) {
    eof = 0,
    number = 1,
    string = 2,
    identifier = 3,
    keyword = 4,
    operator = 5,
    separator = 6,
    comment = 7,
    whitespace = 8,
    err = 9,
    _,
};

// ---------------------------------------------------------------------------
// Token value union  (PolycallValue)
// ---------------------------------------------------------------------------
pub const Value = extern union {
    integer: i64,
    floating: f64,
    string: ?[*:0]const u8,
    boolean: bool,
    pointer: ?*anyopaque,
    raw: u64,
};

// ---------------------------------------------------------------------------
// Token  (PolycallToken)
// ---------------------------------------------------------------------------
pub const Token = extern struct {
    type: TokenType,
    value: Value,
    text: ?[*:0]const u8,
    length: usize,
    line: u32,
    column: u32,
};

// ---------------------------------------------------------------------------
// Token array  (PolycallTokenArray)
// ---------------------------------------------------------------------------
pub const TokenArray = extern struct {
    tokens: ?[*]Token,
    count: usize,
    capacity: usize,
};
