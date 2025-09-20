"""
Info Command - Display Protocol and Binding Information
"""

import argparse
from ...utils import Logger
from ... import get_architecture_info

logger = Logger.get_logger(__name__)

class InfoCommand:
    """Display PyPolyCall architecture and protocol information"""
    
    def get_help(self) -> str:
        return "Display protocol and binding information"
    
    def add_arguments(self, parser: argparse.ArgumentParser) -> None:
        """Add command-specific arguments"""
        parser.add_argument(
            "--detailed",
            action="store_true",
            help="Show detailed architecture information"
        )
    
    async def execute(self, args: argparse.Namespace) -> int:
        """Execute info command"""
        info = get_architecture_info()
        
        print("PyPolyCall Architecture Information")
        print("=" * 50)
        print(f"Binding Version: {info['binding_version']}")
        print(f"Protocol Version: {info['protocol_version']}")
        print(f"Architecture: {info['architecture_pattern']}")
        print(f"Runtime Required: {info['polycall_runtime_required']}")
        print(f"Core Layer Isolated: {info['core_layer_isolated']}")
        print(f"CLI Extensible: {info['cli_extensible']}")
        print(f"Telemetry Integrated: {info['telemetry_integrated']}")
        print(f"FFI Enabled: {info['ffi_enabled']}")
        print(f"Zero Trust: {info['zero_trust_compliant']}")
        
        if args.detailed:
            print("\n" + "=" * 50)
            print("ARCHITECTURE LAYERS:")
            print("- Core: Protocol binding to polycall.exe")
            print("- CLI: Extensible command interface")
            print("- Config: Unified configuration management")
            print("- Utils: Shared utilities and validation")
            print("\nPROTOCOL COMPLIANCE:")
            print("- All operations map to polycall.exe runtime")
            print("- Clean separation prevents violations")
            print("- Extension points maintain architecture")
        
        return 0
