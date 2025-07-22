#!/bin/bash

echo "=== WebServ Configuration Validator ==="

CONFIG_FILE="Parse_Config/default.conf"

if [ ! -f "$CONFIG_FILE" ]; then
    echo "ERROR: Configuration file $CONFIG_FILE not found!"
    exit 1
fi

echo "Validating configuration file: $CONFIG_FILE"
echo ""

# Count servers
SERVER_COUNT=$(grep -c "^server {" "$CONFIG_FILE")
echo "✓ Found $SERVER_COUNT server(s) configured"

# Check for required directives
echo ""
echo "=== Server Configuration Analysis ==="

grep -n "listen" "$CONFIG_FILE" | while read line; do
    echo "✓ $line"
done

grep -n "server_name" "$CONFIG_FILE" | while read line; do
    echo "✓ $line"
done

grep -n "root" "$CONFIG_FILE" | while read line; do
    echo "✓ $line"
done

echo ""
echo "=== Location Blocks ==="
LOCATION_COUNT=$(grep -c "location" "$CONFIG_FILE")
echo "✓ Found $LOCATION_COUNT location block(s) configured"

grep -n "location" "$CONFIG_FILE" | while read line; do
    echo "  $line"
done

echo ""
echo "=== Error Pages ==="
ERROR_PAGE_COUNT=$(grep -c "error_page" "$CONFIG_FILE")
echo "✓ Found $ERROR_PAGE_COUNT error page(s) configured"

echo ""
echo "=== Directory Structure Check ==="

# Check if required directories exist
DIRS=("www" "www/status_errors" "www/uploads" "www/admin" "www/test")

for dir in "${DIRS[@]}"; do
    if [ -d "$dir" ]; then
        echo "✓ Directory $dir exists"
    else
        echo "✗ Directory $dir missing"
    fi
done

echo ""
echo "=== File Structure Check ==="

# Check if required files exist
FILES=("www/index.html" "www/admin/admin.html" "www/test/index.html" "www/uploads/index.html")

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ File $file exists"
    else
        echo "✗ File $file missing"
    fi
done

echo ""
echo "Configuration validation complete!"
echo "Use './test_integration.sh' to test the server with this configuration."
