#define _GNU_SOURCE
#include "../include/adinkra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* --- Arena --- */

Arena arena_create(size_t capacity) {
    Arena a;
    a.base = (uint8_t *)malloc(capacity);
    if (a.base) {
        memset(a.base, 0, capacity);
    }
    a.capacity = a.base ? capacity : 0;
    a.offset = 0;
    return a;
}

void arena_reset(Arena *a) {
    a->offset = 0;
}

void arena_destroy(Arena *a) {
    free(a->base);
    a->base = NULL;
    a->capacity = 0;
    a->offset = 0;
}

void *arena_alloc(Arena *a, size_t size) {
    size = (size + 7) & ~(size_t)7; /* align to 8 bytes */
    if (a->offset + size > a->capacity) return NULL;
    void *ptr = a->base + a->offset;
    a->offset += size;
    return ptr;
}

/* --- Registry --- */

void adinkra_registry_init(Arena *a, AdinkraRegistry *reg, int capacity) {
    reg->symbols = (AdinkraSymbol *)arena_alloc(a, sizeof(AdinkraSymbol) * capacity);
    reg->count = 0;
    reg->capacity = capacity;
}

int adinkra_add_symbol(AdinkraRegistry *reg, const char *name, const char *concept,
                       double semantic_weight) {
    if (reg->count >= reg->capacity) return -1;
    AdinkraSymbol *s = &reg->symbols[reg->count];
    strncpy(s->name, name, ADINKRA_MAX_NAME - 1);
    strncpy(s->concept, concept, ADINKRA_MAX_CONCEPT - 1);
    s->semantic_weight = semantic_weight;
    s->prim_count = 0;
    return reg->count++;
}

void adinkra_add_primitive(AdinkraSymbol *sym, PrimitiveType type,
                           double cx, double cy, double size, double rotation,
                           double stroke_width, uint32_t color) {
    if (sym->prim_count >= ADINKRA_MAX_PRIMITIVES) return;
    GeometricPrimitive *p = &sym->prims[sym->prim_count++];
    p->type = type;
    p->cx = cx;
    p->cy = cy;
    p->size = size;
    p->rotation = rotation;
    p->stroke_width = stroke_width;
    p->color = color;
}

/* --- SVG Generation --- */

