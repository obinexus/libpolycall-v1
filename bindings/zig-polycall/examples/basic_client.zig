//! basic_client.zig — minimal zig-polycall usage example.
//!
//! Demonstrates:
//!   1. Core context init / deinit
//!   2. State machine construction with three states and two transitions
//!   3. Protocol session (handshake → ready)
//!   4. Micro-service lifecycle
//!
//! Build:   zig build run-example
//! or just: zig build -Dlibpolycall_dir=/path/to/libpolycall-v2 run-example

const std  = @import("std");
const pc   = @import("polycall");

pub fn main() !void {
    // ----------------------------------------------------------------
    // 1. Core context
    // ----------------------------------------------------------------
    std.debug.print("libpolycall version: {s}\n", .{pc.PolyCall.version()});

    var ctx = try pc.PolyCall.init();
    defer ctx.deinit();

    try ctx.setFlag(pc.Flags.debug);
    std.debug.print("Context initialised: {}\n", .{ctx.isInitialized()});

    // ----------------------------------------------------------------
    // 2. State machine: idle → connecting → ready
    // ----------------------------------------------------------------
    var sm = try pc.StateMachine.init(ctx.handle, null);
    defer sm.deinit();

    try sm.addState("idle",        null, null, false);
    try sm.addState("connecting",  null, null, false);
    try sm.addState("ready",       null, null, true);

    try sm.addTransition("connect",  0, 1, null, null);
    try sm.addTransition("handshook", 1, 2, null, null);

    std.debug.print("SM states: {}  transitions: {}\n",
        .{ sm.numStates(), sm.numTransitions() });

    try sm.executeTransition("connect");
    std.debug.print("After 'connect',  current state: {}\n", .{sm.currentState()});

    try sm.executeTransition("handshook");
    std.debug.print("After 'handshook', current state: {}\n", .{sm.currentState()});

    // Snapshot / restore round-trip
    const snap = try sm.createSnapshot(sm.currentState());
    std.debug.print("Snapshot checksum: 0x{x:0>8}\n", .{snap.checksum});

    // ----------------------------------------------------------------
    // 3. Checksum utility (no live connection needed)
    // ----------------------------------------------------------------
    const payload = "OBINexus polycall test payload";
    const csum = pc.Protocol.calculateChecksum(payload);
    std.debug.print("Payload checksum: 0x{x:0>8}\n", .{csum});

    const header = pc.Protocol.createHeader(.command, payload.len, pc.ProtocolFlags.reliable);
    const valid  = pc.Protocol.verifyChecksum(&header, payload);
    std.debug.print("Checksum valid: {}\n", .{valid});

    std.debug.print("Version compatible (remote=1): {}\n",
        .{pc.Protocol.versionCompatible(1)});

    // ----------------------------------------------------------------
    // 4. Micro-service (init only — no real network in this example)
    // ----------------------------------------------------------------
    var svc = pc.MicroService.init("demo-service", 9000) catch |err| blk: {
        std.debug.print("MicroService.init skipped (no daemon): {}\n", .{err});
        break :blk null;
    };
    if (svc) |*s| {
        defer s.deinit();
        std.debug.print("MicroService 'demo-service' on port 9000 ready\n", .{});
    }

    std.debug.print("Done.\n", .{});
}
