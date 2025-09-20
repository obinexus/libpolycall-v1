"""
Core Binding Tests
Test the ProtocolBinding clean architecture implementation
"""

import pytest
import asyncio
from pypolycall.core.binding import ProtocolBinding
from pypolycall.exceptions import ValidationError, RuntimeError

class TestProtocolBinding:
    """Test ProtocolBinding clean architecture compliance"""
    
    def test_initialization_validation(self):
        """Test input validation during initialization"""
        # Valid initialization
        binding = ProtocolBinding(
            polycall_host="localhost",
            polycall_port=8084
        )
        assert binding.polycall_host == "localhost"
        assert binding.polycall_port == 8084
        
        # Invalid host
        with pytest.raises(ValidationError):
            ProtocolBinding(polycall_host="", polycall_port=8084)
        
        # Invalid port
        with pytest.raises(ValidationError):
            ProtocolBinding(polycall_host="localhost", polycall_port=0)
    
    @pytest.mark.asyncio
    async def test_architecture_isolation(self):
        """Test that core layer maintains architectural isolation"""
        binding = ProtocolBinding()
        
        # Should not be connected initially
        assert not binding.is_connected
        assert not binding.is_authenticated
        
        # Should require initialization before use
        with pytest.raises(RuntimeError):
            await binding.execute_operation("test", {})
    
    def test_clean_architecture_compliance(self):
        """Test clean architecture principle compliance"""
        binding = ProtocolBinding()
        
        # Should have clean layer separation
        assert hasattr(binding, '_protocol_handler')
        assert hasattr(binding, '_telemetry_observer')
        assert hasattr(binding, '_ffi_bridge')
        assert hasattr(binding, '_state_manager')
        
        # Should not expose internal implementation
        assert not hasattr(binding, 'direct_execute')
        assert not hasattr(binding, 'bypass_protocol')
