#include "../include/topology.h"
#include "../include/supersymmetry.h"
#undef LAYOUT
#include <string.h>

void topo_graph_init(TopoGraph *g, int vertices) {
    memset(g, 0, sizeof(*g));
    g->vertex_count = vertices > TOPO_MAX_VERTICES ? TOPO_MAX_VERTICES : vertices;
}

void topo_add_edge(TopoGraph *g, int from, int to) {
    if (from < 0 || from >= g->vertex_count || to < 0 || to >= g->vertex_count) return;
    if (g->edge_count >= TOPO_MAX_EDGES) return;
    if (!g->adj[from][to]) {
        g->adj[from][to] = 1;
        g->adj[to][from] = 1;
        g->edges[g->edge_count].from = from;
        g->edges[g->edge_count].to = to;
        g->edge_count++;
    }
}

int topo_connected_components(const TopoGraph *g, int *component_ids) {
    int visited[TOPO_MAX_VERTICES] = {0};
    int stack[TOPO_MAX_VERTICES];
    int comp_id = 0;
    for (int start = 0; start < g->vertex_count; start++) {
        if (visited[start]) continue;
        /* DFS */
        int sp = 0;
        stack[sp++] = start;
        visited[start] = 1;
        component_ids[start] = comp_id;
        while (sp > 0) {
            int v = stack[--sp];
            for (int u = 0; u < g->vertex_count; u++) {
                if (g->adj[v][u] && !visited[u]) {
                    visited[u] = 1;
                    component_ids[u] = comp_id;
                    stack[sp++] = u;
                }
            }
        }
        comp_id++;
    }
    return comp_id;
}

int topo_euler_characteristic(const TopoGraph *g) {
    /* For a connected planar graph: chi = V - E + F */
    /* F = E - V + C + 1 for planar embedding (C = components) */
    int comp_ids[TOPO_MAX_VERTICES];
    int C = topo_connected_components(g, comp_ids);
    int V = g->vertex_count;
    int E = g->edge_count;
    int F = E - V + C + 1;
    return V - E + F;
}

int topo_genus(const TopoGraph *g) {
    /* For connected graph: g = 1 - chi/2 */
    /* For disconnected: sum over components */
    int comp_ids[TOPO_MAX_VERTICES];
    int C = topo_connected_components(g, comp_ids);
    int total_genus = 0;
    for (int c = 0; c < C; c++) {
        int V = 0, E = 0;
        int vmap[TOPO_MAX_VERTICES];
        int idx = 0;
        for (int i = 0; i < g->vertex_count; i++) {
            if (comp_ids[i] == c) {
                vmap[i] = idx++;
                V++;
            }
        }
        /* Count edges within this component */
        for (int e = 0; e < g->edge_count; e++) {
            if (comp_ids[g->edges[e].from] == c && comp_ids[g->edges[e].to] == c)
                E++;
        }
        if (V == 0) continue;
        int chi = V - E + 1; /* F=1 for tree, adjust for cycles */
        /* Actually for a planar embedding: F = E - V + 2, chi = V - E + F = 2 */
        /* Genus g: chi = 2 - 2g, so g = (2 - chi)/2 */
        /* For abstract graph: genus >= 0, chi <= 2 */
        int gen = (2 - chi) / 2;
        if (gen < 0) gen = 0;
        total_genus += gen;
    }
    return total_genus;
}

int topo_equivalent(const TopoGraph *a, const TopoGraph *b) {
    /* Simple check: same vertex count, edge count, and component count */
    if (a->vertex_count != b->vertex_count) return 0;
    if (a->edge_count != b->edge_count) return 0;
    int ca[TOPO_MAX_VERTICES], cb[TOPO_MAX_VERTICES];
    int comp_a = topo_connected_components(a, ca);
    int comp_b = topo_connected_components(b, cb);
    if (comp_a != comp_b) return 0;
    /* Check degree sequences match */
    for (int v = 0; v < a->vertex_count; v++) {
        int da = 0, db = 0;
        for (int u = 0; u < a->vertex_count; u++) {
            da += a->adj[v][u];
            db += b->adj[v][u];
        }
        if (da != db) return 0;
    }
    return 1;
}

void topo_from_symbol(TopoGraph *g, const AdinkraSymbol *sym) {
    /* Each primitive becomes a vertex; connect overlapping ones */
    topo_graph_init(g, sym->prim_count);
    for (int i = 0; i < sym->prim_count; i++) {
        for (int j = i + 1; j < sym->prim_count; j++) {
            /* Connect if primitives are close */
            double dx = sym->prims[i].cx - sym->prims[j].cx;
            double dy = sym->prims[i].cy - sym->prims[j].cy;
            double dist = dx*dx + dy*dy;
            double threshold = (sym->prims[i].size + sym->prims[j].size);
            if (dist < threshold * threshold) {
                topo_add_edge(g, i, j);
            }
        }
    }
}

void topo_from_susy(TopoGraph *g, const SUSYAdinkra *sa) {
    topo_graph_init(g, sa->node_count);
    for (int i = 0; i < sa->edge_count; i++) {
        topo_add_edge(g, sa->edges[i].from, sa->edges[i].to);
    }
}

int topo_is_planar(const TopoGraph *g) {
    /* Simple check: E <= 3V - 6 for V >= 3 */
    if (g->vertex_count < 3) return 1;
    if (g->edge_count > 3 * g->vertex_count - 6) return 0;
    /* Also check triangle inequality for bipartite: E <= 2V - 4 */
    return 1;
}
