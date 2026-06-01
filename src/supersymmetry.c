#define _GNU_SOURCE
#include "../include/supersymmetry.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void susy_adinkra_init(SUSYAdinkra *sa, int rank) {
    memset(sa, 0, sizeof(*sa));
    sa->rank = rank;
}

int susy_add_node(SUSYAdinkra *sa, NodeType type, const char *label) {
    if (sa->node_count >= SUSY_MAX_NODES) return -1;
    SUSYNode *n = &sa->nodes[sa->node_count];
    n->id = sa->node_count;
    n->type = type;
    strncpy(n->label, label, sizeof(n->label) - 1);
    /* Layout in a circle */
    int total = sa->node_count + 1;
    for (int i = 0; i < total; i++) {
        double angle = 2.0 * M_PI * i / total - M_PI / 2;
        sa->nodes[i].x = cos(angle) * 100.0;
        sa->nodes[i].y = sin(angle) * 100.0;
    }
    return sa->node_count++;
}

int susy_add_edge(SUSYAdinkra *sa, int from, int to, EdgeColor color, int sign) {
    if (sa->edge_count >= SUSY_MAX_EDGES) return -1;
    if (from < 0 || from >= sa->node_count || to < 0 || to >= sa->node_count) return -1;
    SUSYEdge *e = &sa->edges[sa->edge_count];
    e->from = from;
    e->to = to;
    e->color = color;
    e->sign = sign;
    return sa->edge_count++;
}

const char *susy_edge_color_name(EdgeColor c) {
    switch (c) {
        case EDGE_RED:    return "red";
        case EDGE_GREEN:  return "green";
        case EDGE_BLUE:   return "blue";
        case EDGE_YELLOW: return "#CCCC00";
        case EDGE_ORANGE: return "orange";
        case EDGE_PURPLE: return "purple";
        case EDGE_CYAN:   return "cyan";
        case EDGE_PINK:   return "pink";
    }
    return "black";
}

int susy_verify_chromotopology(const SUSYAdinkra *sa) {
    /* Each color forms a perfect matching: each node incident to exactly 1 edge of each color */
    for (int c = 0; c < sa->rank; c++) {
        EdgeColor ec = (EdgeColor)c;
        int edge_count = 0;
        int node_degrees[SUSY_MAX_NODES] = {0};
        for (int e = 0; e < sa->edge_count; e++) {
            if (sa->edges[e].color == ec) {
                edge_count++;
                node_degrees[sa->edges[e].from]++;
                node_degrees[sa->edges[e].to]++;
            }
        }
        /* Each node should have exactly 1 edge of this color */
        for (int n = 0; n < sa->node_count; n++) {
            if (node_degrees[n] != 1) return 0;
        }
        /* Edge count should be node_count / 2 */
        if (edge_count != sa->node_count / 2) return 0;
    }
    return 1; /* pass */
}

int susy_verify_representation(const SUSYAdinkra *sa) {
    /* Basic checks: balanced boson/fermion nodes, valid edges */
    int bosons = 0, fermions = 0;
    for (int i = 0; i < sa->node_count; i++) {
        if (sa->nodes[i].type == NODE_BOSON) bosons++;
        else fermions++;
    }
    if (bosons == 0 || fermions == 0) return 0;

    /* Each edge must connect boson to fermion */
    for (int i = 0; i < sa->edge_count; i++) {
        NodeType ft = sa->nodes[sa->edges[i].from].type;
        NodeType tt = sa->nodes[sa->edges[i].to].type;
        if (ft == tt) return 0; /* must be boson-fermion */
    }
    return susy_verify_chromotopology(sa);
}

