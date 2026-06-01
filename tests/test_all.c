#define _GNU_SOURCE
#include "../include/adinkra.h"
#include "../include/supersymmetry.h"
#include "../include/glyph.h"
#include "../include/encoding.h"
#include "../include/topology.h"
#include "../include/adinkra_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define TOLERANCE 1e-9
#define assert_dbl(a, b) assert(fabs((a) - (b)) < TOLERANCE)

static int test_count = 0;
#define TEST(name) do { test_count++; printf("  %d. %-50s ", test_count, #name); } while(0)
#define PASS() printf("PASS\n")

/* ===== Arena Tests ===== */
static void test_arena_create_destroy(void) {
    TEST(arena_create_destroy);
    Arena a = arena_create(4096);
    assert(a.base != NULL);
    assert(a.capacity == 4096);
    assert(a.offset == 0);
    arena_destroy(&a);
    assert(a.base == NULL);
    PASS();
}

static void test_arena_alloc_basic(void) {
    TEST(arena_alloc_basic);
    Arena a = arena_create(4096);
    void *p1 = arena_alloc(&a, 100);
    assert(p1 != NULL);
    assert(a.offset >= 100);
    void *p2 = arena_alloc(&a, 200);
    assert(p2 != NULL);
    assert((uint8_t*)p2 >= (uint8_t*)p1 + 100);
    arena_destroy(&a);
    PASS();
}

static void test_arena_alloc_overflow(void) {
    TEST(arena_alloc_overflow);
    Arena a = arena_create(64);
    void *p = arena_alloc(&a, 100);
    assert(p == NULL);
    arena_destroy(&a);
    PASS();
}

static void test_arena_reset(void) {
    TEST(arena_reset);
    Arena a = arena_create(4096);
    arena_alloc(&a, 100);
    assert(a.offset > 0);
    arena_reset(&a);
    assert(a.offset == 0);
    void *p = arena_alloc(&a, 100);
    assert(p != NULL);
    arena_destroy(&a);
    PASS();
}

static void test_arena_alignment(void) {
    TEST(arena_alignment);
    Arena a = arena_create(4096);
    arena_alloc(&a, 3); /* 3 bytes, should align to 8 */
    assert(a.offset == 8);
    arena_alloc(&a, 1);
    assert(a.offset == 16);
    arena_destroy(&a);
    PASS();
}

/* ===== Adinkra Symbol Tests ===== */
static void test_registry_init(void) {
    TEST(registry_init);
    Arena a = arena_create(1024 * 1024);
    AdinkraRegistry reg;
    adinkra_registry_init(&a, &reg, 16);
    assert(reg.count == 0);
    assert(reg.capacity == 16);
    assert(reg.symbols != NULL);
    arena_destroy(&a);
    PASS();
}

static void test_add_symbol(void) {
    TEST(add_symbol);
    Arena a = arena_create(1024 * 1024);
    AdinkraRegistry reg;
    adinkra_registry_init(&a, &reg, 16);
    int idx = adinkra_add_symbol(&reg, "TestSym", "Test concept", 0.5);
    assert(idx == 0);
    assert(strcmp(reg.symbols[0].name, "TestSym") == 0);
    assert(strcmp(reg.symbols[0].concept, "Test concept") == 0);
    assert_dbl(reg.symbols[0].semantic_weight, 0.5);
    arena_destroy(&a);
    PASS();
}

static void test_add_primitive(void) {
    TEST(add_primitive);
    AdinkraSymbol sym = {0};
    adinkra_add_primitive(&sym, PRIM_CIRCLE, 10, 20, 30, 0.5, 2.0, 0xFF0000);
    assert(sym.prim_count == 1);
    assert(sym.prims[0].type == PRIM_CIRCLE);
    assert_dbl(sym.prims[0].cx, 10);
    assert_dbl(sym.prims[0].cy, 20);
    assert_dbl(sym.prims[0].size, 30);
    assert(sym.prims[0].color == 0xFF0000);
    PASS();
}

static void test_builtin_symbols(void) {
    TEST(builtin_symbols);
    Arena a = arena_create(1024 * 1024);
    AdinkraRegistry reg;
    adinkra_registry_init(&a, &reg, 32);
    adinkra_load_builtin_symbols(&a, &reg);
    assert(reg.count == 9);
    assert(strcmp(reg.symbols[0].name, "Gye Nyame") == 0);
    assert(strcmp(reg.symbols[2].name, "Sankofa") == 0);
    arena_destroy(&a);
    PASS();
}

