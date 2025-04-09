# **CS425 Assignment 3: TCP Handshake**

## Team Members

| Team members | Roll no. |
| ------------ | :------: |
| Manikanta    |  220409  |
| Rhema        |  221125  |
| Jyothisha    |  220862  |

## How to Run the Code

Follow these steps to compile and run the TCP handshake client and server:

1.  **Compile the Code**
    Use the provided `Makefile` to compile both the server and client programs. Open your terminal in the assignment directory and type:

    ```bash
    make
    ```

2.  **Run the Server**
    Start the server program. You need administrator rights (root privileges) because it uses raw sockets. Type the following command in your terminal:

    ```bash
    sudo ./server
    ```

    The server will show this message:

    ```
    [+] Server listening on port 12345...
    ```

3.  **Run the Client**
    Open a **new** terminal window. Run the client program, which also requires administrator rights:
    ```bash
    sudo ./client
    ```

**Expected Output:**

- **In the client's terminal:**
  ```
  [+] Client sending SYN...
  [+] TCP Flags:  SYN: 1 ACK: 0 FIN: 0 RST: 0 PSH: 0 SEQ: 200
  [+] SYN sent
  [+] Waiting for SYN-ACK from 127.0.0.1...
  [+] TCP Flags:  SYN: 1 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 400
  [+] Received SYN-ACK from 127.0.0.1
  [+] Client sending final ACK...
  [+] TCP Flags:  SYN: 0 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 600
  [+] Final ACK sent
  [+] Handshake complete.
  ```
- **In the server's terminal:**
  ```
  [+] Server listening on port 12345...
  [+] TCP Flags: SYN: 1 ACK: 0 FIN: 0 RST: 0 PSH: 0 SEQ: 200
  [+] Received SYN from 127.0.0.1
  [+] Sent SYN-ACK
  [+] TCP Flags: SYN: 0 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 600
  [+] Received ACK, handshake complete.
  ```

## Overview

**Assignment Goal:**
The purpose of this assignment is to build the client part of a simplified TCP three-way handshake using raw sockets. Raw sockets let us create and send network packets directly.

**Handshake Steps:**
The handshake follows these specific steps and sequence numbers:

1.  **SYN:** The client sends a SYN packet to the server. This packet has sequence number 200 [cite: 6, A3/client.cpp].
2.  **SYN-ACK:** The server receives the SYN and replies with a SYN-ACK packet. The server uses sequence number 400 and acknowledges the client's sequence number by sending 201 (200 + 1) [cite: 7, A3/server.cpp].
3.  **ACK:** The client receives the SYN-ACK and sends a final ACK packet. This packet uses sequence number 600 and acknowledges the server's sequence number by sending 401 (400 + 1) [cite: 8, A3/client.cpp].

## Implementation Details (client.cpp)

### Main Functions

- **`print_tcp_flags()`:** Displays the TCP flags (SYN, ACK, etc.) and the sequence number of a packet. This helps in debugging and checking if the packets are correct.
- **`create_raw_socket()`:** Creates a raw socket needed to send custom packets.
- **`send_syn()`:** Creates and sends the initial SYN packet from the client to the server with sequence number 200.
- **`send_final_ack()`:** Creates and sends the final ACK packet from the client to the server with sequence number 600, acknowledging the server's SYN-ACK.
- **`perform_hand_shake()`:** Manages the entire handshake process: sends SYN, waits for SYN-ACK, and sends the final ACK. It prints messages at each step.
- **`main()`:** Sets the IP addresses (using localhost `127.0.0.1` for both client and server) and the client port, then starts the handshake by calling `perform_hand_shake()`.

### How it Works

- **Raw Sockets:** The client uses raw sockets with the `IP_HDRINCL` option. This means the program builds the entire IP and TCP headers manually [cite: 4, A3/client.cpp].
- **Packet Construction:** Each packet (SYN, ACK) is carefully built by setting the correct values in the IP and TCP headers, including source/destination addresses, ports, sequence numbers, and flags [cite: A3/client.cpp].
- **Sequence Numbers:** The specific sequence numbers (Client SYN: 200, Server SYN-ACK: 400, Client ACK: 600) are hardcoded as required by the assignment [cite: 10, A3/client.cpp].
- **Error Checks:** The code includes basic error checking (using `perror()`) for operations like socket creation and sending packets [cite: A3/client.cpp].

## Testing

- **Step-by-step Checks:** Each part of the handshake (sending SYN, receiving SYN-ACK, sending ACK) was checked to make sure the packets were created correctly with the right sequence numbers.
- **Full Run Test:** The client and server were run together on the same computer (localhost). The output messages printed by both programs were checked to confirm that the handshake sequence matched the expected steps and sequence numbers.

## Contribution of Team Members

| Team Member              | Contribution (%) | Work Done                                      |
| :----------------------- | :--------------: | :--------------------------------------------- |
| Manikanta <br/> (220409) |      33.3%       | Wrote initial client structure, SYN sending    |
| Rhema <br/> (221125)     |      33.3%       | Implemented SYN-ACK receiving logic, final ACK |
| Jyothisha <br/> (220862) |      33.4%       | Added comments, wrote README, tested code      |

## Sources Used

- Understanding Raw Sockets in C: [https://www.binarytides.com/raw-sockets-c-code-linux/](https://www.binarytides.com/raw-sockets-c-code-linux/) - Helped in learning how to create custom IP/TCP packets using raw sockets.

## Declaration

We, (**Manikanta, Rhema and Jyothisha**) declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.
