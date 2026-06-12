//! root.zig — public surface of the zig-polycall package.
//!
//! Import in your project:
//!   const polycall = @import("polycall");
//!
//! Then use:
//!   const pc = try polycall.PolyCall.init();
//!   defer pc.deinit();

pub const types = @import("types.zig");
pub const c = @import("c.zig");

// High-level wrappers
pub const PolyCall = @import("core.zig").PolyCall;
pub const StateMachine = @import("state_machine.zig").StateMachine;
pub const Protocol = @import("protocol.zig").Protocol;
pub const Endpoint = @import("network.zig").Endpoint;
pub const Program = @import("network.zig").Program;
pub const MicroService = @import("micro.zig").MicroService;
pub const Banking = @import("micro.zig").Banking;

// Convenience re-exports from types
pub const Status = types.Status;
pub const Error = types.Error;
pub const Config = types.Config;
pub const Flags = types.Flags;
pub const MessageType = types.MessageType;
pub const ProtocolState = types.ProtocolState;
pub const ProtocolFlags = types.ProtocolFlags;
pub const NetworkProtocol = types.NetworkProtocol;
pub const NetworkRole = types.NetworkRole;
pub const Token = types.Token;
pub const TokenType = types.TokenType;

test {
    // Pull in all sub-module tests.
    _ = @import("types.zig");
    _ = @import("core.zig");
    _ = @import("state_machine.zig");
    _ = @import("protocol.zig");
    _ = @import("network.zig");
    _ = @import("micro.zig");
}
