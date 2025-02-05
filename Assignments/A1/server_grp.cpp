#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>
#include <netinet/in.h>

#define PORT 12345
#define BUFFER_SIZE 1024

std::unordered_map<int, std::string> clients; // a mapping to store client socket and username pair
std::mutex clients_mutex;                     // a mutex to lock the clients mapping

std::unordered_map<std::string, std::string> users; // a mapping to store user and password pair
std::mutex users_mutex;                             // a mutex to lock the users mapping

std::unordered_map<std::string, std::unordered_set<int>> groups; // a mapping to store group name and set of client sockets
std::mutex groups_mutex;                                         // a mutex to lock the groups mapping

bool authenticate(std::string username, std::string password) // function to authenticate the user
{
    std::lock_guard<std::mutex> lock(users_mutex); // locking the users mapping
    if (users.count(username) && users[username] == password)
    {
        return true;
    }
    return false;
}

void add_client(int client_socket, std::string username) // function to add client to the clients mapping
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &pair : clients)
    {
        if (pair.second == username)
        {
            std::string response = "Error: You are already logged in from another terminal.";
            send(client_socket, response.c_str(), response.size(), 0);
            close(client_socket);
            return;
        }
    }
    clients[client_socket] = username;
}

void welcome_msg(int client_socket) // function to send welcome message to the client
{
    std::string welcome = "Welcome to the chat server!";
    send(client_socket, welcome.c_str(), welcome.size(), 0);
}

void notify_others(int client_socket, std::string message) // function to notify other clients that a new client has joined
{
    std::lock_guard<std::mutex> lock(clients_mutex); // locking the clients mapping
    for (auto &pair : clients)
    {
        if (pair.first != client_socket)
        {
            send(pair.first, message.c_str(), message.size(), 0);
        }
    }
}

void broadcast(std::string username, int client_socket, std::string message) // function to broadcast message to all clients
{
    std::string msg = message.substr(11);
    if (msg.empty()) // Error message if message is empty
    {
        std::string response = "Error: Message cannot be empty.";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }

    std::string formatted_msg = "[Broadcast from " + username + "]: " + msg;

    std::lock_guard<std::mutex> lock(clients_mutex); // locking the clients mapping
    for (auto &pair : clients)                       // sending message to all clients except the sender
    {
        if (pair.first == client_socket)
        {
            continue;
        }
        send(pair.first, formatted_msg.c_str(), formatted_msg.size(), 0);
    }
}

void private_msg(std::string username, int client_socket, std::string message) // function to send private message to a specific client
{
    size_t space = message.find(' ', 5);
    if (space != std::string::npos)
    {
        std::string target_user = message.substr(5, space - 5);
        std::string msg = message.substr(space + 1);
        std::string formatted_msg = "[" + username + "]: " + msg;
        std::lock_guard<std::mutex> lock(clients_mutex);

        if (msg.empty()) // Error message if message is empty
        {
            std::string response = "Error: Message cannot be empty.";
            send(client_socket, response.c_str(), response.size(), 0);
            return;
        }

        bool found = false; // flag to check if the target user is found or not
        for (auto &pair : clients)
        {
            if (pair.second == target_user)
            {
                found = true;
                send(pair.first, formatted_msg.c_str(), formatted_msg.size(), 0);
                break;
            }
        }

        if (!found) // Error message if target user is not found
        {
            bool exist = false;
            std::lock_guard<std::mutex> lock(users_mutex);
            for (auto &pair : users)
            {
                if (pair.first == target_user)
                {
                    exist = true;
                    break;
                }
            }
            if (exist == true)
            {
                std::string response = "Error: User " + target_user + " is not online.";
                send(client_socket, response.c_str(), response.size(), 0);
            }
            else
            {
                std::string response = "Error: User " + target_user + " doesnot exist.";
                send(client_socket, response.c_str(), response.size(), 0);
            }
        }
    }
    else
    {
        std::string response = "Error: Invalid Format.";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }
}

void create_group(int client_socket, std::string message) // function to create a group
{
    std::string group_name = message.substr(14);
    std::lock_guard<std::mutex> lock(groups_mutex);

    if (group_name.empty()) // Error message if group name is empty
    {
        std::string response = "Error: Group name cannot be empty.";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }
    else if (group_name.find(' ') != std::string::npos) // Error message if group name contains space
    {
        std::string response = "Error: Group name cannot contain space.";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }
    else if (!groups.count(group_name)) // Create group if it doesn't exist
    {
        groups[group_name].insert(client_socket); // adding the client who created the group to the group
        std::string response = "Group " + group_name + " created.";
        send(client_socket, response.c_str(), response.size(), 0);
    }
    else // Error message if group already exists
    {
        std::string response = "Error: Group " + group_name + " already exists.";
        send(client_socket, response.c_str(), response.size(), 0);
    }
}

