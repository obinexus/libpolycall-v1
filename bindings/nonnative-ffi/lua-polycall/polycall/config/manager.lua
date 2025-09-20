#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Configuration Management System
OBINexus Aegis Engineering - Systematic Configuration Framework
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

CONFIGURATION METHODOLOGY:
Implements systematic configuration management following waterfall engineering principles.
Supports hierarchical configuration loading with environment variable resolution.
Maintains protocol compliance validation throughout configuration lifecycle.
]]--

local manager = {}

-- Import dependencies
local logger = require('polycall.utils.logger')
local validator = require('polycall.utils.validator')

-- Configuration constants
manager.DEFAULT_CONFIG_PATHS = {
    "lua.polycallrc",
    "config/lua-polycall.conf",
    "/etc/polycall/lua.polycallrc",
    os.getenv("HOME") .. "/.polycallrc"
}

manager.ENVIRONMENT_PREFIX = "POLYCALL_"

-- Default configuration template
manager.DEFAULT_CONFIG = {
    -- Server binding configuration
    port = "3004:8085",
    server_type = "lua",
    workspace = "/opt/polycall/services/lua",
    log_level = "info",
    max_connections = 50,
    
    -- Protocol compliance settings
    adapter_pattern_enforced = true,
    runtime_dependency_required = true,
    zero_trust_validation = true,
    state_machine_binding = true,
    telemetry_integration = true,
    
    -- LibPolyCall protocol configuration
    polycall_runtime_host = "localhost",
    polycall_runtime_port = 8084,
    protocol_version = "1.0",
    architecture_pattern = "adapter",
    
    -- Security settings
    allow_remote = false,
    require_auth = true,
    strict_port_binding = true,
    ssh_cert_required = true,
    auth_method = "ssh_certificate",
    session_timeout = 3600,
    
    -- Performance settings
    max_memory = "512M",
    timeout = 30,
    gc_mode = "incremental",
    
    -- Development environment settings
    debug_mode = false,
    verbose_logging = false,
    stack_trace_enabled = true,
    profiling_enabled = false
}

-- Configuration manager constructor
function manager.new()
    local config_manager = {
        config = {},
        loaded_files = {},
        environment_overrides = {},
        validation_errors = {},
        last_loaded = nil
    }
    
    setmetatable(config_manager, { __index = manager })
    
    -- Initialize with default configuration
    config_manager.config = manager.deep_copy(manager.DEFAULT_CONFIG)
    
    logger.debug("Configuration manager initialized with defaults")
    
    return config_manager
end

-- Load configuration from file
function manager:load_from_file(config_path)
    if not config_path then
        return self:load_from_default_paths()
    end
    
    logger.info("Loading configuration from file", { path = config_path })
    
    -- Validate file path
    local path_valid, path_error = validator.validate_file_path(config_path)
    if not path_valid then
        local error_msg = string.format("Invalid config path: %s", path_error)
        table.insert(self.validation_errors, error_msg)
        return false, error_msg
    end
    
    -- Attempt to read file
    local file_handle, file_error = io.open(config_path, "r")
    if not file_handle then
        local error_msg = string.format("Cannot open config file: %s", file_error)
        table.insert(self.validation_errors, error_msg)
        return false, error_msg
    end
    
    -- Read configuration content
    local config_content = file_handle:read("*a")
    file_handle:close()
    
    -- Parse configuration
    local parsed_config, parse_error = self:parse_config_content(config_content)
    if not parsed_config then
        local error_msg = string.format("Configuration parse error: %s", parse_error)
        table.insert(self.validation_errors, error_msg)
        return false, error_msg
    end
    
    -- Merge with existing configuration
    self:merge_config(parsed_config)
    
    -- Record loaded file
    table.insert(self.loaded_files, {
        path = config_path,
        timestamp = os.time()
    })
    
    self.last_loaded = os.time()
    
    logger.success("Configuration loaded successfully", { 
        path = config_path,
        config_keys = self:count_table_keys(parsed_config)
    })
    
    return true
end

-- Load configuration from default paths
function manager:load_from_default_paths()
    logger.info("Loading configuration from default paths")
    
    local loaded_count = 0
    
    for _, config_path in ipairs(manager.DEFAULT_CONFIG_PATHS) do
        local success, error_msg = self:load_from_file(config_path)
        if success then
            loaded_count = loaded_count + 1
        else
            logger.debug("Skipped config path", { path = config_path, error = error_msg })
        end
    end
    
    if loaded_count == 0 then
        logger.warn("No configuration files found, using defaults")
    else
        logger.success(string.format("Loaded configuration from %d file(s)", loaded_count))
    end
    
    return loaded_count > 0
end

-- Parse configuration content (INI format)
function manager:parse_config_content(content)
    local config = {}
    local current_section = nil
    
    for line in content:gmatch("[^\r\n]+") do
        -- Remove whitespace
        line = line:match("^%s*(.-)%s*$")
        
        -- Skip empty lines and comments
        if line == "" or line:match("^#") then
            goto continue
        end
        
        -- Check for section header
        local section = line:match("^%[(.+)%]$")
        if section then
            current_section = section
            if not config[section] then
                config[section] = {}
            end
            goto continue
        end
        
        -- Parse key-value pair
        local key, value = line:match("^([^=]+)=(.*)$")
        if key and value then
            key = key:match("^%s*(.-)%s*$")  -- Trim whitespace
            value = value:match("^%s*(.-)%s*$")  -- Trim whitespace
            
            -- Convert value to appropriate type
            value = self:convert_config_value(value)
            
            if current_section then
                config[current_section][key] = value
            else
                config[key] = value
            end
        end
        
        ::continue::
    end
    
    -- Flatten sectioned configuration
    return self:flatten_config(config)
