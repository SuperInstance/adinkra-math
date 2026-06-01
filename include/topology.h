#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "adinkra.h"
#include "supersymmetry.h"

/* Graph representation for topological analysis */
#define TOPO_MAX_VERTICES 128
#define TOPO_MAX_EDGES 512

typedef struct {
    int from, to;
} TopoEdge;

typedef struct {
    int adj[TOPO_MAX_VERTICES][TOPO_MAX_VERTICES]; /* adjacency matrix */
    int vertex_count;
    TopoEdge edges[TOPO_MAX_EDGES];
    int edge_count;
} TopoGraph;

void topo_graph_init(TopoGraph *g, int vertices);
void topo_add_edge(TopoGraph *g, int from, int to);

/* Connected components */
int topo_connected_components(const TopoGraph *g, int *component_ids);

/* Euler characteristic: V - E + F (for planar embedding, F = E - V + C + 1) */
int topo_euler_characteristic(const TopoGraph *g);

/* Genus of the graph surface */
int topo_genus(const TopoGraph *g);

/* Check topological equivalence of two graphs */
int topo_equivalent(const TopoGraph *a, const TopoGraph *b);

/* Build graph from AdinkraSymbol */
void topo_from_symbol(TopoGraph *g, const AdinkraSymbol *sym);

/* Build graph from SUSYAdinkra */
void topo_from_susy(TopoGraph *g, const SUSYAdinkra *sa);

/* Planarity check (simple) */
int topo_is_planar(const TopoGraph *g);

#endif
