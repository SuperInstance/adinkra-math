#ifndef ADINKRA_H
#define ADINKRA_H

#include <stddef.h>
#include <stdint.h>

/* Arena allocator */
typedef struct {
    uint8_t *base;
    size_t capacity;
    size_t offset;
} Arena;

Arena arena_create(size_t capacity);
void arena_reset(Arena *a);
void arena_destroy(Arena *a);
void *arena_alloc(Arena *a, size_t size);

/* Geometric primitives */
typedef enum {
    PRIM_CIRCLE,
    PRIM_SPIRAL,
    PRIM_CROSS,
    PRIM_KNOT,
    PRIM_LINE,
    PRIM_ARC,
    PRIM_TRIANGLE,
    PRIM_DIAMOND
} PrimitiveType;

typedef struct {
    PrimitiveType type;
    double cx, cy;          /* center */
    double size;            /* radius / extent */
    double rotation;        /* radians */
    double stroke_width;
    uint32_t color;         /* 0xRRGGBB */
} GeometricPrimitive;

/* Adinkra symbol */
#define ADINKRA_MAX_PRIMITIVES 64
#define ADINKRA_MAX_NAME 64
#define ADINKRA_MAX_CONCEPT 128

typedef struct {
    char name[ADINKRA_MAX_NAME];
    char concept[ADINKRA_MAX_CONCEPT];
    GeometricPrimitive prims[ADINKRA_MAX_PRIMITIVES];
    int prim_count;
    double semantic_weight;  /* significance 0..1 */
} AdinkraSymbol;

typedef struct {
    AdinkraSymbol *symbols;
    int count;
    int capacity;
} AdinkraRegistry;

void adinkra_registry_init(Arena *a, AdinkraRegistry *reg, int capacity);
int adinkra_add_symbol(AdinkraRegistry *reg, const char *name, const char *concept,
                       double semantic_weight);
void adinkra_add_primitive(AdinkraSymbol *sym, PrimitiveType type,
                           double cx, double cy, double size, double rotation,
                           double stroke_width, uint32_t color);

/* SVG output */
char *adinkra_symbol_to_svg(Arena *a, const AdinkraSymbol *sym, double width, double height);
char *adinkra_registry_to_svg(Arena *a, const AdinkraRegistry *reg, double width, double height);

/* SVG primitive renderer (shared with glyph.c) */
void adinkra_prim_to_svg(const GeometricPrimitive *p, char *buf, int bufsize, double ox, double oy);

/* Built-in symbols */
void adinkra_load_builtin_symbols(Arena *a, AdinkraRegistry *reg);

#endif
