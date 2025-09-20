// Package polycall provides Go bindings for LibPolyCall
// Author: OBINexusComputing - Aegis Engineering Team
// Collaborator: Nnamdi Michael Okpala
package polycall

import (
	"bufio"
	"bytes"
	"context"
	"crypto/sha256"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"
)

// Protocol constants matching LibPolyCall specification
const (
	ProtocolVersion    = 1
	ProtocolMagic      = 0x504C43 // "PLC"
	HeaderSize         = 16
	MaxPayloadSize     = 1024 * 1024 // 1MB
	DefaultTimeout     = 5 * time.Second
	DefaultRetryCount  = 3
)

// Message types for LibPolyCall protocol
type MessageType uint8

const (
	MessageHandshake MessageType = 0x01
	MessageAuth      MessageType = 0x02
	MessageCommand   MessageType = 0x03
	MessageResponse  MessageType = 0x04
	MessageError     MessageType = 0x05
	MessageHeartbeat MessageType = 0x06
)

// Protocol flags for message processing
type ProtocolFlag uint16

const (
	FlagNone       ProtocolFlag = 0x00
	FlagEncrypted  ProtocolFlag = 0x01
	FlagCompressed ProtocolFlag = 0x02
	FlagUrgent     ProtocolFlag = 0x04
	FlagReliable   ProtocolFlag = 0x08
)

// Configuration represents the Go binding configuration
type Configuration struct {
	Port               string `ini:"port"`
	ServerType         string `ini:"server_type"`
	Workspace          string `ini:"workspace"`
	LogLevel           string `ini:"log_level"`
	MaxConnections     int    `ini:"max_connections"`
	SupportsFormatting bool   `ini:"supports_formatting"`
	MaxMemory          string `ini:"max_memory"`
	Timeout            int    `ini:"timeout"`
	AllowRemote        bool   `ini:"allow_remote"`
	RequireAuth        bool   `ini:"require_auth"`
	StrictPortBinding  bool   `ini:"strict_port_binding"`
	GoVersion          string `ini:"go_version"`
}

// PolyCallClient represents the main LibPolyCall Go client
type PolyCallClient struct {
	config       *Configuration
	host         string
	port         int
	containerPort int
	connected    bool
	authenticated bool
	conn         net.Conn
	httpClient   *http.Client
	sequence     uint32
	mutex        sync.RWMutex
	pendingReqs  map[uint32]chan []byte
	ctx          context.Context
	cancel       context.CancelFunc
}

// ProtocolHeader represents the LibPolyCall protocol header
type ProtocolHeader struct {
	Version       uint8
	Type          MessageType
	Flags         ProtocolFlag
	Sequence      uint32
	PayloadLength uint32
	Checksum      uint32
}

// NewPolyCallClient creates a new Go PolyCall client instance
func NewPolyCallClient(configPath ...string) (*PolyCallClient, error) {
	config, err := loadConfiguration(configPath...)
	if err != nil {
		return nil, fmt.Errorf("failed to load configuration: %w", err)
	}

	// Parse port mapping (host:container)
	portParts := strings.Split(config.Port, ":")
	if len(portParts) != 2 {
		return nil, fmt.Errorf("invalid port configuration: %s", config.Port)
	}

	hostPort, err := strconv.Atoi(portParts[0])
	if err != nil {
		return nil, fmt.Errorf("invalid host port: %s", portParts[0])
	}

	containerPort, err := strconv.Atoi(portParts[1])
	if err != nil {
		return nil, fmt.Errorf("invalid container port: %s", portParts[1])
	}

	ctx, cancel := context.WithCancel(context.Background())

	client := &PolyCallClient{
		config:        config,
		host:          "localhost",
		port:          hostPort,
		containerPort: containerPort,
		httpClient: &http.Client{
			Timeout: time.Duration(config.Timeout) * time.Second,
		},
		pendingReqs: make(map[uint32]chan []byte),
		ctx:         ctx,
		cancel:      cancel,
	}

	return client, nil
}

// Connect establishes connection to LibPolyCall server
func (c *PolyCallClient) Connect() error {
	c.mutex.Lock()
	defer c.mutex.Unlock()

	if c.connected {
		return fmt.Errorf("already connected")
	}

	// Attempt TCP connection first
	conn, err := net.DialTimeout("tcp", fmt.Sprintf("%s:%d", c.host, c.containerPort), DefaultTimeout)
	if err != nil {
		return fmt.Errorf("failed to connect to LibPolyCall server: %w", err)
	}

	c.conn = conn
	c.connected = true

	// Start message handling goroutine
	go c.handleMessages()

	// Perform handshake
	if err := c.handshake(); err != nil {
		c.Disconnect()
		return fmt.Errorf("handshake failed: %w", err)
	}

	return nil
}

