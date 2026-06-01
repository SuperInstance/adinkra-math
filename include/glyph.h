#ifndef GLYPH_H
#define GLYPH_H

#include "adinkra.h"

/* Composition operations */
typedef enum {
    COMP_STACK,       /* vertical stack */
    COMP_NEST,        /* one inside another */
    COMP_INTERLEAVE,  /* alternating elements */
    COMP_MIRROR,      /* horizontal mirror */
    COMP_OVERLAY,     /* superimpose */
    COMP_SIDE_BY_SIDE /* horizontal arrangement */
} CompositionOp;

/* A glyph is a named group of primitives */
#define GLYPH_MAX_NAME 64

typedef struct Glyph Glyph;

struct Glyph {
    char name[GLYPH_MAX_NAME];
    GeometricPrimitive prims[ADINKRA_MAX_PRIMITIVES];
    int prim_count;
    double origin_x, origin_y;
    double semantic_invariant; /* preserved across composition */
};

typedef struct {
    char name[GLYPH_MAX_NAME];
    CompositionOp op;
    int left_index;   /* index into glyph array */
    int right_index;  /* -1 for unary ops (mirror) */
    double spacing;
} CompositionRule;

#define GLYPH_MAX_COMPOSITIONS 32

typedef struct {
    Glyph glyphs[64];
    int glyph_count;
    CompositionRule rules[GLYPH_MAX_COMPOSITIONS];
    int rule_count;
} GlyphSystem;

void glyph_system_init(GlyphSystem *gs);
int glyph_add(GlyphSystem *gs, const char *name, double semantic_invariant);
void glyph_add_prim(Glyph *g, PrimitiveType type, double cx, double cy,
                    double size, double rotation, double stroke_width, uint32_t color);

/* Compose two glyphs, returns index of new glyph */
int glyph_compose(GlyphSystem *gs, CompositionOp op, int left, int right, double spacing);

/* Verify semantic invariant is preserved */
int glyph_verify_invariant(const GlyphSystem *gs, int composed_index);

/* Get the composed glyph */
Glyph *glyph_get(GlyphSystem *gs, int index);

/* SVG */
char *glyph_to_svg(Arena *a, const Glyph *g, double width, double height);

#endif
