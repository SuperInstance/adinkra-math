#include "adinkra_api.h"
#include <stdio.h>

int main(void) {
    printf("adinkra-math: Symbolic Mathematics of Adinkra Patterns\n");
    printf("=======================================================\n\n");

    AdinkraSystem sys;
    adinkra_system_init(&sys);
    adinkra_system_load_defaults(&sys);

    /* Print loaded symbols */
    printf("Loaded %d Adinkra symbols:\n", sys.registry.count);
    for (int i = 0; i < sys.registry.count; i++) {
        printf("  %s — %s (weight=%.2f, prims=%d)\n",
            sys.registry.symbols[i].name,
            sys.registry.symbols[i].concept,
            sys.registry.symbols[i].semantic_weight,
            sys.registry.symbols[i].prim_count);
    }

    /* SUSY info */
    printf("\nSUSY Adinkra (rank %d): %d nodes, %d edges\n",
        sys.susy.rank, sys.susy.node_count, sys.susy.edge_count);
    printf("  Chromotopology: %s\n",
        susy_verify_chromotopology(&sys.susy) ? "VALID" : "INVALID");
    printf("  Representation: %s\n",
        susy_verify_representation(&sys.susy) ? "VALID" : "INVALID");

    /* Glyph compositions */
    printf("\nGlyph system: %d glyphs, %d composition rules\n",
        sys.glyphs.glyph_count, sys.glyphs.rule_count);

    /* Concepts */
    printf("\nConcept space: %d concepts in %d dimensions\n",
        sys.concepts.count, sys.concepts.dimensions);
    double query[] = {0.8, 0.7, 0.6, 0.4, 0.3, 0.2};
    int nearest = concept_nearest(&sys.concepts, query, 6);
    if (nearest >= 0) {
        printf("  Query nearest concept: %s\n", sys.concepts.concepts[nearest].name);
    }

    /* Topology */
    printf("\nTopology: V=%d, E=%d\n", sys.topo.vertex_count, sys.topo.edge_count);
    int comp[32];
    int nc = topo_connected_components(&sys.topo, comp);
    printf("  Connected components: %d\n", nc);
    printf("  Euler characteristic: %d\n", topo_euler_characteristic(&sys.topo));
    printf("  Genus: %d\n", topo_genus(&sys.topo));
    printf("  Planar: %s\n", topo_is_planar(&sys.topo) ? "yes" : "no");

    /* Generate SVG */
    char *svg = adinkra_system_svg(&sys, 800, 600);
    if (svg) {
        FILE *f = fopen("examples/output.svg", "w");
        if (f) { fprintf(f, "%s", svg); fclose(f); printf("\nSVG saved to examples/output.svg\n"); }
    }

    adinkra_system_destroy(&sys);
    printf("\nDone. Arena freed cleanly.\n");
    return 0;
}
