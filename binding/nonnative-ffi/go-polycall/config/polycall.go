// Package polycall provides Go bindings for LibPolyCall
// LibPolyCall: A Program First Data-Oriented Program Interface Implementation
//
// Author: OBINexusComputing - Aegis Engineering Team
// Collaborator: Nnamdi Michael Okpala - OBINexusComputing
//
// This package implements Go language bindings for the LibPolyCall system,
// following the established program-first architecture where bindings act
// as thin interface mappers to the core LibPolyCall implementation.
//
// Architecture Philosophy:
// - Program-first vs binding-first approach
// - Bindings are thin code mappings, not logic owners
// - Core implementation remains with LibPolyCall binary
// - Language agnostic protocol communication
//
// Usage:
//   client, err := polycall.NewPolyCallClient()
//   if err != nil {
//       log.Fatal(err)
//   }
//   
//   err = client.Connect()
//   if err != nil {
//       log.Fatal(err)
//   }
//   
//   resp, err := client.GetBooks()
//   if err != nil {
//       log.Fatal(err)
//   }
//
// Configuration:
// The Go binding reads configuration from config/go.polycallrc following
// the standard .polycallrc format established by the LibPolyCall ecosystem.
//
// Network Protocol:
// Communication with LibPolyCall core happens via:
// - HTTP requests for standard API operations
// - TCP socket connections for protocol-level communication  
// - State machine integration for advanced features
//
// Zero-Trust Security:
// All communications implement zero-trust architecture with:
// - Cryptographically-seeded authentication
// - Port binding validation
// - Strict security enforcement
package polycall

// Re-export key types and functions for easier access
type (
	// Client represents the main LibPolyCall Go client
	Client = PolyCallClient
	
	// Config represents the Go binding configuration
	Config = Configuration
	
	// MsgType represents protocol message types
	MsgType = MessageType
	
	// Flag represents protocol flags
	Flag = ProtocolFlag
)

// Protocol message type constants
const (
	Handshake MsgType = MessageHandshake
	Auth      MsgType = MessageAuth
	Command   MsgType = MessageCommand
	Response  MsgType = MessageResponse
	Error     MsgType = MessageError
	Heartbeat MsgType = MessageHeartbeat
)

// Protocol flag constants  
const (
	None       Flag = FlagNone
	Encrypted  Flag = FlagEncrypted
	Compressed Flag = FlagCompressed
	Urgent     Flag = FlagUrgent
	Reliable   Flag = FlagReliable
)

// NewClient creates a new LibPolyCall Go client with optional configuration path
// 
// If no configuration path is provided, defaults to config/go.polycallrc
//
// Returns:
//   - *Client: Configured PolyCall client instance
//   - error: Configuration or initialization error
//
// Example:
//   client, err := polycall.NewClient()
//   if err != nil {
//       return fmt.Errorf("client initialization failed: %w", err)
//   }
func NewClient(configPath ...string) (*Client, error) {
	return NewPolyCallClient(configPath...)
}

// Version returns the Go binding version information
func Version() string {
	return "1.0.0"
}

// BuildInfo returns comprehensive build and compatibility information
func BuildInfo() map[string]string {
	return map[string]string{
		"version":           "1.0.0",
		"polycall_version":  "1.0.0",
		"architecture":      "program-first",
		"protocol_version":  "1",
		"go_version":        "1.21+",
		"author":            "OBINexusComputing",
		"collaborator":      "Nnamdi Michael Okpala",
		"project":           "Aegis LibPolyCall",
		"binding_type":      "thin_client",
		"communication":     "http_tcp_hybrid",
		"security":          "zero_trust",
	}
}
