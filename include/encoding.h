#ifndef ENCODING_H
#define ENCODING_H

#include "adinkra.h"

/* Concept encoding: map abstract concepts to geometric features */
#define ENCODING_MAX_CONCEPTS 256
#define ENCODING_MAX_DIM 8
#define ENCODING_MAX_NAME 128

typedef struct {
    char name[ENCODING_MAX_NAME];
    double vector[ENCODING_MAX_DIM]; /* geometric feature vector */
    int dim;
    double weight;
} EncodedConcept;

typedef struct {
    EncodedConcept concepts[ENCODING_MAX_CONCEPTS];
    int count;
    int dimensions;
} ConceptSpace;

void concept_space_init(ConceptSpace *cs, int dimensions);
int concept_encode(ConceptSpace *cs, const char *name,
                   const double *features, int dim, double weight);

/* Geometric distance between concepts */
double concept_distance(const EncodedConcept *a, const EncodedConcept *b);

/* Similarity (1 - normalized distance) */
double concept_similarity(const EncodedConcept *a, const EncodedConcept *b);

/* Nearest concept lookup */
int concept_nearest(const ConceptSpace *cs, const double *query, int dim);

/* K-nearest neighbors */
void concept_k_nearest(const ConceptSpace *cs, const double *query, int dim,
                       int *results, int k);

/* Clustering via geometric proximity (simple k-means) */
#define CLUSTER_MAX 16
typedef struct {
    double centroids[CLUSTER_MAX][ENCODING_MAX_DIM];
    int assignments[ENCODING_MAX_CONCEPTS];
    int k;
    int iterations;
} ConceptClusters;

void concept_cluster(const ConceptSpace *cs, ConceptClusters *out, int k, int max_iter);

#endif
