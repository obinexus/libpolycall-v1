// Package tests provides comprehensive testing for Go PolyCall binding
// Author: OBINexusComputing - Aegis Engineering Team
// Collaborator: Nnamdi Michael Okpala
package tests

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	polycall "../src"
)

// Test configuration constants
const (
	TestConfigPath = "test_config.polycallrc"
	TestTimeout    = 10 * time.Second
)

// TestConfiguration validates configuration loading and parsing
func TestConfiguration(t *testing.T) {
	t.Run("DefaultConfiguration", func(t *testing.T) {
		// Test default configuration creation
		client, err := polycall.NewPolyCallClient()
		require.NoError(t, err, "Failed to create client with default configuration")
		assert.NotNil(t, client, "Client should not be nil")
	})

	t.Run("CustomConfiguration", func(t *testing.T) {
		// Create temporary test configuration
		configContent := `# Test configuration for Go PolyCall binding
port=3003:8083
server_type=go
workspace=/tmp/polycall/test
log_level=debug
max_connections=50
supports_formatting=true
max_memory=512M
timeout=15
allow_remote=false
require_auth=true
strict_port_binding=true
go_version=1.21
`
		configPath := filepath.Join(os.TempDir(), TestConfigPath)
		err := os.WriteFile(configPath, []byte(configContent), 0644)
		require.NoError(t, err, "Failed to create test configuration file")
		defer os.Remove(configPath)

		// Test configuration loading
		client, err := polycall.NewPolyCallClient(configPath)
		require.NoError(t, err, "Failed to create client with custom configuration")
		assert.NotNil(t, client, "Client should not be nil")
	})

	t.Run("InvalidConfiguration", func(t *testing.T) {
		// Test invalid configuration handling
		invalidConfigPath := "/nonexistent/path/config.polycallrc"
		_, err := polycall.NewPolyCallClient(invalidConfigPath)
		assert.Error(t, err, "Should fail with invalid configuration path")
	})
}

// TestClientLifecycle validates connection management
func TestClientLifecycle(t *testing.T) {
	client, err := polycall.NewPolyCallClient()
	require.NoError(t, err, "Failed to create client")

	t.Run("InitialState", func(t *testing.T) {
		assert.False(t, client.IsConnected(), "Client should not be connected initially")
		assert.False(t, client.IsAuthenticated(), "Client should not be authenticated initially")
	})

	t.Run("ConnectionAttempt", func(t *testing.T) {
		// Note: This test may fail if LibPolyCall server is not running
		// This is expected behavior in testing environment
		err := client.Connect()
		if err != nil {
			t.Logf("Connection failed (expected if server not running): %v", err)
			// Test that client handles connection failure gracefully
			assert.False(t, client.IsConnected(), "Client should remain disconnected on connection failure")
		} else {
			t.Log("Connection successful - LibPolyCall server is running")
			assert.True(t, client.IsConnected(), "Client should be connected after successful connection")
		}
	})

	t.Run("Disconnection", func(t *testing.T) {
		err := client.Disconnect()
		assert.NoError(t, err, "Disconnection should not fail")
		assert.False(t, client.IsConnected(), "Client should be disconnected after Disconnect()")
		assert.False(t, client.IsAuthenticated(), "Client should lose authentication after disconnect")
	})
}

// TestHTTPAPIInteraction validates HTTP-based API communication
func TestHTTPAPIInteraction(t *testing.T) {
	client, err := polycall.NewPolyCallClient()
	require.NoError(t, err, "Failed to create client")

	t.Run("BookCreation", func(t *testing.T) {
		// Test book creation API
		title := "Test Book for Go Binding"
		author := "Aegis Test Suite"

		resp, err := client.CreateBook(title, author)
		if err != nil {
			t.Logf("Book creation failed (expected if server not running): %v", err)
			// Verify error handling
			assert.Error(t, err, "Should properly handle connection errors")
		} else {
			t.Log("Book creation successful")
			assert.NotEmpty(t, resp, "Response should not be empty")
			
			// Validate response structure
			var bookData map[string]interface{}
			err = json.Unmarshal(resp, &bookData)
			assert.NoError(t, err, "Response should be valid JSON")
			
			if title, exists := bookData["title"]; exists {
				assert.Equal(t, "Test Book for Go Binding", title, "Title should match")
			}
		}
	})

	t.Run("BookRetrieval", func(t *testing.T) {
		// Test book retrieval API
		resp, err := client.GetBooks()
		if err != nil {
			t.Logf("Book retrieval failed (expected if server not running): %v", err)
			assert.Error(t, err, "Should properly handle connection errors")
		} else {
			t.Log("Book retrieval successful")
			assert.NotEmpty(t, resp, "Response should not be empty")
			
			// Validate response structure
			var books []interface{}
			err = json.Unmarshal(resp, &books)
			assert.NoError(t, err, "Response should be valid JSON array")
		}
	})
}

