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

std::unordered_map<int, std::string> clients;
std::mutex clients_mutex;

std::unordered_map<std::string, std::string> users;
std::mutex users_mutex;

std::unordered_map<std::string, std::unordered_set<int>> groups;
std::mutex groups_mutex;

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    std::string username, password;

    // Send username prompt
    std::string user_prompt = "Enter username: ";
    send(client_socket, user_prompt.c_str(), user_prompt.size(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    username = std::string(buffer);
    username = username.substr(0, username.find('\n')); // Trim newline

    // Send password prompt
    std::string pass_prompt = "Enter password: ";
    send(client_socket, pass_prompt.c_str(), pass_prompt.size(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    password = std::string(buffer);
    password = password.substr(0, password.find('\n')); // Trim newline

    // Authenticate
    bool authenticated = false;
    {
        std::lock_guard<std::mutex> lock(users_mutex);
        if (users.count(username) && users[username] == password)
        {
            authenticated = true;
        }
    }

    if (!authenticated)
    {
        std::string response = "Authentication failed.\n";
        send(client_socket, response.c_str(), response.size(), 0);
        close(client_socket);
        return;
    }

    // Add client
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_socket] = username;
    }

    // Welcome message
    std::string welcome = "Welcome to the chat server!\n";
    send(client_socket, welcome.c_str(), welcome.size(), 0);

    // Notify others
    std::string join_msg = username + " has joined the chat.\n";
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto &pair : clients)
        {
            if (pair.first != client_socket)
            {
                send(pair.first, join_msg.c_str(), join_msg.size(), 0);
            }
        }
    }

    // Handle messages
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
            break;
        std::string message(buffer, bytes_received);
        message = message.substr(0, message.find('\n')); // Trim newline

        if (message.rfind("/broadcast ", 0) == 0)
        {
            std::string msg = message.substr(11);
            std::string formatted_msg = "[" + username + "]: " + msg + "\n";
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (auto &pair : clients)
            {
                send(pair.first, formatted_msg.c_str(), formatted_msg.size(), 0);
            }
        }
        else if (message.rfind("/msg ", 0) == 0)
        {
            size_t space = message.find(' ', 5);
            if (space != std::string::npos)
            {
                std::string target_user = message.substr(5, space - 5);
                std::string msg = message.substr(space + 1);
                std::string formatted_msg = "[" + username + "] (private): " + msg + "\n";
                std::lock_guard<std::mutex> lock(clients_mutex);
                for (auto &pair : clients)
                {
                    if (pair.second == target_user)
                    {
                        send(pair.first, formatted_msg.c_str(), formatted_msg.size(), 0);
                        break;
                    }
                }
            }
        }
        else if (message.rfind("/create_group ", 0) == 0)
        {
            std::string group_name = message.substr(14);
            std::lock_guard<std::mutex> lock(groups_mutex);
            if (!groups.count(group_name))
            {
                groups[group_name].insert(client_socket);
                std::string response = "Group " + group_name + " created.\n";
                send(client_socket, response.c_str(), response.size(), 0);
            }
            else
            {
                std::string response = "Group " + group_name + " already exists.\n";
                send(client_socket, response.c_str(), response.size(), 0);
            }
        }
        else if (message.rfind("/join_group ", 0) == 0)
        {
            std::string group_name = message.substr(12);
            std::lock_guard<std::mutex> lock(groups_mutex);
            if (groups.count(group_name))
            {
                groups[group_name].insert(client_socket);
                std::string response = "You joined group " + group_name + ".\n";
                send(client_socket, response.c_str(), response.size(), 0);
            }
            else
            {
                std::string response = "Group " + group_name + " does not exist.\n";
                send(client_socket, response.c_str(), response.size(), 0);
            }
        }
        else if (message.rfind("/group_msg ", 0) == 0)
        {
            size_t space = message.find(' ', 11);
            if (space != std::string::npos)
            {
                std::string group_name = message.substr(11, space - 11);
                std::string msg = message.substr(space + 1);
                std::string formatted_msg = "[Group " + group_name + "] " + username + ": " + msg + "\n";
                std::lock_guard<std::mutex> lock(groups_mutex);
                if (groups.count(group_name))
                {
                    for (int sock : groups[group_name])
                    {
                        send(sock, formatted_msg.c_str(), formatted_msg.size(), 0);
                    }
                }
                else
                {
                    std::string response = "Group " + group_name + " does not exist.\n";
                    send(client_socket, response.c_str(), response.size(), 0);
                }
            }
        }
        else if (message.rfind("/leave_group ", 0) == 0)
        {
            std::string group_name = message.substr(13);
            std::lock_guard<std::mutex> lock(groups_mutex);
            if (groups.count(group_name))
            {
                groups[group_name].erase(client_socket);
                std::string response = "You left group " + group_name + ".\n";
                send(client_socket, response.c_str(), response.size(), 0);
            }
            else
            {
                std::string response = "Group " + group_name + " does not exist.\n";
                send(client_socket, response.c_str(), response.size(), 0);
            }
        }
        else if (message == "/exit")
        {
            break;
        }
        else
        {
            // Default broadcast
            std::string formatted_msg = "[" + username + "]: " + message + "\n";
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (auto &pair : clients)
            {
                send(pair.first, formatted_msg.c_str(), formatted_msg.size(), 0);
            }
        }
    }

    // Cleanup
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(client_socket);
    }
    {
        std::lock_guard<std::mutex> lock(groups_mutex);
        for (auto &group : groups)
        {
            group.second.erase(client_socket);
        }
    }
    std::string leave_msg = username + " has left the chat.\n";
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto &pair : clients)
        {
            send(pair.first, leave_msg.c_str(), leave_msg.size(), 0);
        }
    }
    close(client_socket);
}

int main()
{
    // Load users
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

    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);
        if (client_socket < 0)
        {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}