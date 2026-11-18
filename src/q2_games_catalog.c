/* SPDX-License-Identifier: GPL-3.0-or-later
 * Rockbox's existing Q2 fork deliberately omits several incompatible titles.
 * List only its plausible compiled-in games here; actual installed files
 * are checked at runtime, so an absent plugin never appears.
 */
#include "q2_games_catalog.h"
#define G "/.rockbox/rocks/games/"
const struct q2_game q2_games_catalog[] = {
    {"Pong",         G "pong.rock"},
    {"Spacerocks",   G "spacerocks.rock"},
    {"Chopper",      G "chopper.rock"},
    {"BrickMania",   G "brickmania.rock"},
    {"Minesweeper",  G "minesweeper.rock"},
    {"Sokoban",      G "sokoban.rock"},
    {"Maze",         G "maze.rock"},
    {"Mazezam",      G "mazezam.rock"},
    {"Blackjack",    G "blackjack.rock"},
    {"Codebuster",   G "codebuster.rock"},
    {"Pegbox",       G "pegbox.rock"},
    {"Snake 2",      G "snake2.rock"},
    {"Sliding Puzzle",G "sliding_puzzle.rock"},
    {"Solitaire",    G "solitaire.rock"},
};
const unsigned q2_games_catalog_count =
    sizeof(q2_games_catalog) / sizeof(q2_games_catalog[0]);

int q2_games_visible_index(unsigned selection,
                           int (*is_installed)(const char *, void *),
                           void *ctx)
{
    unsigned visible = 0;
    if (!is_installed)
        return -1;
    for (unsigned i = 0; i < q2_games_catalog_count; ++i) {
        if (!is_installed(q2_games_catalog[i].path, ctx))
            continue;
        if (visible++ == selection)
            return (int)i;
    }
    return -1;
}
unsigned q2_games_visible_count(int (*is_installed)(const char *, void *),
                                void *ctx)
{
    unsigned count = 0;
    if (!is_installed)
        return 0;
    for (unsigned i = 0; i < q2_games_catalog_count; ++i)
        if (is_installed(q2_games_catalog[i].path, ctx))
            ++count;
    return count;
}
