#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Configuration Validator
OBINexus Aegis Engineering - Systematic Configuration Validation
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing
]]--

local config_validator = {}

-- Import validation utilities
local validator = require('polycall.utils.validator')

-- Validate protocol configuration
function config_validator.validate_protocol_config(config)
    local schema = {
        required_fields = {
            "polycall_runtime_host",
            "polycall_runtime_port",
            "protocol_version",
            "architecture_pattern"
        },
        field_types = {
            polycall_runtime_host = "string",
            polycall_runtime_port = "number",
            protocol_version = "string",
            architecture_pattern = "string"
        },
        field_constraints = {
            architecture_pattern = { allowed_values = { "adapter" } },
            protocol_version = { pattern = "^%d+%.%d+$" }
        }
    }
    
    return validator.validate_table(config, schema)
end

-- Validate security configuration
function config_validator.validate_security_config(config)
    local required_security_fields = {
        "zero_trust_validation",
        "adapter_pattern_enforced",
        "state_machine_binding"
    }
    
    for _, field in ipairs(required_security_fields) do
        if not config[field] then
            return false, string.format("Required security field missing: %s", field)
        end
    end
    
    return true
end

return config_validator
