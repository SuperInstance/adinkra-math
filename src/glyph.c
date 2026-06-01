#include "../include/glyph.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

void glyph_system_init(GlyphSystem *gs) {
    memset(gs, 0, sizeof(*gs));
}

int glyph_add(GlyphSystem *gs, const char *name, double semantic_invariant) {
    if (gs->glyph_count >= 64) return -1;
    Glyph *g = &gs->glyphs[gs->glyph_count];
    strncpy(g->name, name, GLYPH_MAX_NAME - 1);
    g->semantic_invariant = semantic_invariant;
    g->prim_count = 0;
    g->origin_x = 0;
    g->origin_y = 0;
    return gs->glyph_count++;
}

void glyph_add_prim(Glyph *g, PrimitiveType type, double cx, double cy,
                    double size, double rotation, double stroke_width, uint32_t color) {
    if (g->prim_count >= ADINKRA_MAX_PRIMITIVES) return;
    GeometricPrimitive *p = &g->prims[g->prim_count++];
    p->type = type;
    p->cx = cx;
    p->cy = cy;
    p->size = size;
    p->rotation = rotation;
    p->stroke_width = stroke_width;
    p->color = color;
}

static void transform_prims(Glyph *dst, const Glyph *src, double tx, double ty, double sx, double sy) {
    dst->prim_count = src->prim_count;
    for (int i = 0; i < src->prim_count; i++) {
        dst->prims[i] = src->prims[i];
        dst->prims[i].cx = src->prims[i].cx * sx + tx;
        dst->prims[i].cy = src->prims[i].cy * sy + ty;
        dst->prims[i].size = src->prims[i].size * ((sx + sy) / 2);
    }
}

