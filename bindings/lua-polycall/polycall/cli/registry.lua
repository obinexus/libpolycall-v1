#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - CLI Command Registry Implementation
OBINexus Aegis Engineering - Extensible Command Architecture
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

Waterfall Methodology Command System:
Systematic command registration framework enabling extensible CLI architecture
with plugin-style command modules following established design patterns
from the broader LibPolyCall ecosystem collaboration.
]]--

local registry = {}

-- Internal command storage
local registered_commands = {}

-- Registry metadata
registry.VERSION = "1.0.0"
registry.ARCHITECTURE = "plugin-extensible"

-- Command validation schema
local command_interface = {
    execute = "function",
    get_help = "function"
}

-- Validate command module interface
local function validate_command_module(command_module, command_name)
    if type(command_module) ~= "table" then
        error(string.format("Command '%s' must be a table", command_name))
    end
    
    for method, expected_type in pairs(command_interface) do
        if type(command_module[method]) ~= expected_type then
            error(string.format("Command '%s' must implement %s as %s", 
                command_name, method, expected_type))
        end
    end
    
    return true
end

-- Register a command module
function registry.register(command_name, command_module)
    -- Validate command name
    if type(command_name) ~= "string" or command_name == "" then
        error("Command name must be a non-empty string")
    end
    
    -- Check for existing command
    if registered_commands[command_name] then
        error(string.format("Command '%s' is already registered", command_name))
    end
    
    -- Validate command module interface
    validate_command_module(command_module, command_name)
    
    -- Register command
    registered_commands[command_name] = {
        module = command_module,
        name = command_name,
        registered_at = os.time(),
        version = command_module.VERSION or "unknown"
    }
    
    return true
end

-- Unregister a command
function registry.unregister(command_name)
    if not registered_commands[command_name] then
        return false, string.format("Command '%s' not found", command_name)
    end
    
    registered_commands[command_name] = nil
    return true
end

-- Get a specific command module
function registry.get_command(command_name)
    local command_entry = registered_commands[command_name]
    return command_entry and command_entry.module or nil
end

-- Get all registered commands
function registry.get_registered_commands()
    local commands = {}
    for name, entry in pairs(registered_commands) do
        commands[name] = entry.module
    end
    return commands
end

-- Get command metadata
function registry.get_command_info(command_name)
    local command_entry = registered_commands[command_name]
    if not command_entry then
        return nil, string.format("Command '%s' not found", command_name)
    end
    
    return {
        name = command_entry.name,
        version = command_entry.version,
        registered_at = command_entry.registered_at,
        help_text = command_entry.module.get_help(),
        has_execute = type(command_entry.module.execute) == "function"
    }
end

-- List all available commands
function registry.list_commands()
    local command_list = {}
    for name, entry in pairs(registered_commands) do
        table.insert(command_list, {
            name = name,
            version = entry.version,
            help_text = entry.module.get_help()
        })
    end
    
    -- Sort commands alphabetically
    table.sort(command_list, function(a, b)
        return a.name < b.name
    end)
    
    return command_list
end

-- Check if command exists
function registry.has_command(command_name)
    return registered_commands[command_name] ~= nil
end

-- Get registry statistics
function registry.get_statistics()
    local total_commands = 0
    local oldest_registration = nil
    local newest_registration = nil
    
    for name, entry in pairs(registered_commands) do
        total_commands = total_commands + 1
        
        if not oldest_registration or entry.registered_at < oldest_registration then
            oldest_registration = entry.registered_at
        end
        
        if not newest_registration or entry.registered_at > newest_registration then
            newest_registration = entry.registered_at
        end
    end
    
    return {
        total_commands = total_commands,
        oldest_registration = oldest_registration,
        newest_registration = newest_registration,
        registry_version = registry.VERSION,
        architecture = registry.ARCHITECTURE
    }
end

-- Validate all registered commands
function registry.validate_registry()
    local validation_results = {
        valid_commands = {},
        invalid_commands = {},
        total_validated = 0
    }
    
    for name, entry in pairs(registered_commands) do
        validation_results.total_validated = validation_results.total_validated + 1
        
        local success, error_msg = pcall(function()
            validate_command_module(entry.module, name)
        end)
        
        if success then
            table.insert(validation_results.valid_commands, name)
        else
            table.insert(validation_results.invalid_commands, {
                name = name,
                error = error_msg
            })
        end
    end
    
    return validation_results
end

-- Clear all registered commands (primarily for testing)
function registry.clear()
    registered_commands = {}
    return true
end

-- Export command registry for external inspection
function registry.export_registry()
    local exported = {}
    for name, entry in pairs(registered_commands) do
        exported[name] = {
            name = entry.name,
            version = entry.version,
            registered_at = entry.registered_at,
            help_text = entry.module.get_help()
        }
    end
    return exported
end

return registry