static void test_symbol_svg(void) {
    TEST(symbol_svg_output);
    Arena a = arena_create(1024 * 1024);
    AdinkraRegistry reg;
    adinkra_registry_init(&a, &reg, 8);
    int idx = adinkra_add_symbol(&reg, "TestSVG", "SVG test", 0.5);
    adinkra_add_primitive(&reg.symbols[idx], PRIM_CIRCLE, 0, 0, 30, 0, 2, 0);
    char *svg = adinkra_symbol_to_svg(&a, &reg.symbols[idx], 200, 200);
    assert(svg != NULL);
    assert(strstr(svg, "<svg") != NULL);
    assert(strstr(svg, "circle") != NULL);
    assert(strstr(svg, "TestSVG") != NULL);
    arena_destroy(&a);
    PASS();
}

static void test_registry_svg(void) {
    TEST(registry_svg_output);
    Arena a = arena_create(1024 * 1024);
    AdinkraRegistry reg;
    adinkra_registry_init(&a, &reg, 16);
    adinkra_load_builtin_symbols(&a, &reg);
    char *svg = adinkra_registry_to_svg(&a, &reg, 800, 600);
    assert(svg != NULL);
    assert(strstr(svg, "<svg") != NULL);
    assert(strstr(svg, "Gye Nyame") != NULL);
    arena_destroy(&a);
    PASS();
}

static void test_all_primitive_types_svg(void) {
    TEST(all_primitive_types_svg);
    Arena a = arena_create(1024 * 1024);
    AdinkraSymbol sym = {0};
    strncpy(sym.name, "all", 4);
    strncpy(sym.concept, "all prims", 10);
    sym.prim_count = 0;
    adinkra_add_primitive(&sym, PRIM_CIRCLE, 0, 0, 30, 0, 2, 0);
    adinkra_add_primitive(&sym, PRIM_SPIRAL, 0, 50, 25, 0, 2, 0);
    adinkra_add_primitive(&sym, PRIM_CROSS, 0, -50, 20, 0, 2, 0);
    adinkra_add_primitive(&sym, PRIM_KNOT, 50, 0, 25, 0, 2, 0);
    adinkra_add_primitive(&sym, PRIM_LINE, -50, 0, 30, 0.5, 2, 0);
    adinkra_add_primitive(&sym, PRIM_ARC, 0, 0, 20, 0, 2, 0);
    adinkra_add_primitive(&sym, PRIM_TRIANGLE, 30, 30, 15, 0, 2, 0);
    adinkra_add_primitive(&sym, PRIM_DIAMOND, -30, -30, 15, 0, 2, 0);
    assert(sym.prim_count == 8);
    char *svg = adinkra_symbol_to_svg(&a, &sym, 300, 300);
    assert(svg != NULL);
    assert(strstr(svg, "circle") != NULL);
    assert(strstr(svg, "line") != NULL);
    assert(strstr(svg, "polygon") != NULL);
    arena_destroy(&a);
    PASS();
}

static void test_semantic_weights(void) {
    TEST(semantic_weights_range);
    Arena a = arena_create(1024 * 1024);
    AdinkraRegistry reg;
    adinkra_registry_init(&a, &reg, 16);
    adinkra_load_builtin_symbols(&a, &reg);
    for (int i = 0; i < reg.count; i++) {
        assert(reg.symbols[i].semantic_weight >= 0.0);
        assert(reg.symbols[i].semantic_weight <= 1.0);
    }
    arena_destroy(&a);
    PASS();
}

/* ===== SUSY Tests ===== */
static void test_susy_init(void) {
    TEST(susy_init);
    SUSYAdinkra sa;
    susy_adinkra_init(&sa, 4);
    assert(sa.node_count == 0);
    assert(sa.edge_count == 0);
    assert(sa.rank == 4);
    PASS();
}

static void test_susy_add_nodes(void) {
    TEST(susy_add_nodes);
    SUSYAdinkra sa;
    susy_adinkra_init(&sa, 4);
    int b = susy_add_node(&sa, NODE_BOSON, "B0");
    int f = susy_add_node(&sa, NODE_FERMION, "F0");
    assert(b == 0);
    assert(f == 1);
    assert(sa.node_count == 2);
    assert(sa.nodes[0].type == NODE_BOSON);
    assert(sa.nodes[1].type == NODE_FERMION);
    PASS();
}

