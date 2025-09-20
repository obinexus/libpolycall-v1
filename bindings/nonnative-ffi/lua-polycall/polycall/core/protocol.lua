#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Protocol Communication Layer
OBINexus Aegis Engineering - Network Protocol Implementation
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

CRITICAL ADAPTER COMPLIANCE:
This module implements communication with polycall.exe runtime.
All protocol operations MUST route through the runtime binary.
Direct execution is PROHIBITED under the adapter pattern.
]]--

local protocol = {}

-- Import dependencies
local logger = require('polycall.utils.logger')
local state = require('polycall.core.state')

-- Protocol constants
protocol.VERSION = "1.0"
protocol.DEFAULT_TIMEOUT = 30
protocol.MAX_RETRY_ATTEMPTS = 3
protocol.HEARTBEAT_INTERVAL = 60

-- Message types enumeration
protocol.MESSAGE_TYPES = {
    HANDSHAKE = "handshake",
    AUTH = "auth",
    OPERATION = "operation",
    RESPONSE = "response",
    ERROR = "error",
    HEARTBEAT = "heartbeat",
    SHUTDOWN = "shutdown"
}

-- Protocol handler constructor
function protocol.new_handler(client)
    if not client then
        error("Protocol handler requires client instance")
    end
    
    local handler = {
        client = client,
        socket = nil,
        connected = false,
        last_heartbeat = nil,
        message_id_counter = 0,
        pending_operations = {},
        
        -- Configuration
        timeout = protocol.DEFAULT_TIMEOUT,
        retry_attempts = protocol.MAX_RETRY_ATTEMPTS,
        heartbeat_interval = protocol.HEARTBEAT_INTERVAL
    }
    
    setmetatable(handler, { __index = protocol })
    
    logger.debug("Protocol handler initialized", {
        host = client.host,
        port = client.port,
        timeout = handler.timeout
    })
    
    return handler
end

-- Establish connection to polycall.exe runtime
function protocol:establish_connection()
    logger.protocol("Attempting connection to polycall.exe runtime", {
        host = self.client.host,
        port = self.client.port
    })
    
    -- ADAPTER PATTERN ENFORCEMENT:
    -- All connections must route through polycall.exe runtime
    -- Direct execution is prohibited
    error("ADAPTER COMPLIANCE: All connections must route through polycall.exe runtime")
end

-- Send protocol message
function protocol:send_message(message_type, payload)
    if not self.connected then
        return nil, "Protocol handler not connected"
    end
    
    -- Generate unique message ID
    self.message_id_counter = self.message_id_counter + 1
    local message_id = string.format("msg_%d_%d", os.time(), self.message_id_counter)
    
    -- Construct protocol message
    local message = {
        id = message_id,
        type = message_type,
        version = protocol.VERSION,
        timestamp = os.time(),
        payload = payload or {},
        
        -- Protocol metadata
        client_id = self.client.session_id,
        architecture_pattern = "adapter"
    }
    
    logger.protocol(string.format("Sending %s message", message_type), {
        message_id = message_id,
        payload_size = self:count_table_keys(payload or {})
    })
    
    -- ADAPTER PATTERN: Message routing through polycall.exe
    -- Simulate protocol communication for demonstration
    return self:simulate_protocol_communication(message)
end

-- Simulate protocol communication (adapter demonstration)
function protocol:simulate_protocol_communication(message)
    -- This function demonstrates the adapter pattern
    -- In production, this would use actual TCP/HTTP communication
    
    logger.protocol("Message routed to polycall.exe runtime", {
        message_type = message.type,
        message_id = message.id,
        adapter_pattern = true
    })
    
    -- Simulate successful response
    local response = {
        id = message.id .. "_response",
        type = protocol.MESSAGE_TYPES.RESPONSE,
        status = "success",
        timestamp = os.time(),
        original_message_id = message.id,
        payload = {
            result = "Operation routed to polycall.exe runtime",
            architecture_pattern = "adapter",
            runtime_dependency = "polycall.exe"
        }
    }
    
    return response
