#!/bin/bash
# test_dns.sh - Script to test the DNS resolver (dnsresolver.py) for both iterative and recursive modes

# List of domains to test
TEST_DOMAINS=("google.com" "example.com" "nonexistent.domain")

# Modes to test: iterative and recursive
MODES=("iterative" "recursive")

# Check if dnsresolver.py exists
if [ ! -f dnsresolver.py ]; then
  echo "Error: dnsresolver.py not found in the current directory."
  exit 1
fi

# Loop through each mode and domain
for mode in "${MODES[@]}"; do
  echo "========================================"
  echo "Testing mode: $mode"
  echo "========================================"
  
  for domain in "${TEST_DOMAINS[@]}"; do
    echo "----------------------------------------"
    echo "Testing $mode lookup for domain: $domain"
    echo "----------------------------------------"
    
    # Execute the resolver script with the specified mode and domain
    python3 dnsresolver.py "$mode" "$domain"
    
    # Capture the exit status and print a message if the test failed
    status=$?
    if [ $status -ne 0 ]; then
      echo "[ERROR] $mode lookup for $domain failed with exit status $status"
    else
      echo "[INFO] $mode lookup for $domain completed."
    fi
    
    echo ""
    # Pause briefly between tests
    sleep 1
  done
done

echo "All tests completed."
