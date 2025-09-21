"""PyPolycall - Pure FFI Interface to libpolycall"""
import ctypes
import json
import os
from pathlib import Path

class PolycallModule:
    """Pure interface wrapper - no business logic"""
    def __init__(self, name, lib_path):
        self.name = name
        self.lib = ctypes.CDLL(lib_path)
        self._setup_functions()
    
    def _setup_functions(self):
        """Wire up C functions - pure passthrough"""
        pass

class PyPolycall:
    def __init__(self, base_path=None):
        self.base_path = base_path or os.environ.get('POLYCALL_PATH', './build/unix')
        config_path = Path(self.base_path) / 'libpolycall.ffi.json'
        with open(config_path) as f:
            self.config = json.load(f)
    
    def load(self, module_name, use_shared=True):
        """Load a polycall module - pure loader, no logic"""
        if module_name not in self.config['modules']:
            raise ValueError(f"Unknown module: {module_name}")
        
        module_info = self.config['modules'][module_name]
        lib_type = 'shared' if use_shared else 'static'
        lib_path = Path(self.base_path) / module_info[lib_type]
        
        return PolycallModule(module_name, str(lib_path))

# Global loader instance
pypolycall = PyPolycall()
