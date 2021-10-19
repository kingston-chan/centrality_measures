// Centrality Measures API implementation
// COMP2521 Assignment 2

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "CentralityMeasures.h"
#include "Dijkstra.h"
#include "PQ.h"

// Store sum of all reachable node distances and 
// number of reachable nodes
typedef struct SumReach {
	double sum;		// Sum of all reachable node
					// distances
	double n;		// Total number of reachable
					// nodes
} SumReach;

// Store values to help obtain betweeness centrality
typedef struct Path {
	double st;		// Total number of shortest paths
					// from node s to node t
	double stv;		// Number of those paths that include node v
} Path;

// Helper functions
static SumReach sum_of_shortest_path(int *dist, int num_nodes);
static Path find_bc(PredNode **pred, Path p, int t, int s, int v, int f);
static void store_bc(Graph g, NodeValues nvs, int s, int v);

NodeValues closenessCentrality(Graph g) {
	NodeValues nvs;
	nvs.numNodes = GraphNumVertices(g);
	nvs.values = malloc(nvs.numNodes * sizeof(double));
	
	// Find cwf for each nodes in graph
	for (int i = 0; i < nvs.numNodes; i++) {
		ShortestPaths sps = dijkstra(g, i);
		// Obtain the reachable nodes and the sum of all reachable
		// nodes distances
		SumReach sr = sum_of_shortest_path(sps.dist, nvs.numNodes);

		if (sr.n == 1) {
			// Not connected to any other node
			nvs.values[i] = 0;
		} else {
			double N = (double)nvs.numNodes;
			// Wasserman and Faust formula
			double cwf = ((sr.n-1)/(N-1))*((sr.n-1)/sr.sum);
			nvs.values[i] = cwf;
		}

		freeShortestPaths(sps);
	}

	return nvs;
}

NodeValues betweennessCentrality(Graph g) {
	NodeValues nvs;
	nvs.numNodes = GraphNumVertices(g);
	nvs.values = malloc(nvs.numNodes * sizeof(double));

	// Find betweeness centrality for each node
	for (int v = 0; v < nvs.numNodes; v++) {
		nvs.values[v] = 0;
		// Find possible paths that pass through v node
		// such that the starting node is not v
		for (int s = 0; s < nvs.numNodes; s++) {
			if (s != v) {
				// Obtain the total number of shortest paths from
				// node s and the number of those paths that pass
				// through v
				store_bc(g, nvs, s, v);
			}
		}
	}

	return nvs;
}

NodeValues betweennessCentralityNormalised(Graph g) {
	NodeValues nvs = betweennessCentrality(g);

	// Normalise each node's betweenness centrality
	for (int i = 0; i < nvs.numNodes; i++) {
		nvs.values[i] = nvs.values[i]/((nvs.numNodes-1)*(nvs.numNodes-2));
	}

	return nvs;
}

void showNodeValues(NodeValues nvs) {

}

void freeNodeValues(NodeValues nvs) {
	// Free memory allocated to the values array
	free(nvs.values);
}

			/////////////////////////////
			///// HELPER FUNCTIONS //////
			/////////////////////////////

// Helper function to get sum of all reachable node distances and number of 
// reachable nodes
static SumReach sum_of_shortest_path(int *dist, int num_nodes) {
	SumReach sr;
	sr.sum = 0;
	sr.n = 0;

	// Find the sum of all reachable nodes distances and 
	// the number of reachable nodes
	for (int i = 0; i < num_nodes; i++) {
		if (dist[i] != INFINITY) {
			sr.sum += (double)dist[i];
			sr.n++;
		} 
	}

	return sr;
}

// Helper function to acculate the betweeness value for node v
static void store_bc(Graph g, NodeValues nvs, int s, int v) {
	ShortestPaths sps = dijkstra(g, s);
	// Find shortest paths from node s to nodes that are not
	// node s or v (node t)
	for (int t = 0; t < nvs.numNodes; t++) {

		Path p;
		p.st = 0;
		p.stv = 0;

		if (t != s && t != v) {
			// Find the number of shortest paths from node s to 
			// node t and the number of those paths that include
			// node v
			p = find_bc(sps.pred, p, t, s, v, 0);
		}

		// Accumulate the betweeness centrality value
		if (p.st > 0) nvs.values[v] += p.stv/p.st;
	}

	freeShortestPaths(sps);
}

// Helper function to find the number of paths from node s to node t
// and the number of those paths the have node v between them
static Path find_bc(PredNode **pred, Path p, int t, int s, int v, int f) {
	PredNode *p_node = pred[t];

	while (p_node != NULL) {

		if (p_node->v == s) {
			// Reached node s
			if (f == 1) {
				// Node v exists in this path
				p.stv++;
				p.st++;
			} else {
				p.st++;
			}
		} else if (p_node->v == v) {
			// Found node v in this path
			p = find_bc(pred, p, p_node->v, s, v, 1);
		} else if (f == 1) {
			// Node v exists in this path
			p = find_bc(pred, p, p_node->v, s, v, 1);
		} else {
			p = find_bc(pred, p, p_node->v, s, v, 0);
		}

		p_node = p_node->next;
	}

	return p;
}

