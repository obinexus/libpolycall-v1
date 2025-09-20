#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Logging Framework Implementation
OBINexus Aegis Engineering - Systematic Logging Architecture
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

Waterfall Methodology Logging Strategy:
Structured logging framework designed for protocol compliance monitoring,
telemetry integration, and systematic debugging within the adapter pattern architecture.
]]--

local logger = {}

-- ANSI color codes for terminal output
local colors = {
    RESET = '\027[0m',
    RED = '\027[0;31m',
    GREEN = '\027[0;32m',
    YELLOW = '\027[1;33m',
    BLUE = '\027[0;34m',
    MAGENTA = '\027[0;35m',
    CYAN = '\027[0;36m',
    WHITE = '\027[1;37m',
    GRAY = '\027[0;90m'
}

-- Log levels with priority ordering
local log_levels = {
    DEBUG = { priority = 1, color = colors.GRAY, label = "DEBUG" },
    INFO = { priority = 2, color = colors.BLUE, label = "INFO" },
    WARN = { priority = 3, color = colors.YELLOW, label = "WARN" },
    ERROR = { priority = 4, color = colors.RED, label = "ERROR" },
    SUCCESS = { priority = 2, color = colors.GREEN, label = "SUCCESS" },
    PROTOCOL = { priority = 2, color = colors.MAGENTA, label = "PROTOCOL" },
    TELEMETRY = { priority = 1, color = colors.CYAN, label = "TELEMETRY" }
}

-- Logger configuration
local config = {
    level = "INFO",
    colored_output = true,
    timestamp_format = "%Y-%m-%d %H:%M:%S",
    include_caller = false,
    protocol_events_enabled = true,
    telemetry_events_enabled = true
}

-- Set logging level
function logger.set_level(level)
    if log_levels[level] then
        config.level = level
    else
        logger.warn(string.format("Invalid log level '%s', using INFO", level))
        config.level = "INFO"
    end
end

-- Configure colored output
function logger.set_colored_output(enabled)
    config.colored_output = enabled
end

-- Get current log level priority
local function get_current_level_priority()
    return log_levels[config.level].priority
end

-- Check if message should be logged
local function should_log(level)
    return log_levels[level].priority >= get_current_level_priority()
end

-- Format timestamp
local function format_timestamp()
    return os.date(config.timestamp_format)
end

-- Get caller information (optional)
local function get_caller_info()
    if not config.include_caller then
        return ""
    end
    
    local info = debug.getinfo(4, "Sl")
    if info then
        local source = info.source:match("@(.+)") or info.source
        local filename = source:match("([^/\\]+)$") or source
        return string.format(" [%s:%d]", filename, info.currentline)
    end
    
    return ""
end

-- Core logging function
local function log_message(level, message, context)
    if not should_log(level) then
        return
    end
    
    local level_info = log_levels[level]
    local timestamp = format_timestamp()
    local caller_info = get_caller_info()
    
    -- Format base message
    local formatted_message
    if config.colored_output then
        formatted_message = string.format("%s[%s]%s %s%s %s",
            level_info.color, level_info.label, colors.RESET,
            colors.GRAY, timestamp, colors.RESET,
            message
        )
    else
        formatted_message = string.format("[%s] %s %s",
            level_info.label, timestamp, message
        )
    end
    
    -- Add caller information if enabled
    if caller_info ~= "" then
        formatted_message = formatted_message .. caller_info
    end
    
    -- Output message
    print(formatted_message)
    
    -- Log context if provided
    if context and type(context) == "table" then
        for key, value in pairs(context) do
            local context_line
            if config.colored_output then
                context_line = string.format("  %s%s:%s %s", 
                    colors.CYAN, key, colors.RESET, tostring(value))
            else
                context_line = string.format("  %s: %s", key, tostring(value))
            end
            print(context_line)
        end
    end
end

-- Standard logging functions
function logger.debug(message, context)
    log_message("DEBUG", message, context)
end

function logger.info(message, context)
    log_message("INFO", message, context)
end

function logger.warn(message, context)
    log_message("WARN", message, context)
end

function logger.error(message, context)
    log_message("ERROR", message, context)
end

function logger.success(message, context)
    log_message("SUCCESS", message, context)
end

-- Protocol-specific logging
function logger.protocol(message, context)
    if config.protocol_events_enabled then
        log_message("PROTOCOL", message, context)
    end
end

-- Telemetry-specific logging
function logger.telemetry(message, context)
    if config.telemetry_events_enabled then
        log_message("TELEMETRY", message, context)
    end
end

-- Protocol compliance logging
function logger.compliance_violation(message, details)
    local context = {
        violation_type = "protocol_compliance",
        architecture_pattern = "adapter",
        runtime_dependency = "polycall.exe"
    }
    
    if details then
        for key, value in pairs(details) do
            context[key] = value
        end
    end
    
    log_message("ERROR", "PROTOCOL COMPLIANCE VIOLATION: " .. message, context)
end

-- Session tracking
function logger.session_start(session_id, client_config)
    logger.protocol("Session started", {
        session_id = session_id,
        host = client_config.polycall_host,
        port = client_config.polycall_port,
        protocol_version = client_config.protocol_version
    })
end

function logger.session_end(session_id, metrics)
    logger.protocol("Session ended", {
        session_id = session_id,
        duration = metrics.duration,
        operations_count = metrics.operations_count,
        telemetry_events = metrics.telemetry_events
    })
end

-- State transition logging
function logger.state_transition(from_state, to_state, context)
    logger.protocol(string.format("State transition: %s -> %s", from_state, to_state), context)
end

-- Operation execution logging
function logger.operation_start(operation, params)
    logger.protocol(string.format("Operation started: %s", operation), {
        operation = operation,
        params_count = params and logger.count_table_keys(params) or 0,
        timestamp = os.time()
    })
end

function logger.operation_complete(operation, duration, success)
    local status = success and "SUCCESS" or "FAILED"
    logger.protocol(string.format("Operation completed: %s [%s]", operation, status), {
        operation = operation,
        duration_ms = duration,
        success = success,
        timestamp = os.time()
    })
end

-- Utility functions
function logger.count_table_keys(table)
    local count = 0
    for _ in pairs(table) do
        count = count + 1
    end
    return count
end

-- Configuration management
function logger.get_config()
    return {
        level = config.level,
        colored_output = config.colored_output,
        timestamp_format = config.timestamp_format,
        include_caller = config.include_caller,
        protocol_events_enabled = config.protocol_events_enabled,
        telemetry_events_enabled = config.telemetry_events_enabled
    }
end

function logger.configure(new_config)
    for key, value in pairs(new_config) do
        if config[key] ~= nil then
            config[key] = value
        else
            logger.warn(string.format("Unknown configuration option: %s", key))
        end
    end
    
    logger.info("Logger configuration updated", logger.get_config())
end

return logger
