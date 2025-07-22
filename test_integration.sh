#!/bin/bash

echo "=== WebServ Configuration Integration Test ==="
echo "Testing the integrated configuration system..."

# Check if webserv executable exists
if [ ! -f "./webserv" ]; then
    echo "ERROR: webserv executable not found!"
    exit 1
fi

# Check if config file exists
if [ ! -f "Parse_Config/default.conf" ]; then
    echo "ERROR: Configuration file not found!"
    exit 1
fi

echo "✓ WebServ executable found"
echo "✓ Configuration file found"

# Test configuration parsing
echo ""
echo "=== Configuration Content ==="
echo "Current configuration:"
cat Parse_Config/default.conf

echo ""
echo "=== Starting WebServ ==="
echo "WebServ will start with the new integrated configuration..."
echo "You can test it by opening:"
echo "  - http://localhost:8080 (main server)"
echo "  - http://127.0.0.1:8081 (admin server)"  
echo "  - http://localhost:8082 (test server)"
echo ""
echo "Press Ctrl+C to stop the server when done testing"
echo ""

# Start the server
./webserv
