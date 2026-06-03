#!/usr/bin/env lua

--[[
LibPolyCall Trial v1 - Telemetry Observer Framework
OBINexus Aegis Engineering - Silent Protocol Observation
Technical Lead: Nnamdi Michael Okpala - OBINexusComputing

TELEMETRY COMPLIANCE:
Implements silent observation of protocol events for debugging and monitoring.
All telemetry collection is non-intrusive and maintains zero-trust principles.
]]--

local telemetry = {}

-- Import logging utilities
local logger = require('polycall.utils.logger')

-- Telemetry event types
telemetry.EVENT_TYPES = {
    CONNECTION_ATTEMPT = "connection_attempt",
    CONNECTION_SUCCESS = "connection_success",
    CONNECTION_FAILURE = "connection_failure",
    STATE_TRANSITION = "state_transition",
    AUTHENTICATION_ATTEMPT = "authentication_attempt",
    AUTHENTICATION_SUCCESS = "authentication_success",
    AUTHENTICATION_FAILURE = "authentication_failure",
    OPERATION_START = "operation_start",
    OPERATION_SUCCESS = "operation_success",
    OPERATION_FAILURE = "operation_failure",
    PROTOCOL_ERROR = "protocol_error",
    HEARTBEAT_SENT = "heartbeat_sent",
    SESSION_START = "session_start",
    SESSION_END = "session_end"
}

-- Telemetry observer constructor
function telemetry.new_observer()
    local observer = {
        enabled = true,
        observations = {},
        metrics = {
            total_events = 0,
            events_by_type = {},
            start_time = os.time(),
            last_event_time = nil
        },
        
        -- Configuration
        max_observations = 1000,
        auto_cleanup = true,
        retention_period = 3600, -- 1 hour
        
        -- Event filtering
        enabled_events = {},
        disabled_events = {}
    }
    
    -- Initialize event type counters
    for _, event_type in pairs(telemetry.EVENT_TYPES) do
        observer.metrics.events_by_type[event_type] = 0
    end
    
    logger.telemetry("Telemetry observer initialized", {
        max_observations = observer.max_observations,
        retention_period = observer.retention_period
    })
    
    return observer
end

-- Record protocol event
function telemetry.observe_protocol_event(observer, event_type, event_data)
    if not observer or not observer.enabled then
        return false
    end
    
    -- Validate event type
    if not telemetry.EVENT_TYPES[event_type:upper()] then
        logger.warn(string.format("Unknown telemetry event type: %s", event_type))
        return false
    end
    
    -- Check event filtering
    if observer.disabled_events[event_type] then
        return false
    end
    
    if next(observer.enabled_events) and not observer.enabled_events[event_type] then
        return false
    end
    
    -- Create observation record
    local observation = {
        id = telemetry.generate_observation_id(),
        event_type = event_type,
        timestamp = os.time(),
        data = event_data or {},
        
        -- Protocol metadata
        observer_id = observer.id or "default",
        protocol_version = "1.0",
        architecture_pattern = "adapter"
    }
    
    -- Store observation
    table.insert(observer.observations, observation)
    
    -- Update metrics
    observer.metrics.total_events = observer.metrics.total_events + 1
    observer.metrics.events_by_type[event_type] = (observer.metrics.events_by_type[event_type] or 0) + 1
    observer.metrics.last_event_time = observation.timestamp
    
    -- Log telemetry event
    logger.telemetry(string.format("Protocol event observed: %s", event_type), {
        observation_id = observation.id,
        total_events = observer.metrics.total_events
    })
    
    -- Perform auto-cleanup if enabled
    if observer.auto_cleanup then
        telemetry.cleanup_observations(observer)
    end
    
    return true
end

-- Generate unique observation ID
function telemetry.generate_observation_id()
    return string.format("obs_%d_%d", os.time(), math.random(10000, 99999))
end

-- Get telemetry metrics
function telemetry.get_metrics(observer)
    if not observer then
        return nil
    end
    
    local current_time = os.time()
    local uptime = current_time - observer.metrics.start_time
    
    return {
        total_events = observer.metrics.total_events,
        events_by_type = observer.metrics.events_by_type,
        observations_count = #observer.observations,
        start_time = observer.metrics.start_time,
        last_event_time = observer.metrics.last_event_time,
        uptime = uptime,
        
        -- Performance metrics
        events_per_minute = observer.metrics.total_events > 0 and (observer.metrics.total_events / (uptime / 60)) or 0,
        
        -- Configuration
        enabled = observer.enabled,
        max_observations = observer.max_observations,
        retention_period = observer.retention_period
    }
