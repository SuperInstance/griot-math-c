# griot-math-c

> Living memory mathematics in C — West African griot oral tradition as exponential decay, genealogy, and federated memory.

## What This Does

`griot-math-c` implements griot memory mathematics in C. Stories have importance weights that decay exponentially but are boosted by retelling. Track story genealogy, generate praise names, perform call-and-response matching, and federate memories across multiple griots. Use it for embedded caching with cultural decay, knowledge management, or distributed memory in C/C++ systems.

## The Cultural Root

See `griot-math` (PyPI/npm) for the full cultural background. Griots maintain oral histories — stories told often resist decay.

## Install

```bash
git clone https://github.com/SuperInstance/griot-math-c.git
cd griot-math-c
make
```

## Quick Start

```c
#include "griot.h"

int main() {
    Griot *g = griot_create();

    int s1 = griot_add_story(g, "The founding", 1.0, -1);
    int s2 = griot_add_story(g, "The flood", 0.8, s1);
    int s3 = griot_add_story(g, "Rebuilding", 0.9, s2);

    // Tell stories — boosts weight
    griot_tell_story(g, "The founding");
    griot_tell_story(g, "The founding");

    // Apply decay (1 hour)
    griot_apply_decay(g, 3600.0);

    // Tradition score
    double score = griot_tradition_score(g);
    printf("Tradition: %.3f\n", score);

    // Genealogy
    size_t path_len;
    int *path = griot_genealogy(g, s3, &path_len);
    // path = [s1, s2, s3]

    size_t desc_count;
    int *desc = griot_descendants(g, s1, &desc_count);

    // Memory strengths
    size_t n;
    double *strengths = griot_memory_strengths(g, &n);

    // Federation
    Federation *fed = federation_create();
    federation_add_griot(fed, g);
    Griot *g2 = griot_create();
    federation_add_griot(fed, g2);
    federation_sync_story(fed, 0, 1, "The flood");
    double coverage = federation_coverage(fed);

    federation_destroy(fed);
    return 0;
}
```

## API Reference

### Griot
- `Griot *griot_create()` / `void griot_destroy(Griot *g)`
- `int griot_add_story(Griot *g, const char *name, double weight, int parent_id)`
- `int griot_tell_story(Griot *g, const char *name)`
- `void griot_apply_decay(Griot *g, double elapsed)`
- `double griot_tradition_score(const Griot *g)`
- `double *griot_memory_strengths(const Griot *g, size_t *out_count)`

### Genealogy
- `int *griot_genealogy(const Griot *g, int story_id, size_t *path_len)`
- `int *griot_descendants(const Griot *g, int story_id, size_t *desc_count)`

### Praise Names
- `PraiseName *generate_praise_name(const Griot *g, const int *story_ids, size_t n, const char *name)`
- `void praise_name_destroy(PraiseName *pn)`

### Federation
- `Federation *federation_create()` / `void federation_destroy(Federation *f)`
- `void federation_add_griot(Federation *f, Griot *g)`
- `int federation_sync_story(Federation *f, size_t from, size_t to, const char *story_name)`
- `double federation_coverage(const Federation *f)`

## License

MIT
