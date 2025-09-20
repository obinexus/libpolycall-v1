"""
Test Configuration
PyPolyCall Test Setup
"""

import pytest
import asyncio
import os

# Test configuration
POLYCALL_TEST_HOST = os.getenv("POLYCALL_TEST_HOST", "localhost")
POLYCALL_TEST_PORT = int(os.getenv("POLYCALL_TEST_PORT", "8084"))

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
        "timeout": 5.0
    }
