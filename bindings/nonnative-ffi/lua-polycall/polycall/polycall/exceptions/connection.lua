#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Connection Exception Handling
OBINexus Aegis Engineering - Network Connection Error Management
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing
]]--

local connection_exceptions = {}

-- Connection exception types
connection_exceptions.TYPES = {
    TIMEOUT = "connection_timeout",
    REFUSED = "connection_refused",
    HOST_UNREACHABLE = "host_unreachable",
    NETWORK_ERROR = "network_error",
    SSL_ERROR = "ssl_error"
}

-- Create connection exception
function connection_exceptions.create(exception_type, message, host, port)
    return {
        type = exception_type,
        message = message,
        host = host,
        port = port,
        timestamp = os.time()
    }
end

return connection_exceptions
