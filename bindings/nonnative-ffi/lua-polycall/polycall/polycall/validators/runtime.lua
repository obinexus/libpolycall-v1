#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Runtime Validator
OBINexus Aegis Engineering - Runtime Environment Validation
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing
]]--

local runtime_validator = {}

-- Validate polycall.exe runtime availability
function runtime_validator.validate_runtime()
    local success, result = pcall(function()
        local handle = io.popen("which polycall.exe 2>/dev/null")
        local path = handle:read("*l")
        handle:close()
        return path
    end)
    
    return success and result and result ~= ""
end

-- Get runtime version
function runtime_validator.get_runtime_version()
    local success, result = pcall(function()
        local handle = io.popen("polycall.exe --version 2>/dev/null")
        local version_output = handle:read("*l")
        handle:close()
        return version_output
    end)
    
    if success and result then
        return result:match("polycall ([%d%.]+)") or result
    else
        return "unknown"
    end
end

return runtime_validator
