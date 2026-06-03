"""
Core Telemetry Layer
"""

class TelemetryObserver:
    """Telemetry observer for protocol monitoring"""
    
    def __init__(self):
        self._observing = False
    
    async def start_observation(self, protocol_handler):
        """Start telemetry observation"""
        self._observing = True
    
    async def stop_observation(self):
        """Stop telemetry observation"""
        self._observing = False

class MetricsCollector:
    """Metrics collection for telemetry"""
    
    def __init__(self):
        self._metrics = {}
    
    def get_current_metrics(self):
        """Get collected metrics"""
        return self._metrics

__all__ = ["TelemetryObserver", "MetricsCollector"]
