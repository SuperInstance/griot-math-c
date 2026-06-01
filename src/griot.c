#define _POSIX_C_SOURCE 200809L
#include "griot.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

Griot griot_create(size_t capacity, double decay_rate) {
    Griot g;
    g.stories = (Story *)calloc(capacity, sizeof(Story));
    g.story_count = 0;
    g.capacity = capacity;
    g.decay_rate = decay_rate;
    g.current_time = 0;
    return g;
}

void griot_destroy(Griot *g) {
    if (g->stories) {
        for (size_t i = 0; i < g->story_count; i++) {
            free(g->stories[i].name);
        }
        free(g->stories);
        g->stories = NULL;
        g->story_count = 0;
    }
}

int griot_add_story(Griot *g, const char *name, double weight, int parent_id) {
    if (g->story_count >= g->capacity) return -1;
    Story *s = &g->stories[g->story_count];
    s->name = strdup(name);
    s->weight = weight;
    s->tell_count = 0;
    s->last_told = g->current_time;
    s->parent_id = parent_id;
    s->genealogy_depth = (parent_id >= 0 && parent_id < (int)g->story_count) 
        ? g->stories[parent_id].genealogy_depth + 1 : 0;
    g->story_count++;
    return (int)(g->story_count - 1);
}

Story *griot_find_story(Griot *g, const char *name) {
    for (size_t i = 0; i < g->story_count; i++) {
        if (strcmp(g->stories[i].name, name) == 0) return &g->stories[i];
    }
    return NULL;
}

int griot_tell_story(Griot *g, const char *name) {
    Story *s = griot_find_story(g, name);
    if (!s) return -1;
    s->tell_count++;
    s->last_told = g->current_time;
    /* Retelling boosts weight */
    s->weight = fmin(s->weight * 1.1, 10.0);
    return s->tell_count;
}

void griot_apply_decay(Griot *g, double elapsed) {
    g->current_time += elapsed;
    for (size_t i = 0; i < g->story_count; i++) {
        double dt = g->current_time - g->stories[i].last_told;
        g->stories[i].weight *= exp(-g->decay_rate * dt);
        if (g->stories[i].weight < 0.001) g->stories[i].weight = 0.001;
    }
}

double griot_tradition_score(const Griot *g) {
    if (g->story_count == 0) return 0.0;
    double total = 0;
    int roots = 0;
    for (size_t i = 0; i < g->story_count; i++) {
        if (g->stories[i].parent_id < 0) {
            total += g->stories[i].weight;
            roots++;
        }
    }
    return roots > 0 ? total / roots / 10.0 : 0.0;
}

double *griot_memory_strengths(const Griot *g, size_t *out_count) {
    *out_count = g->story_count;
    double *strengths = (double *)malloc(g->story_count * sizeof(double));
    for (size_t i = 0; i < g->story_count; i++) {
        strengths[i] = g->stories[i].weight * (1 + g->stories[i].tell_count * 0.1);
    }
    return strengths;
}

PraiseName griot_generate_praise(const Griot *g, const int *story_ids,
                                  size_t id_count, const char *name) {
    PraiseName pn;
    pn.story_ids = (int *)malloc(id_count * sizeof(int));
    memcpy(pn.story_ids, story_ids, id_count * sizeof(int));
    pn.id_count = id_count;
    pn.name = strdup(name);
    
    double total_weight = 0;
    for (size_t i = 0; i < id_count && i < g->story_count; i++) {
        if (story_ids[i] >= 0 && story_ids[i] < (int)g->story_count) {
            total_weight += g->stories[story_ids[i]].weight;
        }
    }
    pn.density = id_count > 0 ? total_weight / id_count : 0;
    pn.compression_ratio = id_count > 0 ? (double)g->story_count / id_count : 0;
    return pn;
}

void praise_name_destroy(PraiseName *pn) {
    free(pn->story_ids);
    free(pn->name);
    pn->story_ids = NULL;
    pn->name = NULL;
}

CallResponse griot_call_response(const Griot *caller, const Griot *responder,
                                  const char *story_name) {
    CallResponse cr = {-1, -1, 0.0};
    Story *cs = griot_find_story((Griot *)caller, story_name);
    if (!cs) return cr;
    cr.caller_story = (int)(cs - caller->stories);
    
    /* Find best matching story in responder by name similarity (exact match first) */
    Story *rs = griot_find_story((Griot *)responder, story_name);
    if (rs) {
        cr.responder_story = (int)(rs - responder->stories);
        cr.similarity = 1.0;
    } else if (responder->story_count > 0) {
        /* Pick strongest story as fallback */
        int best = 0;
        for (size_t i = 1; i < responder->story_count; i++) {
            if (responder->stories[i].weight > responder->stories[best].weight) best = i;
        }
        cr.responder_story = best;
        cr.similarity = 0.5;
    }
    return cr;
}

int *griot_genealogy(const Griot *g, int story_id, size_t *path_len) {
    if (story_id < 0 || story_id >= (int)g->story_count) {
        *path_len = 0;
        return NULL;
    }
    /* Walk up parent chain */
    int path[256];
    size_t len = 0;
    int current = story_id;
    while (current >= 0 && len < 256) {
        path[len++] = current;
        current = g->stories[current].parent_id;
    }
    /* Reverse and copy */
    int *result = (int *)malloc(len * sizeof(int));
    for (size_t i = 0; i < len; i++) {
        result[i] = path[len - 1 - i];
    }
    *path_len = len;
    return result;
}

int *griot_descendants(const Griot *g, int story_id, size_t *desc_count) {
    size_t count = 0;
    int *descs = (int *)malloc(g->story_count * sizeof(int));
    for (size_t i = 0; i < g->story_count; i++) {
        /* Check if story_id is in ancestor chain */
        int current = (int)i;
        while (current >= 0) {
            if (current == story_id && (int)i != story_id) {
                descs[count++] = (int)i;
                break;
            }
            current = g->stories[current].parent_id;
        }
    }
    *desc_count = count;
    return descs;
}

Federation federation_create(size_t griot_count, double decay_rate) {
    Federation f;
    f.griots = (Griot *)calloc(griot_count, sizeof(Griot));
    f.griot_count = griot_count;
    f.sync_matrix = (int *)calloc(griot_count * griot_count, sizeof(int));
    for (size_t i = 0; i < griot_count; i++) {
        f.griots[i] = griot_create(64, decay_rate);
        f.sync_matrix[i * griot_count + i] = 1; /* self-sync */
    }
    return f;
}

void federation_destroy(Federation *f) {
    for (size_t i = 0; i < f->griot_count; i++) {
        griot_destroy(&f->griots[i]);
    }
    free(f->griots);
    free(f->sync_matrix);
    f->griots = NULL;
    f->sync_matrix = NULL;
}

int federation_sync_story(Federation *f, size_t from, size_t to,
                          const char *story_name) {
    if (from >= f->griot_count || to >= f->griot_count) return -1;
    Story *s = griot_find_story(&f->griots[from], story_name);
    if (!s) return -1;
    griot_add_story(&f->griots[to], s->name, s->weight, -1);
    f->sync_matrix[from * f->griot_count + to] = 1;
    return 0;
}

double federation_coverage(const Federation *f) {
    if (f->griot_count <= 1) return 1.0;
    int total = 0;
    int synced = 0;
    for (size_t i = 0; i < f->griot_count; i++) {
        for (size_t j = 0; j < f->griot_count; j++) {
            if (i != j) {
                total++;
                if (f->sync_matrix[i * f->griot_count + j]) synced++;
            }
        }
    }
    return total > 0 ? (double)synced / total : 0.0;
}
