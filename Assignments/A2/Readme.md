# **CS425 Assignment-2:** _DNS Resolution_

## Team Members

| Team members | Roll no. |
| ------------ | :------: |
| Manikanta    |  220409  |
| Rhema        |  221125  |
| Jyothisha    |  220862  |

## How to Run the Code

To run the DNS resolver, use the following commands from your terminal:

**Iterative Lookup:**

```bash
python3 dnsresolver.py iterative example.com
```

**Recursive Lookup:**

```bash
python3 dnsresolver.py recursive example.com
```

Ensure that the required dependency (`dnspython`) is installed. You can install it via pip:

```bash
pip install dnspython
```

This code implements a DNS resolution system that supports both iterative and recursive lookups as described in the assignment pdf.

## Overview

The resolver supports two types of DNS resolution:

**Iterative DNS Resolution:**

- Begins with querying a list of root servers.
- Extracts NS (nameserver) records from the authority section.
- Iteratively queries through successive levels (ROOT, TLD, AUTH) until an answer is found.
- Implements robust server failover by iterating through available servers in each stage.

**Recursive DNS Resolution:**

- Uses the system’s built-in recursive resolver (`dns.resolver.resolve`) to obtain the final IP address.
- Retrieves and logs NS records as intermediate steps.
- Separates error handling for NS and A record lookups for clearer diagnostics.

## **Implementation Details**

### 1. Query in `send_dns_query`

- **What Changed:**
  - Added the UDP query along with try-except block around to catch exceptions (e.g., timeouts, unreachable servers) and log error messages.
- **Why:**
  - Ensures the resolver handles network issues gracefully, in line with the assignment’s instructions to manage errors without crashing.

### 2. Improved Iterative Lookup – Server Failover

- **What Changed:**
  - Instead of selecting only the first server in the list, the code now loops through all available servers in the current nameserver list.
  - If one server fails to respond, the next server is queried until a valid response is obtained or the list is exhausted.
- **Why:**
  - [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/90)
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

## Code Flow

```bash
dnsresolver.py
├── main()
│   ├── Validate command-line arguments (mode & domain)
│   ├── Record start time
│   ├── if mode == "iterative"
│   │   └── iterative_dns_lookup(domain)
│   │       ├── Initialize next_ns_list = list(ROOT_SERVERS.keys())
│   │       ├── Set stage = "ROOT"
│   │       ├── While next_ns_list is not empty:
│   │       │   ├── response_received ← False
│   │       │   ├── For each ns_ip in next_ns_list:
│   │       │   │   ├── Call send_dns_query(ns_ip, domain)
│   │       │   │   ├── If response is received:
│   │       │   │   │   ├── Set response_received ← True
│   │       │   │   │   ├── If response.answer exists:
│   │       │   │   │   │   └── Print answer and exit iterative lookup
│   │       │   │   │   └── Else:
│   │       │   │   │       └── Call extract_next_nameservers(response) to update next_ns_list; break loop
│   │       │   │   └── Else:
│   │       │   │       └── Print error for ns_ip and continue loop
│   │       │   ├── If no server in the current list responded:
│   │       │   │   └── Print final error and exit iterative lookup
│   │       │   └── Update stage (ROOT → TLD → AUTH) as needed; loop continues
│   │       └── End iterative_dns_lookup
│   └── Else if mode == "recursive"
│       └── recursive_dns_lookup(domain)
│           ├── Try: Use dns.resolver.resolve(domain, "NS")
│           │   └── For each NS record: print NS record
│           ├── Try: Use dns.resolver.resolve(domain, "A")
│           │   └── For each A record: print A record
│           └── Handle exceptions separately for NS and A lookups
└── After lookup, Print total time taken and exit
```

## Challenges and Solutions

| **Challenge**                                      | **Description**                                                                                     | **Solution**                                                                                                                                |
| -------------------------------------------------- | --------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| Network Timeouts & Unreachable Servers             | DNS queries may fail due to network issues such as timeouts or unreachable servers.                 | Added try-except blocks in `send_dns_query` to catch exceptions (e.g., timeouts) and log errors, returning None if a query fails.           |
| Single Server Dependency in Iterative Mode         | Relying on a single nameserver can cause the lookup to fail if that server is unresponsive.         | Modified the iterative lookup to iterate through the list of nameservers. If one fails, the code proceeds to the next available server.     |
| NS Record Extraction Issues                        | Extracting NS records from the DNS response can be problematic if records are missing or malformed. | Enhanced `extract_next_nameservers` with detailed logging and exception handling when resolving NS hostnames to their IP addresses.         |
| Ambiguous Error Identification in Recursive Lookup | Without separation, it's unclear whether failures occur during NS or A record lookups.              | Separated error handling into two distinct try-except blocks in `recursive_dns_lookup` for NS and A lookups to provide clearer diagnostics. |
| Code Readability and Maintainability               | Complex DNS resolution flows can reduce code clarity and hinder future maintenance.                 | Added comprehensive inline comments and documentation throughout the code, making the logic and decision points clear.                      |
| Handling Non-Existent Domains                      | Lookups for domains that do not exist may result in unhandled exceptions or generic error messages. | Implemented error logging for NXDOMAIN and other DNS error codes to clearly indicate resolution failures due to non-existent domains.       |

## **Testing**

A shell script (test_dns.sh) can be created to run automated tests on the resolver:

- It tests both iterative and recursive modes.
- It verifies behavior for valid domains (e.g., google.com, example.com) and for expected failures (e.g., nonexistent.domain).

## Contribution of Team Members

| Team Member              | Contribution (%) | Work Done         |
| ------------------------ | :--------------: | ----------------- |
| Manikanta <br/> (220409) |      33.33%      | Documentation     |
| Rhema <br/> (221125)     |      33.33%      | Code and Comments |
| Jyothisha <br/> (220862) |      33.34%      | Code and Comments |

---

## Sources Referred

- [DNS python](https://dnspython.readthedocs.io/en/latest/manual.html) to understand different classes and inbuilt functions

## Declaration

We, (**Manikanta, Rhema and Jyothisha**) declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.
