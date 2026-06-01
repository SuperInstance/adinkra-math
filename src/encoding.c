#include "../include/encoding.h"
#include <string.h>
#include <math.h>
#include <float.h>

void concept_space_init(ConceptSpace *cs, int dimensions) {
    memset(cs, 0, sizeof(*cs));
    cs->dimensions = dimensions > ENCODING_MAX_DIM ? ENCODING_MAX_DIM : dimensions;
}

int concept_encode(ConceptSpace *cs, const char *name,
                   const double *features, int dim, double weight) {
    if (cs->count >= ENCODING_MAX_CONCEPTS) return -1;
    if (dim > ENCODING_MAX_DIM) return -1;
    EncodedConcept *c = &cs->concepts[cs->count];
    strncpy(c->name, name, ENCODING_MAX_NAME - 1);
    memcpy(c->vector, features, sizeof(double) * dim);
    c->dim = dim;
    c->weight = weight;
    return cs->count++;
}

double concept_distance(const EncodedConcept *a, const EncodedConcept *b) {
    int dim = a->dim < b->dim ? a->dim : b->dim;
    double sum = 0;
    for (int i = 0; i < dim; i++) {
        double d = a->vector[i] - b->vector[i];
        sum += d * d;
    }
    return sqrt(sum);
}

double concept_similarity(const EncodedConcept *a, const EncodedConcept *b) {
    double dist = concept_distance(a, b);
    /* Normalize: max possible distance with dim dimensions in [0,1] range is sqrt(dim) */
    int dim = a->dim < b->dim ? a->dim : b->dim;
    double max_dist = sqrt((double)dim);
    if (max_dist < 1e-9) return 1.0;
    return 1.0 - dist / max_dist;
}

int concept_nearest(const ConceptSpace *cs, const double *query, int dim) {
    int best = -1;
    double best_dist = DBL_MAX;
    EncodedConcept tmp;
    memcpy(tmp.vector, query, sizeof(double) * dim);
    tmp.dim = dim;
    for (int i = 0; i < cs->count; i++) {
        double d = concept_distance(&tmp, &cs->concepts[i]);
        if (d < best_dist) {
            best_dist = d;
            best = i;
        }
    }
    return best;
}

void concept_k_nearest(const ConceptSpace *cs, const double *query, int dim,
                       int *results, int k) {
    /* Simple O(n*k) approach */
    double distances[ENCODING_MAX_CONCEPTS];
    int used[ENCODING_MAX_CONCEPTS];
    memset(used, 0, sizeof(int) * cs->count);
    EncodedConcept tmp;
    memcpy(tmp.vector, query, sizeof(double) * dim);
    tmp.dim = dim;
    for (int i = 0; i < cs->count; i++) {
        distances[i] = concept_distance(&tmp, &cs->concepts[i]);
    }
    for (int ki = 0; ki < k; ki++) {
        int best = -1;
        double best_d = DBL_MAX;
        for (int i = 0; i < cs->count; i++) {
            if (!used[i] && distances[i] < best_d) {
                best_d = distances[i];
                best = i;
            }
        }
        if (best >= 0) {
            results[ki] = best;
            used[best] = 1;
        } else {
            results[ki] = -1;
        }
    }
}

void concept_cluster(const ConceptSpace *cs, ConceptClusters *out, int k, int max_iter) {
    if (k > CLUSTER_MAX) k = CLUSTER_MAX;
    out->k = k;
    out->iterations = 0;

    /* Initialize centroids to first k concepts */
    for (int i = 0; i < k && i < cs->count; i++) {
        memcpy(out->centroids[i], cs->concepts[i].vector, sizeof(double) * cs->dimensions);
    }

    for (int iter = 0; iter < max_iter; iter++) {
        out->iterations++;
        /* Assign each concept to nearest centroid */
        int changed = 0;
        for (int i = 0; i < cs->count; i++) {
            EncodedConcept tmp;
            tmp.dim = cs->dimensions;
            double best_d = DBL_MAX;
            int best_c = 0;
            for (int c = 0; c < k; c++) {
                memcpy(tmp.vector, out->centroids[c], sizeof(double) * cs->dimensions);
                double d = concept_distance(&cs->concepts[i], &tmp);
                if (d < best_d) { best_d = d; best_c = c; }
            }
            if (out->assignments[i] != best_c) {
                out->assignments[i] = best_c;
                changed = 1;
            }
        }
        if (!changed) break;

        /* Recompute centroids */
        for (int c = 0; c < k; c++) {
            double sum[ENCODING_MAX_DIM] = {0};
            int count = 0;
            for (int i = 0; i < cs->count; i++) {
                if (out->assignments[i] == c) {
                    for (int d = 0; d < cs->dimensions; d++)
                        sum[d] += cs->concepts[i].vector[d];
                    count++;
                }
            }
            if (count > 0) {
                for (int d = 0; d < cs->dimensions; d++)
                    out->centroids[c][d] = sum[d] / count;
            }
        }
    }
}