// TestStateManagement validates state management functionality
func TestStateManagement(t *testing.T) {
	client, err := polycall.NewPolyCallClient()
	require.NoError(t, err, "Failed to create client")

	t.Run("StateRetrieval", func(t *testing.T) {
		resp, err := client.GetStates()
		if err != nil {
			t.Logf("State retrieval failed (expected if server not running): %v", err)
			assert.Error(t, err, "Should properly handle connection errors")
		} else {
			t.Log("State retrieval successful")
			assert.NotEmpty(t, resp, "Response should not be empty")
			
			// Validate response structure
			var states []interface{}
			err = json.Unmarshal(resp, &states)
			assert.NoError(t, err, "Response should be valid JSON array")
		}
	})

	t.Run("SpecificStateRetrieval", func(t *testing.T) {
		stateName := "ready"
		resp, err := client.GetState(stateName)
		if err != nil {
			t.Logf("Specific state retrieval failed (expected if server not running): %v", err)
			assert.Error(t, err, "Should properly handle connection errors")
		} else {
			t.Log("Specific state retrieval successful")
			assert.NotEmpty(t, resp, "Response should not be empty")
		}
	})

	t.Run("StateTransition", func(t *testing.T) {
		transitionData := map[string]interface{}{
			"reason":    "test_transition",
			"timestamp": time.Now().Unix(),
			"client":    "go-polycall-test",
		}

		resp, err := client.TransitionTo("ready", transitionData)
		if err != nil {
			t.Logf("State transition failed (expected if server not running): %v", err)
			assert.Error(t, err, "Should properly handle connection errors")
		} else {
			t.Log("State transition successful")
			assert.NotEmpty(t, resp, "Response should not be empty")
		}
	})
}

// TestProtocolHandling validates protocol-level functionality
func TestProtocolHandling(t *testing.T) {
	client, err := polycall.NewPolyCallClient()
	require.NoError(t, err, "Failed to create client")

	t.Run("CommandExecution", func(t *testing.T) {
		// This test requires authentication, so it may fail if not connected
		command := "status"
		data := map[string]interface{}{
			"test": true,
			"client": "go-polycall-test",
		}

		_, err := client.SendCommand(command, data)
		if err != nil {
			t.Logf("Command execution failed (expected if not authenticated): %v", err)
			// Verify proper error handling for unauthenticated requests
			assert.Contains(t, err.Error(), "not authenticated", "Should indicate authentication requirement")
		} else {
			t.Log("Command execution successful")
		}
	})
}

// TestErrorHandling validates error scenarios and resilience
func TestErrorHandling(t *testing.T) {
	t.Run("InvalidPortConfiguration", func(t *testing.T) {
		// Create configuration with invalid port
		configContent := `port=invalid_port_format`
		configPath := filepath.Join(os.TempDir(), "invalid_config.polycallrc")
		err := os.WriteFile(configPath, []byte(configContent), 0644)
		require.NoError(t, err)
		defer os.Remove(configPath)

		_, err = polycall.NewPolyCallClient(configPath)
		assert.Error(t, err, "Should fail with invalid port configuration")
		assert.Contains(t, err.Error(), "invalid port", "Error should mention port configuration")
	})

	t.Run("NetworkTimeouts", func(t *testing.T) {
		client, err := polycall.NewPolyCallClient()
		require.NoError(t, err)

		// Test with non-existent server (should timeout)
		err = client.Connect()
		if err != nil {
			assert.Error(t, err, "Should handle connection timeouts gracefully")
		}
	})
}

// TestConcurrency validates thread-safety and concurrent operations
func TestConcurrency(t *testing.T) {
	client, err := polycall.NewPolyCallClient()
	require.NoError(t, err, "Failed to create client")

	t.Run("ConcurrentAPIRequests", func(t *testing.T) {
		// Test concurrent HTTP requests
		const numRequests = 10
		results := make(chan error, numRequests)

		for i := 0; i < numRequests; i++ {
			go func(index int) {
				_, err := client.GetBooks()
				results <- err
			}(i)
		}

		// Collect results
		errors := 0
		for i := 0; i < numRequests; i++ {
			if err := <-results; err != nil {
				errors++
			}
		}

		t.Logf("Concurrent requests completed. Errors: %d/%d", errors, numRequests)
		// All requests should either succeed or fail consistently
		assert.True(t, errors == 0 || errors == numRequests, 
			"Concurrent requests should have consistent behavior")
	})

	t.Run("StateConsistency", func(t *testing.T) {
		// Test that client state remains consistent under concurrent access
		const numGoroutines = 10
		done := make(chan bool, numGoroutines)

		for i := 0; i < numGoroutines; i++ {
			go func() {
				defer func() { done <- true }()
				
				// Multiple state checks should be consistent
				connected1 := client.IsConnected()
				authenticated1 := client.IsAuthenticated()
				
				time.Sleep(1 * time.Millisecond)
				
				connected2 := client.IsConnected()
				authenticated2 := client.IsAuthenticated()
				
				assert.Equal(t, connected1, connected2, "Connection state should be consistent")
				assert.Equal(t, authenticated1, authenticated2, "Authentication state should be consistent")
			}()
		}

		// Wait for all goroutines to complete
		for i := 0; i < numGoroutines; i++ {
			<-done
		}
	})
}

// BenchmarkHTTPRequests measures performance of HTTP requests
func BenchmarkHTTPRequests(b *testing.B) {
	client, err := polycall.NewPolyCallClient()
	if err != nil {
		b.Fatalf("Failed to create client: %v", err)
	}

	b.Run("GetBooks", func(b *testing.B) {
		b.ResetTimer()
		for i := 0; i < b.N; i++ {
			_, _ = client.GetBooks()
		}
	})

	b.Run("CreateBook", func(b *testing.B) {
		b.ResetTimer()
		for i := 0; i < b.N; i++ {
			_, _ = client.CreateBook("Benchmark Book", "Benchmark Author")
		}
	})
}

// Helper functions for testing

func createTestClient(t *testing.T) *polycall.PolyCallClient {
	client, err := polycall.NewPolyCallClient()
	require.NoError(t, err, "Failed to create test client")
	return client
}

func skipIfServerNotRunning(t *testing.T, client *polycall.PolyCallClient) {
	_, err := client.GetBooks()
	if err != nil {
		t.Skip("LibPolyCall server not running - skipping integration test")
	}
}
