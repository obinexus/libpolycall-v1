"""
IoC Consumer
Inversion of Control patterns for CLI extensions
"""

from .container import IoContainer
from .injector import DependencyInjector

__all__ = ["IoContainer", "DependencyInjector"]