end

-- Get observations by type
function telemetry.get_observations_by_type(observer, event_type)
    if not observer then
        return {}
    end
    
    local filtered_observations = {}
    
    for _, observation in ipairs(observer.observations) do
        if observation.event_type == event_type then
            table.insert(filtered_observations, observation)
        end
    end
    
    return filtered_observations
end

-- Get recent observations
function telemetry.get_recent_observations(observer, time_window)
    if not observer then
        return {}
    end
    
    time_window = time_window or 300 -- Default 5 minutes
    local cutoff_time = os.time() - time_window
    local recent_observations = {}
    
    for _, observation in ipairs(observer.observations) do
        if observation.timestamp >= cutoff_time then
            table.insert(recent_observations, observation)
        end
    end
    
    return recent_observations
end

-- Cleanup old observations
function telemetry.cleanup_observations(observer)
    if not observer or not observer.auto_cleanup then
        return 0
    end
    
    local current_time = os.time()
    local cutoff_time = current_time - observer.retention_period
    local removed_count = 0
    
    -- Remove old observations
    local filtered_observations = {}
    for _, observation in ipairs(observer.observations) do
        if observation.timestamp >= cutoff_time then
            table.insert(filtered_observations, observation)
        else
            removed_count = removed_count + 1
        end
    end
    
    observer.observations = filtered_observations
    
    -- Remove excess observations if over limit
    local excess_count = #observer.observations - observer.max_observations
    if excess_count > 0 then
        for i = 1, excess_count do
            table.remove(observer.observations, 1) -- Remove oldest
            removed_count = removed_count + 1
        end
    end
    
    if removed_count > 0 then
        logger.telemetry(string.format("Cleaned up %d old observations", removed_count))
    end
    
    return removed_count
end

-- Enable/disable telemetry observer
function telemetry.set_enabled(observer, enabled)
    if not observer then
        return false
    end
    
    observer.enabled = enabled
    
    logger.telemetry(string.format("Telemetry observer %s", enabled and "enabled" or "disabled"))
    
    return true
end

-- Configure event filtering
function telemetry.set_event_filter(observer, enabled_events, disabled_events)
    if not observer then
        return false
    end
    
    observer.enabled_events = enabled_events or {}
    observer.disabled_events = disabled_events or {}
    
    logger.telemetry("Event filtering configured", {
        enabled_count = telemetry.count_table_keys(observer.enabled_events),
        disabled_count = telemetry.count_table_keys(observer.disabled_events)
    })
    
    return true
end

-- Export observations to structured format
function telemetry.export_observations(observer, format)
    if not observer then
        return nil, "Observer required"
    end
    
    format = format or "json"
    
    local export_data = {
        metadata = {
            export_timestamp = os.time(),
            observer_metrics = telemetry.get_metrics(observer),
            format_version = "1.0"
        },
        observations = observer.observations
    }
    
    if format == "json" then
        -- In a real implementation, this would use a JSON library
        return export_data, nil
    elseif format == "csv" then
        -- CSV export implementation would go here
        return "CSV export not implemented", "Not implemented"
    else
        return nil, "Unsupported export format"
    end
end

-- Generate telemetry summary report
function telemetry.generate_summary_report(observer)
    if not observer then
        return nil
    end
    
    local metrics = telemetry.get_metrics(observer)
    local recent_observations = telemetry.get_recent_observations(observer, 300)
    
    local report = {
        summary = {
            total_events = metrics.total_events,
            observations_count = metrics.observations_count,
            uptime_minutes = math.floor(metrics.uptime / 60),
            events_per_minute = string.format("%.2f", metrics.events_per_minute)
        },
        
        event_distribution = metrics.events_by_type,
        
        recent_activity = {
            last_5_minutes = #recent_observations,
            last_event_time = metrics.last_event_time and os.date("%Y-%m-%d %H:%M:%S", metrics.last_event_time) or "never"
        },
        
        configuration = {
            enabled = observer.enabled,
            max_observations = observer.max_observations,
            retention_period_hours = observer.retention_period / 3600,
            auto_cleanup = observer.auto_cleanup
        }
    }
    
    return report
end

-- Utility function to count table keys
function telemetry.count_table_keys(table)
    local count = 0
    for _ in pairs(table) do
        count = count + 1
    end
    return count
end

return telemetry
