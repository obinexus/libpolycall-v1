"""
PyPolyCall - LibPolyCall Trial v1 Python Binding
Clean Architecture Implementation for polycall.exe Runtime

ARCHITECTURE OVERVIEW:
- core/: Core functionality mapped to polycall.exe
- cli/: Command-line interface and extensions
- config/: Configuration management
- utils/: Shared utilities

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
        raise ImportError("PyPolyCall requires Python 3.8+ for clean architecture support")

# Perform compliance check
_check_architecture_compliance()

# Core imports - Clean Architecture Pattern
try:
    # Core layer imports
    from .core.binding import ProtocolBinding
    from .core.protocol import ProtocolHandler, MessageTypes, StateTransitions
    from .core.telemetry import TelemetryObserver, MetricsCollector
    from .core.ffi import FFIBridge, NativeInterface
    from .core.state import StateMachine, StateManager
    
    # CLI layer imports (conditional)
    from .cli import CLI, CommandRegistry
    
    # Configuration layer
    from .config import ConfigManager, PolycallConfig
    
    # Utilities
    from .utils import Logger, Validator
    
except ImportError as e:
    warnings.warn(f"PyPolyCall architecture incomplete: {e}", ImportWarning)
    # Define minimal interface for graceful degradation
    ProtocolBinding = None
    CLI = None

# Public API - Clean Architecture Exports
__all__ = [
    # Core Architecture Components
    "ProtocolBinding",
    "ProtocolHandler", 
    "TelemetryObserver",
    "FFIBridge",
    "StateMachine",
    
    # CLI Architecture Components
    "CLI",
    "CommandRegistry",
    
    # Configuration Layer
    "ConfigManager",
    "PolycallConfig",
    
    # Utilities
    "Logger",
    "Validator",
    
    # Protocol Constants
    "MessageTypes",
    "StateTransitions",
    
    # Metadata
    "__version__",
    "__author__",
    "__protocol_version__",
]

def get_architecture_info() -> dict:
    """
    Get architecture compliance information
    
    Returns:
        dict: Architecture and protocol compliance metadata
    """
    return {
        "binding_version": __version__,
        "protocol_version": __protocol_version__,
        "architecture_pattern": "clean_architecture",
        "polycall_runtime_required": True,
        "core_layer_isolated": True,
        "cli_extensible": True,
        "telemetry_integrated": True,
        "ffi_enabled": True,
        "zero_trust_compliant": True,
    }

# Architecture compliance notice
def _show_architecture_notice():
    """Display architecture compliance notice"""
    print("=" * 70)
    print("PYPOLYCALL CLEAN ARCHITECTURE NOTICE")
    print("=" * 70)
    print("Core Layer: Protocol binding to polycall.exe runtime")
    print("CLI Layer: Extensible command interface")
    print("Config Layer: Unified configuration management")
    print("Utils Layer: Shared utilities and validation")
    print("=" * 70)
    print("PROTOCOL COMPLIANCE:")
    print("- All core functions map to polycall.exe operations")
    print("- CLI provides extension points for custom commands")
    print("- Clean separation prevents architecture violations")
    print("=" * 70)

# Show notice in development mode
if __debug__:
    _show_architecture_notice()
