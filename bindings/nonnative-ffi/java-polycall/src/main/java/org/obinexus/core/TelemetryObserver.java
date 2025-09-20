package org.obinexus.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import java.time.Instant;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Silent protocol observation for debugging and metrics
 */
public class TelemetryObserver {
    private static final Logger logger = LoggerFactory.getLogger(TelemetryObserver.class);
    
    private final Map<String, AtomicLong> eventCounters = new ConcurrentHashMap<>();
    private final Map<String, Object> metrics = new ConcurrentHashMap<>();
    private boolean observationEnabled = true;
    
    public void recordEvent(String eventType) {
        recordEvent(eventType, null);
    }
    
    public void recordEvent(String eventType, Object metadata) {
        if (!observationEnabled) return;
        
        eventCounters.computeIfAbsent(eventType, k -> new AtomicLong(0)).incrementAndGet();
        
        logger.trace("Telemetry event: {} at {}", eventType, Instant.now());
        
        if (metadata != null) {
            metrics.put(eventType + "_latest", metadata);
        }
    }
    
    public Map<String, Object> getMetrics() {
        Map<String, Object> result = new ConcurrentHashMap<>(metrics);
        eventCounters.forEach((key, value) -> result.put(key + "_count", value.get()));
        return result;
    }
    
    public void enableObservation() {
        this.observationEnabled = true;
    }
    
    public void disableObservation() {
        this.observationEnabled = false;
    }
}