static void test_susy_add_edge(void) {
    TEST(susy_add_edge);
    SUSYAdinkra sa;
    susy_adinkra_init(&sa, 4);
    susy_add_node(&sa, NODE_BOSON, "B0");
    susy_add_node(&sa, NODE_FERMION, "F0");
    int e = susy_add_edge(&sa, 0, 1, EDGE_RED, 1);
    assert(e == 0);
    assert(sa.edge_count == 1);
    assert(sa.edges[0].color == EDGE_RED);
    assert(sa.edges[0].sign == 1);
    PASS();
}

static void test_susy_rank4(void) {
    TEST(susy_rank4_generation);
    SUSYAdinkra sa;
    susy_generate_rank4(&sa);
    assert(sa.rank == 4);
    assert(sa.node_count == 8); /* 4 bosons + 4 fermions */
    assert(sa.edge_count == 16); /* 4x4 connections */
    PASS();
}

static void test_susy_rank8(void) {
    TEST(susy_rank8_generation);
    SUSYAdinkra sa;
    susy_generate_rank8(&sa);
    assert(sa.rank == 8);
    assert(sa.node_count == 16);
    assert(sa.edge_count == 64);
    PASS();
}

static void test_susy_chromotopology_rank4(void) {
    TEST(susy_chromotopology_rank4);
    SUSYAdinkra sa;
    susy_generate_rank4(&sa);
    int valid = susy_verify_chromotopology(&sa);
    assert(valid == 1);
    PASS();
}

static void test_susy_representation_rank4(void) {
    TEST(susy_representation_rank4);
    SUSYAdinkra sa;
    susy_generate_rank4(&sa);
    int valid = susy_verify_representation(&sa);
    assert(valid == 1);
    PASS();
}

static void test_susy_invalid_representation(void) {
    TEST(susy_invalid_representation);
    SUSYAdinkra sa;
    susy_adinkra_init(&sa, 2);
    susy_add_node(&sa, NODE_BOSON, "B0");
    susy_add_node(&sa, NODE_BOSON, "B1");
    susy_add_edge(&sa, 0, 1, EDGE_RED, 1);
    int valid = susy_verify_representation(&sa);
    assert(valid == 0); /* boson-boson edge = invalid */
    PASS();
}

static void test_susy_svg(void) {
    TEST(susy_svg_output);
    Arena a = arena_create(1024 * 1024);
    SUSYAdinkra sa;
    susy_generate_rank4(&sa);
    char *svg = susy_adinkra_to_svg(&a, &sa, 400, 400);
    assert(svg != NULL);
    assert(strstr(svg, "<svg") != NULL);
    assert(strstr(svg, "B0") != NULL);
    assert(strstr(svg, "F0") != NULL);
    arena_destroy(&a);
    PASS();
}

static void test_susy_edge_colors(void) {
    TEST(susy_edge_color_names);
    assert(strcmp(susy_edge_color_name(EDGE_RED), "red") == 0);
    assert(strcmp(susy_edge_color_name(EDGE_BLUE), "blue") == 0);
    assert(strcmp(susy_edge_color_name(EDGE_GREEN), "green") == 0);
    PASS();
}

/* ===== Glyph Tests ===== */
static void test_glyph_add(void) {
    TEST(glyph_add);
    GlyphSystem gs;
    glyph_system_init(&gs);
    int idx = glyph_add(&gs, "test", 0.5);
    assert(idx == 0);
    assert(strcmp(gs.glyphs[0].name, "test") == 0);
    assert_dbl(gs.glyphs[0].semantic_invariant, 0.5);
    PASS();
}

static void test_glyph_compose_stack(void) {
    TEST(glyph_compose_stack);
    GlyphSystem gs;
    glyph_system_init(&gs);
    int g1 = glyph_add(&gs, "a", 0.6);
    glyph_add_prim(&gs.glyphs[g1], PRIM_CIRCLE, 0, 0, 20, 0, 2, 0);
    int g2 = glyph_add(&gs, "b", 0.8);
    glyph_add_prim(&gs.glyphs[g2], PRIM_CROSS, 0, 0, 15, 0, 2, 0);
    int comp = glyph_compose(&gs, COMP_STACK, g1, g2, 40);
    assert(comp >= 0);
    assert(gs.glyphs[comp].prim_count == 2);
    assert_dbl(gs.glyphs[comp].semantic_invariant, 0.7); /* average */
    PASS();
}

