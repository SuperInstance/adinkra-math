#include "../include/adinkra_api.h"
#include <stdio.h>
#include <string.h>

void adinkra_system_init(AdinkraSystem *sys) {
    sys->arena = arena_create(ADINKRA_SYS_ARENA_SIZE);
    adinkra_registry_init(&sys->arena, &sys->registry, 32);
    susy_adinkra_init(&sys->susy, 4);
    glyph_system_init(&sys->glyphs);
    concept_space_init(&sys->concepts, 6);
    topo_graph_init(&sys->topo, 16);
    sys->initialized = 1;
}

void adinkra_system_destroy(AdinkraSystem *sys) {
    if (sys->initialized) {
        arena_destroy(&sys->arena);
        sys->initialized = 0;
    }
}

void adinkra_system_load_defaults(AdinkraSystem *sys) {
    /* Load built-in Adinkra symbols */
    adinkra_load_builtin_symbols(&sys->arena, &sys->registry);

    /* Generate rank-4 SUSY adinkra */
    susy_generate_rank4(&sys->susy);

    /* Create basic glyphs */
    int g1 = glyph_add(&sys->glyphs, "circle-glyph", 0.5);
    glyph_add_prim(&sys->glyphs.glyphs[g1], PRIM_CIRCLE, 0, 0, 30, 0, 2, 0);

    int g2 = glyph_add(&sys->glyphs, "cross-glyph", 0.7);
    glyph_add_prim(&sys->glyphs.glyphs[g2], PRIM_CROSS, 0, 0, 25, 0, 2, 0);

    int g3 = glyph_add(&sys->glyphs, "diamond-glyph", 0.6);
    glyph_add_prim(&sys->glyphs.glyphs[g3], PRIM_DIAMOND, 0, 0, 28, 0, 2, 0);

    /* Compose some glyphs */
    glyph_compose(&sys->glyphs, COMP_STACK, g1, g2, 50);
    glyph_compose(&sys->glyphs, COMP_MIRROR, g3, -1, 0);
    glyph_compose(&sys->glyphs, COMP_NEST, g1, g3, 0);
    glyph_compose(&sys->glyphs, COMP_SIDE_BY_SIDE, g1, g2, 60);
    glyph_compose(&sys->glyphs, COMP_OVERLAY, g1, g2, 0);

    /* Encode concepts */
    double wisdom[] = {0.9, 0.8, 0.7, 0.3, 0.2, 0.1};
    double strength[] = {0.8, 0.3, 0.2, 0.9, 0.7, 0.6};
    double unity[] = {0.5, 0.9, 0.8, 0.5, 0.9, 0.8};
    double freedom[] = {0.7, 0.4, 0.9, 0.8, 0.3, 0.7};
    double patience[] = {0.3, 0.7, 0.5, 0.4, 0.8, 0.9};
    double adapt[] = {0.6, 0.6, 0.8, 0.7, 0.7, 0.5};

    concept_encode(&sys->concepts, "wisdom", wisdom, 6, 0.9);
    concept_encode(&sys->concepts, "strength", strength, 6, 0.8);
    concept_encode(&sys->concepts, "unity", unity, 6, 0.7);
    concept_encode(&sys->concepts, "freedom", freedom, 6, 0.85);
    concept_encode(&sys->concepts, "patience", patience, 6, 0.75);
    concept_encode(&sys->concepts, "adaptability", adapt, 6, 0.7);

    /* Build topology from SUSY */
    topo_from_susy(&sys->topo, &sys->susy);
}

char *adinkra_system_svg(AdinkraSystem *sys, double width, double height) {
    return adinkra_registry_to_svg(&sys->arena, &sys->registry, width, height);
}

void adinkra_system_analyze(AdinkraSystem *sys) {
    /* Analyze topology of each symbol */
    for (int i = 0; i < sys->registry.count; i++) {
        TopoGraph tg;
        topo_from_symbol(&tg, &sys->registry.symbols[i]);
        /* Could store results — for now just exercises the code */
    }
    /* Analyze SUSY topology */
    topo_from_susy(&sys->topo, &sys->susy);
}
