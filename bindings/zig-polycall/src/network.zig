//! network.zig — idiomatic Zig wrapper over libpolycall's network layer.
//! Mirrors: core/include/libpolycall/core/network.h
//!          core/include/libpolycall/socket/network.h

const std = @import("std");
const c = @import("c.zig");
const types = @import("types.zig");

// Re-export raw structs consumers may need to fill.
pub const Handlers = c.NetworkHandlers;
pub const Packet = c.NetworkPacket;

/// A managed network endpoint (client or server side).
///
/// Example — TCP server:
/// ```zig
/// var ep = Endpoint{
///     .inner = std.mem.zeroes(c.NetworkEndpoint),
/// };
/// ep.inner.port  = 8080;
/// ep.inner.protocol = .tcp;
/// ep.inner.role     = .server;
/// try ep.init();
/// defer ep.close();
/// ```
pub const Endpoint = struct {
    inner: c.NetworkEndpoint,

    pub fn init(self: *Endpoint) types.Error!void {
        if (!c.net_init(&self.inner)) return types.Error.Network;
    }

    pub fn close(self: *Endpoint) void {
        c.net_close(&self.inner);
    }

    pub fn send(self: *Endpoint, packet: *c.NetworkPacket) types.Error!isize {
        const n = c.net_send(&self.inner, packet);
        if (n < 0) return types.Error.Network;
        return n;
    }

    pub fn receive(self: *Endpoint, packet: *c.NetworkPacket) types.Error!isize {
        const n = c.net_receive(&self.inner, packet);
        if (n < 0) return types.Error.Network;
        return n;
    }

    pub fn setUserData(self: *Endpoint, data: ?*anyopaque) void {
        self.inner.user_data = data;
    }

    pub fn isPortInUse(port: u16) bool {
        return c.net_is_port_in_use(port);
    }

    pub fn releasePort(port: u16) bool {
        return c.net_release_port(port);
    }
};

/// A managed network program (owns an array of endpoints + client states).
pub const Program = struct {
    inner: c.NetworkProgram,

    pub fn init(self: *Program) void {
        c.net_init_program(&self.inner);
    }

    pub fn deinit(self: *Program) void {
        c.net_cleanup_program(&self.inner);
    }

    /// Blocking event loop — runs until `inner.running` is set to false.
    pub fn run(self: *Program) void {
        c.net_run(&self.inner);
    }

    pub fn stop(self: *Program) void {
        self.inner.running = false;
    }
};
