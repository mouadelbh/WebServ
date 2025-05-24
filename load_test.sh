#!/bin/bash

PORT=8082
NUM_CLIENTS=10  # Number of concurrent clients
TEST_URL="http://localhost:$PORT"

# Test function for each client
test_client() {
    local client_id=$1
    local start_time=$(date +%s.%N)
    response=$(curl -s -o /dev/null -w "%{http_code}" "$TEST_URL")
    local end_time=$(date +%s.%N)
    local elapsed=$(echo "$end_time - $start_time" | bc)

    if [ "$response" -eq 200 ]; then
        echo "Client $client_id: Success (${elapsed}s)"
    else
        echo "Client $client_id: Failed (HTTP $response)"
    fi
}

# Run tests in parallel
echo "Testing $NUM_CLIENTS concurrent clients..."
for i in $(seq 1 $NUM_CLIENTS); do
    test_client $i &
done

# Wait for all tests to complete
wait
echo "All tests completed"
