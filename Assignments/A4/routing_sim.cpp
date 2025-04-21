#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

// Define a large cost value to represent "infinite" distance (i.e., no direct link)
const int INF = 9999;

// This function prints the routing table for a single node in the Distance Vector Routing protocol.
// It displays the destination, cost, and next hop for each possible destination.
//
// Parameters:
// - node: The index of the node whose routing table is being printed.
// - dist: A 2D matrix where dist[i][j] represents the cost from node i to node j.
// - nextHop: A 2D matrix where nextHop[i][j] represents the next-hop node that node i should forward
//            packets to in order to reach destination j optimally.
void printDVRTable(int node,
                   const vector<vector<int>> &dist,
                   const vector<vector<int>> &nextHop)
{
    cout << "Node " << node << " Routing Table (Distance Vector):\n";
    cout << "Dest\tCost\tNext Hop\n";
    int n = dist.size();
    for (int dest = 0; dest < n; ++dest)
    {
        // Display destination
        cout << dest << "\t";
        // Display cost (INF if unreachable)
        if (dist[node][dest] >= INF)
            cout << "INF\t";
        else
            cout << dist[node][dest] << "\t";
        // Display next hop (- if no path)
        if (nextHop[node][dest] == -1)
            cout << "-";
        else
            cout << nextHop[node][dest];
        cout << "\n";
    }
    cout << endl;
}

// simulateDVR
// -----------
// This function simulates the Distance Vector Routing protocol. It uses the
// Bellman-Ford algorithm to compute the shortest paths for each node in the
// network. The algorithm iterates until all nodes have converged to their
// optimal routing paths.
//
// Parameters:
// - graph: A 2D adjacency matrix representing the network. Each element
//          graph[i][j] indicates the cost of the link between node i and
//          node j, or INF if there is no direct link.
void simulateDVR(const vector<vector<int>> &graph)
{
    int n = graph.size();
    // dist[i][j]: current best-known cost from node i to j
    vector<vector<int>> dist = graph;
    // nextHop[i][j]: the neighbor to which i forwards packets destined for j
    vector<vector<int>> nextHop(n, vector<int>(n, -1));

    // Initialize direct neighbors: next hop for adjacent nodes is the node itself
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            if (i != j && graph[i][j] < INF)
                nextHop[i][j] = j;
        }
    }

    bool updated;
    // Repeat relaxation until no update occurs (i.e., convergence)
    do
    {
        updated = false;
        // For each node i, try improving route to k via an intermediate node j
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                if (dist[i][j] >= INF)
                    continue; // skip unreachable intermediates
                for (int k = 0; k < n; ++k)
                {
                    if (dist[j][k] < INF &&
                        dist[i][k] > dist[i][j] + dist[j][k])
                    {
                        // Found a shorter path from i to k via j
                        dist[i][k] = dist[i][j] + dist[j][k];
                        nextHop[i][k] = nextHop[i][j];
                        updated = true;
                    }
                }
            }
        }
    } while (updated);

    // Print final routing tables for all nodes
    cout << "--- Distance Vector Routing Tables (Final) ---\n";
    for (int i = 0; i < n; ++i)
        printDVRTable(i, dist, nextHop);
}

/**
 * This function prints the routing table for a single node in the Link State Routing protocol.
 * It uses the predecessor array to determine the next-hop nodes on the shortest paths.
 * Parameters:
 *  src    The source node index whose routing table is being printed.
 *  dist   An array containing the shortest-path distances from the source node to all other nodes.
 *  prev   An array of predecessors where prev[v] = u indicates that node u precedes node v on the shortest path.
 */
void printLSRTable(int src,
                   const vector<int> &dist,
                   const vector<int> &prev)
{
    cout << "Node " << src << " Routing Table (Link State):\n";
    cout << "Dest\tCost\tNext Hop\n";
    int n = dist.size();
    for (int dest = 0; dest < n; ++dest)
    {
        if (dest == src)
            continue; // skip route to self

        // Display cost
        cout << dest << "\t";
        if (dist[dest] >= INF)
            cout << "INF\t";
        else
            cout << dist[dest] << "\t";

        // Trace back from destination to find first hop
        int hop = dest;
        while (prev[hop] != -1 && prev[hop] != src)
        {
            hop = prev[hop];
        }
        // If no path exists, indicate unreachable
        if (prev[hop] == -1)
            cout << "-";
        else
            cout << hop;

        cout << "\n";
    }
    cout << endl;
}

/**
 * simulateLSR
 * -----------
 * Implements the Link State Routing protocol using Dijkstra's algorithm.
 * For each node, it computes shortest paths to all other nodes in the network.
 *
 * Parameters: graph  Adjacency matrix of the network: graph[i][j] is cost of link i-j,
 *               or INF if no direct link exists.
 */
void simulateLSR(const vector<vector<int>> &graph)
{
    int n = graph.size();

    // Run Dijkstra's algorithm from each node as source
    for (int src = 0; src < n; ++src)
    {
        vector<int> dist(n, INF); // shortest known distance from src
        vector<int> prev(n, -1);  // predecessor on shortest path
        vector<bool> visited(n, false);

        dist[src] = 0; // distance to self is zero

        for (int iter = 0; iter < n; ++iter)
        {
            // Select the unvisited node with smallest distance
            int u = -1;
            for (int i = 0; i < n; ++i)
            {
                if (!visited[i] && (u == -1 || dist[i] < dist[u]))
                    u = i;
            }
            if (u == -1 || dist[u] >= INF)
                break; // no reachable unvisited nodes remain

            visited[u] = true;
            // Relax edges outgoing from u
            for (int v = 0; v < n; ++v)
            {
                if (graph[u][v] < INF && dist[v] > dist[u] + graph[u][v])
                {
                    dist[v] = dist[u] + graph[u][v];
                    prev[v] = u;
                }
            }
        }

        // Print the routing table for this source node
        printLSRTable(src, dist, prev);
    }
}

/**
 * readGraphFromFile
 * -----------------
 * Reads a network topology from a text file. The first line contains an integer n,
 * the number of nodes. The following n lines each contain n space-separated integers,
 * representing the adjacency matrix of link costs.
 *
 *  filename  --> Path to the input file
 * returns         Adjacency matrix of size n x n
 */
vector<vector<int>> readGraphFromFile(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Cannot open input file '" << filename << "'\n";
        exit(EXIT_FAILURE);
    }

    int n;
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n, INF));

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            int cost;
            file >> cost;
            if (cost >= 0)
                graph[i][j] = cost;
            // Negative or missing cost interpreted as INF
        }
    }
    file.close();
    return graph;
}

/**
 * main
 * ----
 * Entry point of the simulation program.
 * Expects a single command-line argument: path to the topology file.
 * Calls both DVR and LSR simulation routines and prints their routing tables.
 */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <topology_file>\n";
        return EXIT_FAILURE;
    }

    string filename = argv[1];
    // Load network topology from file
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return EXIT_SUCCESS;
}
