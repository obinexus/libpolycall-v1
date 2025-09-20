#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - State Machine Implementation
OBINexus Aegis Engineering - Protocol State Management
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

PROTOCOL COMPLIANCE:
Implements finite state automaton for protocol session management.
All state transitions must be validated according to LibPolyCall specification.
]]--

local state = {}

-- Import logging utilities
local logger = require('polycall.utils.logger')

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

-- Valid state transitions mapping
state.TRANSITIONS = {
    [state.STATES.INIT] = { 
        state.STATES.HANDSHAKE, 
        state.STATES.ERROR 
    },
    [state.STATES.HANDSHAKE] = { 
        state.STATES.AUTH, 
        state.STATES.ERROR 
    },
    [state.STATES.AUTH] = { 
        state.STATES.READY, 
        state.STATES.ERROR 
    },
    [state.STATES.READY] = { 
        state.STATES.EXECUTING, 
        state.STATES.SHUTDOWN, 
        state.STATES.ERROR 
    },
    [state.STATES.EXECUTING] = { 
        state.STATES.READY, 
        state.STATES.ERROR 
    },
    [state.STATES.ERROR] = { 
        state.STATES.SHUTDOWN,
        state.STATES.INIT  -- Allow recovery
    },
    [state.STATES.SHUTDOWN] = {} -- Terminal state
}

-- State metadata for protocol tracking
state.STATE_METADATA = {
    [state.STATES.INIT] = {
        description = "Initial protocol state",
        timeout = 30,
        retry_allowed = false
    },
    [state.STATES.HANDSHAKE] = {
        description = "Protocol handshake in progress",
        timeout = 10,
        retry_allowed = true
    },
    [state.STATES.AUTH] = {
        description = "Zero-trust authentication",
        timeout = 15,
        retry_allowed = true
    },
    [state.STATES.READY] = {
        description = "Ready for operation execution",
        timeout = nil,
        retry_allowed = false
    },
    [state.STATES.EXECUTING] = {
        description = "Operation execution in progress",
        timeout = 60,
        retry_allowed = false
    },
    [state.STATES.ERROR] = {
        description = "Error state - recovery required",
        timeout = nil,
        retry_allowed = true
    },
    [state.STATES.SHUTDOWN] = {
        description = "Graceful shutdown complete",
        timeout = nil,
        retry_allowed = false
    }
}

-- Validate state transition
function state.validate_transition(from_state, to_state)
    if not from_state or not to_state then
        logger.error("State transition validation requires both from_state and to_state")
        return false
    end
    
    local valid_transitions = state.TRANSITIONS[from_state] or {}
    
    for _, valid_state in ipairs(valid_transitions) do
        if valid_state == to_state then
            logger.debug(string.format("Valid state transition: %s -> %s", from_state, to_state))
            return true
        end
    end
    
    logger.warn(string.format("Invalid state transition attempted: %s -> %s", from_state, to_state))
    return false
end

-- Get valid transitions for a state
function state.get_valid_transitions(current_state)
    return state.TRANSITIONS[current_state] or {}
end

-- Get state metadata
function state.get_state_metadata(state_name)
    return state.STATE_METADATA[state_name]
end

-- Check if state is terminal
function state.is_terminal_state(state_name)
    local valid_transitions = state.TRANSITIONS[state_name] or {}
    return #valid_transitions == 0
end

-- Check if state allows retry operations
function state.allows_retry(state_name)
    local metadata = state.get_state_metadata(state_name)
    return metadata and metadata.retry_allowed or false
end

-- Get state timeout
function state.get_state_timeout(state_name)
    local metadata = state.get_state_metadata(state_name)
    return metadata and metadata.timeout
end

-- State machine context object
function state.new_context(initial_state)
    initial_state = initial_state or state.STATES.INIT
    
    return {
        current_state = initial_state,
        previous_state = nil,
        transition_history = {},
        transition_count = 0,
        created_at = os.time(),
        last_transition = nil,
        error_count = 0,
        retry_count = 0
    }
end

-- Execute state transition with context
function state.transition(context, to_state, transition_data)
    if not context or not to_state then
        logger.error("State transition requires context and target state")
        return false, "Invalid transition parameters"
    end
    
    local from_state = context.current_state
    
    -- Validate transition
    if not state.validate_transition(from_state, to_state) then
        local error_msg = string.format("Invalid state transition: %s -> %s", from_state, to_state)
        logger.error(error_msg)
        return false, error_msg
    end
    
    -- Update context
    context.previous_state = from_state
    context.current_state = to_state
    context.transition_count = context.transition_count + 1
    context.last_transition = os.time()
    
    -- Record transition history
    table.insert(context.transition_history, {
        from_state = from_state,
        to_state = to_state,
        timestamp = context.last_transition,
        data = transition_data
    })
    
    -- Handle error state tracking
    if to_state == state.STATES.ERROR then
        context.error_count = context.error_count + 1
    end
    
    -- Log transition
    logger.state_transition(from_state, to_state, {
        transition_count = context.transition_count,
        error_count = context.error_count,
        data = transition_data
    })
    
    return true
end

-- Get context information
function state.get_context_info(context)
    if not context then
        return nil
    end
    
    return {
        current_state = context.current_state,
        previous_state = context.previous_state,
        transition_count = context.transition_count,
        error_count = context.error_count,
        retry_count = context.retry_count,
        created_at = context.created_at,
        last_transition = context.last_transition,
        uptime = os.time() - context.created_at,
        valid_transitions = state.get_valid_transitions(context.current_state),
        can_retry = state.allows_retry(context.current_state),
        is_terminal = state.is_terminal_state(context.current_state)
    }
end

-- Reset context to initial state
function state.reset_context(context)
    if not context then
        return false
    end
    
    context.current_state = state.STATES.INIT
    context.previous_state = nil
    context.transition_count = 0
    context.last_transition = os.time()
    context.error_count = 0
    context.retry_count = context.retry_count + 1
    
    -- Clear transition history
    context.transition_history = {}
    
    logger.info("State context reset to INIT", {
        retry_count = context.retry_count
    })
    
    return true
end

return state
