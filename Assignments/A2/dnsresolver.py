# importing the required libraries
import dns.message
import dns.query
import dns.rdatatype
import dns.resolver
import time

# Root DNS servers used to start the iterative resolution process
ROOT_SERVERS = {
    "198.41.0.4": "Root (a.root-servers.net)",
    "199.9.14.201": "Root (b.root-servers.net)",
    "192.33.4.12": "Root (c.root-servers.net)",
    "199.7.91.13": "Root (d.root-servers.net)",
    "192.203.230.10": "Root (e.root-servers.net)"
}

TIMEOUT = 3  # Timeout in seconds for each DNS query attempt

def send_dns_query(server, domain):
    """
    Sends a DNS query to the given server for an A record of the specified domain.
    Returns the response if successful, otherwise returns None.
    """
    try:
        # Construct the DNS query for A record
        query = dns.message.make_query(domain, dns.rdatatype.A)

        # Send the query over UDP and storing the response
        response = dns.query.udp(query, server, timeout=TIMEOUT)

        return response
    except Exception as e:
        print(f"[ERROR] Exception while querying server {server}: {e}")
        return None # If an error occurs (timeout, unreachable server, etc.), return None

def extract_next_nameservers(response):
    """
    Extracts NS records from the authority section of the DNS response.
    Then resolves each NS hostname to its corresponding IP address.
    Returns a list of IP addresses for the next nameservers.
    """
    ns_ips = []    # List to store resolved nameserver IP addresses
    ns_names = []  # List to store nameserver domain names


    # Loop through the authority section to extract NS records
    '''
    This section of code loops over each resource record set (rrset) in the authority section of the DNS response.
    It checks if the record type is NS (nameserver).
    If it is NS record, it converts the NS record to text and adds it to the ns_names list and also prints it.
    '''
    for rrset in response.authority:
        if rrset.rdtype == dns.rdatatype.NS:
            for rr in rrset:
                ns_name = rr.to_text()
                ns_names.append(ns_name)
                print(f"Extracted NS hostname: {ns_name}")


    # Resolve each nameserver hostname to an IP address
    '''
    This section of code loops over each nameserver in the ns_names list and tries to resolve the nameserver to an IP address using the system's default DNS resolver.
    If the resolution is successful, it adds the IP address to the ns_ips list and prints the resolution success message.
    If the resolution fails, it prints an error message.
    '''
    for ns in ns_names:
        try:
            # Using system resolver to get the A record for the nameserver
            answer = dns.resolver.resolve(ns, "A")
            for rdata in answer:
                ns_ip = rdata.to_text()
                ns_ips.append(ns_ip)
                print(f"Resolved {ns} to {ns_ip}")
        except Exception as e:
            print(f"[ERROR] Failed to resolve NS {ns}: {e}")

    # Return the list of resolved nameserver IP addresses
    return ns_ips

def iterative_dns_lookup(domain):
    """
    Performs an iterative DNS resolution starting from the root servers.
    It queries root servers, then TLD servers, then authoritative servers,
    following the hierarchy until an answer is found or resolution fails.
    """

    '''
    Now if one server fails to respond, the code will move to the next server in the list.
    If all servers fail to respond, the code will print a final failure message.
    '''

    # Print a message indicating the start of iterative resolution for a given domain
    print(f"[Iterative DNS Lookup] Resolving {domain}")

    next_ns_list = list(ROOT_SERVERS.keys())  # Start with the root server IPs
    stage = "ROOT"  # Track resolution stage (ROOT, TLD, AUTH)

    while next_ns_list:
        response_received = False  # Flag to track if any server in the current list responded

        # Loop through the list of nameservers to query until receiving a response
        for ns_ip in next_ns_list:
            response = send_dns_query(ns_ip, domain)

            if response:  # If response is received
                response_received = True
                print(f"[DEBUG] Querying {stage} server ({ns_ip}) - SUCCESS")
                
                # If an answer is found in the response, print and return it
                if response.answer:
                    print(f"[SUCCESS] {domain} -> {response.answer[0][0]}")
                    return
                
                # # If no answer, extract the next set of nameservers from the authority section of the response
                next_ns_list = extract_next_nameservers(response)
                break  # Stop querying other nameservers in the list
            else:
                print(f"[ERROR] Query failed for {stage} server ({ns_ip})")

        # If none of the servers in the current list responded, resolution fails
        if not response_received:
            print(f"[ERROR] Resolution failed. No response from {stage} servers: {next_ns_list}")
            return  # Stop resolution if no nameservers respond
               
        # Update resolution stage: from ROOT to TLD to AUTH
        if stage == "ROOT":
            stage = "TLD"
        elif stage == "TLD":
            stage = "AUTH"
        # If already AUTH, we continue querying using the provided nameservers
        
    
    # If the loop completes without returning (when list is empty), resolution fails
    print("[ERROR] List of nameservers is empty. Resolution failed.") # Final failure message if no nameservers respond
    return

def recursive_dns_lookup(domain):
    """ 
    Performs recursive DNS resolution using the system's default resolver.
    This approach relies on a resolver (like Google DNS or a local ISP resolver)
    to fetch the result recursively.
    """
    print(f"[Recursive DNS Lookup] Resolving {domain}")
       
    try:
       # Using system resolver to get the NS for the domain
        answer = dns.resolver.resolve(domain, "NS")
        for rdata in answer: # Loop over the NS records
            print(f"[SUCCESS] {domain} -> {rdata}")
    except Exception as e:
        print(f"[ERROR] Recursive lookup failed: {e}")  # Handle resolution failure


    try:
        # Using system resolver to get the A record for the domain
        answer = dns.resolver.resolve(domain, "A")
        for rdata in answer: # Loop over the A records
            print(f"[SUCCESS] {domain} -> {rdata}")
    except Exception as e:
        print(f"[ERROR] Recursive lookup failed: {e}")  # Handle resolution failure

if __name__ == "__main__":
    import sys

    '''
    This section of code checks if the correct number of arguments i.e., 3 are provided.
    If the number of arguments is incorrect, the code prints a usage message and exits.
    If the number of arguments is correct, the code proceeds to get the mode and domain from the command line arguments.
    It also records the start time of the DNS resolution process to calculate the execution time.
    '''
    if len(sys.argv) != 3 or sys.argv[1] not in {"iterative", "recursive"}:
        print("Usage: python3 dnsresolver.py <iterative|recursive> <domain>")
        sys.exit(1)

    mode = sys.argv[1]  # Get the mode: iterative or recursive
    domain = sys.argv[2]  # Get the domain to resolve
    start_time = time.time()  # Record start time

    # Execute the selected DNS resolution mode
    if mode == "iterative":
        iterative_dns_lookup(domain)
    else:
        recursive_dns_lookup(domain)

    print(f"Time taken: {time.time() - start_time:.3f} seconds")  # Print execution time
