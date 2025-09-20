"""
Validation utilities
"""

import re

class ValidationError(Exception):
    """Validation error exception"""
    pass

class Validator:
    """Input validation utilities"""
    
    @staticmethod
    def validate_host(host: str) -> None:
        """Validate host parameter"""
        if not host or not isinstance(host, str):
            raise ValidationError("Host must be a non-empty string")
        
        if not re.match(r'^[a-zA-Z0-9.-]+$', host):
            raise ValidationError(f"Invalid host format: {host}")
    
    @staticmethod
    def validate_port(port: int) -> None:
        """Validate port parameter"""
        if not isinstance(port, int) or port < 1 or port > 65535:
            raise ValidationError(f"Port must be between 1 and 65535, got: {port}")
    
    @staticmethod
    def validate_runtime_version(version: str) -> bool:
        """Validate runtime version"""
        if not version:
            return False
        return re.match(r'^\d+\.\d+\.\d+', version) is not None