static void test_glyph_compose_mirror(void) {
    TEST(glyph_compose_mirror);
    GlyphSystem gs;
    glyph_system_init(&gs);
    int g1 = glyph_add(&gs, "mir", 0.5);
    glyph_add_prim(&gs.glyphs[g1], PRIM_LINE, 10, 0, 20, 0, 2, 0);
    int comp = glyph_compose(&gs, COMP_MIRROR, g1, -1, 0);
    assert(comp >= 0);
    assert(gs.glyphs[comp].prim_count == 2);
    PASS();
}

static void test_glyph_compose_nest(void) {
    TEST(glyph_compose_nest);
    GlyphSystem gs;
    glyph_system_init(&gs);
    int g1 = glyph_add(&gs, "outer", 0.7);
    glyph_add_prim(&gs.glyphs[g1], PRIM_CIRCLE, 0, 0, 30, 0, 2, 0);
    int g2 = glyph_add(&gs, "inner", 0.6);
    glyph_add_prim(&gs.glyphs[g2], PRIM_CROSS, 0, 0, 20, 0, 2, 0);
    int comp = glyph_compose(&gs, COMP_NEST, g1, g2, 0);
    assert(comp >= 0);
    assert_dbl(gs.glyphs[comp].semantic_invariant, 0.42); /* 0.7 * 0.6 */
    PASS();
}

static void test_glyph_compose_side_by_side(void) {
    TEST(glyph_compose_side_by_side);
    GlyphSystem gs;
    glyph_system_init(&gs);
    int g1 = glyph_add(&gs, "left", 0.5);
    glyph_add_prim(&gs.glyphs[g1], PRIM_CIRCLE, 0, 0, 20, 0, 2, 0);
    int g2 = glyph_add(&gs, "right", 0.7);
    glyph_add_prim(&gs.glyphs[g2], PRIM_DIAMOND, 0, 0, 20, 0, 2, 0);
    int comp = glyph_compose(&gs, COMP_SIDE_BY_SIDE, g1, g2, 60);
    assert(comp >= 0);
    PASS();
}

static void test_glyph_invariant_preserved(void) {
    TEST(glyph_invariant_preserved);
    GlyphSystem gs;
    glyph_system_init(&gs);
    int g1 = glyph_add(&gs, "a", 0.6);
    int g2 = glyph_add(&gs, "b", 0.8);
    int comp = glyph_compose(&gs, COMP_STACK, g1, g2, 40);
    assert(glyph_verify_invariant(&gs, comp) == 1);
    PASS();
}

static void test_glyph_svg(void) {
    TEST(glyph_svg_output);
    Arena a = arena_create(1024 * 1024);
    GlyphSystem gs;
    glyph_system_init(&gs);
    int g1 = glyph_add(&gs, "test-svg", 0.5);
    glyph_add_prim(&gs.glyphs[g1], PRIM_CIRCLE, 0, 0, 25, 0, 2, 0);
    char *svg = glyph_to_svg(&a, &gs.glyphs[g1], 200, 200);
    assert(svg != NULL);
    assert(strstr(svg, "test-svg") != NULL);
    arena_destroy(&a);
    PASS();
}

/* ===== Encoding Tests ===== */
static void test_concept_encode(void) {
    TEST(concept_encode);
    ConceptSpace cs;
    concept_space_init(&cs, 4);
    double v[] = {1.0, 0.5, 0.3, 0.8};
    int idx = concept_encode(&cs, "test", v, 4, 0.9);
    assert(idx == 0);
    assert(strcmp(cs.concepts[0].name, "test") == 0);
    assert_dbl(cs.concepts[0].vector[0], 1.0);
    PASS();
}

static void test_concept_distance_zero(void) {
    TEST(concept_distance_zero);
    EncodedConcept a = {.dim = 3, .vector = {1, 2, 3}};
    EncodedConcept b = {.dim = 3, .vector = {1, 2, 3}};
    assert_dbl(concept_distance(&a, &b), 0.0);
    PASS();
}

static void test_concept_distance_positive(void) {
    TEST(concept_distance_positive);
    EncodedConcept a = {.dim = 2, .vector = {0, 0}};
    EncodedConcept b = {.dim = 2, .vector = {3, 4}};
    assert_dbl(concept_distance(&a, &b), 5.0);
    PASS();
}

