"""
Validation Utilities
Input validation and protocol compliance checking
"""

import re
from typing import Any, Optional
from ..exceptions import ValidationError

class Validator:
    """
    Unified Validation System
    
    RESPONSIBILITIES:
    - Input parameter validation
    - Protocol compliance checking
    - Configuration validation
    """
    
    @staticmethod
    def validate_host(host: str) -> None:
        """Validate host parameter"""
        if not host or not isinstance(host, str):
            raise ValidationError("Host must be a non-empty string")
        
        # Basic host validation (IPv4, IPv6, hostname)
        if not re.match(r'^[a-zA-Z0-9.-]+$', host):
            raise ValidationError(f"Invalid host format: {host}")
    
    @staticmethod
    def validate_port(port: int) -> None:
        """Validate port parameter"""
        if not isinstance(port, int) or port < 1 or port > 65535:
            raise ValidationError(f"Port must be between 1 and 65535, got: {port}")
    
    @staticmethod
    def validate_runtime_version(version: Optional[str]) -> bool:
        """Validate polycall.exe runtime version compatibility"""
        if not version:
            return False
        
        # Simple version validation - could be expanded
        return re.match(r'^\d+\.\d+\.\d+', version) is not None
