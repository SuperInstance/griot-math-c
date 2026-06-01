#ifndef GRIOT_MATH_H
#define GRIOT_MATH_H

#include <stddef.h>

/* A story in the griot's memory */
typedef struct {
    char *name;         /* Story identifier */
    double weight;      /* Current weight (decays over time) */
    int tell_count;     /* How many times told */
    double last_told;   /* Timestamp of last telling */
    int parent_id;      /* Parent story (-1 = root) */
    int genealogy_depth;/* Depth in genealogy tree */
} Story;

/* The griot — a living memory keeper */
typedef struct {
    Story *stories;
    size_t story_count;
    size_t capacity;
    double decay_rate;  /* Exponential decay rate */
    double current_time;
} Griot;

/* Praise name — compressed semantic representation */
typedef struct {
    int *story_ids;
    size_t id_count;
    char *name;
    double compression_ratio;
    double density;
} PraiseName;

/* Call and response between two griots */
typedef struct {
    int caller_story;
    int responder_story;
    double similarity;
} CallResponse;

/* Federation of griots */
typedef struct {
    Griot *griots;
    size_t griot_count;
    int *sync_matrix;   /* n×n sync status */
} Federation;

/* Griot lifecycle */
Griot griot_create(size_t capacity, double decay_rate);
void griot_destroy(Griot *g);
int griot_add_story(Griot *g, const char *name, double weight, int parent_id);
Story *griot_find_story(Griot *g, const char *name);
int griot_tell_story(Griot *g, const char *name);
void griot_apply_decay(Griot *g, double elapsed);
double griot_tradition_score(const Griot *g);
double *griot_memory_strengths(const Griot *g, size_t *out_count);

/* Praise names */
PraiseName griot_generate_praise(const Griot *g, const int *story_ids, 
                                  size_t id_count, const char *name);
void praise_name_destroy(PraiseName *pn);

/* Call and response */
CallResponse griot_call_response(const Griot *caller, const Griot *responder,
                                  const char *story_name);

/* Genealogy */
int *griot_genealogy(const Griot *g, int story_id, size_t *path_len);
int *griot_descendants(const Griot *g, int story_id, size_t *desc_count);

/* Federation */
Federation federation_create(size_t griot_count, double decay_rate);
void federation_destroy(Federation *f);
int federation_sync_story(Federation *f, size_t from, size_t to, 
                          const char *story_name);
double federation_coverage(const Federation *f);
#endif
