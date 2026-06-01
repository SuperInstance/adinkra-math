#ifndef ADINKRA_API_H
#define ADINKRA_API_H

#include "adinkra.h"
#include "supersymmetry.h"
#include "glyph.h"
#include "encoding.h"
#include "topology.h"

/* Unified system */
#define ADINKRA_SYS_ARENA_SIZE (4 * 1024 * 1024) /* 4MB */

typedef struct {
    Arena arena;
    AdinkraRegistry registry;
    SUSYAdinkra susy;
    GlyphSystem glyphs;
    ConceptSpace concepts;
    TopoGraph topo;
    int initialized;
} AdinkraSystem;

void adinkra_system_init(AdinkraSystem *sys);
void adinkra_system_destroy(AdinkraSystem *sys);

/* Convenience: load everything built-in */
void adinkra_system_load_defaults(AdinkraSystem *sys);

/* Generate SVG for the full system state */
char *adinkra_system_svg(AdinkraSystem *sys, double width, double height);

/* Analyze topology of all symbols */
void adinkra_system_analyze(AdinkraSystem *sys);

#endif
