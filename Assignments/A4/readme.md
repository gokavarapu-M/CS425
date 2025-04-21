# **CS425 Assignment 4: Routing Protocols**

## Team Members

| Team members | Roll no. |
| ------------ | :------: |
| Manikanta    |  220409  |
| Rhema        |  221125  |
| Jyothisha    |  220862  |

## How to Run the Code

1. **Compile the Code**  
   Use the provided `Makefile` to compile the simulator:

   ```bash
   make
   ```

2. **Run the Simulator**  
   Execute the generated binary with an input topology file:
   ```bash
   ./routing_sim <input_file.txt>
   ```
   - `<input_file.txt>` should contain an `n×n` adjacency matrix of link costs.

## Expected Output

A successful run will first display the **Distance Vector Routing** tables—showing each node’s destinations, costs, and next hops—and then the **Link State Routing** tables in the same format.

## Overview

This assignment implements two fundamental routing protocols on a static network topology:

1. **Distance Vector Routing (DVR):**  
   This routing is based on the Bellman–Ford algorithm, each node iteratively shares its routing table with neighbors until convergence.

2. **Link State Routing (LSR):**  
   This routing type employs Dijkstra’s algorithm at every node to compute shortest paths to all others, building a global view of the network.

## Implementation Details (`routing_sim.cpp`)

### Distance Vector Routing

- **INF Definition:**  
  `INF = 9999` represents “no direct link” without risk of overflow.

- **Bellman–Ford Relaxation:**  
  Repeatedly relaxes edges for each node until no updates occur to guarantee convergence.

- **Next Hop Matrix:**  
  `nextHop[i][j]` stores the immediate neighbor to which node _i_ forwards packets destined for _j_.

### Link State Routing

- **Dijkstra’s Algorithm:**  
  For each source node, computes shortest distances (`dist[]`) and predecessors (`prev[]`) across the entire topology.

- **Route Reconstruction:**  
  Backtracks from each destination via `prev[]` to identify the first hop on the path.

- **Unreachable Detection:**  
  Destinations with `dist[dest] ≥ INF` are marked unreachable (“INF” cost).

## Assumptions

- The network is static: link costs do not change during simulation.
- All weights are non-negative
- For completeness we assume any negative weights or missing weights as infinity.
  </br> **Why?** [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/185)
- We print only final iteration tables in DVR
  </br> **Why?** [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/189)
- We print distance as **'INF'** for unreachable nodes and **'-'** for next hop such cases
  </br> **Why?** [Piazza post](https://piazza.com/class/m5h01uph1h12eb/post/187)

## Testing

- **Unit Tests:**  
  Small graphs (3–5 nodes) verified the Bellman–Ford and Dijkstra implementations against known shortest paths.

- **End-to-End Tests:**  
  Larger sample topologies confirmed that the printed routing tables matched hand‑computed expectations.

## Contribution of Team Members

| Team Member              | Contribution (%) | Work Done                                  |
| :----------------------- | :--------------: | :----------------------------------------- |
| Manikanta <br/> (220409) |       40%        | Implemented LSR algorithms, commented code |
| Rhema <br/> (221125)     |       30%        | Implemented DVR                            |
| Jyothisha <br/> (220862) |       30%        | prepared README                            |

## Sources Used

1. Distance-vector routing protocol [Wikipedia](https://en.wikipedia.org/wiki/Distance-vector_routing_protocol)
2. Bellman–Ford algorithm [GFG](https://www.geeksforgeeks.org/bellman-ford-algorithm-dp-23/)
3. Dijkstra’s algorithm [GFG](https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/)

## Declaration

We, (**Manikanta, Rhema and Jyothisha**) declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.
