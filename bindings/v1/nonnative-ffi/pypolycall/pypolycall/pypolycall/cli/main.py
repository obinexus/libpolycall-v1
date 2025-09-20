"""
Main CLI Entry Point
Extensible Command-Line Interface for PyPolyCall
"""

import sys
import argparse
import asyncio
from typing import List, Optional
from ..core import ProtocolBinding
from ..config import ConfigManager
from ..utils import Logger
from .registry import CommandRegistry
from .extensions import ExtensionManager

logger = Logger.get_logger(__name__)

class CLI:
    """
    Main CLI Class - Extensible Architecture
    
    RESPONSIBILITIES:
    - Command parsing and routing
    - Extension management
    - Core binding integration
    - User interaction handling
    """
    
    def __init__(self):
        self.config_manager = ConfigManager()
        self.command_registry = CommandRegistry()
        self.extension_manager = ExtensionManager()
        self.core_binding: Optional[ProtocolBinding] = None
        
        # Register built-in commands
        self._register_builtin_commands()
    
    def _register_builtin_commands(self) -> None:
        """Register built-in CLI commands"""
        from .commands import (
            InfoCommand, TestCommand, ConnectCommand, 
            ConfigCommand, TelemetryCommand
        )
        
        self.command_registry.register('info', InfoCommand())
        self.command_registry.register('test', TestCommand())
        self.command_registry.register('connect', ConnectCommand())
        self.command_registry.register('config', ConfigCommand())
        self.command_registry.register('telemetry', TelemetryCommand())
    
    def create_parser(self) -> argparse.ArgumentParser:
        """Create comprehensive argument parser"""
        parser = argparse.ArgumentParser(
            description="PyPolyCall - LibPolyCall Trial v1 Python Binding CLI",
            epilog="Protocol compliance: Requires polycall.exe runtime"
        )
        
        parser.add_argument(
            "--version", 
            action="version",
            version="PyPolyCall 1.0.0"
        )
        
        parser.add_argument(
            "--host",
            default="localhost",
            help="polycall.exe runtime host (default: localhost)"
        )
        
        parser.add_argument(
            "--port", 
            type=int,
            default=8084,
            help="polycall.exe runtime port (default: 8084)"
        )
        
        parser.add_argument(
            "--config",
            help="Configuration file path"
        )
        
        parser.add_argument(
            "--log-level",
            choices=["DEBUG", "INFO", "WARNING", "ERROR"],
            default="INFO",
            help="Logging level (default: INFO)"
        )
        
        parser.add_argument(
            "--extension-dir",
            help="Extension directory path"
        )
        
        # Subcommand parsing
        subparsers = parser.add_subparsers(dest="command", help="Available commands")
        
        # Register subcommands from registry
        for cmd_name, cmd_instance in self.command_registry.get_all_commands():
            cmd_parser = subparsers.add_parser(cmd_name, help=cmd_instance.get_help())
            cmd_instance.add_arguments(cmd_parser)
        
        return parser
    
    async def initialize_core_binding(self, host: str, port: int, config_path: Optional[str] = None) -> None:
        """Initialize core protocol binding"""
        # Load configuration
        if config_path:
            config = await self.config_manager.load_config(config_path)
        else:
            config = await self.config_manager.get_default_config()
        
        # Create core binding
        self.core_binding = ProtocolBinding(
            polycall_host=host,
            polycall_port=port,
            binding_config=config
        )
        
        await self.core_binding.initialize()
        logger.info("Core binding initialized")
    
    async def execute_command(self, args: argparse.Namespace) -> int:
        """Execute parsed command"""
        if not args.command:
            print("No command specified. Use --help for available commands.")
            return 1
        
        # Get command from registry
        command = self.command_registry.get_command(args.command)
        if not command:
            print(f"Unknown command: {args.command}")
            return 1
        
        try:
            # Provide core binding to command if needed
            if hasattr(command, 'set_core_binding'):
                command.set_core_binding(self.core_binding)
            
            # Execute command
            result = await command.execute(args)
            return result if isinstance(result, int) else 0
            
        except Exception as e:
            logger.error(f"Command execution failed: {e}")
            print(f"Error: {e}")
            return 1
    
    async def run(self, argv: Optional[List[str]] = None) -> int:
        """Main CLI execution"""
        parser = self.create_parser()
        args = parser.parse_args(argv)
        
        # Setup logging
        Logger.setup_logging(args.log_level)
        
        try:
            # Load extensions if specified
            if args.extension_dir:
                await self.extension_manager.load_extensions(args.extension_dir, self.command_registry)
            
            # Initialize core binding for commands that need it
            if args.command in ['test', 'connect', 'telemetry']:
                await self.initialize_core_binding(args.host, args.port, args.config)
            
            # Execute command
            return await self.execute_command(args)
            
        except KeyboardInterrupt:
            print("\nOperation cancelled by user")
            return 130
        except Exception as e:
            logger.error(f"CLI execution failed: {e}")
            print(f"Fatal error: {e}")
            return 1
        finally:
            # Cleanup
            if self.core_binding:
                await self.core_binding.shutdown()

def main() -> None:
    """Synchronous CLI entry point"""
    cli = CLI()
    exit_code = asyncio.run(cli.run())
    sys.exit(exit_code)

if __name__ == "__main__":
    main()
