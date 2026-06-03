"""
CLI Extensions
Extension management and IoC consumer patterns
"""

from .manager import ExtensionManager
from .loader import ExtensionLoader

__all__ = ["ExtensionManager", "ExtensionLoader"]
