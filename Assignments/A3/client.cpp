#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

// Define server port as per assignment instructions
#define SERVER_PORT 12345

// Define hard-coded sequence numbers based on server logic:
// Client's initial SYN sequence: 200
// Server's SYN-ACK: server sequence 400, acknowledgment of 201 (200+1)
// Client's final ACK: sequence 600, acknowledgment of server's seq+1 (400+1)
#define CLIENT_SYN_SEQ 200
#define CLIENT_ACK_FINAL_SEQ 600
#define SERVER_SYN_SEQ 400

// Packet size = IP header + TCP header
#define PACKET_SIZE (sizeof(struct iphdr) + sizeof(struct tcphdr))

// Function to create and return a raw socket with IP_HDRINCL enabled.
int create_raw_socket()
{
    // Create raw socket with TCP protocol
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Tell the kernel that headers are included in the packet
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
    {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

// Function to construct and send a TCP packet given the required header fields.
void send_packet(int sock, const char *src_ip, int src_port, const char *dst_ip, int dst_port,
                 uint32_t seq, uint32_t ack_seq, bool syn, bool ack)
{
    // Allocate buffer for the packet
    char packet[PACKET_SIZE];
    memset(packet, 0, sizeof(packet));

    // Pointers to the IP header and TCP header parts of the packet buffer
    struct iphdr *ip = (struct iphdr *)packet;
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));

    // ---------------- IP HEADER ----------------
    ip->ihl = 5;     // IP header length
    ip->version = 4; // IPv4
    ip->tos = 0;     // Type of service
    ip->tot_len = htons(PACKET_SIZE);
    ip->id = htons(54321);         // Identification (arbitrary)
    ip->frag_off = 0;              // Fragment offset
    ip->ttl = 64;                  // Time to live
    ip->protocol = IPPROTO_TCP;    // Protocol
    ip->saddr = inet_addr(src_ip); // Source IP address
    ip->daddr = inet_addr(dst_ip); // Destination IP address

    // ---------------- TCP HEADER ----------------
    tcp->source = htons(src_port);         // Source port
    tcp->dest = htons(dst_port);           // Destination port
    tcp->seq = htonl(seq);                 // Sequence number as provided
    tcp->ack_seq = htonl(ack_seq);         // Acknowledgment number as provided
    tcp->doff = sizeof(struct tcphdr) / 4; // Data offset (header size in 32-bit words)

    // Set flags: SYN and ACK as needed
    tcp->syn = syn ? 1 : 0;
    tcp->ack = ack ? 1 : 0;
    tcp->fin = 0;
    tcp->rst = 0;
    tcp->psh = 0;

    tcp->window = htons(8192); // Window size (arbitrary valid value)
    tcp->check = 0;            // Checksum (set to 0; kernel may compute it)

    // Destination address structure required for sendto()
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dst_port);
    dest_addr.sin_addr.s_addr = inet_addr(dst_ip);

    // Send the packet using sendto()
    if (sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror("sendto() failed");
    }
    else
    {
        std::cout << "[+] Packet sent: "
                  << "seq=" << seq << " ack_seq=" << ack_seq
                  << " SYN=" << tcp->syn << " ACK=" << tcp->ack << std::endl;
    }
}

int main()
{
    // Assume the client is running on localhost and connecting to server on localhost.
    const char *client_ip = "127.0.0.1";
    const char *server_ip = "127.0.0.1";
    // Use an arbitrary client source port (must be >1024)
    int client_port = 54321;

    // Create raw socket for sending and receiving
    int sock = create_raw_socket();

    // ----------- STEP 1: Send SYN packet -----------
    std::cout << "[*] Sending SYN packet..." << std::endl;
    // SYN packet: SYN set, no ACK, sequence number = 200, ack_seq = 0 (not used)
    send_packet(sock, client_ip, client_port, server_ip, SERVER_PORT, CLIENT_SYN_SEQ, 0, true, false);

    // ----------- STEP 2: Wait for SYN-ACK response -----------
    std::cout << "[*] Waiting for SYN-ACK response..." << std::endl;
    char buffer[65536];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    bool synack_received = false;
    uint32_t server_seq = 0; // Will be used to compute ack for the final packet

    while (!synack_received)
    {
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &addr_len);
        if (data_size < 0)
        {
            perror("recvfrom() failed");
            continue;
        }

        // Parse the IP and TCP headers from the received packet
        struct iphdr *ip = (struct iphdr *)buffer;
        // TCP header offset is computed using IP header length (ip->ihl)
        struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip->ihl * 4));

        // Check if the packet is from the server port (i.e. source port in packet) and for our client port (destination)
        if (ntohs(tcp->source) != SERVER_PORT || ntohs(tcp->dest) != client_port)
            continue;

        // Check that this is the SYN-ACK packet (SYN and ACK flags set)
        if (tcp->syn == 1 && tcp->ack == 1)
        {
            // The server should be acknowledging our SYN, hence ack_seq should equal CLIENT_SYN_SEQ + 1 (i.e., 201)
            uint32_t expected_ack = CLIENT_SYN_SEQ + 1;
            if (ntohl(tcp->ack_seq) != expected_ack)
            {
                std::cerr << "[-] Received SYN-ACK with incorrect ack_seq: Expected " << expected_ack
                          << ", Got " << ntohl(tcp->ack_seq) << std::endl;
                continue;
            }
            // Save server's sequence number (should be 400 as per server code)
            server_seq = ntohl(tcp->seq);
            std::cout << "[+] Received SYN-ACK from " << inet_ntoa(src_addr.sin_addr)
                      << " with server sequence: " << server_seq << std::endl;
            synack_received = true;
        }
    }

    // ----------- STEP 3: Send Final ACK -----------
    std::cout << "[*] Sending final ACK to complete handshake..." << std::endl;
    // According to assignment, client must now send an ACK with sequence number = 600.
    // Typically, the ACK number would be server_seq + 1.
    uint32_t final_ack_seq = server_seq + 1; // Expected to be 401 if server_seq was 400.
    // However, note that the server checks for client sequence number = 600.
    send_packet(sock, client_ip, client_port, server_ip, SERVER_PORT, CLIENT_ACK_FINAL_SEQ, final_ack_seq, false, true);

    std::cout << "[+] Handshake complete. Exiting." << std::endl;

    close(sock);
    return 0;
}
