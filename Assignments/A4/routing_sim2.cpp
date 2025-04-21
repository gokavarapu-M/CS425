#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

// Define a large value for infinity cost
const int INF = 9999;

// Function to print the routing table for a node in Distance Vector Routing
void printDVRTable(int node, const vector<vector<int>> &table, const vector<vector<int>> &nextHop)
{
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i)
    {
        cout << i << "\t" << table[node][i] << "\t";
        if (nextHop[node][i] == -1)
            cout << "-";
        else
            cout << nextHop[node][i];
        cout << endl;
    }
    cout << endl;
}

// Function to simulate Distance Vector Routing
// using the Bellman-Ford algorithm
void simulateDVR(const vector<vector<int>> &graph)
{
    int n = graph.size();
    vector<vector<int>> dist = graph;
    vector<vector<int>> nextHop(n, vector<int>(n, -1));

    // This loop initializes the next hop for each node to its direct neighbor
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (graph[i][j] != INF && i != j)
                nextHop[i][j] = j;

    bool updated; // Flag to check if any distance was updated
    // Repeat the process until no updates occur
    do
    {
        updated = false;
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                for (int k = 0; k < n; ++k)
                {
                    if (dist[i][k] > dist[i][j] + dist[j][k])
                    {
                        dist[i][k] = dist[i][j] + dist[j][k];
                        nextHop[i][k] = nextHop[i][j];
                        updated = true;
                    }
                }
            }
        }
    } while (updated);

    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i)
        printDVRTable(i, dist, nextHop);
}

// Function to print the routing table for a node in Link State Routing
void printLSRTable(int src, const vector<int> &dist, const vector<int> &prev)
{
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i)
    {
        if (i == src)
            continue;
        cout << i << "\t" << dist[i] << "\t";
        int hop = i;
        while (prev[hop] != src && prev[hop] != -1)
            hop = prev[hop];
        cout << (prev[hop] == -1 ? -1 : hop) << endl;
    }
    cout << endl;
}

// Function to simulate Link State Routing
// using Dijkstra's algorithm
void simulateLSR(const vector<vector<int>> &graph)
{
    int n = graph.size();

    // Dijkstra's algorithm for each node
    for (int src = 0; src < n; ++src)
    {
        vector<int> dist(n, INF);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);
        dist[src] = 0;

        for (int i = 0; i < n; ++i)
        {
            int u = -1;
            for (int j = 0; j < n; ++j)
                if (!visited[j] && (u == -1 || dist[j] < dist[u]))
                    u = j;

            if (dist[u] == INF)
                break;
            visited[u] = true;

            for (int v = 0; v < n; ++v)
            {
                if (graph[u][v] != INF && dist[v] > dist[u] + graph[u][v])
                {
                    dist[v] = dist[u] + graph[u][v];
                    prev[v] = u;
                }
            }
        }

        printLSRTable(src, dist, prev);
    }
}

// Function to read the graph from a file
vector<vector<int>> readGraphFromFile(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open()) // Check if the file opened successfully
    {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }

    int n;
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> graph[i][j];

    file.close();
    return graph;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string filename = argv[1];
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}
