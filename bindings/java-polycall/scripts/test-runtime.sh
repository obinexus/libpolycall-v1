#!/bin/bash
set -e

echo "Testing polycall.exe runtime connection..."

# Check if polycall.exe is running
if ! netstat -an | grep -q ":8084"; then
    echo "WARNING: polycall.exe runtime not detected on port 8084"
    echo "Start runtime with: polycall.exe server --port 8084"
    exit 1
fi

# Test connection
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test --host localhost --port 8084

echo "Runtime connection test complete"