static void prim_to_svg(const GeometricPrimitive *p, char *buf, int bufsize, double ox, double oy) {
    double cx = p->cx + ox, cy = p->cy + oy;
    const char *color_str = "black";
    char color_buf[16];
    if (p->color != 0) {
        snprintf(color_buf, sizeof(color_buf), "#%06X", p->color);
        color_str = color_buf;
    }
    switch (p->type) {
        case PRIM_CIRCLE:
            snprintf(buf, bufsize,
                "  <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%.2f\" transform=\"rotate(%.2f,%.2f,%.2f)\"/>\n",
                cx, cy, p->size, color_str, p->stroke_width, p->rotation * 180.0 / M_PI, cx, cy);
            break;
        case PRIM_CROSS: {
            double s = p->size;
            double r = p->rotation;
            double dx1 = cos(r) * s, dy1 = sin(r) * s;
            double dx2 = cos(r + M_PI/2) * s, dy2 = sin(r + M_PI/2) * s;
            snprintf(buf, bufsize,
                "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n"
                "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
                cx - dx1, cy - dy1, cx + dx1, cy + dy1, color_str, p->stroke_width,
                cx - dx2, cy - dy2, cx + dx2, cy + dy2, color_str, p->stroke_width);
            break;
        }
        case PRIM_LINE: {
            double ex = cx + cos(p->rotation) * p->size;
            double ey = cy + sin(p->rotation) * p->size;
            snprintf(buf, bufsize,
                "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
                cx, cy, ex, ey, color_str, p->stroke_width);
            break;
        }
        case PRIM_SPIRAL:
            snprintf(buf, bufsize,
                "  <path d=\"M %.2f %.2f A %.2f %.2f 0 1 1 %.2f %.2f A %.2f %.2f 0 1 1 %.2f %.2f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
                cx, cy, p->size, p->size, cx, cy + p->size,
                p->size * 0.7, p->size * 0.7, cx, cy - p->size * 0.5,
                color_str, p->stroke_width);
            break;
        case PRIM_KNOT:
            snprintf(buf, bufsize,
                "  <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n"
                "  <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n"
                "  <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
                cx - p->size * 0.3, cy, p->size * 0.5, color_str, p->stroke_width,
                cx + p->size * 0.3, cy, p->size * 0.5, color_str, p->stroke_width,
                cx, cy - p->size * 0.3, p->size * 0.5, color_str, p->stroke_width);
            break;
        case PRIM_ARC:
            snprintf(buf, bufsize,
                "  <path d=\"M %.2f %.2f A %.2f %.2f 0 0 1 %.2f %.2f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
                cx - p->size, cy, p->size, p->size, cx + p->size, cy,
                color_str, p->stroke_width);
            break;
        case PRIM_TRIANGLE: {
            double s = p->size;
            snprintf(buf, bufsize,
                "  <polygon points=\"%.2f,%.2f %.2f,%.2f %.2f,%.2f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
                cx, cy - s, cx - s * 0.866, cy + s * 0.5, cx + s * 0.866, cy + s * 0.5,
                color_str, p->stroke_width);
            break;
        }
        case PRIM_DIAMOND:
            snprintf(buf, bufsize,
                "  <polygon points=\"%.2f,%.2f %.2f,%.2f %.2f,%.2f %.2f,%.2f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
                cx, cy - p->size, cx + p->size * 0.6, cy, cx, cy + p->size, cx - p->size * 0.6, cy,
                color_str, p->stroke_width);
            break;
        default:
            buf[0] = '\0';
    }
}

/* Public wrapper for glyph.c */
void adinkra_prim_to_svg(const GeometricPrimitive *p, char *buf, int bufsize, double ox, double oy) {
    prim_to_svg(p, buf, bufsize, ox, oy);
}

char *adinkra_symbol_to_svg(Arena *a, const AdinkraSymbol *sym, double width, double height) {
    /* Allocate generous buffer */
    size_t bufsize = 8192;
    char *svg = (char *)arena_alloc(a, bufsize);
    if (!svg) return NULL;
    int pos = 0;
    pos += snprintf(svg + pos, bufsize - pos,
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%.0f\" height=\"%.0f\">\n"
        " <title>%s</title>\n"
        " <desc>%s</desc>\n",
        width, height, sym->name, sym->concept);

    double ox = width / 2, oy = height / 2;
    char prim_buf[1024];
    for (int i = 0; i < sym->prim_count; i++) {
        prim_to_svg(&sym->prims[i], prim_buf, sizeof(prim_buf), ox, oy);
        pos += snprintf(svg + pos, bufsize - pos, "%s", prim_buf);
    }
    pos += snprintf(svg + pos, bufsize - pos, "</svg>\n");
    return svg;
}

char *adinkra_registry_to_svg(Arena *a, const AdinkraRegistry *reg, double width, double height) {
    size_t bufsize = 65536;
    char *svg = (char *)arena_alloc(a, bufsize);
    if (!svg) return NULL;
    int pos = 0;
    pos += snprintf(svg + pos, bufsize - pos,
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%.0f\" height=\"%.0f\">\n",
        width, height);

    int cols = 4;
    double cell_w = width / cols;
    double cell_h = height / ((reg->count + cols - 1) / cols);
    for (int i = 0; i < reg->count; i++) {
        int col = i % cols, row = i / cols;
        double ox = col * cell_w + cell_w / 2;
        double oy = row * cell_h + cell_h / 2;
        pos += snprintf(svg + pos, bufsize - pos, " <text x=\"%.0f\" y=\"%.0f\" font-size=\"10\" text-anchor=\"middle\">%s</text>\n",
            ox, oy - 30, reg->symbols[i].name);
        char prim_buf[1024];
        double scale = 0.3;
        for (int j = 0; j < reg->symbols[i].prim_count; j++) {
            GeometricPrimitive sp = reg->symbols[i].prims[j];
            sp.cx *= scale; sp.cy *= scale; sp.size *= scale;
            prim_to_svg(&sp, prim_buf, sizeof(prim_buf), ox, oy);
            pos += snprintf(svg + pos, bufsize - pos, "%s", prim_buf);
        }
    }
    pos += snprintf(svg + pos, bufsize - pos, "</svg>\n");
    return svg;
}

