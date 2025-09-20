"""
Telemetry Observer - Silent Protocol Observation
Maps telemetry collection to polycall.exe telemetry system
"""

import asyncio
import time
from typing import Dict, Any, Optional, Callable
from ...utils import Logger

logger = Logger.get_logger(__name__)

class TelemetryObserver:
    """
    Silent Protocol Observer for polycall.exe Runtime
    
    RESPONSIBILITIES:
    - Silent observation of protocol communications
    - Real-time metrics collection
    - State transition tracking
    - Error pattern analysis
    """
    
    def __init__(self):
        self._observing = False
        self._protocol_handler = None
        self._metrics_collector = None
        self._event_handlers: Dict[str, Callable] = {}
        
    async def start_observation(self, protocol_handler) -> None:
        """Start silent protocol observation"""
        from .metrics import MetricsCollector
        
        self._protocol_handler = protocol_handler
        self._metrics_collector = MetricsCollector()
        
        # Register protocol event handlers
        await self._register_protocol_handlers()
        
        self._observing = True
        logger.info("Telemetry observation started")
    
    async def _register_protocol_handlers(self) -> None:
        """Register handlers for protocol events"""
        # State transition observation
        self._protocol_handler.on_state_change(self._on_state_transition)
        
        # Request/response observation
        self._protocol_handler.on_request(self._on_request)
        self._protocol_handler.on_response(self._on_response)
        
        # Error observation
        self._protocol_handler.on_error(self._on_error)
    
    async def _on_state_transition(self, from_state: str, to_state: str) -> None:
        """Handle state transition events"""
        if self._metrics_collector:
            await self._metrics_collector.record_state_transition(from_state, to_state)
        
        logger.debug(f"State transition observed: {from_state} -> {to_state}")
    
    async def _on_request(self, request_data: Dict[str, Any]) -> None:
        """Handle request events"""
        if self._metrics_collector:
            await self._metrics_collector.record_request(request_data)
    
    async def _on_response(self, response_data: Dict[str, Any]) -> None:
        """Handle response events"""
        if self._metrics_collector:
            await self._metrics_collector.record_response(response_data)
    
    async def _on_error(self, error_data: Dict[str, Any]) -> None:
        """Handle error events"""
        if self._metrics_collector:
            await self._metrics_collector.record_error(error_data)
        
        logger.warning(f"Protocol error observed: {error_data}")
    
    async def stop_observation(self) -> None:
        """Stop telemetry observation"""
        self._observing = False
        
        if self._metrics_collector:
            await self._metrics_collector.flush_metrics()
        
        logger.info("Telemetry observation stopped")
    
    def get_metrics(self) -> Dict[str, Any]:
        """Get collected metrics"""
        if self._metrics_collector:
            return self._metrics_collector.get_current_metrics()
        return {}
