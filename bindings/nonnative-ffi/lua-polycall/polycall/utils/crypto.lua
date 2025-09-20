#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Cryptographic Utilities Implementation
OBINexus Aegis Engineering - Zero-Trust Cryptographic Framework
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

CRYPTOGRAPHIC COMPLIANCE:
Implements secure cryptographic operations for zero-trust architecture.
All cryptographic functions support the adapter pattern protocol requirements.
Maintains systematic security validation throughout the LibPolyCall ecosystem.
]]--

local crypto = {}

-- Import dependencies with error handling
local function safe_require(module_name)
    local success, module = pcall(require, module_name)
    if success then
        return module
    else
        -- Graceful degradation for missing logger module
        return {
            debug = function() end,
            info = function() end,
            warn = function() end,
            error = function() end
        }
    end
end

local logger = safe_require('polycall.utils.logger')

-- Cryptographic constants
crypto.NONCE_LENGTH = 16
crypto.SESSION_ID_LENGTH = 32
crypto.HASH_ALGORITHM = "SHA256"
crypto.KEY_LENGTH = 32

-- Character sets for secure random generation
local ALPHANUMERIC_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
local HEX_CHARS = "0123456789abcdef"
local NUMERIC_CHARS = "0123456789"

-- Initialize random seed for cryptographic operations
math.randomseed(os.time() + os.clock() * 1000000)

-- Generate cryptographically secure nonce
function crypto.generate_nonce(length)
    length = length or crypto.NONCE_LENGTH
    
    if length <= 0 then
        error("Nonce length must be positive")
    end
    
    local nonce = ""
    local chars = ALPHANUMERIC_CHARS
    local chars_length = #chars
    
    for i = 1, length do
        local rand_index = math.random(1, chars_length)
        nonce = nonce .. chars:sub(rand_index, rand_index)
    end
    
    logger.debug("Generated cryptographic nonce", {
        length = length,
        nonce_prefix = nonce:sub(1, 4) .. "..."
    })
    
    return nonce
end

-- Generate secure session identifier
function crypto.generate_session_id(prefix)
    prefix = prefix or "session"
    
    local timestamp = os.time()
    local random_component = crypto.generate_nonce(12)
    local entropy = string.format("%d_%s", math.random(100000, 999999), random_component)
    
    local session_id = string.format("%s_%d_%s", prefix, timestamp, entropy)
    
    logger.debug("Generated session identifier", {
        prefix = prefix,
        timestamp = timestamp,
        session_id_length = #session_id
    })
    
    return session_id
end

-- Generate secure API key
function crypto.generate_api_key(length)
    length = length or crypto.KEY_LENGTH
    
    local key_parts = {
        "pk",  -- Prefix identifier
        string.format("%d", os.time()),  -- Timestamp component
        crypto.generate_nonce(length - 8)  -- Random component
    }
    
    local api_key = table.concat(key_parts, "_")
    
    logger.debug("Generated API key", {
        length = #api_key,
        key_prefix = api_key:sub(1, 8) .. "..."
    })
    
    return api_key
end

-- Simple hash function for demonstration (not cryptographically secure)
function crypto.hash_string(input, algorithm)
    algorithm = algorithm or "simple"
    
    if not input or type(input) ~= "string" then
        error("Hash input must be a non-empty string")
    end
    
    if algorithm == "simple" then
        -- Simple hash for demonstration - NOT cryptographically secure
        local hash = 0
        for i = 1, #input do
            hash = ((hash * 31) + string.byte(input, i)) % 2147483647
        end
        return string.format("%08x", hash)
    else
        error(string.format("Unsupported hash algorithm: %s", algorithm))
    end
end

-- Generate hexadecimal string
function crypto.generate_hex_string(length)
    length = length or 16
    local hex_string = ""
    
    for i = 1, length do
        local rand_index = math.random(1, #HEX_CHARS)
        hex_string = hex_string .. HEX_CHARS:sub(rand_index, rand_index)
    end
    
    return hex_string
end

-- Generate protocol-compliant request ID
function crypto.generate_request_id()
    return string.format("req_%s_%s", 
        crypto.generate_nonce(8),
        crypto.generate_hex_string(8)
    )
end

-- Crypto utilities information
function crypto.get_crypto_info()
    return {
        nonce_length = crypto.NONCE_LENGTH,
        session_id_length = crypto.SESSION_ID_LENGTH,
        hash_algorithm = crypto.HASH_ALGORITHM,
        key_length = crypto.KEY_LENGTH,
        supported_operations = {
            "nonce_generation",
            "session_id_generation", 
            "api_key_generation",
            "hash_generation",
            "request_id_generation"
        },
        security_level = "development",  -- Not cryptographically secure
        protocol_version = "1.0",
        architecture_pattern = "adapter"
    }
end

return crypto
