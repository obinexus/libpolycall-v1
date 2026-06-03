"""
Protocol Binding Integration Tests
"""

import pytest
import asyncio
from pypolycall.core.binding import ProtocolBinding

class TestProtocolBindingIntegration:
    """Test protocol binding integration"""
    
    @pytest.mark.asyncio
    async def test_binding_lifecycle(self):
        """Test complete binding lifecycle"""
        binding = ProtocolBinding()
        
        # Test connection
        connected = await binding.connect()
        assert connected == True
        assert binding.is_connected == True
        
        # Test authentication
        auth_result = await binding.authenticate({"user": "test"})
        assert auth_result == True
        assert binding.is_authenticated == True
        
        # Test operation execution
        result = await binding.execute_operation("test_op", {"param": "value"})
        assert result is not None
        assert result["status"] == "success"
        
        # Test shutdown
        await binding.shutdown()
        assert binding.is_connected == False
        assert binding.is_authenticated == False
    
    def test_binding_initialization(self):
        """Test binding initialization parameters"""
        binding = ProtocolBinding(
            polycall_host="test-host",
            polycall_port=9999
        )
        
        assert binding.polycall_host == "test-host"
        assert binding.polycall_port == 9999
        assert not binding.is_connected
        assert not binding.is_authenticated
