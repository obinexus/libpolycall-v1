//! micro.zig — idiomatic Zig wrapper over the polycall micro-service API.
//! Mirrors: core/include/libpolycall/core/polycall_micro.h

const std = @import("std");
const c = @import("c.zig");
const types = @import("types.zig");

/// A managed micro-service instance.
///
/// Example:
/// ```zig
/// var svc = try MicroService.init("credit-score", 9090);
/// defer svc.deinit();
/// try svc.start();
/// // ... handle requests ...
/// try svc.stop();
/// ```
pub const MicroService = struct {
    inner: c.MicroService,

    pub fn init(name: [:0]const u8, port: u16) types.Error!MicroService {
        var svc = MicroService{
            .inner = .{
                .service_name = name.ptr,
                .port = port,
                .context = null,
            },
        };
        if (c.polycall_micro_init(&svc.inner) != 0) return types.Error.Unknown;
        return svc;
    }

    pub fn deinit(self: *MicroService) void {
        c.polycall_micro_cleanup(&self.inner);
    }

    pub fn start(self: *MicroService) types.Error!void {
        if (c.polycall_micro_start(&self.inner) != 0) return types.Error.Unknown;
    }

    pub fn stop(self: *MicroService) types.Error!void {
        if (c.polycall_micro_stop(&self.inner) != 0) return types.Error.Unknown;
    }
};

/// Banking subsystem helpers (thin wrappers — no lifecycle state needed).
pub const Banking = struct {
    pub fn init() types.Error!void {
        if (c.polycall_banking_init() != 0) return types.Error.Unknown;
    }

    pub fn process(request: ?*const anyopaque, response: ?*anyopaque) types.Error!void {
        if (c.polycall_banking_process(request, response) != 0) return types.Error.Unknown;
    }

    pub fn cleanup() void {
        c.polycall_banking_cleanup();
    }
};
