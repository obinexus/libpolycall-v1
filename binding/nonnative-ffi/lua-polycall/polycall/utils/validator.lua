#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Input Validation Framework
OBINexus Aegis Engineering - Systematic Data Validation
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

VALIDATION METHODOLOGY:
Implements systematic input validation following defensive programming principles.
All external data must be validated before processing within the adapter framework.
]]--

local validator = {}

-- Validation error types
validator.ERROR_TYPES = {
    TYPE_MISMATCH = "type_mismatch",
    REQUIRED_FIELD = "required_field",
    INVALID_VALUE = "invalid_value",
    OUT_OF_RANGE = "out_of_range",
    INVALID_FORMAT = "invalid_format",
    CONSTRAINT_VIOLATION = "constraint_violation"
}

-- Type validation functions
function validator.assert_string(value, error_message)
    if type(value) ~= "string" then
        error(error_message or string.format("Expected string, got %s", type(value)))
    end
    return true
end

function validator.assert_number(value, error_message)
    if type(value) ~= "number" then
        error(error_message or string.format("Expected number, got %s", type(value)))
    end
    return true
end

function validator.assert_table(value, error_message)
    if type(value) ~= "table" then
        error(error_message or string.format("Expected table, got %s", type(value)))
    end
    return true
end

function validator.assert_function(value, error_message)
    if type(value) ~= "function" then
        error(error_message or string.format("Expected function, got %s", type(value)))
    end
    return true
end

function validator.assert_boolean(value, error_message)
    if type(value) ~= "boolean" then
        error(error_message or string.format("Expected boolean, got %s", type(value)))
    end
    return true
end

-- Non-asserting validation functions
function validator.is_string(value)
    return type(value) == "string"
end

function validator.is_number(value)
    return type(value) == "number"
end

function validator.is_table(value)
    return type(value) == "table"
end

function validator.is_function(value)
    return type(value) == "function"
end

function validator.is_boolean(value)
    return type(value) == "boolean"
end

-- String validation
function validator.validate_string(value, options)
    options = options or {}
    
    if not validator.is_string(value) then
        return false, validator.ERROR_TYPES.TYPE_MISMATCH, "Value must be a string"
    end
    
    -- Check required
    if options.required and (not value or value == "") then
        return false, validator.ERROR_TYPES.REQUIRED_FIELD, "String value is required"
    end
    
    -- Check length constraints
    if options.min_length and #value < options.min_length then
        return false, validator.ERROR_TYPES.OUT_OF_RANGE, 
            string.format("String length must be at least %d characters", options.min_length)
    end
    
    if options.max_length and #value > options.max_length then
        return false, validator.ERROR_TYPES.OUT_OF_RANGE,
            string.format("String length must not exceed %d characters", options.max_length)
    end
    
    -- Check pattern matching
    if options.pattern and not value:match(options.pattern) then
        return false, validator.ERROR_TYPES.INVALID_FORMAT,
            string.format("String does not match required pattern: %s", options.pattern)
    end
    
    -- Check allowed values
    if options.allowed_values then
        local valid = false
        for _, allowed_value in ipairs(options.allowed_values) do
            if value == allowed_value then
                valid = true
                break
            end
        end
        if not valid then
            return false, validator.ERROR_TYPES.INVALID_VALUE,
                "String value not in allowed list"
        end
    end
    
    return true
end

-- Number validation
function validator.validate_number(value, options)
    options = options or {}
    
    if not validator.is_number(value) then
        return false, validator.ERROR_TYPES.TYPE_MISMATCH, "Value must be a number"
    end
    
    -- Check required
    if options.required and value == nil then
        return false, validator.ERROR_TYPES.REQUIRED_FIELD, "Number value is required"
    end
    
    -- Check range constraints
    if options.min_value and value < options.min_value then
        return false, validator.ERROR_TYPES.OUT_OF_RANGE,
            string.format("Number must be at least %s", options.min_value)
    end
    
    if options.max_value and value > options.max_value then
        return false, validator.ERROR_TYPES.OUT_OF_RANGE,
            string.format("Number must not exceed %s", options.max_value)
    end
    
    -- Check if integer required
    if options.integer_only and math.floor(value) ~= value then
        return false, validator.ERROR_TYPES.INVALID_VALUE, "Number must be an integer"
    end
    
    -- Check if positive required
    if options.positive_only and value <= 0 then
        return false, validator.ERROR_TYPES.INVALID_VALUE, "Number must be positive"
    end
    
    return true
end

