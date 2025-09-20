"""
Command Registry - Extensible Command Management
"""

from typing import Dict, Optional

class CommandRegistry:
    """Command registry for CLI extensibility"""
    
    def __init__(self):
        self._commands: Dict[str, object] = {}
    
    def register(self, name: str, command_instance: object) -> None:
        """Register a command instance"""
        self._commands[name] = command_instance
    
    def get_command(self, name: str) -> Optional[object]:
        """Get command instance by name"""
        return self._commands.get(name)
    
    def list_commands(self) -> list:
        """List all command names"""
        return list(self._commands.keys())
