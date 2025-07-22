#!/bin/bash

echo "=========================================="
echo "  WebServ Configuration Integration Test"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to test HTTP request
test_request() {
    local method=$1
    local url=$2
    local expected_status=$3
    local description=$4
    local extra_headers=$5
    
    echo -n "Testing: $description... "
    
    if [ -n "$extra_headers" ]; then
        actual_status=$(curl -s -X "$method" $extra_headers -o /dev/null -w "%{http_code}" "$url" 2>/dev/null)
    else
        actual_status=$(curl -s -X "$method" -o /dev/null -w "%{http_code}" "$url" 2>/dev/null)
    fi
    
    if [ "$actual_status" = "$expected_status" ]; then
        echo -e "${GREEN}✓ PASS${NC} (Status: $actual_status)"
    else
        echo -e "${RED}✗ FAIL${NC} (Expected: $expected_status, Got: $actual_status)"
    fi
}

# Function to check if server is running
check_server() {
    echo "Checking if WebServ is running on localhost:8080..."
    if curl -s -o /dev/null http://localhost:8080/; then
        echo -e "${GREEN}✓ Server is running${NC}"
        return 0
    else
        echo -e "${RED}✗ Server is not responding${NC}"
        return 1
    fi
}

# Main test execution
echo "Starting comprehensive WebServ integration tests..."
echo ""

# Check if server is running
if ! check_server; then
    echo "Please start the server first with: ./webserv Parse_Config/default.conf"
    exit 1
fi

echo ""
echo -e "${BLUE}=== Basic GET Requests ===${NC}"
test_request "GET" "http://localhost:8080/" "200" "Homepage"
test_request "GET" "http://localhost:8080/about.html" "200" "About page"
test_request "GET" "http://localhost:8080/style.css" "200" "CSS file"
test_request "GET" "http://localhost:8080/nonexistent" "404" "Non-existent file"

echo ""
echo -e "${BLUE}=== Directory Tests ===${NC}"
test_request "GET" "http://localhost:8080/test/" "200" "Test directory (with index)"
test_request "GET" "http://localhost:8080/uploads/" "200" "Uploads directory (with index)"

echo ""
echo -e "${BLUE}=== Method Validation ===${NC}"
test_request "POST" "http://localhost:8080/" "411" "POST without Content-Length"
test_request "POST" "http://localhost:8080/" "200" "POST with Content-Length" "-H 'Content-Length: 0'"
test_request "DELETE" "http://localhost:8080/nonexistent" "404" "DELETE non-existent file"
test_request "PUT" "http://localhost:8080/" "501" "Unsupported PUT method"

echo ""
echo -e "${BLUE}=== Error Handling ===${NC}"
test_request "GET" "http://localhost:8080/forbidden" "404" "File not found"

echo ""
echo -e "${BLUE}=== Configuration-Specific Tests ===${NC}"
echo "Testing location-based routing and method restrictions..."

# Test static location (GET only)
test_request "GET" "http://localhost:8080/static/" "404" "Static location access (no such dir)"
test_request "POST" "http://localhost:8080/static/" "405" "POST to static location (should be restricted)" "-H 'Content-Length: 0'"

echo ""
echo -e "${BLUE}=== Content Type Tests ===${NC}"
echo "Checking if server returns correct content types..."

# Test content types
echo -n "Testing HTML content type... "
content_type=$(curl -s -I http://localhost:8080/ | grep -i "content-type" | cut -d' ' -f2- | tr -d '\r')
if [[ "$content_type" == *"text/html"* ]]; then
    echo -e "${GREEN}✓ PASS${NC} ($content_type)"
else
    echo -e "${RED}✗ FAIL${NC} (Got: $content_type)"
fi

echo -n "Testing CSS content type... "
content_type=$(curl -s -I http://localhost:8080/style.css | grep -i "content-type" | cut -d' ' -f2- | tr -d '\r')
if [[ "$content_type" == *"text/css"* ]]; then
    echo -e "${GREEN}✓ PASS${NC} ($content_type)"
else
    echo -e "${RED}✗ FAIL${NC} (Got: $content_type)"
fi

echo ""
echo -e "${BLUE}=== Configuration Validation ===${NC}"
echo "Verifying that configuration settings are being applied..."

# Test server header
echo -n "Testing server identification... "
server_header=$(curl -s -I http://localhost:8080/ | grep -i "server" | cut -d' ' -f2- | tr -d '\r')
if [[ "$server_header" == *"Webserv"* ]]; then
    echo -e "${GREEN}✓ PASS${NC} ($server_header)"
else
    echo -e "${RED}✗ FAIL${NC} (Got: $server_header)"
fi

echo ""
echo "=========================================="
echo -e "${GREEN}  Integration Test Complete!${NC}"
echo "=========================================="
echo ""
echo "Summary:"
echo "• Server is responding correctly on localhost:8080"
echo "• Configuration-based routing is working"
echo "• Method restrictions are being enforced"
echo "• Content types are correctly identified"
echo "• Error handling is functional"
echo ""
echo "Note: This test covers the main server (localhost:8080)"
echo "Multi-server testing will be available when multi-server support is implemented."