static void test_concept_similarity_identical(void) {
    TEST(concept_similarity_identical);
    EncodedConcept a = {.dim = 3, .vector = {0.5, 0.5, 0.5}};
    assert_dbl(concept_similarity(&a, &a), 1.0);
    PASS();
}

static void test_concept_nearest(void) {
    TEST(concept_nearest);
    ConceptSpace cs;
    concept_space_init(&cs, 3);
    double v1[] = {1, 0, 0};
    double v2[] = {0, 1, 0};
    double v3[] = {0, 0, 1};
    concept_encode(&cs, "x-axis", v1, 3, 1);
    concept_encode(&cs, "y-axis", v2, 3, 1);
    concept_encode(&cs, "z-axis", v3, 3, 1);
    double q[] = {0.9, 0.1, 0.1};
    int nearest = concept_nearest(&cs, q, 3);
    assert(nearest == 0); /* closest to x-axis */
    PASS();
}

static void test_concept_k_nearest(void) {
    TEST(concept_k_nearest);
    ConceptSpace cs;
    concept_space_init(&cs, 2);
    double v1[] = {0, 0};
    double v2[] = {1, 1};
    double v3[] = {2, 2};
    double v4[] = {10, 10};
    concept_encode(&cs, "p0", v1, 2, 1);
    concept_encode(&cs, "p1", v2, 2, 1);
    concept_encode(&cs, "p2", v3, 2, 1);
    concept_encode(&cs, "far", v4, 2, 1);
    double q[] = {0.5, 0.5};
    int results[3];
    concept_k_nearest(&cs, q, 2, results, 3);
    assert(results[0] == 0 || results[0] == 1);
    assert(results[2] == 2); /* p2 is third nearest, 'far' would be 4th */
    PASS();
}

static void test_concept_clustering(void) {
    TEST(concept_clustering);
    ConceptSpace cs;
    concept_space_init(&cs, 2);
    /* Two clear clusters */
    for (int i = 0; i < 5; i++) {
        double v[] = {0.1 + i*0.02, 0.1 + i*0.02};
        char name[16]; snprintf(name, 16, "c1_%d", i);
        concept_encode(&cs, name, v, 2, 1);
    }
    for (int i = 0; i < 5; i++) {
        double v[] = {0.9 - i*0.02, 0.9 - i*0.02};
        char name[16]; snprintf(name, 16, "c2_%d", i);
        concept_encode(&cs, name, v, 2, 1);
    }
    ConceptClusters clusters;
    concept_cluster(&cs, &clusters, 2, 20);
    assert(clusters.k == 2);
    assert(clusters.iterations > 0);
    /* First 5 should be in same cluster */
    int c = clusters.assignments[0];
    for (int i = 1; i < 5; i++) {
        assert(clusters.assignments[i] == c);
    }
    /* Last 5 should be in different cluster */
    assert(clusters.assignments[5] != c);
    PASS();
}

static void test_concept_similarity_range(void) {
    TEST(concept_similarity_range);
    ConceptSpace cs;
    concept_space_init(&cs, 6);
    double wisdom[] = {0.9, 0.8, 0.7, 0.3, 0.2, 0.1};
    double strength[] = {0.8, 0.3, 0.2, 0.9, 0.7, 0.6};
    concept_encode(&cs, "wisdom", wisdom, 6, 0.9);
    concept_encode(&cs, "strength", strength, 6, 0.8);
    double sim = concept_similarity(&cs.concepts[0], &cs.concepts[1]);
    assert(sim >= 0.0 && sim <= 1.0);
    PASS();
}

/* ===== Topology Tests ===== */
static void test_topo_init(void) {
    TEST(topo_init);
    TopoGraph g;
    topo_graph_init(&g, 10);
    assert(g.vertex_count == 10);
    assert(g.edge_count == 0);
    PASS();
}

static void test_topo_add_edge(void) {
    TEST(topo_add_edge);
    TopoGraph g;
    topo_graph_init(&g, 4);
    topo_add_edge(&g, 0, 1);
    topo_add_edge(&g, 1, 2);
    assert(g.edge_count == 2);
    assert(g.adj[0][1] == 1);
    assert(g.adj[1][0] == 1);
    assert(g.adj[0][2] == 0);
    PASS();
}