void join_group(int client_socket, std::string message) // function to join a group
{
    std::string group_name = message.substr(12);
    std::lock_guard<std::mutex> lock(groups_mutex);

    if (group_name.empty()) // Error message if group name is empty
    {
        std::string response = "Error: Group name cannot be empty.";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }
    else if (group_name.find(' ') != std::string::npos) // Error message if group name contains space
    {
        std::string response = "Error: Group name cannot contain space.";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }
    else if (groups.count(group_name))
    {
        if (groups[group_name].count(client_socket)) // Error message if client is already a member of the group
        {
            std::string response = "Error: You are already a member of group " + group_name + ".";
            send(client_socket, response.c_str(), response.size(), 0);
            return;
        }
        groups[group_name].insert(client_socket);
        std::string response = "You joined the group " + group_name + ".";
        send(client_socket, response.c_str(), response.size(), 0);

        // This part of code is to informed other members of the group that a new member has joined the group
        // for (int sock : groups[group_name])
        // {
        //     if (sock != client_socket)
        //     {
        //         std::string join_msg = username + " has joined group " + group_name + ".";
        //         send(sock, join_msg.c_str(), join_msg.size(), 0);
        //     }
        // }
    }
    else // Error message if group doesn't exist
    {
        std::string response = "Error: Group " + group_name + " doesnot exist.";
        send(client_socket, response.c_str(), response.size(), 0);
    }
}

void leave_group(int client_socket, std::string message) // function to leave a group
{
    std::string group_name = message.substr(13);
    std::lock_guard<std::mutex> lock(groups_mutex);

    if (group_name.empty()) // Error message if group name is empty
    {
        std::string response = "Error: Group name cannot be empty.";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }
    else if (group_name.find(' ') != std::string::npos) // Error message if group name contains space
    {
        std::string response = "Error: Group name cannot contain space.";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }
    else if (groups.count(group_name))
    {
        if (!groups[group_name].count(client_socket)) // Error message if client is not a member of the group
        {
            std::string response = "Error: You are not a member of group " + group_name + ".";
            send(client_socket, response.c_str(), response.size(), 0);
            return;
        }
        groups[group_name].erase(client_socket);
        std::string response = "You left the group " + group_name + ".";
        send(client_socket, response.c_str(), response.size(), 0);

        // This part of code is to informed other members of the group that a member has left the group
        // for (int sock : groups[group_name])
        // {
        //     std::string leave_msg = username + " has left group " + group_name + ".";
        //     send(sock, leave_msg.c_str(), leave_msg.size(), 0);
        // }
    }
    else // Error message if group doesn't exist
    {
        std::string response = "Error: Group " + group_name + " does not exist.";
        send(client_socket, response.c_str(), response.size(), 0);
    }
}

void cleanup(int client_socket) // function to cleanup the client
{
    // Removing client from clients mapping and groups mapping
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(client_socket);
    }
    // Removing client from groups mapping
    {
        std::lock_guard<std::mutex> lock(groups_mutex);
        for (auto &group : groups)
        {
            group.second.erase(client_socket);
        }
    }
}

void group_msg(std::string username, int client_socket, std::string message) // function to send message to a group
{
    size_t space = message.find(' ', 11);
    if (space != std::string::npos)
    {
        std::string group_name = message.substr(11, space - 11);
        std::string msg = message.substr(space + 1);
        std::string formatted_msg = "[Group " + group_name + "] " + username + ": " + msg;

        if (msg.empty()) // Error message if message is empty
        {
            std::string response = "Error: Message cannot be empty.";
            send(client_socket, response.c_str(), response.size(), 0);
            return;
        }

        std::lock_guard<std::mutex> lock(groups_mutex); // locking the groups mapping
        if (groups.count(group_name) && groups[group_name].count(client_socket))
        {
            for (int sock : groups[group_name])
            {
                if (sock != client_socket)
                {
                    send(sock, formatted_msg.c_str(), formatted_msg.size(), 0);
                }
            }
        }
        else if (groups.count(group_name) && !groups[group_name].count(client_socket)) // Error message if client is not a member of the group
        {
            std::string response = "Error: You are not a member of group " + group_name + ".";
            send(client_socket, response.c_str(), response.size(), 0);
        }
        else // Error message if group doesn't exist
        {
            std::string response = "Error: Group " + group_name + " does not exist.";
            send(client_socket, response.c_str(), response.size(), 0);
        }
    }
    else // Error message if invalid format
    {
        std::string response = "Error: Invalid format.";
        send(client_socket, response.c_str(), response.size(), 0);
    }
}

