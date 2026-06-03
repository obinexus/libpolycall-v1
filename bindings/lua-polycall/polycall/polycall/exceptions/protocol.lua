#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Protocol Exception Handling
OBINexus Aegis Engineering - Systematic Protocol Error Management
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing
]]--

local protocol_exceptions = {}

-- Protocol exception types
protocol_exceptions.TYPES = {
    CONNECTION_FAILED = "connection_failed",
    HANDSHAKE_FAILED = "handshake_failed",
    AUTH_FAILED = "authentication_failed",
    PROTOCOL_VIOLATION = "protocol_violation",
    ADAPTER_COMPLIANCE_VIOLATION = "adapter_compliance_violation",
    STATE_TRANSITION_INVALID = "state_transition_invalid"
}

-- Create protocol exception
function protocol_exceptions.create(exception_type, message, context)
    return {
        type = exception_type,
        message = message,
        context = context or {},
        timestamp = os.time(),
        protocol_version = "1.0",
        architecture_pattern = "adapter"
    }
end

return protocol_exceptions
