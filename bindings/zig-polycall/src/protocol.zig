//! protocol.zig — idiomatic Zig wrapper over the polycall protocol layer.
//! Mirrors: core/include/libpolycall/core/polycall_protocol.h

const std = @import("std");
const c = @import("c.zig");
const types = @import("types.zig");
const network = @import("network.zig");

pub const Callbacks = c.ProtocolCallbacks;
pub const Config = c.ProtocolConfig;

/// Protocol version constant (matches POLYCALL_PROTOCOL_VERSION = 1).
pub const protocol_version: u8 = 1;

/// A managed protocol session context.
///
/// Typical server flow:
/// ```zig
/// var proto = try Protocol.init(pc.handle, &endpoint.inner, config);
/// defer proto.deinit();
/// try proto.startHandshake();
/// try proto.authenticate("token", "token".len);
/// ```
pub const Protocol = struct {
    ctx: c.ProtocolContext,

    // ------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------

    pub fn init(
        pc_ctx: ?*c.Context,
        endpoint: *c.NetworkEndpoint,
        config: *const c.ProtocolConfig,
    ) types.Error!Protocol {
        var proto = Protocol{ .ctx = std.mem.zeroes(c.ProtocolContext) };
        if (!c.polycall_protocol_init(&proto.ctx, pc_ctx, endpoint, config)) {
            return types.Error.Unknown;
        }
        return proto;
    }

    pub fn deinit(self: *Protocol) void {
        c.polycall_protocol_cleanup(&self.ctx);
    }

    // ------------------------------------------------------------------
    // Handshake & auth
    // ------------------------------------------------------------------

    pub fn startHandshake(self: *Protocol) types.Error!void {
        if (!c.polycall_protocol_start_handshake(&self.ctx)) return types.Error.Network;
    }

    pub fn completeHandshake(self: *Protocol) types.Error!void {
        if (!c.polycall_protocol_complete_handshake(&self.ctx)) return types.Error.Network;
    }

    pub fn authenticate(self: *Protocol, credentials: []const u8) types.Error!void {
        if (!c.polycall_protocol_authenticate(
            &self.ctx,
            credentials.ptr,
            credentials.len,
        )) return types.Error.Network;
    }

    // ------------------------------------------------------------------
    // Send / receive
    // ------------------------------------------------------------------

    pub fn send(
        self: *Protocol,
        msg_type: types.MessageType,
        payload: []const u8,
        flags: u16,
    ) types.Error!void {
        if (!c.polycall_protocol_send(
            &self.ctx,
            msg_type,
            payload.ptr,
            payload.len,
            flags,
        )) return types.Error.Network;
    }

    pub fn process(self: *Protocol, data: []const u8) types.Error!void {
        if (!c.polycall_protocol_process(&self.ctx, data.ptr, data.len)) {
            return types.Error.Network;
        }
    }

    pub fn update(self: *Protocol) void {
        c.polycall_protocol_update(&self.ctx);
    }

    // ------------------------------------------------------------------
    // State inspection
    // ------------------------------------------------------------------

    pub fn state(self: *const Protocol) types.ProtocolState {
        return c.polycall_protocol_get_state(&self.ctx);
    }

    pub fn canTransitionTo(self: *const Protocol, target: types.ProtocolState) bool {
        return c.polycall_protocol_can_transition(&self.ctx, target);
    }

    pub fn isConnected(self: *const Protocol) bool {
        return c.polycall_protocol_is_connected(&self.ctx);
    }

    pub fn isAuthenticated(self: *const Protocol) bool {
        return c.polycall_protocol_is_authenticated(&self.ctx);
    }

    pub fn isError(self: *const Protocol) bool {
        return c.polycall_protocol_is_error(&self.ctx);
    }

    pub fn lastError(self: *const Protocol) ?[]const u8 {
        const raw = c.polycall_protocol_get_error(&self.ctx) orelse return null;
        return std.mem.span(raw);
    }

    // ------------------------------------------------------------------
    // Utilities
    // ------------------------------------------------------------------

    pub fn createHeader(
        msg_type: types.MessageType,
        payload_length: usize,
        flags: u16,
    ) types.MessageHeader {
        return c.polycall_protocol_create_header(msg_type, payload_length, flags);
    }

    pub fn calculateChecksum(data: []const u8) u32 {
        return c.polycall_protocol_calculate_checksum(data.ptr, data.len);
    }

    pub fn verifyChecksum(
        header: *const types.MessageHeader,
        payload: []const u8,
    ) bool {
        return c.polycall_protocol_verify_checksum(header, payload.ptr, payload.len);
    }

    pub fn versionCompatible(remote: u8) bool {
        return c.polycall_protocol_version_compatible(remote);
    }
};
