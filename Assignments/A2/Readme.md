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
python3 dnsresolver.py iterative google.com
```

Output

```bash
[Iterative DNS Lookup] Resolving google.com
[DEBUG] Querying ROOT server (198.41.0.4) - SUCCESS
Extracted NS hostname: l.gtld-servers.net.
Extracted NS hostname: j.gtld-servers.net.
Extracted NS hostname: h.gtld-servers.net.
Extracted NS hostname: d.gtld-servers.net.
Extracted NS hostname: b.gtld-servers.net.
Extracted NS hostname: f.gtld-servers.net.
Extracted NS hostname: k.gtld-servers.net.
Extracted NS hostname: m.gtld-servers.net.
Extracted NS hostname: i.gtld-servers.net.
Extracted NS hostname: g.gtld-servers.net.
Extracted NS hostname: a.gtld-servers.net.
Extracted NS hostname: c.gtld-servers.net.
Extracted NS hostname: e.gtld-servers.net.
Resolved l.gtld-servers.net. to 192.41.162.30
Resolved j.gtld-servers.net. to 192.48.79.30
Resolved h.gtld-servers.net. to 192.54.112.30
Resolved d.gtld-servers.net. to 192.31.80.30
Resolved b.gtld-servers.net. to 192.33.14.30
Resolved f.gtld-servers.net. to 192.35.51.30
Resolved k.gtld-servers.net. to 192.52.178.30
Resolved m.gtld-servers.net. to 192.55.83.30
Resolved i.gtld-servers.net. to 192.43.172.30
Resolved g.gtld-servers.net. to 192.42.93.30
Resolved a.gtld-servers.net. to 192.5.6.30
Resolved c.gtld-servers.net. to 192.26.92.30
Resolved e.gtld-servers.net. to 192.12.94.30
[DEBUG] Querying TLD server (192.41.162.30) - SUCCESS
Extracted NS hostname: ns2.google.com.
Extracted NS hostname: ns1.google.com.
Extracted NS hostname: ns3.google.com.
Extracted NS hostname: ns4.google.com.
Resolved ns2.google.com. to 216.239.34.10
Resolved ns1.google.com. to 216.239.32.10
Resolved ns3.google.com. to 216.239.36.10
Resolved ns4.google.com. to 216.239.38.10
[DEBUG] Querying AUTH server (216.239.34.10) - SUCCESS
[SUCCESS] google.com -> 142.250.207.238
Time taken: 0.438 seconds
```

**Recursive Lookup:**

```bash
python3 dnsresolver.py recursive example.com
```

Output

```bash
[Recursive DNS Lookup] Resolving google.com
[SUCCESS] google.com -> ns4.google.com.
[SUCCESS] google.com -> ns3.google.com.
[SUCCESS] google.com -> ns2.google.com.
[SUCCESS] google.com -> ns1.google.com.
[SUCCESS] google.com -> 172.217.167.206
Time taken : 0.014 seconds
```

Ensure that the required dependency (`dnspython`) is installed. You can install it via pip:

```bash
pip install dnspython
```

This code implements a DNS resolution system that supports both iterative and recursive lookups as described in the assignment pdf.

## Overview

**Iterative DNS Resolution:**

- It begins by querying a predefined set of root servers.
- Extracts NS (nameserver) records from the authority section of the DNS responses.
- Iteratively queries successive levels (TLD and authoritative servers) until the final answer is obtained.
- Incorporates robust failover by iterating through available servers if one fails to respond.
- Captures the exception and prints it when it occurs.
- At last it prints the time taken for resolving the given domain.

**Recursive DNS Resolution:**

- It makes use of the system’s built-in recursive resolver (via `dns.resolver.resolve`) to retrieve the final IP address.
- Logs intermediate NS records to provide insight into the resolution process.
- Implements separate error handling for NS record lookups and A record lookups to allow clearer diagnostics.
- At last it prints the time taken for resolving the given domain.

## **Implementation Details**

### TODOs Implemented

1. **Sending DNS Queries via UDP**
   - Implemented `send_dns_query` function to construct and send a DNS query over UDP.
   - Used `dns.message.make_query(domain, dns.rdatatype.A)` to create a query.
   - Sent the query using `dns.query.udp(query, server, timeout=TIMEOUT)`.
2. **Resolving Next Nameservers**
   - Implemented `extract_next_nameservers` function to parse NS records from the authority section.
   - Extracted NS hostnames and resolved them to IP addresses using `dns.resolver.resolve(ns, "A")`.
3. **Iterative DNS Resolution Process**
   - Besides the given template, a loop is implemented over the nameserver list such that if one nameserver fails (or times out), the resolver checks with the next available server.
   - We also printed error when server not respond or time out
   - Started resolution from root servers.
   - Queried nameservers in a loop, extracting and resolving NS records at each stage (ROOT → TLD → AUTH).
   - Stopped when an IP address was found.
4. **Handling Errors**
   - Implemented error handling for DNS query failures (timeouts, unreachable servers, etc.).
   - If no response was received from any nameserver at a stage, resolution failed with an error message.
5. **Recursive DNS Resolution**
   - Implemented `recursive_dns_lookup` to use the system's default resolver.
   - Queried both `NS` and `A` records of the domain and printed results.

### Error Handling & Output

- **Query Failure**: If a query fails (timeout, unreachable server), an error is printed:
  ```
  [ERROR] Exception while querying server <IP>: <error message>
  ```
- **NS Resolution Failure**: If a nameserver's IP could not be resolved:
  ```
  [ERROR] Failed to resolve NS <hostname>: <error message>
  ```
- **Debug Messages:** Debug messages are printed to indicate successful queries, for example:
  ```
  [DEBUG] Querying {stage} server ({ns_ip}) - SUCCESS
  ```
- **Stage failure:** Added printing of errors when a server at a stage `(ROOT | TLD | AUTH)` does not respond or times out using:
  ```
  print(f"[ERROR] Query failed for {stage} server ({ns_ip})")
  ```
- **Final Resolution Failure**: If no nameserver responds at any stage:
  ```
  [ERROR] Resolution failed. No response from <stage> servers.
  ```
- **Empty Nameserver List:** If the nameserver list becomes empty during iterative resolution, the error is printed:
  ```
  print("[ERROR] List of nameservers is empty. Resolution failed.")
  ```
- **Recursive Lookup Error:** If an error occurs during recursive DNS lookup, it is handled by printing:

  ```
  print(f"[ERROR] Recursive lookup failed: {e}")
  ```

- **Successful Resolution Output**:
  ```
  [SUCCESS] <domain> -> <IP address>
  ```
- **Time Taken**: The execution time is printed at the end:
  ```
  Time taken: <seconds> seconds
  ```

### Additional Changes

#### Error Handling in sending query

- Added the try-except block around UDP query along with in `send_dns_query()` to catch exceptions (e.g., timeouts, unreachable servers) and log error messages.
  <br/>
- **Why:** Ensures the resolver handles network issues gracefully, in line with the assignment’s instructions to manage errors without crashing.

#### EDNS Extension Enabled

- Enabled EDNS with a 4096-byte UDP buffer by calling `query.use_edns(0, payload=4096)`.
- This increases the response capacity, which is especially useful when resolving large domains.

#### Improved Iterative Lookup – Server Failover

- Instead of selecting only the first server in the list, the code now loops through all available servers in the current nameserver list.
- If one server fails to respond, the next server is queried until a valid response is obtained or the list is exhausted.
- **Why:** [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/90)
  <br/> Enhances robustness and reliability by ensuring that transient failures on one server do not stop the resolution process.

### Assumptions

- **Nameserver List Order**: The order of nameservers is not sorted before printing.
  <br/>
  **Why?** [Piazza Post](https://piazza.com/class/m5h01uph1h12eb/post/112)
- **Answer Section Handling**: Assumed that `response.answer[0][0]` always contains an IP address and not a CNAME.
  <br/>
  **Why?** [Piazza Post](https://piazza.com/class/m5h01uph1h12eb/post/119)
- **IPv4 Only**: The implementation only supports IPv4 addresses (A records), and does not handle IPv6 (AAAA records).
  <br/>
  **Why?** [Piazza Post](https://piazza.com/class/m5h01uph1h12eb/post/110)
- **Using inbuilt dns resolver**: Used inbuilt dns resolver in `extract_next_nameservers` and recursive implementation.
  <br/>
  **Why?** [Piazza Post](https://piazza.com/class/m5h01uph1h12eb/post/114)

### Function Overview

| Function                             | Description                                                            |
| ------------------------------------ | ---------------------------------------------------------------------- |
| `send_dns_query(server, domain)`     | Sends a DNS query to the specified server for an A record.             |
| `extract_next_nameservers(response)` | Extracts and resolves NS records from a DNS response.                  |
| `iterative_dns_lookup(domain)`       | Performs iterative DNS resolution starting from root servers.          |
| `recursive_dns_lookup(domain)`       | Performs recursive DNS resolution using the system's default resolver. |

### **Code Flow**

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

### Testing & Validation

- **Timeout Testing:**

  - The implementation was tested by changing the `TIMEOUT` value to simulate slower responses and verify that the error handling correctly prints timeout errors.

- **Unavailable Root Servers:**

  - Some root servers were temporarily commented out from the `ROOT_SERVERS` list to simulate scenarios where certain servers are unreachable. This ensured that the iterative process moves on to the next available server and prints the appropriate error messages.

- **Fake Servers Testing:**
  - The resolver was also tested by substituting all nameservers with fake IP addresses. This test verified that the implementation correctly outputs all the error messages, including both the `[ERROR] Query failed for {stage} server ({ns_ip})` and the final failure message `[ERROR] List of nameservers is empty. Resolution failed.`
- **Automated Testing:**
  - A shell script (test_dns.sh) can be created to run automated tests on the resolver:
  - It tests both iterative and recursive modes.
  - It verifies behavior for valid domains (e.g., google.com, example.com) and for expected failures (e.g., nonexistent.domain).

## Contribution of Team Members

| Team Member              | Contribution (%) | Work Done                 |
| ------------------------ | :--------------: | ------------------------- |
| Manikanta <br/> (220409) |      33.33%      | Documentation and Testing |
| Rhema <br/> (221125)     |      33.33%      | Code and Comments         |
| Jyothisha <br/> (220862) |      33.34%      | Code and Comments         |

---

## Sources Referred

- [DNS python](https://dnspython.readthedocs.io/en/latest/manual.html) to understand different classes and inbuilt functions
- [DNS Network Programming](https://www.geeksforgeeks.org/network-programming-in-python-dns-look-up/) a geeks for geeks article

## Declaration

We, (**Manikanta, Rhema and Jyothisha**) declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.

**Note:** When testing with domains like google.com, you may notice that iterative and recursive queries return different IP addresses. This is expected behavior due to load balancing, round-robin DNS, caching differences, and geo-location optimizations used by many large-scale services.
