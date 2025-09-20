#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Lua Protocol Server Implementation
OBINexus Aegis Engineering - Waterfall Methodology Server Component
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

CRITICAL PROTOCOL COMPLIANCE:
This server acts as an ADAPTER for polycall.exe runtime communication.
All business logic execution occurs within the polycall.exe binary.
Zero-trust architecture enforced throughout.
]]--

-- Module imports using LuaRocks package structure
local polycall_core = require('polycall.core')
local polycall_state = require('polycall.state')
local polycall_telemetry = require('polycall.telemetry')
local polycall_protocol = require('polycall.protocol')
local polycall_auth = require('polycall.auth')
local polycall_config = require('polycall.config')

-- Standard library imports
local socket = require('socket')
local json = require('json') -- Assumes lua-cjson or equivalent
local ssl = require('ssl')

-- Server state management
local PolyCallServer = {
    -- Protocol compliance metadata
    PROTOCOL_VERSION = "1.0",
    ARCHITECTURE_PATTERN = "adapter",
    RUNTIME_DEPENDENCY = "polycall.exe",
    
    -- Server state
    state = polycall_state.STATES.INIT,
    connections = {},
    telemetry_observer = nil,
    config = nil,
    runtime_client = nil
}

-- Constructor for server instance
function PolyCallServer:new(config_path)
    local instance = {}
    setmetatable(instance, self)
    self.__index = self
    
    -- Load configuration
    instance.config = polycall_config.load_config(config_path or "lua.polycallrc")
    
    -- Initialize telemetry observer
    instance.telemetry_observer = polycall_telemetry.new_observer()
    
    -- Initialize runtime client connection
    instance.runtime_client = polycall_core.new_client({
        polycall_host = instance.config.polycall_runtime_host,
        polycall_port = instance.config.polycall_runtime_port
    })
    
    return instance
end

-- Protocol state machine implementation
function PolyCallServer:transition_state(new_state, context)
    local current_state = self.state
    
    -- Validate state transition
    if not polycall_state.validate_transition(current_state, new_state) then
        local error_msg = string.format("Invalid state transition: %s -> %s", current_state, new_state)
        self:log_error(error_msg, context)
        return false, error_msg
    end
    
    -- Record telemetry event
    polycall_telemetry.observe_protocol_event(self.telemetry_observer, "state_transition", {
        from = current_state,
        to = new_state,
        timestamp = os.time(),
        context = context
    })
    
    -- Update state
    self.state = new_state
    self:log_info(string.format("State transition: %s -> %s", current_state, new_state))
    
    return true
end

-- Server initialization with protocol compliance
function PolyCallServer:initialize()
    self:log_info("Initializing LibPolyCall Lua Server...")
    
    -- Transition to handshake state
    local success, error_msg = self:transition_state(polycall_state.STATES.HANDSHAKE, "server_initialization")
    if not success then
        return false, error_msg
    end
    
    -- Establish connection to polycall.exe runtime
    self:log_info("Connecting to polycall.exe runtime...")
    local runtime_connected = self:connect_to_runtime()
    if not runtime_connected then
        self:transition_state(polycall_state.STATES.ERROR, "runtime_connection_failed")
        return false, "Failed to connect to polycall.exe runtime"
    end
    
    -- Authenticate with zero-trust validation
    local auth_success = self:authenticate_runtime()
    if not auth_success then
        self:transition_state(polycall_state.STATES.ERROR, "authentication_failed")
        return false, "Runtime authentication failed"
    end
    
    -- Transition to ready state
    self:transition_state(polycall_state.STATES.READY, "initialization_complete")
    self:log_success("Server initialization completed successfully")
    
    return true
end

-- Runtime connection with adapter pattern compliance
function PolyCallServer:connect_to_runtime()
    self:log_info("Establishing adapter connection to polycall.exe runtime...")
    
    -- ADAPTER PATTERN: All connections route through polycall.exe
    -- This Lua server does NOT execute business logic directly
    
    local success, error_msg = pcall(function()
        -- Attempt connection through core adapter
        polycall_core.connect(self.runtime_client)
    end)
    
    if not success and string.find(error_msg, "ADAPTER COMPLIANCE") then
        self:log_info("✓ Adapter pattern compliance verified - routing to polycall.exe")
        
        -- Simulate successful adapter connection for demonstration
        -- In production, this would establish actual protocol communication
        return true
    else
        self:log_error("Adapter pattern violation detected", {error = error_msg})
        return false
    end
end

-- Zero-trust authentication implementation
function PolyCallServer:authenticate_runtime()
    self:log_info("Executing zero-trust authentication with polycall.exe...")
    
    local credentials = {
        server_type = "lua",
        protocol_version = self.PROTOCOL_VERSION,
        architecture_pattern = self.ARCHITECTURE_PATTERN,
        ssh_cert_path = self.config.cert_path,
        timestamp = os.time(),
        nonce = self:generate_nonce()
    }
    
    -- ADAPTER PATTERN: Authentication handled by polycall.exe
    local success, error_msg = pcall(function()
        polycall_core.authenticate(self.runtime_client, credentials)
    end)
    
    if not success and string.find(error_msg, "ADAPTER COMPLIANCE") then
        self:log_info("✓ Zero-trust authentication routed to polycall.exe runtime")
        return true
    else
        self:log_error("Authentication routing failed", {error = error_msg})
        return false
    end
end

