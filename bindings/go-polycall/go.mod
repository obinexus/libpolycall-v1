// go.mod - LibPolyCall Trial v1 Go Binding Module Definition
// Architecture: Protocol-compliant adapter for polycall.exe runtime
// Author: OBINexusComputing - Aegis Engineering Team
// Collaborator: Nnamdi Michael Okpala

module github.com/obinexus/libpolycall-v1trial/bindings/go-polycall

go 1.21

require (
	github.com/stretchr/testify v1.8.4
	golang.org/x/crypto v0.17.0
	golang.org/x/net v0.19.0
	gopkg.in/yaml.v3 v3.0.1
)

require (
	github.com/davecgh/go-spew v1.1.1 // indirect
	github.com/pmezard/go-difflib v1.0.0 // indirect
	github.com/stretchr/objx v0.5.0 // indirect
)

// Development and testing dependencies
require (
	github.com/golang/protobuf v1.5.3 // Protocol buffer serialization
	github.com/gorilla/websocket v1.5.1 // WebSocket protocol support
)

// Indirect dependencies for protocol handling
require golang.org/x/sys v0.15.0 // System-level operations

// Replace directive for local development against LibPolyCall core
// Activate during integration testing with local polycall.exe runtime
// replace github.com/obinexus/libpolycall-core => ../../libpolycall

// CRITICAL: Protocol Compliance Requirements
// This module implements the ADAPTER PATTERN as specified in LibPolyCall Trial v1
//
// Architectural constraints:
// - Runtime Dependency: ALL operations REQUIRE polycall.exe runtime
// - Zero-Trust Security: Cryptographic validation at every state transition
// - State Machine Binding: Finite automaton pattern compliance
// - Program-First Paradigm: NO direct code execution, ONLY protocol translation
//
// Waterfall Development Phase: Module Definition & Dependency Resolution
// Next Phase: Interface Implementation & Protocol Handler Development

// Toolchain specification for build consistency across Aegis development team
toolchain go1.21.5