end

-- Convert string value to appropriate type
function manager:convert_config_value(value)
    -- Boolean conversion
    if value:lower() == "true" then
        return true
    elseif value:lower() == "false" then
        return false
    end
    
    -- Number conversion
    local number_value = tonumber(value)
    if number_value then
        return number_value
    end
    
    -- String value (remove quotes if present)
    if value:match('^".*"$') or value:match("^'.*'$") then
        return value:sub(2, -2)
    end
    
    return value
end

-- Flatten sectioned configuration
function manager:flatten_config(sectioned_config)
    local flattened = {}
    
    for section_name, section_data in pairs(sectioned_config) do
        if type(section_data) == "table" then
            for key, value in pairs(section_data) do
                local flattened_key = string.format("%s_%s", section_name, key)
                flattened[flattened_key] = value
            end
        else
            flattened[section_name] = section_data
        end
    end
    
    return flattened
end

-- Load environment variable overrides
function manager:load_environment_overrides()
    logger.debug("Loading environment variable overrides")
    
    local override_count = 0
    
    -- Scan environment variables with POLYCALL_ prefix
    for env_var, env_value in pairs(os.getenv() or {}) do
        if env_var:match("^" .. manager.ENVIRONMENT_PREFIX) then
            local config_key = env_var:sub(#manager.ENVIRONMENT_PREFIX + 1):lower()
            local converted_value = self:convert_config_value(env_value)
            
            self.environment_overrides[config_key] = converted_value
            self.config[config_key] = converted_value
            
            override_count = override_count + 1
            
            logger.debug("Environment override applied", {
                env_var = env_var,
                config_key = config_key,
                value_type = type(converted_value)
            })
        end
    end
    
    if override_count > 0 then
        logger.info(string.format("Applied %d environment variable overrides", override_count))
    end
    
    return override_count
end

-- Merge configuration data
function manager:merge_config(new_config)
    for key, value in pairs(new_config) do
        self.config[key] = value
    end
end

-- Get configuration value
function manager:get(key, default_value)
    local value = self.config[key]
    if value ~= nil then
        return value
    end
    return default_value
end

-- Set configuration value
function manager:set(key, value)
    self.config[key] = value
    logger.debug("Configuration value set", { key = key, value_type = type(value) })
end

-- Validate configuration
function manager:validate_config()
    logger.info("Validating configuration")
    
    self.validation_errors = {}
    
    -- Validate required fields
    local required_fields = {
        "polycall_runtime_host",
        "polycall_runtime_port",
        "protocol_version",
        "architecture_pattern"
    }
    
    for _, field in ipairs(required_fields) do
        if not self.config[field] then
            local error_msg = string.format("Required configuration field missing: %s", field)
            table.insert(self.validation_errors, error_msg)
        end
    end
    
    -- Validate protocol compliance settings
    if not self.config.adapter_pattern_enforced then
        table.insert(self.validation_errors, "Adapter pattern enforcement is required")
    end
    
    if self.config.architecture_pattern ~= "adapter" then
        table.insert(self.validation_errors, "Architecture pattern must be 'adapter'")
    end
    
    -- Validate network configuration
    if self.config.polycall_runtime_host and self.config.polycall_runtime_port then
        local endpoint_valid, endpoint_error = validator.validate_endpoint(
            self.config.polycall_runtime_host,
            self.config.polycall_runtime_port
        )
        if not endpoint_valid then
            table.insert(self.validation_errors, endpoint_error)
        end
    end
    
    local is_valid = #self.validation_errors == 0
    
    if is_valid then
        logger.success("Configuration validation passed")
    else
        logger.error("Configuration validation failed", {
            error_count = #self.validation_errors
        })
        for _, error in ipairs(self.validation_errors) do
            logger.error("Validation error: " .. error)
        end
    end
    
    return is_valid, self.validation_errors
end

-- Get full configuration
function manager:get_config()
    return manager.deep_copy(self.config)
end

-- Get configuration summary
function manager:get_summary()
    return {
        loaded_files = #self.loaded_files,
        environment_overrides = self:count_table_keys(self.environment_overrides),
        validation_errors = #self.validation_errors,
        last_loaded = self.last_loaded,
        config_keys = self:count_table_keys(self.config),
        
        -- Key configuration values
        runtime_host = self.config.polycall_runtime_host,
        runtime_port = self.config.polycall_runtime_port,
        protocol_version = self.config.protocol_version,
        architecture_pattern = self.config.architecture_pattern,
        adapter_pattern_enforced = self.config.adapter_pattern_enforced
    }
end

-- Utility functions
function manager:count_table_keys(table)
    local count = 0
    for _ in pairs(table) do
        count = count + 1
    end
    return count
end

function manager.deep_copy(original)
    local copy = {}
    for key, value in pairs(original) do
        if type(value) == "table" then
            copy[key] = manager.deep_copy(value)
        else
            copy[key] = value
        end
    end
    return copy
end

return manager