// Disconnect closes the connection to LibPolyCall server
func (c *PolyCallClient) Disconnect() error {
	c.mutex.Lock()
	defer c.mutex.Unlock()

	if !c.connected {
		return nil
	}

	c.cancel()
	
	if c.conn != nil {
		c.conn.Close()
		c.conn = nil
	}

	c.connected = false
	c.authenticated = false

	return nil
}

// SendHTTPRequest sends HTTP request to LibPolyCall server
func (c *PolyCallClient) SendHTTPRequest(method, path string, data interface{}) ([]byte, error) {
	url := fmt.Sprintf("http://%s:%d%s", c.host, c.containerPort, path)
	
	var reqBody io.Reader
	if data != nil {
		jsonData, err := json.Marshal(data)
		if err != nil {
			return nil, fmt.Errorf("failed to marshal request data: %w", err)
		}
		reqBody = bytes.NewReader(jsonData)
	}

	req, err := http.NewRequestWithContext(c.ctx, method, url, reqBody)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	if data != nil {
		req.Header.Set("Content-Type", "application/json")
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("request failed: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		return nil, fmt.Errorf("request failed with status %d", resp.StatusCode)
	}

	return io.ReadAll(resp.Body)
}

// SendCommand sends a command to LibPolyCall server
func (c *PolyCallClient) SendCommand(command string, data interface{}) ([]byte, error) {
	if !c.authenticated {
		return nil, fmt.Errorf("not authenticated")
	}

	payload := map[string]interface{}{
		"command": command,
		"data":    data,
	}

	payloadBytes, err := json.Marshal(payload)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal command payload: %w", err)
	}

	return c.sendMessage(MessageCommand, payloadBytes, FlagReliable)
}

// GetStates retrieves all states from LibPolyCall server
func (c *PolyCallClient) GetStates() ([]byte, error) {
	return c.SendHTTPRequest("GET", "/states", nil)
}

// GetState retrieves a specific state from LibPolyCall server
func (c *PolyCallClient) GetState(stateName string) ([]byte, error) {
	path := fmt.Sprintf("/states/%s", stateName)
	return c.SendHTTPRequest("GET", path, nil)
}

// TransitionTo executes a state transition
func (c *PolyCallClient) TransitionTo(stateName string, data interface{}) ([]byte, error) {
	path := fmt.Sprintf("/transition/%s", stateName)
	return c.SendHTTPRequest("POST", path, data)
}

// CreateBook creates a book (example API call)
func (c *PolyCallClient) CreateBook(title, author string) ([]byte, error) {
	bookData := map[string]string{
		"title":  title,
		"author": author,
	}
	return c.SendHTTPRequest("POST", "/books", bookData)
}

// GetBooks retrieves all books (example API call)
func (c *PolyCallClient) GetBooks() ([]byte, error) {
	return c.SendHTTPRequest("GET", "/books", nil)
}

// IsConnected returns whether the client is connected
func (c *PolyCallClient) IsConnected() bool {
	c.mutex.RLock()
	defer c.mutex.RUnlock()
	return c.connected
}

// IsAuthenticated returns whether the client is authenticated
func (c *PolyCallClient) IsAuthenticated() bool {
	c.mutex.RLock()
	defer c.mutex.RUnlock()
	return c.authenticated
}

// Private methods

func (c *PolyCallClient) handshake() error {
	magicBytes := make([]byte, 4)
	binary.LittleEndian.PutUint32(magicBytes, ProtocolMagic)
	
	_, err := c.sendMessage(MessageHandshake, magicBytes, FlagReliable)
	if err != nil {
		return fmt.Errorf("handshake message failed: %w", err)
	}

	// Wait for handshake response
	// In a full implementation, this would wait for the response
	c.authenticated = true
	return nil
}

func (c *PolyCallClient) sendMessage(msgType MessageType, payload []byte, flags ProtocolFlag) ([]byte, error) {
	if !c.connected || c.conn == nil {
		return nil, fmt.Errorf("not connected")
	}

	c.sequence++
	header := ProtocolHeader{
		Version:       ProtocolVersion,
		Type:          msgType,
		Flags:         flags,
		Sequence:      c.sequence,
		PayloadLength: uint32(len(payload)),
		Checksum:      calculateChecksum(payload),
	}

	// Create message
	var buf bytes.Buffer
	if err := binary.Write(&buf, binary.LittleEndian, header); err != nil {
		return nil, fmt.Errorf("failed to write header: %w", err)
	}
	buf.Write(payload)

	// Send message
	if _, err := c.conn.Write(buf.Bytes()); err != nil {
		return nil, fmt.Errorf("failed to send message: %w", err)
	}

	return payload, nil
}

