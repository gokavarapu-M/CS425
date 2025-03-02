# **CS425 Assignment-2:** _DNS Resolution_

## Team Members

| Team members | Roll no. |
| ------------ | :------: |
| Manikanta    |  220409  |
| Rhema        |  221125  |
| Jyothisha    |  220862  |

## How to Run the Code

To run the DNS resolver, use the following commands from your terminal:

- **Iterative Lookup:**
  ```bash
  python3 dnsresolver.py iterative example.com
  ```
- **Recursive Lookup:**
  ```bash
  python3 dnsresolver.py recursive example.com
  ```
  Ensure that the required dependency (`dnspython`) is installed. You can install it via pip:

```bash
pip install dnspython
```

This assignment implements a DNS resolution system that supports both iterative and recursive lookups as described in the assignment.

## Overview

The resolver supports two types of DNS resolution:

- **Iterative DNS Resolution:**
  - Begins with querying a list of root servers.
  - Extracts NS (nameserver) records from the authority section.
  - Iteratively queries through successive levels (ROOT, TLD, AUTH) until an answer is found.
  - Implements robust server failover by iterating through available servers in each stage.
- **Recursive DNS Resolution:**
  - Uses the system’s built-in recursive resolver (`dns.resolver.resolve`) to obtain the final IP address.
  - Retrieves and logs NS records as intermediate steps.
  - Separates error handling for NS and A record lookups for clearer diagnostics.

## **Implementation Details**

### 1. Error Handling in `send_dns_query`

- **What Changed:**
  - Added a try-except block around the UDP query to catch exceptions (e.g., timeouts, unreachable servers) and log error messages.
- **Why:**
  - Ensures the resolver handles network issues gracefully, in line with the assignment’s instructions to manage errors without crashing.

### 2. Improved Iterative Lookup – Server Failover

- **What Changed:**
  - Instead of selecting only the first server in the list, the code now loops through all available servers in the current nameserver list.
  - If one server fails to respond, the next server is queried until a valid response is obtained or the list is exhausted.
- **Why:**
  - Enhances robustness and reliability by ensuring that transient failures on one server do not stop the resolution process.

### 3. Enhanced NS Record Extraction and Resolution

- **What Changed:**
  - Detailed logging has been added during the extraction of NS records from the authority section.
  - When resolving each NS hostname to its corresponding A record, exceptions are caught and logged.
- **Why:**
  - Provides clear traceability and debugging information, making it easier to diagnose failures during the referral process.

### 4. Detailed Exception Handling in Recursive Lookup

- **What Changed:**
  - The recursive lookup function now separates the NS record query and the A record query into two try-except blocks.
  - Specific error messages are logged for failures in either step.
- **Why:**
  - Helps distinguish whether failures occur during the intermediate NS lookup or the final A record lookup, aligning with the assignment’s requirements for graceful error handling.

### 5. Comprehensive Inline Documentation

- **What Changed:**
  - The `dnsresolver.py` now include detailed inline comments explaining each function and block of code.
- **Why:**
  - Improves code readability and maintainability.
  - Meets the assignment’s rubric for code quality (15% for comments) and documentation (25%).

## **Testing**

A shell script (test_dns.sh) can be created to run automated tests on the resolver:

- It tests both iterative and recursive modes.
- It verifies behavior for valid domains (e.g., google.com, example.com) and for expected failures (e.g., nonexistent.domain).