static void test_topo_no_duplicate_edges(void) {
    TEST(topo_no_duplicate_edges);
    TopoGraph g;
    topo_graph_init(&g, 4);
    topo_add_edge(&g, 0, 1);
    topo_add_edge(&g, 1, 0); /* duplicate */
    assert(g.edge_count == 1);
    PASS();
}

static void test_topo_connected(void) {
    TEST(topo_connected);
    TopoGraph g;
    topo_graph_init(&g, 4);
    topo_add_edge(&g, 0, 1);
    topo_add_edge(&g, 1, 2);
    topo_add_edge(&g, 2, 3);
    int comp[4];
    int n = topo_connected_components(&g, comp);
    assert(n == 1);
    PASS();
}

static void test_topo_disconnected(void) {
    TEST(topo_disconnected);
    TopoGraph g;
    topo_graph_init(&g, 6);
    topo_add_edge(&g, 0, 1);
    topo_add_edge(&g, 1, 2);
    /* 3,4,5 are isolated */
    int comp[6];
    int n = topo_connected_components(&g, comp);
    assert(n == 4); /* {0,1,2}, {3}, {4}, {5} */
    assert(comp[0] == comp[1] && comp[1] == comp[2]);
    assert(comp[3] != comp[0]);
    PASS();
}

static void test_topo_euler(void) {
    TEST(topo_euler_characteristic);
    /* Triangle: V=3, E=3, C=1, F=2 → chi = 3 - 3 + 2 = 2 */
    TopoGraph g;
    topo_graph_init(&g, 3);
    topo_add_edge(&g, 0, 1);
    topo_add_edge(&g, 1, 2);
    topo_add_edge(&g, 2, 0);
    int chi = topo_euler_characteristic(&g);
    assert(chi == 2);
    PASS();
}

static void test_topo_genus(void) {
    TEST(topo_genus);
    /* Tree on 3 nodes: V=3, E=2, chi=2 → genus=0 */
    TopoGraph g;
    topo_graph_init(&g, 3);
    topo_add_edge(&g, 0, 1);
    topo_add_edge(&g, 1, 2);
    int gen = topo_genus(&g);
    assert(gen == 0);
    PASS();
}

static void test_topo_equivalent(void) {
    TEST(topo_equivalent);
    TopoGraph a, b;
    topo_graph_init(&a, 3);
    topo_add_edge(&a, 0, 1);
    topo_add_edge(&a, 1, 2);
    topo_graph_init(&b, 3);
    topo_add_edge(&b, 0, 1);
    topo_add_edge(&b, 1, 2);
    assert(topo_equivalent(&a, &b) == 1);
    PASS();
}

static void test_topo_not_equivalent(void) {
    TEST(topo_not_equivalent);
    TopoGraph a, b;
    topo_graph_init(&a, 3);
    topo_add_edge(&a, 0, 1);
    topo_add_edge(&a, 1, 2);
    topo_graph_init(&b, 3);
    topo_add_edge(&b, 0, 1);
    topo_add_edge(&b, 0, 2);
    assert(topo_equivalent(&a, &b) == 0); /* different degree seqs */
    PASS();
}

static void test_topo_from_susy(void) {
    TEST(topo_from_susy);
    SUSYAdinkra sa;
    susy_generate_rank4(&sa);
    TopoGraph g;
    topo_from_susy(&g, &sa);
    assert(g.vertex_count == 8);
    assert(g.edge_count == 16);
    int comp[8];
    int n = topo_connected_components(&g, comp);
    assert(n == 1); /* fully connected via bipartite */
    PASS();
}

static void test_topo_from_symbol(void) {
    TEST(topo_from_symbol);
    AdinkraSymbol sym = {0};
    adinkra_add_primitive(&sym, PRIM_CIRCLE, 0, 0, 30, 0, 2, 0);
    adinkra_add_primitive(&sym, PRIM_CIRCLE, 20, 0, 20, 0, 2, 0); /* overlapping */
    TopoGraph g;
    topo_from_symbol(&g, &sym);
    assert(g.vertex_count == 2);
    assert(g.edge_count == 1);
    PASS();
}

static void test_topo_planar(void) {
    TEST(topo_planar_check);
    TopoGraph g;
    topo_graph_init(&g, 4);
    topo_add_edge(&g, 0, 1);
    topo_add_edge(&g, 1, 2);
    topo_add_edge(&g, 2, 3);
    assert(topo_is_planar(&g) == 1);
    PASS();
}

