"""
Core FFI Layer  
Foreign Function Interface for polycall.exe Integration
"""

from .bridge import FFIBridge
from .native import NativeInterface

__all__ = ["FFIBridge", "NativeInterface"]
