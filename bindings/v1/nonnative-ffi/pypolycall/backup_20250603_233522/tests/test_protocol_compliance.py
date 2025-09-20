"""
Protocol Compliance Tests
Verify PyPolyCall binding follows protocol law
"""

import pytest
import asyncio
from pypolycall.core import ProtocolBinding
from pypolycall.protocol.constants import DEFAULT_POLYCALL_PORT

class TestProtocolCompliance:
    """Test protocol compliance requirements"""
    
    def test_binding_requires_runtime(self):
        """Test that binding requires polycall.exe runtime"""
        binding = ProtocolBinding()
        
        # Should not allow operations without runtime connection
        with pytest.raises(RuntimeError):
            asyncio.run(binding.execute_request("/test", {}))
    
    def test_no_bypass_allowed(self):
        """Test that protocol cannot be bypassed"""
        binding = ProtocolBinding()
        
        # Should not have direct execution methods
        assert not hasattr(binding, 'direct_execute')
        assert not hasattr(binding, 'bypass_protocol')
    
    def test_adapter_pattern(self):
        """Test that binding follows adapter pattern"""
        binding = ProtocolBinding()
        
        # Should have protocol handler reference
        assert hasattr(binding, 'protocol_handler')
        
        # Should submit to external runtime
        assert binding.polycall_host is not None
        assert binding.polycall_port == DEFAULT_POLYCALL_PORT
    
    @pytest.mark.asyncio
    async def test_connection_required(self):
        """Test that connection to polycall.exe is required"""
        binding = ProtocolBinding()
        
        # Should not be connected initially
        assert not binding.is_connected
        
        # Should require connection for operations
        with pytest.raises(RuntimeError):
            await binding.execute_request("/test", {})
