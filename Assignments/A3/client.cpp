#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

// Global definitions matching the assignment and the server's expectations.
#define SERVER_PORT 12345
#define CLIENT_SYN_SEQ 200
#define CLIENT_FINAL_ACK_SEQ 600
#define PACKET_SIZE (sizeof(struct iphdr) + sizeof(struct tcphdr))
// Although the server sends a SYN-ACK with sequence 400, we capture the value during the handshake.

using namespace std;

// Function to print TCP flags in a style similar to the server's output.
void print_tcp_flags(struct tcphdr *tcp)
{
    cout << "[+] TCP Flags:  "
         << "SYN: " << (int)tcp->syn
         << " ACK: " << (int)tcp->ack
         << " FIN: " << (int)tcp->fin
         << " RST: " << (int)tcp->rst
         << " PSH: " << (int)tcp->psh
         << " SEQ: " << ntohl(tcp->seq) << endl;
}

// Create a raw socket with IP_HDRINCL enabled.
int create_raw_socket()
{
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
    {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

// Constructs and sends an SYN packet.
void send_syn(int sock, const char *client_ip, int client_port, const char *server_ip)
{
    char packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);

    // IP and TCP header pointers.
    struct iphdr *ip = (struct iphdr *)packet;
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));

    // Filling IP header.
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(PACKET_SIZE);
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = inet_addr(client_ip);
    ip->daddr = inet_addr(server_ip);

    // TCP header for the SYN.
    tcp->source = htons(client_port);
    tcp->dest = htons(SERVER_PORT);
    tcp->seq = htonl(CLIENT_SYN_SEQ);
    tcp->ack_seq = 0;
    tcp->doff = sizeof(struct tcphdr) / 4;
    tcp->syn = 1;
    tcp->ack = 0;
    tcp->fin = 0;
    tcp->rst = 0;
    tcp->psh = 0;
    tcp->window = htons(8192);
    tcp->check = 0; // Kernel may compute checksum.

    // Prepare destination structure.
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SERVER_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(server_ip);

    cout << "[+] Client sending SYN..." << endl;
    // Print the TCP header flags (matches server's debug style).
    print_tcp_flags(tcp);

    if (sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror("sendto() failed for SYN packet");
    }
    else
    {
        cout << "[+] SYN sent" << endl;
    }
}

// Constructs and sends the final ACK packet (client’s ACK to complete the handshake).
void send_final_ack(int sock, const char *client_ip, int client_port, const char *server_ip, uint32_t server_seq)
{
    char packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);

    // IP and TCP header pointers.
    struct iphdr *ip = (struct iphdr *)packet;
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));

    // Filling IP header.
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(PACKET_SIZE);
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = inet_addr(client_ip);
    ip->daddr = inet_addr(server_ip);

    // TCP header for the final ACK.
    tcp->source = htons(client_port);
    tcp->dest = htons(SERVER_PORT);
    tcp->seq = htonl(CLIENT_FINAL_ACK_SEQ);
    tcp->ack_seq = htonl(server_seq + 1);
    tcp->doff = sizeof(struct tcphdr) / 4;
    tcp->syn = 0;
    tcp->ack = 1;
    tcp->fin = 0;
    tcp->rst = 0;
    tcp->psh = 0;
    tcp->window = htons(8192);
    tcp->check = 0;

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SERVER_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(server_ip);

    cout << "[+] Client sending final ACK..." << endl;
    print_tcp_flags(tcp);

    if (sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror("sendto() failed for final ACK packet");
    }
    else
    {
        cout << "[+] Final ACK sent" << endl;
    }
}

// Performs the complete three-way handshake by sending the SYN, waiting for the SYN-ACK, and sending the ACK.
void perform_hand_shake(const char *client_ip, int client_port, const char *server_ip)
{
    int sock = create_raw_socket();

    // Step 1: Send SYN to the server.
    send_syn(sock, client_ip, client_port, server_ip);

    // Step 2: Wait for SYN-ACK from the server.
    cout << "[+] Waiting for SYN-ACK from " << server_ip << "..." << endl;
    char buffer[65536];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    uint32_t server_seq = 0;
    bool synack_received = false;

    while (!synack_received)
    {
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &addr_len);
        if (data_size < 0)
        {
            perror("recvfrom() failed");
            continue;
        }
        struct iphdr *ip = (struct iphdr *)buffer;
        struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip->ihl * 4));

        // Make sure the packet is from the expected server port and destined to our client port.
        if (ntohs(tcp->source) != SERVER_PORT || ntohs(tcp->dest) != client_port)
            continue;

        // Print the TCP flags received (aligned with server output style).
        print_tcp_flags(tcp);

        // Check for SYN-ACK: SYN and ACK must be set, and ack_seq should be CLIENT_SYN_SEQ+1.
        if (tcp->syn == 1 && tcp->ack == 1 && ntohl(tcp->ack_seq) == CLIENT_SYN_SEQ + 1)
        {
            server_seq = ntohl(tcp->seq);
            cout << "[+] Received SYN-ACK from " << inet_ntoa(src_addr.sin_addr) << endl;
            synack_received = true;
        }
    }

    // Step 3: Send final ACK.
    send_final_ack(sock, client_ip, client_port, server_ip, server_seq);
    cout << "[+] Handshake complete." << endl;
    close(sock);
}

int main()
{
    // Client and server are assumed to be running on localhost.
    const char *client_ip = "127.0.0.1";
    const char *server_ip = "127.0.0.1";

    // Use an arbitrary client source port (>1024).
    int client_port = 54321;

    // Initiate the handshake.
    std::cout << "[+] Starting TCP handshake..." << std::endl;
    perform_hand_shake(client_ip, client_port, server_ip);

    return 0;
}