void susy_generate_rank4(SUSYAdinkra *sa) {
    susy_adinkra_init(sa, 4);
    /* 4 bosons + 4 fermions */
    for (int i = 0; i < 4; i++) {
        char label[16];
        snprintf(label, sizeof(label), "B%d", i);
        susy_add_node(sa, NODE_BOSON, label);
    }
    for (int i = 0; i < 4; i++) {
        char label[16];
        snprintf(label, sizeof(label), "F%d", i);
        susy_add_node(sa, NODE_FERMION, label);
    }
    /* Each color forms a perfect matching (bijection B→F) */
    /* Use different permutations for each color */
    EdgeColor colors[4] = {EDGE_RED, EDGE_GREEN, EDGE_BLUE, EDGE_YELLOW};
    int perms[4][4] = {
        {0, 1, 2, 3},
        {1, 0, 3, 2},
        {2, 3, 0, 1},
        {3, 2, 1, 0}
    };
    int signs[4][4] = {
        {1,  1,  1,  1},
        {1, -1,  1, -1},
        {1,  1, -1, -1},
        {1, -1, -1,  1}
    };
    for (int c = 0; c < 4; c++) {
        for (int b = 0; b < 4; b++) {
            susy_add_edge(sa, b, 4 + perms[c][b], colors[c], signs[b][c]);
        }
    }
}

void susy_generate_rank8(SUSYAdinkra *sa) {
    susy_adinkra_init(sa, 8);
    /* 8 bosons + 8 fermions */
    for (int i = 0; i < 8; i++) {
        char label[16];
        snprintf(label, sizeof(label), "B%d", i);
        susy_add_node(sa, NODE_BOSON, label);
    }
    for (int i = 0; i < 8; i++) {
        char label[16];
        snprintf(label, sizeof(label), "F%d", i);
        susy_add_node(sa, NODE_FERMION, label);
    }
    /* 8 colors, each forming a perfect matching */
    EdgeColor colors[8] = {EDGE_RED, EDGE_GREEN, EDGE_BLUE, EDGE_YELLOW,
                           EDGE_ORANGE, EDGE_PURPLE, EDGE_CYAN, EDGE_PINK};
    /* Use simple permutations: identity + shifts + XOR-based */
    for (int c = 0; c < 8; c++) {
        for (int b = 0; b < 8; b++) {
            int f = (b + c) % 8; /* simple shift permutation */
            int sign = ((b ^ c) & 1) ? -1 : 1;
            susy_add_edge(sa, b, 8 + f, colors[c], sign);
        }
    }
}

char *susy_adinkra_to_svg(Arena *a, const SUSYAdinkra *sa, double width, double height) {
    size_t bufsize = 32768;
    char *svg = (char *)arena_alloc(a, bufsize);
    if (!svg) return NULL;
    int pos = 0;
    double cx = width / 2, cy = height / 2;
    pos += snprintf(svg + pos, bufsize - pos,
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%.0f\" height=\"%.0f\">\n", width, height);

    /* Edges */
    for (int i = 0; i < sa->edge_count; i++) {
        const SUSYEdge *e = &sa->edges[i];
        double x1 = sa->nodes[e->from].x + cx;
        double y1 = sa->nodes[e->from].y + cy;
        double x2 = sa->nodes[e->to].x + cx;
        double y2 = sa->nodes[e->to].y + cy;
        const char *col = susy_edge_color_name(e->color);
        pos += snprintf(svg + pos, bufsize - pos,
            "  <line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" stroke=\"%s\" stroke-width=\"1.5\"%s/>\n",
            x1, y1, x2, y2, col, e->sign < 0 ? " stroke-dasharray=\"4,4\"" : "");
    }

    /* Nodes */
    for (int i = 0; i < sa->node_count; i++) {
        const SUSYNode *n = &sa->nodes[i];
        double nx = n->x + cx, ny = n->y + cy;
        if (n->type == NODE_BOSON) {
            pos += snprintf(svg + pos, bufsize - pos,
                "  <circle cx=\"%.1f\" cy=\"%.1f\" r=\"8\" fill=\"white\" stroke=\"black\" stroke-width=\"2\"/>\n",
                nx, ny);
        } else {
            pos += snprintf(svg + pos, bufsize - pos,
                "  <rect x=\"%.1f\" y=\"%.1f\" width=\"14\" height=\"14\" fill=\"black\" stroke=\"black\" stroke-width=\"2\" transform=\"rotate(45,%.1f,%.1f)\"/>\n",
                nx - 7, ny - 7, nx, ny);
        }
        pos += snprintf(svg + pos, bufsize - pos,
            "  <text x=\"%.1f\" y=\"%.1f\" font-size=\"9\" text-anchor=\"middle\" dy=\"20\">%s</text>\n",
            nx, ny, n->label);
    }

    pos += snprintf(svg + pos, bufsize - pos, "</svg>\n");
    return svg;
}
