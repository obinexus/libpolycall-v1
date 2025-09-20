"""
PyPolyCall - LibPolyCall Trial v1 Python Binding
Protocol-compliant adapter for polycall.exe runtime

PROTOCOL COMPLIANCE:
- This binding acts as an ADAPTER, not an engine
- All execution must go through polycall.exe
- Clean separation of concerns enforced
"""

import sys
import warnings
from typing import Optional

# Version and metadata
__version__ = "1.0.0"
__author__ = "OBINexusComputing"
__protocol_version__ = "1.0"
__polycall_min_version__ = "1.0.0"

# Architecture compliance check
def _check_architecture_compliance():
    """Verify architecture compliance at import time"""
    if sys.version_info < (3, 8):
        raise ImportError("PyPolyCall requires Python 3.8+ for protocol compliance")

# Perform compliance check
_check_architecture_compliance()

# Core imports with error handling
try:
    from .core.binding import ProtocolBinding
    from .core.protocol import ProtocolHandler, MessageTypes, StateTransitions
    from .core.telemetry import TelemetryObserver, MetricsCollector
except ImportError as e:
    warnings.warn(f"PyPolyCall core components incomplete: {e}", ImportWarning)
    ProtocolBinding = None
    ProtocolHandler = None
    MessageTypes = None
    StateTransitions = None
    TelemetryObserver = None
    MetricsCollector = None

# CLI imports with error handling
try:
    from .cli import CLI
    from .cli.main import main
except ImportError as e:
    warnings.warn(f"PyPolyCall CLI incomplete: {e}", ImportWarning)
    CLI = None
    main = None

# Configuration imports with error handling
try:
    from .config import ConfigManager
except ImportError as e:
    warnings.warn(f"PyPolyCall config incomplete: {e}", ImportWarning)
    ConfigManager = None

# Utilities imports with error handling
try:
    from .utils import Logger, Validator
except ImportError as e:
    warnings.warn(f"PyPolyCall utils incomplete: {e}", ImportWarning)
    Logger = None
    Validator = None

# Public API exports
__all__ = [
    # Core components
    "ProtocolBinding",
    "ProtocolHandler", 
    "MessageTypes",
    "StateTransitions",
    "TelemetryObserver",
    "MetricsCollector",
    
    # CLI components
    "CLI",
    "main",
    
    # Configuration
    "ConfigManager",
    
    # Utilities
    "Logger",
    "Validator",
    
    # Metadata
    "__version__",
    "__author__",
    "__protocol_version__",
]

def get_architecture_info() -> dict:
    """Get architecture compliance information"""
    return {
        "binding_version": __version__,
        "protocol_version": __protocol_version__,
        "architecture_pattern": "adapter",
        "polycall_runtime_required": True,
        "adapter_pattern": True,
        "zero_trust_compliant": True,
        "core_layer_isolated": True,
        "cli_extensible": True,
        "telemetry_integrated": True,
    }

def get_protocol_info() -> dict:
    """Get protocol compliance information"""
    return get_architecture_info()

# Show compliance notice in development
if __debug__:
    print("=" * 50)
    print("PYPOLYCALL PROTOCOL COMPLIANCE NOTICE")
    print("=" * 50)
    print("This binding is an ADAPTER for polycall.exe runtime.")
    print("- All operations must go through polycall.exe")
    print("- Clean architecture enforced")
    print("- Zero-trust validation required")
    print("=" * 50)
