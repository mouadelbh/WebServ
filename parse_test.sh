#!/bin/bash

HOST=localhost
PORT=8080

function send_partial_request() {
  echo "Test: Incomplete request line with delay"
  {
    printf 'GE'
    sleep 1
    printf 'T / HTTP/1.1\r\nHost: example.com\r\n\r\n'
  } | nc $HOST $PORT
  echo -e "\n---"
}

function send_malformed_request() {
  echo "Test: Malformed request line (bad method)"
  printf "G3T / HTTP/1.1\r\nHost: example.com\r\n\r\n" | nc $HOST $PORT
  echo -e "\n---"
}

function send_incomplete_headers() {
  echo "Test: Incomplete headers with delay"
  {
    printf "GET / HTTP/1.1\r\nHos"
    sleep 1
    printf "t: example.com\r\n\r\n"
  } | nc $HOST $PORT
  echo -e "\n---"
}

function send_malformed_headers() {
  echo "Test: Malformed header line (missing colon)"
  printf "GET / HTTP/1.1\r\nHost example.com\r\n\r\n" | nc $HOST $PORT
  echo -e "\n---"
}

function send_missing_host() {
  echo "Test: Missing Host header"
  printf "GET / HTTP/1.1\r\nUser-Agent: test\r\n\r\n" | nc $HOST $PORT
  echo -e "\n---"
}

function send_incomplete_body() {
  echo "Test: Incomplete POST body with delay"
  {
    printf "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 5\r\n\r\nabc"
    sleep 1
    printf "de"
  } | nc $HOST $PORT
  echo -e "\n---"
}

function send_body_too_large() {
  echo "Test: Body too large"
  printf "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 100000\r\n\r\n" | nc $HOST $PORT
  echo -e "\n---"
}

function send_unsupported_method() {
  echo "Test: Unsupported HTTP method"
  printf "TRACE / HTTP/1.1\r\nHost: example.com\r\n\r\n" | nc $HOST $PORT
  echo -e "\n---"
}

function send_unsupported_version() {
  echo "Test: Unsupported HTTP version"
  printf "GET / HTTP/2.0\r\nHost: example.com\r\n\r\n" | nc $HOST $PORT
  echo -e "\n---"
}

# Run all tests
send_partial_request
send_malformed_request
send_incomplete_headers
send_malformed_headers
send_missing_host
send_incomplete_body
send_body_too_large
send_unsupported_method
send_unsupported_version