func (c *PolyCallClient) handleMessages() {
	defer c.Disconnect()

	for {
		select {
		case <-c.ctx.Done():
			return
		default:
			// Read and process messages
			if err := c.readMessage(); err != nil {
				if c.connected {
					fmt.Printf("Message handling error: %v\n", err)
				}
				return
			}
		}
	}
}

func (c *PolyCallClient) readMessage() error {
	if c.conn == nil {
		return fmt.Errorf("connection is nil")
	}

	// Set read deadline
	c.conn.SetReadDeadline(time.Now().Add(DefaultTimeout))

	// Read header
	headerBytes := make([]byte, HeaderSize)
	if _, err := io.ReadFull(c.conn, headerBytes); err != nil {
		return fmt.Errorf("failed to read header: %w", err)
	}

	// Parse header
	var header ProtocolHeader
	if err := binary.Read(bytes.NewReader(headerBytes), binary.LittleEndian, &header); err != nil {
		return fmt.Errorf("failed to parse header: %w", err)
	}

	// Validate header
	if header.Version != ProtocolVersion {
		return fmt.Errorf("protocol version mismatch: expected %d, got %d", ProtocolVersion, header.Version)
	}

	if header.PayloadLength > MaxPayloadSize {
		return fmt.Errorf("payload size exceeds maximum: %d > %d", header.PayloadLength, MaxPayloadSize)
	}

	// Read payload
	payload := make([]byte, header.PayloadLength)
	if header.PayloadLength > 0 {
		if _, err := io.ReadFull(c.conn, payload); err != nil {
			return fmt.Errorf("failed to read payload: %w", err)
		}
	}

	// Verify checksum
	if calculateChecksum(payload) != header.Checksum {
		return fmt.Errorf("checksum verification failed")
	}

	// Process message based on type
	return c.processMessage(header, payload)
}

func (c *PolyCallClient) processMessage(header ProtocolHeader, payload []byte) error {
	switch header.Type {
	case MessageHandshake:
		// Handle handshake response
		return nil
	case MessageAuth:
		// Handle authentication response
		c.authenticated = true
		return nil
	case MessageResponse:
		// Handle command response
		if ch, exists := c.pendingReqs[header.Sequence]; exists {
			delete(c.pendingReqs, header.Sequence)
			ch <- payload
			close(ch)
		}
		return nil
	case MessageError:
		// Handle error response
		return fmt.Errorf("server error: %s", string(payload))
	case MessageHeartbeat:
		// Handle heartbeat
		return nil
	default:
		return fmt.Errorf("unknown message type: %d", header.Type)
	}
}

func calculateChecksum(data []byte) uint32 {
	hash := sha256.Sum256(data)
	return binary.LittleEndian.Uint32(hash[:4])
}

func loadConfiguration(configPaths ...string) (*Configuration, error) {
	var configPath string
	
	if len(configPaths) > 0 && configPaths[0] != "" {
		configPath = configPaths[0]
	} else {
		// Default configuration path
		configPath = filepath.Join("config", "go.polycallrc")
	}

	file, err := os.Open(configPath)
	if err != nil {
		return nil, fmt.Errorf("failed to open config file: %w", err)
	}
	defer file.Close()

	config := &Configuration{
		Port:               "3003:8083",
		ServerType:         "go",
		Workspace:          "/opt/polycall/services/go",
		LogLevel:           "info",
		MaxConnections:     100,
		SupportsFormatting: true,
		MaxMemory:          "1G",
		Timeout:            30,
		AllowRemote:        false,
		RequireAuth:        true,
		StrictPortBinding:  true,
		GoVersion:          "1.21",
	}

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}

		parts := strings.SplitN(line, "=", 2)
		if len(parts) != 2 {
			continue
		}

		key := strings.TrimSpace(parts[0])
		value := strings.TrimSpace(parts[1])

		switch key {
		case "port":
			config.Port = value
		case "server_type":
			config.ServerType = value
		case "workspace":
			config.Workspace = value
		case "log_level":
			config.LogLevel = value
		case "max_connections":
			if val, err := strconv.Atoi(value); err == nil {
				config.MaxConnections = val
			}
		case "supports_formatting":
			config.SupportsFormatting = value == "true"
		case "max_memory":
			config.MaxMemory = value
		case "timeout":
			if val, err := strconv.Atoi(value); err == nil {
				config.Timeout = val
			}
		case "allow_remote":
			config.AllowRemote = value == "true"
		case "require_auth":
			config.RequireAuth = value == "true"
		case "strict_port_binding":
			config.StrictPortBinding = value == "true"
		case "go_version":
			config.GoVersion = value
		}
	}

	return config, scanner.Err()
}
