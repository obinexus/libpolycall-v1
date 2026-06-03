#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - CLI Framework Implementation
OBINexus Aegis Engineering - Command-Line Interface Architecture
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

Waterfall Methodology Implementation:
Systematic command registry with extensible architecture following
collaborative development patterns established in the broader ecosystem.
]]--

local cli = {}

-- Import command modules
local info_command = require('polycall.cli.commands.info')
local test_command = require('polycall.cli.commands.test')
local telemetry_command = require('polycall.cli.commands.telemetry')
local registry = require('polycall.cli.registry')
local logger = require('polycall.utils.logger')

-- CLI metadata
cli.VERSION = "1.0.0"
cli.DESCRIPTION = "LibPolyCall Trial v1 - Lua Adapter CLI"
cli.AUTHOR = "Nnamdi Michael Okpala - OBINexusComputing"

-- Initialize command registry
local function initialize_command_registry()
    -- Register core commands
    registry.register("info", info_command)
    registry.register("test", test_command) 
    registry.register("telemetry", telemetry_command)
    
    logger.debug("CLI command registry initialized with core commands")
end

-- Display help information
local function show_help()
    print(string.format("%s v%s", cli.DESCRIPTION, cli.VERSION))
    print(string.format("Technical Lead: %s", cli.AUTHOR))
    print("Architecture: Adapter Pattern (Program-First)")
    print("")
    print("Usage: lua-polycall <command> [options]")
    print("")
    print("Commands:")
    
    local commands = registry.get_registered_commands()
    for command_name, command_module in pairs(commands) do
        local help_text = command_module.get_help and command_module.get_help() or "No description available"
        print(string.format("  %-12s %s", command_name, help_text))
    end
    
    print("")
    print("Global Options:")
    print("  --verbose    Enable verbose output")
    print("  --debug      Enable debug logging")
    print("  --version    Show version information")
    print("  --help       Show this help message")
    print("")
    print("Protocol Compliance:")
    print("  - All operations route through polycall.exe runtime")
    print("  - Adapter pattern enforced for zero-trust architecture") 
    print("  - State machine transitions validated")
    print("  - Silent telemetry observation enabled")
end

-- Display version information
local function show_version()
    print(string.format("lua-polycall %s", cli.VERSION))
    print("LibPolyCall Trial v1 - Lua Adapter Binding")
    print("Protocol Version: 1.0")
    print("Architecture: Adapter Pattern")
    print(string.format("Author: %s", cli.AUTHOR))
    print("License: MIT")
end

-- Parse global options
local function parse_global_options(args)
    local options = {
        verbose = false,
        debug = false,
        help = false,
        version = false
    }
    
    local filtered_args = {}
    
    for i, arg in ipairs(args) do
        if arg == "--verbose" then
            options.verbose = true
        elseif arg == "--debug" then
            options.debug = true
        elseif arg == "--help" or arg == "-h" then
            options.help = true
        elseif arg == "--version" or arg == "-v" then
            options.version = true
        else
            table.insert(filtered_args, arg)
        end
    end
    
    return options, filtered_args
end

-- Configure logging based on options
local function configure_logging(options)
    if options.debug then
        logger.set_level("DEBUG")
    elseif options.verbose then
        logger.set_level("INFO")
    else
        logger.set_level("WARN")
    end
end

-- Main CLI execution function
function cli.run(args)
    -- Initialize registry
    initialize_command_registry()
    
    -- Handle empty arguments
    if not args or #args == 0 then
        show_help()
        return 0
    end
    
    -- Parse global options
    local options, filtered_args = parse_global_options(args)
    
    -- Configure logging
    configure_logging(options)
    
    -- Handle global options
    if options.help then
        show_help()
        return 0
    end
    
    if options.version then
        show_version()
        return 0
    end
    
    -- Extract command name
    local command_name = filtered_args[1]
    if not command_name then
        logger.error("No command specified")
        show_help()
        return 1
    end
    
    -- Extract command arguments
    local command_args = {}
    for i = 2, #filtered_args do
        table.insert(command_args, filtered_args[i])
    end
    
    -- Execute command
    local command_module = registry.get_command(command_name)
    if not command_module then
        logger.error(string.format("Unknown command: %s", command_name))
        print("")
        show_help()
        return 1
    end
    
    -- Execute command with error handling
    local success, result = pcall(function()
        return command_module.execute(command_args, options)
    end)
    
    if not success then
        logger.error(string.format("Command execution failed: %s", result))
        return 1
    end
    
    -- Return command exit code
    return result or 0
end

-- Command execution context
function cli.create_execution_context(command_args, options)
    return {
        args = command_args or {},
        options = options or {},
        verbose = options and options.verbose or false,
        debug = options and options.debug or false,
        cli_version = cli.VERSION,
        author = cli.AUTHOR
    }
end

return cli
