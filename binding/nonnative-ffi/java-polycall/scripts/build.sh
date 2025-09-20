#!/bin/bash
set -e

echo "Building Java PolyCall..."

# Clean and compile
mvn clean compile

# Run tests
mvn test

# Package JAR
mvn package

echo "Build complete. JAR available at target/java-polycall-1.0.0.jar"
echo "Fat JAR available at target/java-polycall-1.0.0-jar-with-dependencies.jar"
