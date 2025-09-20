"""
Configuration Manager
Centralized configuration handling with validation
"""

import os
import yaml
import json
from typing import Dict, Any, Optional
from pathlib import Path
from ..utils import Logger, Validator
from ..exceptions import ConfigurationError

logger = Logger.get_logger(__name__)

class ConfigManager:
    """
    Centralized Configuration Manager
    
    RESPONSIBILITIES:
    - Configuration file loading and parsing
    - Environment variable integration
    - Configuration validation
    - Default configuration provision
    """
    
    def __init__(self):
        self._config_cache: Dict[str, Any] = {}
        self._default_config_path = Path.home() / '.pypolycall' / 'config.yaml'
    
    async def load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from file with validation"""
        try:
            config_file = Path(config_path)
            
            if not config_file.exists():
                raise ConfigurationError(f"Configuration file not found: {config_path}")
            
            # Load based on file extension
            if config_file.suffix.lower() in ['.yaml', '.yml']:
                with open(config_file, 'r') as f:
                    config = yaml.safe_load(f)
            elif config_file.suffix.lower() == '.json':
                with open(config_file, 'r') as f:
                    config = json.load(f)
            else:
                raise ConfigurationError(f"Unsupported configuration format: {config_file.suffix}")
            
            # Validate configuration
            validated_config = await self._validate_config(config)
            
            # Cache configuration
            self._config_cache[config_path] = validated_config
            
            logger.info(f"Configuration loaded from {config_path}")
            return validated_config
            
        except Exception as e:
            logger.error(f"Failed to load configuration: {e}")
            raise ConfigurationError(f"Configuration loading failed: {e}")
    
    async def get_default_config(self) -> Dict[str, Any]:
        """Get default configuration with environment overrides"""
        default_config = {
            'core': {
                'polycall_host': os.getenv('PYPOLYCALL_HOST', 'localhost'),
                'polycall_port': int(os.getenv('PYPOLYCALL_PORT', '8084')),
                'connection_timeout': 30,
                'retry_attempts': 3,
            },
            'telemetry': {
                'enabled': True,
                'silent_observation': True,
                'metrics_interval': 60,
            },
            'ffi': {
                'enabled': True,
                'library_path': os.getenv('PYPOLYCALL_FFI_PATH'),
            },
            'cli': {
                'extension_dir': os.getenv('PYPOLYCALL_EXTENSIONS'),
                'auto_load_extensions': True,
            },
            'logging': {
                'level': os.getenv('PYPOLYCALL_LOG_LEVEL', 'INFO'),
                'format': 'structured',
            }
        }
        
        return await self._validate_config(default_config)
    
    async def _validate_config(self, config: Dict[str, Any]) -> Dict[str, Any]:
        """Validate configuration against schema"""
        # Import here to avoid circular imports
        from .schema import ConfigSchema
        
        validator = ConfigSchema()
        return validator.validate(config)
