"""
Exception Hierarchy
Clean exception handling for PyPolyCall architecture
"""

class PyPolyCallError(Exception):
    """Base exception for PyPolyCall"""
    pass

class ProtocolError(PyPolyCallError):
    """Protocol-related errors"""
    pass

class RuntimeError(PyPolyCallError):
    """Runtime connection and execution errors"""
    pass

class ConfigurationError(PyPolyCallError):
    """Configuration-related errors"""
    pass

class ValidationError(PyPolyCallError):
    """Input validation errors"""
    pass

class TelemetryError(PyPolyCallError):
    """Telemetry collection errors"""
    pass

class FFIError(PyPolyCallError):
    """FFI bridge errors"""
    pass

__all__ = [
    "PyPolyCallError",
    "ProtocolError", 
    "RuntimeError",
    "ConfigurationError",
    "ValidationError",
    "TelemetryError",
    "FFIError"
]
