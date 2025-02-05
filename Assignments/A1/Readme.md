# **CS425 Assignment-1:** *Chat Server*

## How to Complie and Run

### Compiling

```bash
make
```

This make call runs a bash code which complies `server_grp.cpp` and `cilent_grp.cpp`

### Running the Server

```bash
./server_grp
```
On successful launch of server, you will receive a message

```bash
Server started :-)
Server listening on port 12345
```

### Running a Client

```bash
./client_grp
```
You can run multiple cilents on multiple terminals, only restriction is you can't run same cilent on different terminals.
<br/> On success, you we receieve a welcome message from server.
```bash
Welcome to the chat server!
```

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
- **User Status Management**: No explicit "online/offline" status tracking. Implicitly maintained as `cilents` and `users` maps.
- **Rate Limiting**: No restrictions on the frequency of messages sent.
- **Notification**: No notifications when users join/leave groups (commented in code).

---

## **Design Decisions**

### **Concurrency Model**

- **Multithreading**: Each client connection is handled in a separate thread.
- **Why Threads?**: Threads are lightweight compared to processes and allow shared memory access (e.g., `clients`, `groups` maps). This simplifies synchronization.

### **Synchronization**

- **Mutexes**: `std::mutex` is used to synchronize access to shared resources:
  - `clients_mutex`: Protects the `clients` map (socket-to-username mapping) only contain active users.
  - `users_mutex`: Protects the `users` map (user-to-password mapping) contains user credintials.
  - `groups_mutex`: Protects the `groups` map (groupname-to-groups mapping).
- **Reason**: Prevents race conditions when multiple threads access or modify shared data.

### **Data Structures**

- **`unordered_map` for Clients/Groups**: Provides O(1) average complexity for lookups.
- **`unordered_set` for Group Members**: Ensures unique client sockets (which implies unique cilents) in groups.

### **Edge Cases**

- A group with zero user can exist.
  <br/> **Why?**: [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/61)
- Message can only be sent to active user but not to any user in `users.txt`
  <br/> **Why?**: [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/23)
- Group name cannot contain empty spaces.
  <br/> **Why?**: [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/67)
- Clients can send empty messages.
  <br/> **Why?** [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/68)
- When a cilent disconnects (s)he is removed from all groups.
  <br/> **Why?**: To maintain completness in `cilents` map. [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/32)
- A person who creates group is implicitly added to the group.
  <br/> **Why?**: As can be seen in Listing 2, page 5 of the assignment pdf, client creating the group doesn't need to execute the `/join_group` command. This client is implicitly part of the group.
- Error of this kind are not resolved.

```
bob: /msg
alice: /msg bob how are you?

after the above implementation, the I/O space of bob becomes as follows

bob: /msghow are you
```

- **Why**: [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/64)
- A group with zero users can exist
  <br/> **Why?**: [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/61)
- Assuming `users.txt` doesn't change once server is started and usernames, passwords doesn't contain spaces.
- Server don't maintain meta data once it stopped running.
  <br/> **Why**: [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/42)
- Cilents are disconnected once the server stops.
  <br/> **Why**: They can't send messages without server.
- Cilent can't open multiple connections it will throw an error.
  <br/> **Why?**: [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/31)

### **Error Handling**

- **Client Errors**: Invalid commands, Invalid formats, missing message content(empty messages), and authentication failures return appropriate error messages.
- Below are some example of error conditions.

1. Invalid command: `/hi all`, `/hi` doesn't exist.
2. Invalid format: `/create_group CS 425`, group name contains no space.
3. Empty message: `\broadcast `, empty message returns error.
4. Note that a group with zero cilents can exist.
5. If a command has $n$ parts then it should be separated by atleast $n-1$ spacings. Otherwise server will reply as invalid format.

- **Example**: `/msg bob` will return `Error: Invalid Format.`,
  <br/> while `/msg bob ` will return `Error: Empty message.`
  
**Note:** Spaces are not considered as empty message

- **Server Errors**: The server detects and handles socket failures, ensuring stable operation.

### **Authentication Strategy**

- The server reads `users.txt` at startup and stores credentials in `users` map.
- **Reason**: This makes authentication fast and avoids repeatedly reading the file for each login.

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
```
main()
    │
    ├── load_users()                    # Load users from users.txt
    │   └── Read username:password pairs
    │
    ├── create_server_socket()          # Initialize server
    │   ├── Create socket
    │   ├── Bind to port
    │   └── Listen for connections
    │
    └── accept_clients()                # Main accept loop
        └── For each new connection
            └── handle_client()         # New thread
```
for each thread
```
handle_client(client_socket)
    │
    ├── Authentication
    │   ├── Send "Enter username: "
    │   ├── Receive username
    │   ├── Send "Enter password: "
    │   ├── Receive password
    │   └── authenticate()
    │       └── Check against users map
    │
    ├── Client Setup
    │   ├── add_client()               # Add to clients map
    │   ├── welcome_msg()              # Send welcome message
    │   └── notify_others()            # Broadcast join message
    │
    ├── Message Processing Loop
    │   └── While client connected
    │       ├── Receive message
    │       │
    │       ├── If "/broadcast"
    │       │   └── broadcast()
    │       │
    │       ├── If "/msg"
    │       │   └── private_msg()
    │       │
    │       ├── If "/create_group"
    │       │   └── create_group()
    │       │
    │       ├── If "/join_group"
    │       │   └── join_group()
    │       │
    │       ├── If "/group_msg"
    │       │   └── group_msg()
    │       │
    │       ├── If "/leave_group"
    │       │   └── leave_group()
    │       │
    │       |── If "/exit"
    │       |   └── Break loop
    |       |
    |       └── If no command matched send "Error: Invalid command" 
    │
    └── Cleanup
        ├── cleanup()                  # Remove from clients map and groups
        ├── notify_others()            # Broadcast leave
        └── close(client_socket)       # Close connection
            
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
| Error handling for edge cases                       | Added checks for invalid commands, empty messages, and authentication failures. |

## Contribution of Team Members

| Team Member        | Contribution (%) | Work Done                                                                                                                              |
| ------------------ | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| Manikanta (220409) | 40%              | Designed server architecture, modularized code , Functionalities, Commenting code, Conducted testing, debugging, and documentation.    |
| Rhema (221125)     | 30%              | Implemented authentication, handled multithreading, edge cases like empty messages, Commenting Code, Conducted testing, debugging.     |
| Jyothisha (220862) | 30%              | Implemented server architecture, Resolved edge cases like invalid format, Implemented string parsing, Conducted testing, documentation |

---

## Sources Referred

- [Mutex](https://www.geeksforgeeks.org/std-mutex-in-cpp/) and [lock_guard()](https://www.geeksforgeeks.org/stdunique_lock-or-stdlock_guard-which-is-better/) from geeks for geeks.
- C++ Reference (std::thread, std::mutex)
- Linked post on [locks](https://www.linkedin.com/advice/0/what-some-common-pitfalls-best-practices-when-using)
- cpp [filesystem](https://devdocs.io/cpp-filesystem/) reference from Devdocs.
- Socket programming in cpp from [Gfg](https://www.geeksforgeeks.org/socket-programming-in-cpp/)

## Declaration

We declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.

## Feedback

- The provided hints were useful, especially regarding `std::mutex` for thread safety.
- _Suggestion_: Providing a bash code for stress testing makes testing easy. Because implementing it on own is not a part of course.
