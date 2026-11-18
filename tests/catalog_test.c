#include "q2_games_catalog.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static int all(const char *path, void *ctx) {(void)path;(void)ctx;return 1;}
static int none(const char *path, void *ctx) {(void)path;(void)ctx;return 0;}
static int two(const char *path, void *ctx) {
    (void)ctx;
    return strstr(path, "/pong.rock") || strstr(path, "/spacerocks.rock");
}
int main(void) {
    assert(q2_games_catalog_count >= 10);
    assert(q2_games_visible_count(all, NULL) == q2_games_catalog_count);
    assert(q2_games_visible_count(none, NULL) == 0);
    assert(q2_games_visible_count(two, NULL) == 2);
    assert(q2_games_visible_index(0, two, NULL) == 0);
    assert(q2_games_visible_index(1, two, NULL) == 1);
    assert(q2_games_visible_index(2, two, NULL) == -1);
    assert(q2_games_visible_index(0, NULL, NULL) == -1);
    assert(q2_games_visible_count(NULL, NULL) == 0);
    for (unsigned i=0; i<q2_games_catalog_count; ++i)
        for (unsigned j=i+1; j<q2_games_catalog_count; ++j)
            assert(strcmp(q2_games_catalog[i].path,q2_games_catalog[j].path));
    puts("Catalog tests passed");
    return 0;
}