int glyph_compose(GlyphSystem *gs, CompositionOp op, int left, int right, double spacing) {
    if (gs->glyph_count >= 64) return -1;
    if (left < 0 || left >= gs->glyph_count) return -1;

    Glyph *result = &gs->glyphs[gs->glyph_count];
    const Glyph *gl = &gs->glyphs[left];

    switch (op) {
        case COMP_STACK: {
            if (right < 0 || right >= gs->glyph_count) return -1;
            const Glyph *gr = &gs->glyphs[right];
            snprintf(result->name, GLYPH_MAX_NAME, "%s+%s(stacked)", gl->name, gr->name);
            result->semantic_invariant = (gl->semantic_invariant + gr->semantic_invariant) / 2.0;
            /* Left on top, right on bottom */
            transform_prims(result, gl, 0, -spacing/2, 1, 1);
            int base = result->prim_count;
            for (int i = 0; i < gr->prim_count && base + i < ADINKRA_MAX_PRIMITIVES; i++) {
                result->prims[base + i] = gr->prims[i];
                result->prims[base + i].cy += spacing/2;
            }
            result->prim_count = base + gr->prim_count;
            if (result->prim_count > ADINKRA_MAX_PRIMITIVES) result->prim_count = ADINKRA_MAX_PRIMITIVES;
            break;
        }
        case COMP_NEST: {
            if (right < 0 || right >= gs->glyph_count) return -1;
            const Glyph *gr = &gs->glyphs[right];
            snprintf(result->name, GLYPH_MAX_NAME, "%s(%s)", gl->name, gr->name);
            result->semantic_invariant = gl->semantic_invariant * gr->semantic_invariant;
            transform_prims(result, gl, 0, 0, 1, 1);
            int base = result->prim_count;
            for (int i = 0; i < gr->prim_count && base + i < ADINKRA_MAX_PRIMITIVES; i++) {
                result->prims[base + i] = gr->prims[i];
                result->prims[base + i].cx *= 0.5;
                result->prims[base + i].cy *= 0.5;
                result->prims[base + i].size *= 0.5;
            }
            result->prim_count = base + gr->prim_count;
            if (result->prim_count > ADINKRA_MAX_PRIMITIVES) result->prim_count = ADINKRA_MAX_PRIMITIVES;
            break;
        }
        case COMP_INTERLEAVE: {
            if (right < 0 || right >= gs->glyph_count) return -1;
            const Glyph *gr = &gs->glyphs[right];
            snprintf(result->name, GLYPH_MAX_NAME, "%s~%s", gl->name, gr->name);
            result->semantic_invariant = (gl->semantic_invariant + gr->semantic_invariant) / 2.0;
            int max_prims = gl->prim_count > gr->prim_count ? gl->prim_count : gr->prim_count;
            result->prim_count = 0;
            for (int i = 0; i < max_prims && result->prim_count < ADINKRA_MAX_PRIMITIVES - 1; i++) {
                if (i < gl->prim_count) result->prims[result->prim_count++] = gl->prims[i];
                if (i < gr->prim_count) {
                    result->prims[result->prim_count] = gr->prims[i];
                    result->prims[result->prim_count].cx += 3;
                    result->prim_count++;
                }
            }
            break;
        }
        case COMP_MIRROR: {
            snprintf(result->name, GLYPH_MAX_NAME, "%s(mirror)", gl->name);
            result->semantic_invariant = gl->semantic_invariant;
            transform_prims(result, gl, 0, 0, 1, 1);
            int base = result->prim_count;
            for (int i = 0; i < gl->prim_count && base + i < ADINKRA_MAX_PRIMITIVES; i++) {
                result->prims[base + i] = gl->prims[i];
                result->prims[base + i].cx = -gl->prims[i].cx;
            }
            result->prim_count = base + gl->prim_count;
            if (result->prim_count > ADINKRA_MAX_PRIMITIVES) result->prim_count = ADINKRA_MAX_PRIMITIVES;
            break;
        }
        case COMP_OVERLAY: {
            if (right < 0 || right >= gs->glyph_count) return -1;
            const Glyph *gr = &gs->glyphs[right];
            snprintf(result->name, GLYPH_MAX_NAME, "%s&%s", gl->name, gr->name);
            result->semantic_invariant = fmax(gl->semantic_invariant, gr->semantic_invariant);
            transform_prims(result, gl, 0, 0, 1, 1);
            int base = result->prim_count;
            for (int i = 0; i < gr->prim_count && base + i < ADINKRA_MAX_PRIMITIVES; i++) {
                result->prims[base + i] = gr->prims[i];
            }
            result->prim_count = base + gr->prim_count;
            if (result->prim_count > ADINKRA_MAX_PRIMITIVES) result->prim_count = ADINKRA_MAX_PRIMITIVES;
            break;
        }
        case COMP_SIDE_BY_SIDE: {
            if (right < 0 || right >= gs->glyph_count) return -1;
            const Glyph *gr = &gs->glyphs[right];
            snprintf(result->name, GLYPH_MAX_NAME, "%s|%s", gl->name, gr->name);
            result->semantic_invariant = (gl->semantic_invariant + gr->semantic_invariant) / 2.0;
            transform_prims(result, gl, -spacing/2, 0, 0.7, 0.7);
            int base = result->prim_count;
            for (int i = 0; i < gr->prim_count && base + i < ADINKRA_MAX_PRIMITIVES; i++) {
                result->prims[base + i] = gr->prims[i];
                result->prims[base + i].cx = gr->prims[i].cx * 0.7 + spacing/2;
                result->prims[base + i].cy *= 0.7;
                result->prims[base + i].size *= 0.7;
            }
            result->prim_count = base + gr->prim_count;
            if (result->prim_count > ADINKRA_MAX_PRIMITIVES) result->prim_count = ADINKRA_MAX_PRIMITIVES;
            break;
        }
    }

    /* Record composition rule */
    if (gs->rule_count < GLYPH_MAX_COMPOSITIONS) {
        CompositionRule *r = &gs->rules[gs->rule_count];
        snprintf(r->name, GLYPH_MAX_NAME, "%s", result->name);
        r->op = op;
        r->left_index = left;
        r->right_index = right;
        r->spacing = spacing;
        gs->rule_count++;
    }

    return gs->glyph_count++;
}

int glyph_verify_invariant(const GlyphSystem *gs, int composed_index) {
    if (composed_index < 0 || composed_index >= gs->glyph_count) return 0;
    /* Semantic invariant should be between 0 and 1 */
    double inv = gs->glyphs[composed_index].semantic_invariant;
    return inv >= 0.0 && inv <= 1.0;
}

Glyph *glyph_get(GlyphSystem *gs, int index) {
    if (index < 0 || index >= gs->glyph_count) return NULL;
    return &gs->glyphs[index];
}

char *glyph_to_svg(Arena *a, const Glyph *g, double width, double height) {
    size_t bufsize = 8192;
    char *svg = (char *)arena_alloc(a, bufsize);
    if (!svg) return NULL;
    int pos = 0;
    double ox = width / 2, oy = height / 2;
    pos += snprintf(svg + pos, bufsize - pos,
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%.0f\" height=\"%.0f\">\n"
        " <title>%s</title>\n", width, height, g->name);

    char prim_buf[1024];
    for (int i = 0; i < g->prim_count; i++) {
        adinkra_prim_to_svg(&g->prims[i], prim_buf, sizeof(prim_buf), ox, oy);
        pos += snprintf(svg + pos, bufsize - pos, "%s", prim_buf);
    }
    pos += snprintf(svg + pos, bufsize - pos, "</svg>\n");
    return svg;
}
