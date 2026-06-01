# adinkra-math

A C99 library implementing the mathematics of symbolic encoding — representing abstract concepts as geometric patterns with provable properties.

Bridges two meanings of "adinkra":
- **West African (Akan) visual symbols** encoding philosophical concepts
- **Physics adinkras** — graphical tools for studying representations of supersymmetry algebras

## Modules

| Header | Description |
|--------|-------------|
| `adinkra.h` | Geometric primitives (circles, spirals, crosses, knots) → symbolic patterns with SVG output |
| `supersymmetry.h` | SUSY adinkras with rank-4/rank-8 chromotopology and representation verification |
| `glyph.h` | Glyph composition grammar: stack, nest, interleave, mirror with semantic invariant preservation |
| `encoding.h` | Concept → geometric vector encoding, nearest-concept lookup, k-means clustering |
| `topology.h` | Topological analysis: connected components, genus, Euler characteristic, equivalence |
| `adinkra_api.h` | Unified `AdinkraSystem` combining all modules |

## Build

```bash
make          # build library + example
make test     # run 55 tests
make example  # build and run example
make clean    # clean artifacts
```

## Design

- **C99**, no external dependencies
- **Arena allocator** — single allocation, single free
- All floating point uses `double`
- SVG output generation for all visual elements
- 55 tests using assert macros

## Usage

```c
#include "adinkra_api.h"

AdinkraSystem sys;
adinkra_system_init(&sys);
adinkra_system_load_defaults(&sys);

// Access symbols, SUSY, glyphs, concepts, topology...
char *svg = adinkra_system_svg(&sys, 800, 600);

adinkra_system_destroy(&sys); // single free, no leaks
```