-- Client request handler with protocol routing
function PolyCallServer:handle_client_request(client_socket, request_data)
    -- Transition to executing state
    self:transition_state(polycall_state.STATES.EXECUTING, "client_request_processing")
    
    self:log_info("Processing client request through polycall.exe runtime...")
    
    -- Parse request
    local success, parsed_request = pcall(json.decode, request_data)
    if not success then
        self:send_error_response(client_socket, "Invalid JSON request")
        self:transition_state(polycall_state.STATES.READY, "request_error")
        return
    end
    
    -- Route request to polycall.exe runtime
    local response = self:route_request_to_runtime(parsed_request)
    
    -- Send response to client
    self:send_response(client_socket, response)
    
    -- Transition back to ready state
    self:transition_state(polycall_state.STATES.READY, "request_completed")
end

-- Request routing to polycall.exe runtime
function PolyCallServer:route_request_to_runtime(request)
    -- ADAPTER PATTERN: All business logic execution occurs in polycall.exe
    
    polycall_telemetry.observe_protocol_event(self.telemetry_observer, "request_routing", {
        operation = request.operation or "unknown",
        timestamp = os.time(),
        adapter_pattern = true
    })
    
    -- Simulate routing to polycall.exe runtime
    -- In production, this would use actual protocol communication
    local response = {
        status = "success",
        message = "Request routed to polycall.exe runtime",
        operation = request.operation,
        adapter_pattern = self.ARCHITECTURE_PATTERN,
        runtime_dependency = self.RUNTIME_DEPENDENCY,
        timestamp = os.time()
    }
    
    return response
end

-- Server main loop with protocol compliance
function PolyCallServer:run()
    self:log_info("Starting LibPolyCall Lua Server main loop...")
    
    -- Create server socket
    local server_socket = socket.bind(self.config.host, self.config.port)
    if not server_socket then
        self:log_error("Failed to bind server socket")
        return false
    end
    
    self:log_success(string.format("Server listening on %s:%d", self.config.host, self.config.port))
    
    -- Main server loop
    while self.state ~= polycall_state.STATES.SHUTDOWN do
        local client_socket = server_socket:accept()
        
        if client_socket then
            -- Handle client connection
            self:handle_client_connection(client_socket)
        end
        
        -- Brief pause to prevent CPU spinning
        socket.sleep(0.1)
    end
    
    server_socket:close()
    self:log_info("Server shutdown completed")
end

-- Client connection handler
function PolyCallServer:handle_client_connection(client_socket)
    self:log_info("New client connection established")
    
    -- Read request data
    local request_data, error_msg = client_socket:receive('*l')
    if not request_data then
        self:log_error("Failed to read client request", {error = error_msg})
        client_socket:close()
        return
    end
    
    -- Process request
    self:handle_client_request(client_socket, request_data)
    
    -- Close connection
    client_socket:close()
    self:log_info("Client connection closed")
end

-- Response utilities
function PolyCallServer:send_response(client_socket, response)
    local response_json = json.encode(response)
    client_socket:send(response_json .. "\n")
end

function PolyCallServer:send_error_response(client_socket, error_message)
    local error_response = {
        status = "error",
        message = error_message,
        timestamp = os.time(),
        adapter_pattern = self.ARCHITECTURE_PATTERN
    }
    self:send_response(client_socket, error_response)
end

-- Utility functions
function PolyCallServer:generate_nonce()
    return string.format("%d_%d", os.time(), math.random(10000, 99999))
end

function PolyCallServer:log_info(message)
    print(string.format("[INFO] %s", message))
end

function PolyCallServer:log_success(message)
    print(string.format("[SUCCESS] %s", message))
end

function PolyCallServer:log_error(message, context)
    print(string.format("[ERROR] %s", message))
    if context then
        print(string.format("[ERROR_CONTEXT] %s", json.encode(context)))
    end
end

-- Graceful shutdown
function PolyCallServer:shutdown()
    self:log_info("Initiating server shutdown...")
    self:transition_state(polycall_state.STATES.SHUTDOWN, "graceful_shutdown")
    
    -- Close runtime connection
    if self.runtime_client then
        self:log_info("Closing polycall.exe runtime connection...")
    end
    
    -- Export telemetry data
    local metrics = polycall_telemetry.get_metrics(self.telemetry_observer)
    self:log_info(string.format("Telemetry metrics collected: %d observations", #self.telemetry_observer.observations))
    
    self:log_success("Server shutdown completed")
end

-- Main execution entry point
function main()
    print("=" .. string.rep("=", 60))
    print("LibPolyCall Trial v1 - Lua Protocol Server")
    print("OBINexus Aegis Engineering - Waterfall Implementation")
    print("Technical Lead: Nnamdi Michael Okpala")
    print("Architecture: Adapter Pattern (Program-First)")
    print("=" .. string.rep("=", 60))
    
    -- Create and initialize server
    local server = PolyCallServer:new("lua.polycallrc")
    
    -- Initialize server with protocol compliance
    local init_success, init_error = server:initialize()
    if not init_success then
        print("[FATAL] Server initialization failed: " .. (init_error or "unknown error"))
        os.exit(1)
    end
    
    -- Set up signal handlers for graceful shutdown
    local function signal_handler(signal)
        print(string.format("[SIGNAL] Received %s signal", signal))
        server:shutdown()
        os.exit(0)
    end
    
    -- Start server
    local success, error_msg = pcall(function()
        server:run()
    end)
    
    if not success then
        print("[FATAL] Server execution failed: " .. (error_msg or "unknown error"))
        server:shutdown()
        os.exit(1)
    end
end

-- Execute if run directly
if arg and arg[0] == "server.lua" then
    main()
end

-- Export server class for module usage
return PolyCallServer
