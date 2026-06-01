#define _POSIX_C_SOURCE 200809L
#include "../src/griot.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#define ASSERT_FEQ(a, b) assert(fabs((a) - (b)) < 1e-4)

void test_create_destroy() {
    Griot g = griot_create(10, 0.1);
    assert(g.story_count == 0);
    assert(g.capacity == 10);
    griot_destroy(&g);
    printf("  PASS: create_destroy\n");
}

void test_add_find_story() {
    Griot g = griot_create(10, 0.1);
    int id = griot_add_story(&g, "origin", 5.0, -1);
    assert(id == 0);
    assert(g.story_count == 1);
    Story *s = griot_find_story(&g, "origin");
    assert(s != NULL);
    ASSERT_FEQ(s->weight, 5.0);
    griot_destroy(&g);
    printf("  PASS: add_find_story\n");
}

void test_tell_story() {
    Griot g = griot_create(10, 0.1);
    griot_add_story(&g, "tale", 3.0, -1);
    int count = griot_tell_story(&g, "tale");
    assert(count == 1);
    count = griot_tell_story(&g, "tale");
    assert(count == 2);
    Story *s = griot_find_story(&g, "tale");
    assert(s->weight > 3.0); /* Boosted by retelling */
    griot_destroy(&g);
    printf("  PASS: tell_story\n");
}

void test_decay() {
    Griot g = griot_create(10, 0.5);
    griot_add_story(&g, "old", 10.0, -1);
    griot_apply_decay(&g, 2.0);
    Story *s = griot_find_story(&g, "old");
    assert(s->weight < 10.0); /* Decayed */
    assert(s->weight > 0.0);
    griot_destroy(&g);
    printf("  PASS: decay\n");
}

void test_tradition_score() {
    Griot g = griot_create(10, 0.0);
    griot_add_story(&g, "root1", 8.0, -1);
    griot_add_story(&g, "root2", 4.0, -1);
    griot_add_story(&g, "child", 3.0, 0);
    double score = griot_tradition_score(&g);
    assert(score > 0);
    griot_destroy(&g);
    printf("  PASS: tradition_score\n");
}

void test_memory_strengths() {
    Griot g = griot_create(10, 0.0);
    griot_add_story(&g, "a", 5.0, -1);
    griot_tell_story(&g, "a"); /* tell_count = 1 */
    size_t count;
    double *strengths = griot_memory_strengths(&g, &count);
    assert(count == 1);
    assert(strengths[0] > 5.0); /* Boosted by tell count */
    free(strengths);
    griot_destroy(&g);
    printf("  PASS: memory_strengths\n");
}

void test_genealogy() {
    Griot g = griot_create(10, 0.0);
    griot_add_story(&g, "grandparent", 5.0, -1);  /* id=0 */
    griot_add_story(&g, "parent", 4.0, 0);         /* id=1 */
    griot_add_story(&g, "child", 3.0, 1);           /* id=2 */
    
    size_t path_len;
    int *path = griot_genealogy(&g, 2, &path_len);
    assert(path_len == 3);
    assert(path[0] == 0); /* grandparent */
    assert(path[1] == 1); /* parent */
    assert(path[2] == 2); /* child */
    free(path);
    griot_destroy(&g);
    printf("  PASS: genealogy\n");
}

void test_descendants() {
    Griot g = griot_create(10, 0.0);
    griot_add_story(&g, "root", 5.0, -1);    /* id=0 */
    griot_add_story(&g, "child1", 4.0, 0);   /* id=1 */
    griot_add_story(&g, "child2", 3.0, 0);   /* id=2 */
    griot_add_story(&g, "grandchild", 2.0, 1);/* id=3 */
    
    size_t desc_count;
    int *descs = griot_descendants(&g, 0, &desc_count);
    assert(desc_count == 3); /* child1, child2, grandchild */
    free(descs);
    griot_destroy(&g);
    printf("  PASS: descendants\n");
}

void test_praise_name() {
    Griot g = griot_create(10, 0.0);
    griot_add_story(&g, "a", 5.0, -1);
    griot_add_story(&g, "b", 4.0, -1);
    
    int ids[] = {0, 1};
    PraiseName pn = griot_generate_praise(&g, ids, 2, "The Great Tradition");
    assert(pn.id_count == 2);
    assert(pn.compression_ratio == 1.0);
    assert(pn.density > 0);
    praise_name_destroy(&pn);
    griot_destroy(&g);
    printf("  PASS: praise_name\n");
}

void test_call_response() {
    Griot g1 = griot_create(10, 0.0);
    Griot g2 = griot_create(10, 0.0);
    griot_add_story(&g1, "shared_tale", 5.0, -1);
    griot_add_story(&g2, "shared_tale", 4.0, -1);
    
    CallResponse cr = griot_call_response(&g1, &g2, "shared_tale");
    assert(cr.caller_story == 0);
    assert(cr.responder_story == 0);
    ASSERT_FEQ(cr.similarity, 1.0);
    
    griot_destroy(&g1);
    griot_destroy(&g2);
    printf("  PASS: call_response\n");
}

void test_call_response_fallback() {
    Griot g1 = griot_create(10, 0.0);
    Griot g2 = griot_create(10, 0.0);
    griot_add_story(&g1, "unique_tale", 5.0, -1);
    griot_add_story(&g2, "other_tale", 4.0, -1);
    
    CallResponse cr = griot_call_response(&g1, &g2, "unique_tale");
    assert(cr.caller_story == 0);
    ASSERT_FEQ(cr.similarity, 0.5);
    
    griot_destroy(&g1);
    griot_destroy(&g2);
    printf("  PASS: call_response_fallback\n");
}

void test_federation() {
    Federation f = federation_create(3, 0.0);
    griot_add_story(&f.griots[0], "origin", 5.0, -1);
    federation_sync_story(&f, 0, 1, "origin");
    
    assert(f.griots[1].story_count == 1);
    double coverage = federation_coverage(&f);
    assert(coverage > 0);
    
    federation_destroy(&f);
    printf("  PASS: federation\n");
}

void test_federation_full_sync() {
    Federation f = federation_create(2, 0.0);
    griot_add_story(&f.griots[0], "tale", 5.0, -1);
    griot_add_story(&f.griots[1], "saga", 4.0, -1);
    federation_sync_story(&f, 0, 1, "tale");
    federation_sync_story(&f, 1, 0, "saga");
    double coverage = federation_coverage(&f);
    ASSERT_FEQ(coverage, 1.0);
    federation_destroy(&f);
    printf("  PASS: federation_full_sync\n");
}

void test_empty_tradition() {
    Griot g = griot_create(10, 0.0);
    double score = griot_tradition_score(&g);
    ASSERT_FEQ(score, 0.0);
    griot_destroy(&g);
    printf("  PASS: empty_tradition\n");
}

void test_tell_nonexistent() {
    Griot g = griot_create(10, 0.0);
    int result = griot_tell_story(&g, "ghost");
    assert(result == -1);
    griot_destroy(&g);
    printf("  PASS: tell_nonexistent\n");
}

int main() {
    printf("Running griot-math-c tests:\n");
    test_create_destroy();
    test_add_find_story();
    test_tell_story();
    test_decay();
    test_tradition_score();
    test_memory_strengths();
    test_genealogy();
    test_descendants();
    test_praise_name();
    test_call_response();
    test_call_response_fallback();
    test_federation();
    test_federation_full_sync();
    test_empty_tradition();
    test_tell_nonexistent();
    printf("\n15 passed, 0 failed\n");
    return 0;
}