/* --- Built-in symbols --- */

void adinkra_load_builtin_symbols(Arena *a, AdinkraRegistry *reg) {
    (void)a;
    /* Gye Nyame - supremacy of God */
    int idx = adinkra_add_symbol(reg, "Gye Nyame", "Supremacy of God", 0.95);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, 0, 0, 40, 0, 2.5, 0x000000);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_SPIRAL, 10, 0, 25, 0, 2.0, 0x000000);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CROSS, 0, 0, 20, 0.5, 1.5, 0x333333);

    /* Adwene Pa - good thinking */
    idx = adinkra_add_symbol(reg, "Adwene Pa", "Good thinking, good heart", 0.80);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_DIAMOND, 0, 0, 35, 0, 2.0, 0x8B4513);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, 0, 0, 25, 0, 1.5, 0x8B4513);

    /* Sankofa - learn from the past */
    idx = adinkra_add_symbol(reg, "Sankofa", "Return and fetch it - learn from past", 0.90);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_SPIRAL, 0, 0, 35, 0, 2.5, 0x654321);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_TRIANGLE, -15, 15, 15, 0, 2.0, 0x654321);

    /* Nkyinkyim - initiative and adaptability */
    idx = adinkra_add_symbol(reg, "Nkyinkyim", "Initiative, dynamism, versatility", 0.75);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_KNOT, 0, 0, 30, 0, 2.0, 0x228B22);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_ARC, 20, 0, 20, 0, 1.5, 0x228B22);

    /* Dwennimmen - humility and strength */
    idx = adinkra_add_symbol(reg, "Dwennimmen", "Humility with strength", 0.85);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, -15, 0, 25, 0, 2.0, 0xDAA520);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, 15, 0, 25, 0, 2.0, 0xDAA520);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CROSS, 0, 0, 15, 0.785, 1.5, 0xDAA520);

    /* Nkonsondepie - chain links, unity */
    idx = adinkra_add_symbol(reg, "Nkonsondepie", "Chain links - unity and bonds", 0.70);
    for (int i = 0; i < 4; i++) {
        adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, (double)(i - 1) * 20, 0, 12, 0, 2.0, 0x4169E1);
    }

    /* Akoma - patience and tolerance */
    idx = adinkra_add_symbol(reg, "Akoma", "Patience and tolerance (heart)", 0.80);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_TRIANGLE, 0, 5, 30, 3.14159, 2.0, 0xCC0000);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, -12, -5, 15, 0, 1.5, 0xCC0000);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, 12, -5, 15, 0, 1.5, 0xCC0000);

    /* Mate Masie - wisdom and knowledge */
    idx = adinkra_add_symbol(reg, "Mate Masie", "What I hear, I keep - wisdom", 0.85);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_DIAMOND, 0, 0, 30, 0, 2.5, 0x800080);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_DIAMOND, 0, 0, 20, 0.5, 1.5, 0x800080);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, 0, 0, 10, 0, 1.0, 0x800080);

    /* Fawohodie - independence and freedom */
    idx = adinkra_add_symbol(reg, "Fawohodie", "Independence, freedom, emancipation", 0.90);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CROSS, 0, 0, 35, 0, 2.5, 0xFF6600);
    adinkra_add_primitive(&reg->symbols[idx], PRIM_CIRCLE, 0, 0, 20, 0, 1.5, 0xFF6600);
}
