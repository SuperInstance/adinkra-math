#ifndef SUPERSYMMETRY_H
#define SUPERSYMMETRY_H

#include "adinkra.h"

/* SUSY adinkra node */
typedef enum {
    NODE_BOSON,
    NODE_FERMION
} NodeType;

typedef struct {
    int id;
    NodeType type;
    double x, y;
    char label[16];
} SUSYNode;

/* SUSY edge with color */
typedef enum {
    EDGE_RED,
    EDGE_GREEN,
    EDGE_BLUE,
    EDGE_YELLOW,
    EDGE_ORANGE,
    EDGE_PURPLE,
    EDGE_CYAN,
    EDGE_PINK
} EdgeColor;

typedef struct {
    int from, to;
    EdgeColor color;
    int sign;  /* +1 or -1 */
} SUSYEdge;

/* SUSY Adinkra */
#define SUSY_MAX_NODES 32
#define SUSY_MAX_EDGES 128

typedef struct {
    SUSYNode nodes[SUSY_MAX_NODES];
    int node_count;
    SUSYEdge edges[SUSY_MAX_EDGES];
    int edge_count;
    int rank;  /* 4 or 8 */
} SUSYAdinkra;

void susy_adinkra_init(SUSYAdinkra *sa, int rank);
int susy_add_node(SUSYAdinkra *sa, NodeType type, const char *label);
int susy_add_edge(SUSYAdinkra *sa, int from, int to, EdgeColor color, int sign);

/* Chromotopology verification */
int susy_verify_chromotopology(const SUSYAdinkra *sa);

/* SUSY representation verification */
int susy_verify_representation(const SUSYAdinkra *sa);

/* Generate rank-4 and rank-8 adinkras */
void susy_generate_rank4(SUSYAdinkra *sa);
void susy_generate_rank8(SUSYAdinkra *sa);

/* SVG */
char *susy_adinkra_to_svg(Arena *a, const SUSYAdinkra *sa, double width, double height);

/* Edge color names */
const char *susy_edge_color_name(EdgeColor c);

#endif
