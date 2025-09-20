#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Lua Binding Usage Examples
OBINexus Aegis Engineering - Technical Implementation Examples
Collaborative Development: Nnamdi Michael Okpala - OBINexusComputing

Protocol-compliant require path resolution for LuaRocks package structure
]]--

-- Example 1: Basic Module Import Resolution
-- Demonstrates proper require paths for installed rock package
local polycall_core = require('polycall.core')
local polycall_state = require('polycall.state')
local polycall_telemetry = require('polycall.telemetry')
local polycall_cli = require('polycall.cli')

-- Example 2: Validation Module Import
local setup_validator = require('polycall.validators.setup')

-- Example 3: Protocol-Compliant Client Initialization
function create_protocol_client()
    print("=== LibPolyCall Lua Binding - Protocol Client Example ===")
    
    -- Configuration for polycall.exe runtime connection
    local client_config = {
        polycall_host = "localhost",
        polycall_port = 8084,
        protocol_version = "1.0",
        architecture_pattern = "adapter"
    }
    
    -- Initialize client through core adapter
    local client = polycall_core.new_client(client_config)
    
    print("Client initialized with adapter pattern compliance")
    print("Host: " .. client.host)
    print("Port: " .. client.port)
    print("Protocol: " .. polycall_core.PROTOCOL_VERSION)
    print("Architecture: " .. polycall_core.ARCHITECTURE_PATTERN)
    
    return client
end

-- Example 4: State Machine Integration
function demonstrate_state_transitions()
    print("\n=== State Machine Protocol Compliance ===")
    
    -- Validate state transition rules
    local transitions = {
        {polycall_state.STATES.INIT, polycall_state.STATES.HANDSHAKE},
        {polycall_state.STATES.HANDSHAKE, polycall_state.STATES.AUTH},
        {polycall_state.STATES.AUTH, polycall_state.STATES.READY},
        {polycall_state.STATES.READY, polycall_state.STATES.EXECUTING}
    }
    
    for _, transition in ipairs(transitions) do
        local from, to = transition[1], transition[2]
        local valid = polycall_state.validate_transition(from, to)
        print(string.format("Transition %s -> %s: %s", from, to, valid and "VALID" or "INVALID"))
    end
end

-- Example 5: Telemetry Observer Implementation
function setup_protocol_telemetry()
    print("\n=== Telemetry Integration Example ===")
    
    local observer = polycall_telemetry.new_observer()
    
    -- Simulate protocol events
    polycall_telemetry.observe_protocol_event(observer, "connection_attempt", {
        timestamp = os.time(),
        host = "localhost",
        port = 8084
    })
    
    polycall_telemetry.observe_protocol_event(observer, "state_transition", {
        from = "init",
        to = "handshake",
        protocol_version = "1.0"
    })
    
    print("Telemetry observations recorded: " .. #observer.observations)
    
    return observer
end

-- Example 6: Setup Validation Check
function validate_environment_setup()
    print("\n=== Environment Validation ===")
    
    local setup_valid = setup_validator.validate_environment()
    if setup_valid then
        print("✓ Setup validation passed - .polycall-lua-setup-complete found")
    else
        print("✗ Setup validation failed - run ./scripts/setup-lua-polycall.sh")
        return false
    end
    
    return true
end

-- Example 7: CLI Integration Pattern
function demonstrate_cli_integration(args)
    print("\n=== CLI Integration Example ===")
    
    -- Simulate CLI command execution
    local test_args = args or {"info"}
    local exit_code = polycall_cli.run(test_args)
    
    print("CLI command executed with exit code: " .. exit_code)
    return exit_code
end

-- Example 8: Error Handling Pattern for Adapter Compliance
function demonstrate_adapter_compliance()
    print("\n=== Adapter Pattern Compliance ===")
    
    local client = polycall_core.new_client({
        polycall_host = "localhost",
        polycall_port = 8084
    })
    
    -- Attempt operations that should route through polycall.exe
    local success, error_msg = pcall(function()
        polycall_core.connect(client)
    end)
    
    if not success and string.find(error_msg, "ADAPTER COMPLIANCE") then
        print("✓ Adapter pattern properly enforced - direct execution prevented")
        print("Error message: " .. error_msg)
    else
        print("✗ Adapter pattern violation detected")
    end
end

-- Main Execution Function
function main()
    print("LibPolyCall Trial v1 - Lua Binding Examples")
    print("Aegis Engineering Methodology - Waterfall Implementation")
    print("Technical Collaboration: Nnamdi Michael Okpala")
    print("=" .. string.rep("=", 60))
    
    -- Validate setup before proceeding
    if not validate_environment_setup() then
        print("Environment setup required before examples can execute")
        os.exit(1)
    end
    
    -- Execute demonstration sequence
    local client = create_protocol_client()
    demonstrate_state_transitions()
    local observer = setup_protocol_telemetry()
    demonstrate_cli_integration()
    demonstrate_adapter_compliance()
    
    print("\n" .. string.rep("=", 60))
    print("Example execution completed - Protocol compliance validated")
    print("Next steps: luarocks make lua-polycall-1.0-1.rockspec")
end

-- Execute if run directly
if arg and arg[0] == "lua-polycall-examples.lua" then
    main()
end

-- Export functions for module usage
return {
    create_protocol_client = create_protocol_client,
    demonstrate_state_transitions = demonstrate_state_transitions,
    setup_protocol_telemetry = setup_protocol_telemetry,
    validate_environment_setup = validate_environment_setup,
    demonstrate_cli_integration = demonstrate_cli_integration,
    demonstrate_adapter_compliance = demonstrate_adapter_compliance,
    main = main
}
