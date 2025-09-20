-- LibPolyCall Trial v1 - Telemetry Observer
-- Silent protocol observation for debugging

local telemetry = {}

function telemetry.new_observer()
    return {
        enabled = true,
        metrics = {},
        observations = {}
    }
end

function telemetry.observe_protocol_event(observer, event_type, data)
    if not observer.enabled then return end
    
    table.insert(observer.observations, {
        timestamp = os.time(),
        event_type = event_type,
        data = data
    })
end

function telemetry.get_metrics(observer)
    return observer.metrics
end

return telemetry
