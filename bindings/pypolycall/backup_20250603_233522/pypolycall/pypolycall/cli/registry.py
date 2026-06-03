"""
Command Registry - Extensible Command Management
"""

from typing import Dict, Optional, Tuple, Iterator
from ..utils import Logger

logger = Logger.get_logger(__name__)

class CommandRegistry:
    """
    Extensible Command Registry
    
    RESPONSIBILITIES:
    - Command registration and lookup
    - Extension command integration
    - Command validation and lifecycle
    """
    
    def __init__(self):
        self._commands: Dict[str, object] = {}
    
    def register(self, name: str, command_instance: object) -> None:
        """Register a command instance"""
        if name in self._commands:
            logger.warning(f"Overriding existing command: {name}")
        
        self._commands[name] = command_instance
        logger.info(f"Command registered: {name}")
    
    def unregister(self, name: str) -> bool:
        """Unregister a command"""
        if name in self._commands:
            del self._commands[name]
            logger.info(f"Command unregistered: {name}")
            return True
        return False
    
    def get_command(self, name: str) -> Optional[object]:
        """Get command instance by name"""
        return self._commands.get(name)
    
    def get_all_commands(self) -> Iterator[Tuple[str, object]]:
        """Get all registered commands"""
        return iter(self._commands.items())
    
    def list_commands(self) -> list:
        """List all command names"""
        return list(self._commands.keys())
