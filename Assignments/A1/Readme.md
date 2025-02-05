# **Chat Server with Groups and Private Messaging**

## **Assignment Features**

### **Implemented Features**

- **TCP Server**: The server listens on a specific port (`12345`) and accepts multiple concurrent client connections.
- **User Authentication**: Users must log in with a username and password from `users.txt` before accessing chat functionalities.
- **Private Messaging**: Users can send direct messages to specific users with `/msg <username> <message>`.
- **Broadcast Messaging**: Users can broadcast messages to all connected clients using `/broadcast <message>`.
- **Group Management**:
  - **Create a group** using `/create_group <group_name>`.
  - **Join an existing group** using `/join_group <group_name>`.
  - **Leave a group** using `/leave_group <group_name>`.
  - **Send messages to a group** using `/group_msg <group_name> <message>`.
- **Threaded Handling of Clients**: Each client connection is handled in a separate thread.
- **Synchronization**: Proper mutex locking is used to prevent race conditions on shared resources (users, clients, groups).

### **Not Implemented Features**

- **Admin Privileges**: There are no administrative controls for managing users or groups.
- **Message History**: Messages are not stored persistently; only active users see messages.
- **User Status Management**: No explicit "online/offline" status tracking.
- **Rate Limiting**: No restrictions on the frequency of messages sent.
- **Notification**: No notifications when users join/leave groups (commented in code).

---

## **Design Decisions**

### **Concurrency Model**

- **Multithreading**: Each client connection is handled in a separate thread.
- **Why Threads?**: Threads are lightweight compared to processes and allow shared memory access (e.g., `clients`, `groups` maps). This simplifies synchronization.

### **Synchronization**

- **Mutexes**: `std::mutex` is used to synchronize access to shared resources:
  - `clients_mutex`: Protects the `clients` map (socket-to-username mapping).
  - `users_mutex`: Protects the `users` map (user credentials).
  - `groups_mutex`: Protects the `groups` map (group-to-clients mapping).
- **Reason**: Prevents race conditions when multiple threads access or modify shared data.

### **Data Structures**

- **`unordered_map` for Clients/Groups**: Provides O(1) average complexity for lookups.
- **`unordered_set` for Group Members**: Ensures unique client sockets in groups.

### **Error Handling**

- **Client Errors**: Invalid commands, Invalid formats, missing message content(empty messages), and authentication failures return appropriate error messages.
- Below are some example of error conditions.

1. Invalid command: `\hi all`, `\hi` doesn't exist.
2. Invalid format: `\create_group CS 425`, group name contains no space.
3. Empty message: `\broadcast `, empty message returns error.
4. Note that a group with zero cilents can exist.

- **Server Errors**: The server detects and handles socket failures gracefully, ensuring stable operation.

### **Authentication Strategy**

- The server reads `users.txt` at startup and stores credentials in `users` map.
- **Reason**: This ensures authentication is fast and avoids repeatedly reading the file for each login attempt.

---

## **Implementation Details**

### **High-Level Function Overview**

| Function                 | Description                                                                         |
| ------------------------ | ----------------------------------------------------------------------------------- |
| `authenticate()`         | Checks username-password pairs against `users.txt`. which are stored in `users` map |
| `add_client()`           | Adds an authenticated client to the `clients` map.                                  |
| `welcome_msg()`          | Sends a welcome message to all connected clients.                                   |
| `broadcast()`            | Sends a message to all connected clients.                                           |
| `private_msg()`          | Sends a message to a specific user.                                                 |
| `create_group()`         | Creates a new group.                                                                |
| `join_group()`           | Adds a client to an existing group.                                                 |
| `leave_group()`          | Removes a client from a group.                                                      |
| `group_msg()`            | Sends a message to all members of a group.                                          |
| `cleanup()`              | Cleans up client data when they disconnect.                                         |
| `handle_client()`        | Handles client communication in a separate thread.                                  |
| `load_users()`           | Loads users from `users.txt` at startup.                                            |
| `create_server_socket()` | Initializes and binds the server socket.                                            |
| `accept_clients()`       | Accepts incoming client connections and spawns a new thread for each.               |

### **Code Flow (Server-Side)**

```plaintext
1. Start the server.
2. Load user credentials from users.txt.
3. Bind and listen on PORT 12345.
4. Accept incoming client connections.
5. For each client:
   a. Authenticate user.
   b. If authenticated, add to active clients.
   c. Handle commands (/msg, /broadcast, /create_group, etc.).
   d. If disconnected, clean up client data.
```

### **Code Flow (Client-Side)**

```plaintext
1. Connect to server on PORT 12345.
2. Enter username and password for authentication.
3. If authenticated:
   a. Start a separate thread to listen for incoming messages.
   b. Read user input and send commands/messages to the server.
4. If the user types "/exit" or press "ctrl + C", close the connection.
```

---

## Testing

### Correctness Testing

- Verified that all commands work as expected (e.g., valid users can log in, messages are sent and received correctly, etc.).
- Tested authentication failures (wrong password, non-existent user).
- Ensured proper error messages are sent for invalid commands.

### Stress Testing

- Connected multiple clients simultaneously to test concurrency.
- Sent large messages to check for buffer overflows.
- Created multiple groups and added many members to validate scalability.

---

## Server Restrictions

| Parameter             | Restriction                                                             |
| --------------------- | ----------------------------------------------------------------------- |
| Max Clients           | Limited by system socket limitations (typically 1024 file descriptors). |
| Max Groups            | No hard limit; restricted by system memory.                             |
| Max Members per Group | No explicit limit; depends on system performance.                       |
| Max Message Size      | 1024 bytes (limited by BUFFER_SIZE).                                    |

---

## Challenges and Solutions

| Challenge                                           | Solution                                                                        |
| --------------------------------------------------- | ------------------------------------------------------------------------------- |
| Concurrency issues (race conditions on shared data) | Used std::mutex to synchronize access.                                          |
| Handling multiple clients efficiently               | Used threads instead of processes to minimize overhead.                         |
| Message parsing complexities                        | Implemented structured string parsing with proper validation.                   |
| Error handling for edge cases                       | Added checks for invalid commands, empty messages, and authentication failures. |

## Contribution of Team Members

| Team Member        | Contribution (%) | Work Done                                                                                              |
| ------------------ | ---------------- | ------------------------------------------------------------------------------------------------------ |
| Manikanta (220409) | 40%              | Designed server architecture, handled multithreading, Conducted testing, debugging, and documentation. |
| Rhema (221125)     | 30%              | Implemented authentication and messaging functions.                                                    |
| Jyothisha (220862) | 30%              | -                                                                                                      |

---

## Sources Referred

- Beej's Guide to Network Programming
- C++ Reference (std::thread, std::mutex, std::unordered_map)
- Stack Overflow discussions on socket programming

## Declaration

We declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.

## Feedback

- The assignment helped in understanding socket programming, multithreading, and synchronization.
- The provided hints were useful, especially regarding std::mutex for thread safety.
- _Suggestion_: Include additional test cases in the starter code to help verify correctness quickly.

## How to Complie and Run

### Compiling

```bash
make
```

This make call runs a bash code

```
g++ -std=c++11 server_grp.cpp -o server_grp -pthread
g++ -std=c++11 client_grp.cpp -o client_grp -pthread
```

### Running the Server

```bash
./server_grp
```

### Running the Client

```bash
./client_grp
```