/* ===== Unified API Tests ===== */
static void test_system_init_destroy(void) {
    TEST(system_init_destroy);
    AdinkraSystem sys;
    adinkra_system_init(&sys);
    assert(sys.initialized == 1);
    assert(sys.arena.base != NULL);
    adinkra_system_destroy(&sys);
    assert(sys.initialized == 0);
    PASS();
}

static void test_system_load_defaults(void) {
    TEST(system_load_defaults);
    AdinkraSystem sys;
    adinkra_system_init(&sys);
    adinkra_system_load_defaults(&sys);
    assert(sys.registry.count == 9);
    assert(sys.susy.node_count == 8);
    assert(sys.glyphs.glyph_count > 3);
    assert(sys.concepts.count == 6);
    adinkra_system_destroy(&sys);
    PASS();
}

static void test_system_svg(void) {
    TEST(system_svg_output);
    AdinkraSystem sys;
    adinkra_system_init(&sys);
    adinkra_system_load_defaults(&sys);
    char *svg = adinkra_system_svg(&sys, 800, 600);
    assert(svg != NULL);
    assert(strstr(svg, "<svg") != NULL);
    adinkra_system_destroy(&sys);
    PASS();
}

static void test_system_analyze(void) {
    TEST(system_analyze);
    AdinkraSystem sys;
    adinkra_system_init(&sys);
    adinkra_system_load_defaults(&sys);
    adinkra_system_analyze(&sys);
    /* Should not crash */
    assert(sys.topo.vertex_count == 8);
    adinkra_system_destroy(&sys);
    PASS();
}

static void test_system_susy_rank8(void) {
    TEST(system_susy_rank8);
    AdinkraSystem sys;
    adinkra_system_init(&sys);
    susy_generate_rank8(&sys.susy);
    assert(sys.susy.node_count == 16);
    assert(sys.susy.edge_count == 64);
    topo_from_susy(&sys.topo, &sys.susy);
    assert(sys.topo.vertex_count == 16);
    adinkra_system_destroy(&sys);
    PASS();
}

int main(void) {
    printf("\n=== adinkra-math test suite ===\n\n");

    printf("[Arena]\n");
    test_arena_create_destroy();
    test_arena_alloc_basic();
    test_arena_alloc_overflow();
    test_arena_reset();
    test_arena_alignment();

    printf("\n[Adinkra Symbols]\n");
    test_registry_init();
    test_add_symbol();
    test_add_primitive();
    test_builtin_symbols();
    test_symbol_svg();
    test_registry_svg();
    test_all_primitive_types_svg();
    test_semantic_weights();

    printf("\n[Supersymmetry]\n");
    test_susy_init();
    test_susy_add_nodes();
    test_susy_add_edge();
    test_susy_rank4();
    test_susy_rank8();
    test_susy_chromotopology_rank4();
    test_susy_representation_rank4();
    test_susy_invalid_representation();
    test_susy_svg();
    test_susy_edge_colors();

    printf("\n[Glyph Composition]\n");
    test_glyph_add();
    test_glyph_compose_stack();
    test_glyph_compose_mirror();
    test_glyph_compose_nest();
    test_glyph_compose_side_by_side();
    test_glyph_invariant_preserved();
    test_glyph_svg();

    printf("\n[Concept Encoding]\n");
    test_concept_encode();
    test_concept_distance_zero();
    test_concept_distance_positive();
    test_concept_similarity_identical();
    test_concept_nearest();
    test_concept_k_nearest();
    test_concept_clustering();
    test_concept_similarity_range();

    printf("\n[Topology]\n");
    test_topo_init();
    test_topo_add_edge();
    test_topo_no_duplicate_edges();
    test_topo_connected();
    test_topo_disconnected();
    test_topo_euler();
    test_topo_genus();
    test_topo_equivalent();
    test_topo_not_equivalent();
    test_topo_from_susy();
    test_topo_from_symbol();
    test_topo_planar();

    printf("\n[Unified API]\n");
    test_system_init_destroy();
    test_system_load_defaults();
    test_system_svg();
    test_system_analyze();
    test_system_susy_rank8();

    printf("\n=== %d tests passed ===\n\n", test_count);
    return 0;
}