void handle_client(int client_socket) // function to handle each client (thread function)
{
    char buffer[BUFFER_SIZE];
    std::string username, password; // username and password of the client received from the client_socket

    // Sending username prompt to cilent
    std::string user_prompt = "Enter username: ";
    send(client_socket, user_prompt.c_str(), user_prompt.size(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    username = std::string(buffer);
    username = username.substr(0, username.find('\n'));

    // Sending password prompt to client
    std::string pass_prompt = "Enter password: ";
    send(client_socket, pass_prompt.c_str(), pass_prompt.size(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    password = std::string(buffer);
    password = password.substr(0, password.find('\n'));

    // Authentication
    bool authenticated = authenticate(username, password); // checking if the user is authenticated or not

    if (!authenticated) // if not authenticated, send error message and close the client socket
    {
        std::string response = "Authentication failed.";
        send(client_socket, response.c_str(), response.size(), 0);
        close(client_socket);
        return;
    }

    // Adding client after authentication
    add_client(client_socket, username);

    // Welcome message
    welcome_msg(client_socket);

    // Notify others that a new client has joined
    notify_others(client_socket, username + " has joined the chat.");

    // Handling various commands/messages
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
            break;
        std::string message(buffer, bytes_received);
        message = message.substr(0, message.find('\n')); // Trim newline

        if (message.rfind("/broadcast ", 0) == 0) // Broadcast message to all clients
        {
            broadcast(username, client_socket, message);
        }
        else if (message.rfind("/msg ", 0) == 0) // Private message to a specific client
        {
            private_msg(username, client_socket, message);
        }
        else if (message.rfind("/create_group ", 0) == 0) // Create a group
        {
            create_group(client_socket, message);
        }
        else if (message.rfind("/join_group ", 0) == 0) // Join a group
        {
            join_group(client_socket, message);
        }
        else if (message.rfind("/group_msg ", 0) == 0) // Message sent to a group
        {
            group_msg(username, client_socket, message);
        }
        else if (message.rfind("/leave_group ", 0) == 0) // Leave a group
        {
            leave_group(client_socket, message);
        }
        else if (message == "/exit") // Exit the chat or disconnect the client from the server
        {
            break;
        }
        else // Error message if invalid format i.e., no command is matched
        {
            std::string formatted_msg = "Error: Invalid format.";
            send(client_socket, formatted_msg.c_str(), formatted_msg.size(), 0);
        }
    }

    // Cleanup i.e., remove client from clients and groups mapping
    cleanup(client_socket);

    // Notify others that a client has left
    std::string leave_msg = username + " has left the chat.";
    notify_others(client_socket, leave_msg);

    close(client_socket);
}

int load_users() // function to load users from users.txt file
{
    // Load users for user.txt file
    std::ifstream file("users.txt");
    if (!file)
    {
        std::cerr << "Failed to open users.txt" << std::endl;
        return 1;
    }
    std::string line;
    while (std::getline(file, line))
    {
        size_t colon = line.find(':');
        if (colon != std::string::npos)
        {
            std::string username = line.substr(0, colon);
            std::string password = line.substr(colon + 1);
            users[username] = password.substr(0, password.length() - 1);
        }
    }
    file.close();
    return 0;
}

int create_server_socket()
{
    // Creating server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) // prompting error when socket creation fails
    {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) // prompting error when binding fails
    {
        std::cerr << "Bind failed" << std::endl;
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) < 0) // prompting error when server doesn't listen
    {
        std::cerr << "Listen failed" << std::endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Server started :-)" << std::endl;
    std::cout << "Server listening on port " << PORT << std::endl;
    // std::cout << "Press Ctrl+C to quit" << std::endl;

    // Printing users on command line
    // for (auto user : users)
    // {
    //     std::cout << user.first << " " << user.second << std::endl;
    // }
    return server_socket;
}

void accept_clients(int server_socket)
{
    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len); // accepting client socket
        if (client_socket < 0)                                                            // prompting error when accept fails
        {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::thread(handle_client, client_socket).detach(); // creating a thread for each client
    }
}

int main()
{
    load_users(); // loading users from users.txt file

    int server_socket = create_server_socket(); // creating server socket

    accept_clients(server_socket); // accepting clients

    close(server_socket); // closing the server socket
    return 0;
}