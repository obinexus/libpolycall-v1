// Package polycall provides the public API for LibPolyCall Trial v1 Go binding
// Architecture: Protocol-compliant adapter implementing program-first paradigm
// Author: OBINexusComputing - Aegis Engineering Team
// Collaborator: Nnamdi Michael Okpala

package polycall

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/obinexus/libpolycall-v1trial/bindings/go-polycall/internal"
)

// Client represents the LibPolyCall binding adapter interface
// CRITICAL: This client NEVER executes user code directly
// All operations flow through polycall.exe runtime for protocol compliance
type Client struct {
	// Protocol state management
	state          internal.ProtocolState
	stateMutex     sync.RWMutex
	
	// Runtime communication channels
	transport      *internal.Transport
	protocolHandler *internal.ProtocolHandler
	
	// Connection management
	host           string
	port           int
	connected      bool
	authenticated  bool
	
	// Context for operation lifecycle management
	ctx            context.Context
	cancel         context.CancelFunc
	
	// Telemetry for silent observation (non-intrusive)
	telemetry      *internal.TelemetryObserver
}

// NewClient initializes a new LibPolyCall binding client
// Returns an adapter that requires polycall.exe runtime for all operations
func NewClient(opts ...ClientOption) (*Client, error) {
	ctx, cancel := context.WithCancel(context.Background())
	
	client := &Client{
		state:      internal.INIT,
		host:       "localhost",
		port:       8084, // Default polycall.exe port
		ctx:        ctx,
		cancel:     cancel,
		telemetry:  internal.NewTelemetryObserver(),
	}
	
	// Apply configuration options
	for _, opt := range opts {
		if err := opt(client); err != nil {
			cancel()
			return nil, fmt.Errorf("client configuration failed: %w", err)
		}
	}
	
	// Initialize protocol components
	var err error
	client.transport, err = internal.NewTransport(client.host, client.port)
	if err != nil {
		cancel()
		return nil, fmt.Errorf("transport initialization failed: %w", err)
	}
	
	client.protocolHandler = internal.NewProtocolHandler(client.transport, client.telemetry)
	
	// Record initialization telemetry
	client.telemetry.RecordEvent("client_initialized", map[string]interface{}{
		"host": client.host,
		"port": client.port,
		"timestamp": time.Now().Unix(),
	})
	
	return client, nil
}

// Connect establishes connection to polycall.exe runtime
// Implements required state transition: INIT → HANDSHAKE → AUTH
func (c *Client) Connect() error {
	c.stateMutex.Lock()
	defer c.stateMutex.Unlock()
	
	if c.state != internal.INIT {
		return fmt.Errorf("invalid state for connection: %s", c.state)
	}
	
	// Begin handshake protocol with polycall.exe
	if err := c.protocolHandler.InitiateHandshake(c.ctx); err != nil {
		c.state = internal.ERROR
		return fmt.Errorf("handshake failed: %w", err)
	}
	
	c.state = internal.HANDSHAKE
	c.telemetry.RecordEvent("handshake_initiated", nil)
	
	// Perform zero-trust authentication
	if err := c.protocolHandler.Authenticate(c.ctx); err != nil {
		c.state = internal.ERROR
		return fmt.Errorf("authentication failed: %w", err)
	}
	
	c.state = internal.AUTH
	c.connected = true
	c.authenticated = true
	c.telemetry.RecordEvent("authentication_successful", nil)
	
	// Transition to ready state
	c.state = internal.READY
	c.telemetry.RecordEvent("client_ready", nil)
	
	return nil
}

// ExecuteFeature triggers execution of a registered feature through polycall.exe
// Implements state transition: READY → EXECUTING → READY
func (c *Client) ExecuteFeature(featureName string, params map[string]interface{}) (*ExecutionResult, error) {
	c.stateMutex.Lock()
	defer c.stateMutex.Unlock()
	
	if c.state != internal.READY {
		return nil, fmt.Errorf("client not ready for execution, current state: %s", c.state)
	}
	
	if !c.authenticated {
		return nil, fmt.Errorf("authentication required for feature execution")
	}
	
	// Transition to executing state
	c.state = internal.EXECUTING
	c.telemetry.RecordEvent("execution_started", map[string]interface{}{
		"feature": featureName,
		"timestamp": time.Now().Unix(),
	})
	
	// Execute through polycall.exe runtime (adapter pattern)
	result, err := c.protocolHandler.ExecuteFeature(c.ctx, featureName, params)
	if err != nil {
		c.state = internal.ERROR
		c.telemetry.RecordEvent("execution_failed", map[string]interface{}{
			"feature": featureName,
			"error": err.Error(),
		})
		return nil, fmt.Errorf("feature execution failed: %w", err)
	}
	
	// Return to ready state
	c.state = internal.READY
	c.telemetry.RecordEvent("execution_completed", map[string]interface{}{
		"feature": featureName,
		"duration_ms": result.Duration.Milliseconds(),
	})
	
	return result, nil
}

// GetState returns current protocol state for debugging
func (c *Client) GetState() internal.ProtocolState {
	c.stateMutex.RLock()
	defer c.stateMutex.RUnlock()
	return c.state
}

// IsConnected returns connection status
func (c *Client) IsConnected() bool {
	c.stateMutex.RLock()
	defer c.stateMutex.RUnlock()
	return c.connected
}

// IsAuthenticated returns authentication status
func (c *Client) IsAuthenticated() bool {
	c.stateMutex.RLock()
	defer c.stateMutex.RUnlock()
	return c.authenticated
}

// GetTelemetry returns telemetry data for debugging (non-intrusive observation)
func (c *Client) GetTelemetry() map[string]interface{} {
	return c.telemetry.GetMetrics()
}

// Disconnect cleanly terminates connection to polycall.exe runtime
func (c *Client) Disconnect() error {
	c.stateMutex.Lock()
	defer c.stateMutex.Unlock()
	
	if !c.connected {
		return nil
	}
	
	// Clean protocol termination
	if err := c.protocolHandler.Shutdown(c.ctx); err != nil {
		c.telemetry.RecordEvent("shutdown_error", map[string]interface{}{
			"error": err.Error(),
		})
		// Continue with cleanup despite error
	}
	
	// Cancel context and cleanup
	c.cancel()
	c.connected = false
	c.authenticated = false
	c.state = internal.INIT
	
	c.telemetry.RecordEvent("client_disconnected", nil)
	
	return nil
}

// ExecutionResult encapsulates the result of feature execution
type ExecutionResult struct {
	FeatureName string                 `json:"feature_name"`
	Success     bool                   `json:"success"`
	Data        map[string]interface{} `json:"data"`
	Duration    time.Duration          `json:"duration"`
	Timestamp   time.Time              `json:"timestamp"`
}

// ClientOption defines configuration option for client initialization
type ClientOption func(*Client) error

// WithHost configures the polycall.exe runtime host
func WithHost(host string) ClientOption {
	return func(c *Client) error {
		if host == "" {
			return fmt.Errorf("host cannot be empty")
		}
		c.host = host
		return nil
	}
}

// WithPort configures the polycall.exe runtime port
func WithPort(port int) ClientOption {
	return func(c *Client) error {
		if port <= 0 || port > 65535 {
			return fmt.Errorf("invalid port: %d", port)
		}
		c.port = port
		return nil
	}
}

// WithTelemetryEnabled configures telemetry observation
func WithTelemetryEnabled(enabled bool) ClientOption {
	return func(c *Client) error {
		if c.telemetry != nil {
			c.telemetry.SetEnabled(enabled)
		}
		return nil
	}
}
