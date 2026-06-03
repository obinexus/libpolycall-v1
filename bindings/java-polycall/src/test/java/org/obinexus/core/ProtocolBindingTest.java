package org.obinexus.core;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

public class ProtocolBindingTest {
    
    @Test
    public void testProtocolBindingInitialization() {
        ProtocolBinding binding = new ProtocolBinding("localhost", 8084);
        assertNotNull(binding);
        assertFalse(binding.isConnected());
        assertFalse(binding.isAuthenticated());
    }
}
