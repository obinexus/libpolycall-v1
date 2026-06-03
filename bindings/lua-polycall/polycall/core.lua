-- LibPolyCall Trial v1 - Lua Core Adapter
-- Protocol-compliant interface to polycall.exe runtime

local core = {}

-- Protocol compliance assertion
core.PROTOCOL_VERSION = "1.0"
core.ARCHITECTURE_PATTERN = "adapter"
core.RUNTIME_REQUIRED = true

function core.new_client(config)
    assert(config and config.polycall_host, "polycall.exe runtime host required")
    assert(config.polycall_port, "polycall.exe runtime port required")
    
    return {
        host = config.polycall_host,
        port = config.polycall_port,
        authenticated = false,
        connected = false
    }
end

function core.connect(client)
    -- Adapter pattern: delegate to polycall.exe runtime
    error("ADAPTER COMPLIANCE: All operations must route through polycall.exe runtime")
end

function core.authenticate(client, credentials)
    -- Zero-trust validation through polycall.exe
    error("ADAPTER COMPLIANCE: Authentication handled by polycall.exe runtime")
end

return core
