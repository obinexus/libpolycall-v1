#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Lua Main Module Entry Point
OBINexus Aegis Engineering - Protocol-Compliant Package Architecture
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

CRITICAL PROTOCOL COMPLIANCE:
This module acts as the main entry point for the LibPolyCall Lua adapter binding.
All execution must route through polycall.exe runtime following program-first architecture.
]]--

local polycall = {}

-- Protocol compliance metadata
polycall.VERSION = "1.0.0"
polycall.PROTOCOL_VERSION = "1.0"
polycall.ARCHITECTURE_PATTERN = "adapter"
polycall.RUNTIME_DEPENDENCY = "polycall.exe"
polycall.AUTHOR = "Nnamdi Michael Okpala - OBINexusComputing"
polycall.PROJECT = "libpolycall-v1trial"

-- Load core modules with error handling
local function safe_require(module_name)
    local success, module = pcall(require, module_name)
    if not success then
        error(string.format("Failed to load required module: %s - %s", module_name, module))
    end
    return module
end

-- Core component imports
polycall.core = {
    binding = safe_require('polycall.core.binding'),
    protocol = safe_require('polycall.core.protocol'),
    state = safe_require('polycall.core.state'),
    telemetry = safe_require('polycall.core.telemetry'),
    auth = safe_require('polycall.core.auth')
}

-- Configuration management
polycall.config = {
    manager = safe_require('polycall.config.manager'),
    validator = safe_require('polycall.config.validator')
}

-- CLI framework
polycall.cli = {
    main = safe_require('polycall.cli.main'),
    registry = safe_require('polycall.cli.registry')
}

-- Utilities
polycall.utils = {
    logger = safe_require('polycall.utils.logger'),
    validator = safe_require('polycall.utils.validator'),
    crypto = safe_require('polycall.utils.crypto')
}

-- Exception handling
polycall.exceptions = {
    protocol = safe_require('polycall.exceptions.protocol'),
    connection = safe_require('polycall.exceptions.connection')
}

-- Validation framework
polycall.validators = {
    setup = safe_require('polycall.validators.setup'),
    runtime = safe_require('polycall.validators.runtime'),
    ssh = safe_require('polycall.validators.ssh')
}

-- Protocol compliance validation on module load
local function validate_protocol_compliance()
    local logger = polycall.utils.logger
    local validator = polycall.validators.setup
    
    logger.info("Validating LibPolyCall Lua adapter protocol compliance...")
    
    -- Validate setup completion
    if not validator.validate_environment() then
        error("Protocol compliance validation failed: setup-lua-polycall.sh not executed")
    end
    
    -- Validate adapter pattern enforcement
    if polycall.ARCHITECTURE_PATTERN ~= "adapter" then
        error("Protocol violation: adapter pattern not enforced")
    end
    
    logger.info("Protocol compliance validation successful")
end

-- Main client factory function
function polycall.new_client(config)
    validate_protocol_compliance()
    
    local client_config = config or {}
    
    -- Set default configuration values
    client_config.polycall_host = client_config.polycall_host or "localhost"
    client_config.polycall_port = client_config.polycall_port or 8084
    client_config.protocol_version = polycall.PROTOCOL_VERSION
    client_config.architecture_pattern = polycall.ARCHITECTURE_PATTERN
    
    -- Create and return protocol binding client
    return polycall.core.binding.new_client(client_config)
end

-- CLI entry point
function polycall.main(args)
    return polycall.cli.main.run(args or arg)
end

-- Protocol information
function polycall.get_info()
    return {
        version = polycall.VERSION,
        protocol_version = polycall.PROTOCOL_VERSION,
        architecture = polycall.ARCHITECTURE_PATTERN,
        runtime_dependency = polycall.RUNTIME_DEPENDENCY,
        author = polycall.AUTHOR,
        project = polycall.PROJECT,
        adapter_pattern_enforced = true,
        zero_trust_compliant = true,
        state_machine_binding = true,
        telemetry_integrated = true
    }
end

-- Module initialization
validate_protocol_compliance()

return polycall
