//! core.zig — idiomatic Zig wrapper over the libpolycall core context API.
//! Mirrors: core/include/libpolycall/core/polycall.h

const std = @import("std");
const c = @import("c.zig");
pub const types = @import("types.zig");

/// A managed polycall context.
/// Call `init` or `initWithConfig` to obtain one; call `deinit` when done.
/// The context is NOT thread-safe — use external locking if shared.
pub const PolyCall = struct {
    handle: *c.Context,

    // ------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------

    /// Initialise with default settings.
    pub fn init() types.Error!PolyCall {
        var handle: ?*c.Context = null;
        try c.polycall_init(&handle).toError();
        return .{ .handle = handle.? };
    }

    /// Initialise with an explicit `Config`.
    pub fn initWithConfig(config: types.Config) types.Error!PolyCall {
        var handle: ?*c.Context = null;
        try c.polycall_init_with_config(&handle, &config).toError();
        return .{ .handle = handle.? };
    }

    /// Release all resources.  Safe to call multiple times (idempotent at
    /// the C layer because the pointer is written to null).
    pub fn deinit(self: *PolyCall) void {
        var h: ?*c.Context = self.handle;
        c.polycall_cleanup(&h);
        self.handle = undefined;
    }

    // ------------------------------------------------------------------
    // Introspection
    // ------------------------------------------------------------------

    pub fn isInitialized(self: PolyCall) bool {
        return c.polycall_is_initialized(self.handle);
    }

    /// Returns a null-terminated version string, e.g. "2.0.0".
    pub fn version() []const u8 {
        return std.mem.span(c.polycall_get_version() orelse "unknown");
    }

    /// Returns the last error message, or null if none.
    pub fn lastError(self: PolyCall) ?[]const u8 {
        const raw = c.polycall_get_last_error(self.handle) orelse return null;
        return std.mem.span(raw);
    }

    // ------------------------------------------------------------------
    // Flags
    // ------------------------------------------------------------------

    pub fn setFlag(self: PolyCall, flag: c_uint) types.Error!void {
        try c.polycall_set_flag(self.handle, flag).toError();
    }

    pub fn clearFlag(self: PolyCall, flag: c_uint) types.Error!void {
        try c.polycall_clear_flag(self.handle, flag).toError();
    }

    // ------------------------------------------------------------------
    // User data
    // ------------------------------------------------------------------

    pub fn getUserData(self: PolyCall) ?*anyopaque {
        return c.polycall_get_user_data(self.handle);
    }

    pub fn setUserData(self: PolyCall, data: ?*anyopaque) types.Error!void {
        try c.polycall_set_user_data(self.handle, data).toError();
    }
};
