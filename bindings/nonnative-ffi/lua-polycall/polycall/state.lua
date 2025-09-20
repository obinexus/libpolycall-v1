-- LibPolyCall Trial v1 - State Machine Adapter
-- Protocol state transitions through polycall.exe

local state = {}

-- State machine definitions
state.STATES = {
    INIT = "init",
    HANDSHAKE = "handshake", 
    AUTH = "auth",
    READY = "ready",
    EXECUTING = "executing",
    SHUTDOWN = "shutdown",
    ERROR = "error"
}

state.TRANSITIONS = {
    [state.STATES.INIT] = { state.STATES.HANDSHAKE, state.STATES.ERROR },
    [state.STATES.HANDSHAKE] = { state.STATES.AUTH, state.STATES.ERROR },
    [state.STATES.AUTH] = { state.STATES.READY, state.STATES.ERROR },
    [state.STATES.READY] = { state.STATES.EXECUTING, state.STATES.SHUTDOWN, state.STATES.ERROR },
    [state.STATES.EXECUTING] = { state.STATES.READY, state.STATES.ERROR },
    [state.STATES.ERROR] = { state.STATES.SHUTDOWN }
}

function state.validate_transition(from, to)
    local valid_transitions = state.TRANSITIONS[from] or {}
    for _, valid_state in ipairs(valid_transitions) do
        if valid_state == to then
            return true
        end
    end
    return false
end

return state
