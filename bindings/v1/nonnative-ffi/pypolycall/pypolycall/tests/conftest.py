"""
Test Configuration
PyPolyCall Clean Architecture Test Setup
"""

import pytest
import asyncio
import os
from pathlib import Path

# Test configuration
POLYCALL_TEST_HOST = os.getenv("POLYCALL_TEST_HOST", "localhost")
POLYCALL_TEST_PORT = int(os.getenv("POLYCALL_TEST_PORT", "8084"))
TEST_CONFIG_DIR = Path(__file__).parent / "test_configs"

@pytest.fixture
def event_loop():
    """Create event loop for async tests"""
    loop = asyncio.new_event_loop()
    yield loop
    loop.close()

@pytest.fixture
def polycall_config():
    """Provide polycall.exe test configuration"""
    return {
        "host": POLYCALL_TEST_HOST,
        "port": POLYCALL_TEST_PORT,
        "timeout": 5.0,
        "test_mode": True
    }

@pytest.fixture
def test_config_path():
    """Provide test configuration file path"""
    config_file = TEST_CONFIG_DIR / "test_config.yaml"
    config_file.parent.mkdir(exist_ok=True)
    
    # Create test configuration
    test_config = """
core:
  polycall_host: localhost
  polycall_port: 8084
  connection_timeout: 10
  
telemetry:
  enabled: true
  test_mode: true
  
cli:
  test_mode: true
"""
    
    with open(config_file, 'w') as f:
        f.write(test_config)
    
    yield str(config_file)
    
    # Cleanup
    if config_file.exists():
        config_file.unlink()
