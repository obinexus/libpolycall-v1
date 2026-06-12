//! state_machine.zig — idiomatic Zig wrapper over the polycall state machine.
//! Mirrors: core/include/libpolycall/core/polycall_state_machine.h

const std = @import("std");
const c = @import("c.zig");
const types = @import("types.zig");

// Re-export callback types for consumer convenience.
pub const StateAction = c.StateAction;
pub const GuardCondition = c.GuardCondition;
pub const StateIntegrityCheck = c.StateIntegrityCheck;
pub const Snapshot = c.StateSnapshot;
pub const Diagnostics = c.StateDiagnostics;

/// A managed state machine.
///
/// Example:
/// ```zig
/// var sm = try StateMachine.init(ctx.handle, null);
/// defer sm.deinit();
/// try sm.addState("idle", null, null, false);
/// try sm.addState("running", null, null, false);
/// try sm.addTransition("start", 0, 1, null, null);
/// try sm.executeTransition("start");
/// ```
pub const StateMachine = struct {
    ptr: *c.StateMachine,

    // ------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------

    /// Create a new state machine attached to `ctx_handle`.
    /// Pass a custom `integrity_check` function or null to skip.
    pub fn init(ctx_handle: ?*c.Context, integrity_check: StateIntegrityCheck) types.Error!StateMachine {
        var ptr: *c.StateMachine = undefined;
        try c.polycall_sm_create_with_integrity(ctx_handle, &ptr, integrity_check).toError();
        return .{ .ptr = ptr };
    }

    pub fn deinit(self: *StateMachine) void {
        c.polycall_sm_destroy(self.ptr);
        self.ptr = undefined;
    }

    // ------------------------------------------------------------------
    // State management
    // ------------------------------------------------------------------

    /// Add a named state.  Returns its auto-assigned id (0-based index).
    pub fn addState(
        self: *StateMachine,
        name: [:0]const u8,
        on_enter: StateAction,
        on_exit: StateAction,
        is_final: bool,
    ) types.Error!void {
        try c.polycall_sm_add_state(self.ptr, name.ptr, on_enter, on_exit, is_final).toError();
    }

    pub fn lockState(self: *StateMachine, state_id: c_uint) types.Error!void {
        try c.polycall_sm_lock_state(self.ptr, state_id).toError();
    }

    pub fn unlockState(self: *StateMachine, state_id: c_uint) types.Error!void {
        try c.polycall_sm_unlock_state(self.ptr, state_id).toError();
    }

    pub fn verifyIntegrity(self: *StateMachine, state_id: c_uint) types.Error!void {
        try c.polycall_sm_verify_state_integrity(self.ptr, state_id).toError();
    }

    pub fn getStateVersion(self: *const StateMachine, state_id: c_uint) types.Error!c_uint {
        var ver: c_uint = 0;
        try c.polycall_sm_get_state_version(self.ptr, state_id, &ver).toError();
        return ver;
    }

    // ------------------------------------------------------------------
    // Transition management
    // ------------------------------------------------------------------

    pub fn addTransition(
        self: *StateMachine,
        name: [:0]const u8,
        from_state: c_uint,
        to_state: c_uint,
        action: StateAction,
        guard: GuardCondition,
    ) types.Error!void {
        try c.polycall_sm_add_transition(
            self.ptr,
            name.ptr,
            from_state,
            to_state,
            action,
            guard,
        ).toError();
    }

    pub fn executeTransition(self: *StateMachine, name: [:0]const u8) types.Error!void {
        try c.polycall_sm_execute_transition(self.ptr, name.ptr).toError();
    }

    // ------------------------------------------------------------------
    // Snapshot / restore
    // ------------------------------------------------------------------

    pub fn createSnapshot(self: *const StateMachine, state_id: c_uint) types.Error!Snapshot {
        var snap: Snapshot = undefined;
        try c.polycall_sm_create_state_snapshot(self.ptr, state_id, &snap).toError();
        return snap;
    }

    pub fn restoreSnapshot(self: *StateMachine, snapshot: *const Snapshot) types.Error!void {
        try c.polycall_sm_restore_state_from_snapshot(self.ptr, snapshot).toError();
    }

    // ------------------------------------------------------------------
    // Diagnostics
    // ------------------------------------------------------------------

    pub fn getDiagnostics(self: *const StateMachine, state_id: c_uint) types.Error!Diagnostics {
        var diag: Diagnostics = undefined;
        try c.polycall_sm_get_state_diagnostics(self.ptr, state_id, &diag).toError();
        return diag;
    }

    // ------------------------------------------------------------------
    // Convenience
    // ------------------------------------------------------------------

    /// Current state index.
    pub fn currentState(self: *const StateMachine) c_uint {
        return self.ptr.current_state;
    }

    /// Number of states registered.
    pub fn numStates(self: *const StateMachine) c_uint {
        return self.ptr.num_states;
    }

    /// Number of transitions registered.
    pub fn numTransitions(self: *const StateMachine) c_uint {
        return self.ptr.num_transitions;
    }
};
