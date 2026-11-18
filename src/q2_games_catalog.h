/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef Q2_GAMES_CATALOG_H
#define Q2_GAMES_CATALOG_H
struct q2_game {
    const char *title;
    const char *path;
};
extern const struct q2_game q2_games_catalog[];
extern const unsigned q2_games_catalog_count;
/* Returns an index into q2_games_catalog or -1 for an invalid selection. */
int q2_games_visible_index(unsigned selection,
                           int (*is_installed)(const char *path, void *ctx),
                           void *ctx);
unsigned q2_games_visible_count(int (*is_installed)(const char *, void *),
                                void *ctx);
#endif
