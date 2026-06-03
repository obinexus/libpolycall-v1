#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Cryptographic Utilities
OBINexus Aegis Engineering - Zero-Trust Cryptographic Operations
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing
]]--

local crypto = {}

-- Generate cryptographic nonce
function crypto.generate_nonce(length)
    length = length or 16
    local chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    local nonce = ""
    
    math.randomseed(os.time())
    
    for i = 1, length do
        local rand_index = math.random(1, #chars)
        nonce = nonce .. chars:sub(rand_index, rand_index)
    end
    
    return nonce
end

-- Generate session ID
function crypto.generate_session_id()
    return string.format("session_%d_%s", os.time(), crypto.generate_nonce(8))
end

-- Basic hash function (demonstration)
function crypto.hash_string(input)
    -- Simple hash for demonstration - not cryptographically secure
    local hash = 0
    for i = 1, #input do
        hash = (hash * 31 + string.byte(input, i)) % 2147483647
    end
    return string.format("%08x", hash)
end

return crypto
