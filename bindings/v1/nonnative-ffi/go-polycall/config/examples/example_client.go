// Package main demonstrates Go PolyCall client usage
// Author: OBINexusComputing - Aegis Engineering Team
// Collaborator: Nnamdi Michael Okpala
package main

import (
	"encoding/json"
	"fmt"
	"log"
	"time"

	polycall "../src"
)

// BookData represents book information for API demonstrations
type BookData struct {
	Title  string `json:"title"`
	Author string `json:"author"`
	ID     string `json:"id,omitempty"`
}

// StateData represents state information from LibPolyCall
type StateData struct {
	Name     string `json:"name"`
	IsLocked bool   `json:"is_locked"`
	Endpoint string `json:"endpoint"`
}

func main() {
	fmt.Println("🔧 Aegis LibPolyCall Go Client Demonstration")
	fmt.Println("=" * 50)

	// Initialize client with configuration
	client, err := polycall.NewPolyCallClient()
	if err != nil {
		log.Fatalf("❌ Failed to initialize PolyCall client: %v", err)
	}

	// Demonstrate connection lifecycle
	fmt.Println("\n📡 Phase 1: Connection Management")
	if err := demonstrateConnection(client); err != nil {
		log.Printf("⚠️  Connection demonstration failed: %v", err)
	}

	// Demonstrate HTTP API calls
	fmt.Println("\n📋 Phase 2: HTTP API Interaction")
	if err := demonstrateHTTPAPI(client); err != nil {
		log.Printf("⚠️  HTTP API demonstration failed: %v", err)
	}

	// Demonstrate state management
	fmt.Println("\n🔄 Phase 3: State Management")
	if err := demonstrateStateManagement(client); err != nil {
		log.Printf("⚠️  State management demonstration failed: %v", err)
	}

	// Cleanup
	fmt.Println("\n🧹 Phase 4: Cleanup and Disconnection")
	if err := client.Disconnect(); err != nil {
		log.Printf("⚠️  Disconnection failed: %v", err)
	} else {
		fmt.Println("✅ Successfully disconnected from LibPolyCall server")
	}

	fmt.Println("\n🎯 Go PolyCall client demonstration completed")
}

func demonstrateConnection(client *polycall.PolyCallClient) error {
	fmt.Println("  🔌 Attempting connection to LibPolyCall server...")
	
	// Connection may fail if server isn't running - this is expected
	if err := client.Connect(); err != nil {
		fmt.Printf("  ⚠️  Connection failed (server may not be running): %v\n", err)
		fmt.Println("  💡 To run LibPolyCall server: ./bin/polycall -f /opt/polycall/config.Polycallfile")
		return nil // Continue with HTTP-only demonstrations
	}

	fmt.Printf("  ✅ Connected: %t\n", client.IsConnected())
	fmt.Printf("  ✅ Authenticated: %t\n", client.IsAuthenticated())
	
	return nil
}

func demonstrateHTTPAPI(client *polycall.PolyCallClient) error {
	// Demonstrate book creation
	fmt.Println("  📚 Creating test book...")
	book := BookData{
		Title:  "Go Programming Patterns",
		Author: "Aegis Engineering Team",
	}

	createResp, err := client.CreateBook(book.Title, book.Author)
	if err != nil {
		fmt.Printf("  ⚠️  Book creation failed: %v\n", err)
		fmt.Println("  💡 Ensure LibPolyCall server is running and accessible")
	} else {
		fmt.Printf("  ✅ Book created successfully: %s\n", string(createResp))
		
		// Parse response to show structured data
		var createdBook BookData
		if json.Unmarshal(createResp, &createdBook) == nil {
			fmt.Printf("     📖 Created: %s by %s\n", createdBook.Title, createdBook.Author)
		}
	}

	// Demonstrate book retrieval
	fmt.Println("  📚 Retrieving books list...")
	booksResp, err := client.GetBooks()
	if err != nil {
		fmt.Printf("  ⚠️  Books retrieval failed: %v\n", err)
	} else {
		fmt.Printf("  ✅ Books retrieved successfully: %s\n", string(booksResp))
		
		// Parse and display books
		var books []BookData
		if json.Unmarshal(booksResp, &books) == nil {
			fmt.Printf("     📊 Total books in system: %d\n", len(books))
			for i, book := range books {
				fmt.Printf("     %d. %s by %s\n", i+1, book.Title, book.Author)
			}
		}
	}

	return nil
}

func demonstrateStateManagement(client *polycall.PolyCallClient) error {
	// Demonstrate state retrieval
	fmt.Println("  🔄 Retrieving system states...")
	statesResp, err := client.GetStates()
	if err != nil {
		fmt.Printf("  ⚠️  States retrieval failed: %v\n", err)
		fmt.Println("  💡 State management requires LibPolyCall server with state machine enabled")
	} else {
		fmt.Printf("  ✅ States retrieved successfully: %s\n", string(statesResp))
		
		// Parse and display states
		var states []StateData
		if json.Unmarshal(statesResp, &states) == nil {
			fmt.Printf("     📊 Total states in system: %d\n", len(states))
			for _, state := range states {
				lockStatus := "unlocked"
				if state.IsLocked {
					lockStatus = "locked"
				}
				fmt.Printf("     🔹 %s (%s) - %s\n", state.Name, lockStatus, state.Endpoint)
			}
		}
	}

	// Demonstrate state transition
	fmt.Println("  🔄 Attempting state transition...")
	transitionData := map[string]interface{}{
		"reason": "Go client demonstration",
		"timestamp": time.Now().Unix(),
	}

	transitionResp, err := client.TransitionTo("ready", transitionData)
	if err != nil {
		fmt.Printf("  ⚠️  State transition failed: %v\n", err)
	} else {
		fmt.Printf("  ✅ State transition successful: %s\n", string(transitionResp))
	}

	return nil
}

// Additional utility functions for demonstration purposes

func demonstrateAdvancedFeatures(client *polycall.PolyCallClient) error {
	fmt.Println("  🚀 Advanced Feature Demonstrations")

	// Demonstrate command execution
	if client.IsAuthenticated() {
		fmt.Println("    🎯 Executing custom command...")
		cmdData := map[string]interface{}{
			"action": "status_check",
			"client": "go-polycall",
			"version": "1.0.0",
		}

		cmdResp, err := client.SendCommand("status", cmdData)
		if err != nil {
			fmt.Printf("    ⚠️  Command execution failed: %v\n", err)
		} else {
			fmt.Printf("    ✅ Command executed: %s\n", string(cmdResp))
		}
	}

	return nil
}

func printSystemInfo() {
	fmt.Println("\n📊 System Information:")
	fmt.Println("  🔧 LibPolyCall Go Binding v1.0.0")
	fmt.Println("  🏗️  Architecture: Program-first interface design")
	fmt.Println("  🔗 Protocol: LibPolyCall binary communication")
	fmt.Println("  🌐 Transport: HTTP/TCP hybrid approach")
	fmt.Println("  👥 Collaboration: Aegis Engineering Team & Nnamdi Okpala")
}