-- Table validation
function validator.validate_table(value, schema)
    if not validator.is_table(value) then
        return false, validator.ERROR_TYPES.TYPE_MISMATCH, "Value must be a table"
    end
    
    if not schema then
        return true -- No schema validation
    end
    
    -- Validate required fields
    if schema.required_fields then
        for _, field_name in ipairs(schema.required_fields) do
            if value[field_name] == nil then
                return false, validator.ERROR_TYPES.REQUIRED_FIELD,
                    string.format("Required field '%s' is missing", field_name)
            end
        end
    end
    
    -- Validate field types
    if schema.field_types then
        for field_name, expected_type in pairs(schema.field_types) do
            local field_value = value[field_name]
            if field_value ~= nil and type(field_value) ~= expected_type then
                return false, validator.ERROR_TYPES.TYPE_MISMATCH,
                    string.format("Field '%s' must be of type %s", field_name, expected_type)
            end
        end
    end
    
    -- Validate field constraints
    if schema.field_constraints then
        for field_name, constraints in pairs(schema.field_constraints) do
            local field_value = value[field_name]
            if field_value ~= nil then
                local field_valid, error_type, error_message = validator.validate_field(field_value, constraints)
                if not field_valid then
                    return false, error_type, string.format("Field '%s': %s", field_name, error_message)
                end
            end
        end
    end
    
    return true
end

-- Field validation based on constraints
function validator.validate_field(value, constraints)
    local value_type = type(value)
    
    if value_type == "string" then
        return validator.validate_string(value, constraints)
    elseif value_type == "number" then
        return validator.validate_number(value, constraints)
    elseif value_type == "table" then
        return validator.validate_table(value, constraints)
    else
        return true -- No specific validation for other types
    end
end

-- Protocol-specific validation
function validator.validate_protocol_message(message)
    local schema = {
        required_fields = { "id", "type", "version", "timestamp" },
        field_types = {
            id = "string",
            type = "string",
            version = "string",
            timestamp = "number",
            payload = "table"
        },
        field_constraints = {
            id = { min_length = 1, max_length = 100 },
            type = { allowed_values = { "handshake", "auth", "operation", "response", "error", "heartbeat", "shutdown" } },
            version = { pattern = "^%d+%.%d+$" },
            timestamp = { min_value = 0 }
        }
    }
    
    return validator.validate_table(message, schema)
end

-- Configuration validation
function validator.validate_client_config(config)
    local schema = {
        required_fields = { "polycall_host", "polycall_port" },
        field_types = {
            polycall_host = "string",
            polycall_port = "number",
            protocol_version = "string",
            architecture_pattern = "string",
            timeout = "number"
        },
        field_constraints = {
            polycall_host = { min_length = 1, max_length = 255 },
            polycall_port = { min_value = 1, max_value = 65535, integer_only = true },
            protocol_version = { pattern = "^%d+%.%d+$" },
            architecture_pattern = { allowed_values = { "adapter" } },
            timeout = { min_value = 1, max_value = 300 }
        }
    }
    
    return validator.validate_table(config, schema)
end

-- Authentication credentials validation
function validator.validate_auth_credentials(credentials)
    local schema = {
        required_fields = { "username" },
        field_types = {
            username = "string",
            api_key = "string",
            auth_method = "string",
            timestamp = "number",
            nonce = "string"
        },
        field_constraints = {
            username = { min_length = 1, max_length = 100, pattern = "^[%w_%-%.]+$" },
            api_key = { min_length = 10 },
            auth_method = { allowed_values = { "ssh_certificate", "api_key", "token", "zero_trust" } },
            timestamp = { min_value = 0 }
        }
    }
    
    return validator.validate_table(credentials, schema)
end

-- Network endpoint validation
function validator.validate_endpoint(host, port)
    local host_valid, _, host_error = validator.validate_string(host, {
        required = true,
        min_length = 1,
        max_length = 255
    })
    
    if not host_valid then
        return false, "Invalid host: " .. host_error
    end
    
    local port_valid, _, port_error = validator.validate_number(port, {
        required = true,
        min_value = 1,
        max_value = 65535,
        integer_only = true
    })
    
    if not port_valid then
        return false, "Invalid port: " .. port_error
    end
    
    return true
end

-- File path validation
function validator.validate_file_path(path)
    if not validator.is_string(path) then
        return false, "File path must be a string"
    end
    
    if #path == 0 then
        return false, "File path cannot be empty"
    end
    
    -- Basic path validation (not platform-specific)
    if path:match("%.%.") then
        return false, "File path contains invalid directory traversal"
    end
    
    return true
end

-- Email validation (basic)
function validator.validate_email(email)
    if not validator.is_string(email) then
        return false, "Email must be a string"
    end
    
    local pattern = "^[%w%._%+-]+@[%w%._%+-]+%.%w+$"
    if not email:match(pattern) then
        return false, "Invalid email format"
    end
    
    return true
end

-- URL validation (basic)
function validator.validate_url(url)
    if not validator.is_string(url) then
        return false, "URL must be a string"
    end
    
    local pattern = "^https?://[%w%._%+-]+"
    if not url:match(pattern) then
        return false, "Invalid URL format"
    end
    
    return true
end

-- Validation result helper
function validator.create_validation_result(success, error_type, error_message, field_name)
    return {
        success = success,
        error_type = error_type,
        error_message = error_message,
        field_name = field_name,
        timestamp = os.time()
    }
end

return validator
