"""
Core Protocol Binding - Clean Architecture Implementation
Maps all user operations to polycall.exe runtime
"""

import asyncio
import logging
from typing import Any, Dict, Optional, Callable
from ..exceptions import ProtocolError, RuntimeError as PyPolyCallRuntimeError
from ..utils import Logger, Validator

logger = Logger.get_logger(__name__)

class ProtocolBinding:
    """
    Core Protocol Binding - Clean Architecture Pattern
    
    RESPONSIBILITIES:
    - Map user operations to polycall.exe runtime
    - Maintain protocol state consistency
    - Enforce zero-trust validation
    - Provide clean interface for upper layers
    """
    
    def __init__(self, 
                 polycall_host: str = "localhost",
                 polycall_port: int = 8084,
                 binding_config: Optional[Dict[str, Any]] = None):
        """
        Initialize Core Protocol Binding
        
        Args:
            polycall_host: polycall.exe runtime host
            polycall_port: polycall.exe runtime port
            binding_config: Core binding configuration
        """
        # Validate inputs
        Validator.validate_host(polycall_host)
        Validator.validate_port(polycall_port)
        
        self.polycall_host = polycall_host
        self.polycall_port = polycall_port
        self.config = binding_config or {}
        
        # Core components - Clean Architecture Dependencies
        self._protocol_handler = None
        self._telemetry_observer = None
        self._ffi_bridge = None
        self._state_manager = None
        
        # Connection state
        self._connected = False
        self._authenticated = False
        self._runtime_version = None
        
        logger.info(f"ProtocolBinding initialized for {polycall_host}:{polycall_port}")
    
    async def initialize(self) -> None:
        """Initialize core components with dependency injection"""
        from .protocol import ProtocolHandler
        from .telemetry import TelemetryObserver
        from .ffi import FFIBridge
        from .state import StateManager
        
        self._protocol_handler = ProtocolHandler(
            host=self.polycall_host,
            port=self.polycall_port
        )
        
        self._telemetry_observer = TelemetryObserver()
        self._ffi_bridge = FFIBridge()
        self._state_manager = StateManager()
        
        logger.info("Core components initialized")
    
    async def connect(self) -> bool:
        """
        Connect to polycall.exe runtime with full protocol handshake
        
        Returns:
            bool: Connection success with version verification
        """
        if not self._protocol_handler:
            await self.initialize()
        
        try:
            # Protocol handshake sequence
            await self._protocol_handler.connect()
            
            # Verify runtime version compatibility
            runtime_info = await self._protocol_handler.get_runtime_info()
            self._runtime_version = runtime_info.get('version')
            
            if not Validator.validate_runtime_version(self._runtime_version):
                raise ProtocolError(f"Incompatible runtime version: {self._runtime_version}")
            
            self._connected = True
            
            # Initialize telemetry observation
            await self._telemetry_observer.start_observation(self._protocol_handler)
            
            logger.info(f"Connected to polycall.exe v{self._runtime_version}")
            return True
            
        except Exception as e:
            logger.error(f"Connection failed: {e}")
            raise PyPolyCallRuntimeError(f"Failed to connect to polycall.exe: {e}")
    
    async def authenticate(self, credentials: Dict[str, Any]) -> bool:
        """
        Authenticate with polycall.exe runtime using zero-trust validation
        
        Args:
            credentials: Authentication credentials
            
        Returns:
            bool: Authentication success
        """
        if not self._connected:
            raise PyPolyCallRuntimeError("Must connect before authentication")
        
        try:
            # Zero-trust authentication through protocol handler
            auth_result = await self._protocol_handler.authenticate(credentials)
            
            if auth_result.success:
                self._authenticated = True
                
                # Initialize FFI bridge after authentication
                await self._ffi_bridge.initialize(self._protocol_handler)
                
                # Start state management
                await self._state_manager.initialize(self._protocol_handler)
                
                logger.info("Authentication successful - Runtime ready")
                return True
            else:
                logger.warning("Authentication failed")
                return False
                
        except Exception as e:
            logger.error(f"Authentication error: {e}")
            raise PyPolyCallRuntimeError(f"Authentication failed: {e}")
    
    async def execute_operation(self, operation: str, params: Dict[str, Any]) -> Any:
        """
        Execute operation through polycall.exe runtime
        
        Args:
            operation: Operation identifier
            params: Operation parameters
            
        Returns:
            Any: Operation result from runtime
        """
        if not self._authenticated:
            raise PyPolyCallRuntimeError("Must authenticate before operation execution")
        
        # All operations go through protocol handler - NO BYPASS
        return await self._protocol_handler.execute_operation(operation, params)
    
    async def shutdown(self) -> None:
        """Clean shutdown of all core components"""
        try:
            if self._telemetry_observer:
                await self._telemetry_observer.stop_observation()
            
            if self._state_manager:
                await self._state_manager.shutdown()
            
            if self._ffi_bridge:
                await self._ffi_bridge.shutdown()
            
            if self._protocol_handler:
                await self._protocol_handler.disconnect()
            
            self._connected = False
            self._authenticated = False
            
            logger.info("Core binding shutdown complete")
            
        except Exception as e:
            logger.error(f"Shutdown error: {e}")
    
    # Properties for clean architecture access
    @property
    def is_connected(self) -> bool:
        """Check runtime connection status"""
        return self._connected
    
    @property
    def is_authenticated(self) -> bool:
        """Check authentication status"""
        return self._authenticated
    
    @property
    def runtime_version(self) -> Optional[str]:
        """Get connected runtime version"""
        return self._runtime_version
    
    @property
    def telemetry(self):
        """Access telemetry observer"""
        return self._telemetry_observer
    
    @property
    def state_manager(self):
        """Access state manager"""
        return self._state_manager
    
    @property
    def ffi_bridge(self):
        """Access FFI bridge"""
        return self._ffi_bridge