end

-- Execute operation through protocol
function protocol:execute_operation(operation, params)
    if not operation then
        return nil, "Operation name required"
    end
    
    logger.operation_start(operation, params)
    local start_time = os.time()
    
    -- Prepare operation payload
    local payload = {
        operation = operation,
        params = params or {},
        session_id = self.client.session_id,
        timestamp = start_time
    }
    
    -- Send operation message
    local response, error_msg = self:send_message(protocol.MESSAGE_TYPES.OPERATION, payload)
    
    local duration = os.time() - start_time
    local success = response ~= nil
    
    logger.operation_complete(operation, duration, success)
    
    if response then
        return response.payload, nil
    else
        return nil, error_msg
    end
end

-- Send handshake message
function protocol:handshake()
    local payload = {
        protocol_version = protocol.VERSION,
        client_type = "lua-polycall",
        architecture_pattern = "adapter",
        capabilities = {
            "state_machine",
            "zero_trust_auth",
            "telemetry_observation"
        }
    }
    
    return self:send_message(protocol.MESSAGE_TYPES.HANDSHAKE, payload)
end

-- Send authentication message
function protocol:authenticate(credentials)
    if not credentials then
        return nil, "Authentication credentials required"
    end
    
    local payload = {
        credentials = credentials,
        auth_method = "zero_trust",
        timestamp = os.time()
    }
    
    return self:send_message(protocol.MESSAGE_TYPES.AUTH, payload)
end

-- Send heartbeat message
function protocol:send_heartbeat()
    local payload = {
        client_status = "active",
        last_activity = self.client.last_activity,
        uptime = os.time() - (self.client.connection_timestamp or os.time())
    }
    
    local response = self:send_message(protocol.MESSAGE_TYPES.HEARTBEAT, payload)
    
    if response then
        self.last_heartbeat = os.time()
        logger.telemetry("Heartbeat sent successfully", {
            last_heartbeat = self.last_heartbeat
        })
    end
    
    return response
end

-- Close protocol connection
function protocol:close()
    if not self.connected then
        return true
    end
    
    logger.protocol("Closing protocol connection")
    
    -- Send shutdown message
    local shutdown_response = self:send_message(protocol.MESSAGE_TYPES.SHUTDOWN, {
        reason = "client_shutdown",
        timestamp = os.time()
    })
    
    -- Reset connection state
    self.connected = false
    self.socket = nil
    self.last_heartbeat = nil
    self.pending_operations = {}
    
    logger.protocol("Protocol connection closed successfully")
    
    return true
end

-- Check if heartbeat is required
function protocol:heartbeat_required()
    if not self.last_heartbeat then
        return true
    end
    
    local elapsed = os.time() - self.last_heartbeat
    return elapsed >= self.heartbeat_interval
end

-- Get protocol statistics
function protocol:get_statistics()
    return {
        connected = self.connected,
        message_count = self.message_id_counter,
        last_heartbeat = self.last_heartbeat,
        pending_operations = #self.pending_operations,
        timeout = self.timeout,
        retry_attempts = self.retry_attempts,
        heartbeat_interval = self.heartbeat_interval,
        client_host = self.client.host,
        client_port = self.client.port
    }
end

-- Utility function
function protocol:count_table_keys(table)
    local count = 0
    for _ in pairs(table) do
        count = count + 1
    end
    return count
end

-- Protocol message validation
function protocol:validate_message(message)
    if type(message) ~= "table" then
        return false, "Message must be a table"
    end
    
    if not message.id or not message.type or not message.version then
        return false, "Message missing required fields"
    end
    
    if not protocol.MESSAGE_TYPES[message.type:upper()] then
        return false, "Invalid message type"
    end
    
    return true
end

return protocol
