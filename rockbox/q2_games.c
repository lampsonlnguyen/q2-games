/* SPDX-License-Identifier: GPL-3.0-or-later
 * Q2 Games: menu plugin running within the *real* Rockbox Q2 process.
 * No custom ELF loader; handoff uses Rockbox's documented plugin API.
 */
#include "plugin.h"
#include "q2_games_catalog.h"
/* A single .rock plugin object includes the portable catalog source. */
#include "q2_games_catalog.inc"

#define LEFT 20
#define TOP 48
#define ROW_H 30
#define ROWS 7
static int present(const char *path, void *ctx)
{
    int fd;
    (void)ctx;
    fd = rb->open(path, O_RDONLY);
    if (fd < 0)
        return 0;
    rb->close(fd);
    return 1;
}

static void paint(unsigned selection, unsigned count)
{
    unsigned start = selection >= ROWS ? selection - ROWS + 1 : 0;
    rb->lcd_set_background(LCD_RGBPACK(15, 24, 38));
    rb->lcd_set_foreground(LCD_RGBPACK(238, 237, 229));
    rb->lcd_clear_display();
    rb->lcd_putsxy(LEFT, 22, "Q2 GAMES / ROCKBOX");
    for (unsigned row = 0; row < ROWS && start + row < count; ++row) {
        unsigned slot = start + row;
        int idx = q2_games_visible_index(slot, present, NULL);
        int y = TOP + (int)row * ROW_H;
        if (idx < 0)
            break;
        if (slot == selection) {
            rb->lcd_set_foreground(LCD_RGBPACK(66, 125, 174));
            rb->lcd_fillrect(LEFT - 6, y - 4, LCD_WIDTH - 2 * LEFT + 12,
                             ROW_H - 2);
        }
        rb->lcd_set_foreground(LCD_RGBPACK(255, 255, 255));
        rb->lcd_putsxy(LEFT, y, q2_games_catalog[idx].title);
    }
    rb->lcd_set_foreground(LCD_RGBPACK(170, 183, 197));
    rb->lcd_putsxy(LEFT, LCD_HEIGHT - 28, "WHEEL SELECT  CENTER PLAY");
    rb->lcd_update();
}

enum plugin_status plugin_start(const void *parameter)
{
    unsigned selection = 0;
    unsigned count;
    int action;
    (void)parameter;
    count = q2_games_visible_count(present, NULL);
    if (count == 0) {
        rb->splash(HZ * 2, "No games installed");
        return PLUGIN_OK;
    }
    for (;;) {
        paint(selection, count);
        action = rb->get_action(CONTEXT_STD, HZ);
        switch (action) {
            case ACTION_STD_PREV:
            case ACTION_STD_PREVREPEAT:
                if (selection)
                    --selection;
                break;
            case ACTION_STD_NEXT:
            case ACTION_STD_NEXTREPEAT:
                if (selection + 1 < count)
                    ++selection;
                break;
            case ACTION_STD_OK: {
                int idx = q2_games_visible_index(selection, present, NULL);
                if (idx >= 0) {
                    /* Rockbox schedules the requested plugin on return. */
                    rb->plugin_open(q2_games_catalog[idx].path, NULL);
                    return PLUGIN_GOTO_PLUGIN;
                }
                break;
            }
            case ACTION_STD_CANCEL:
                return PLUGIN_OK;
            default:
                if (rb->default_event_handler(action) == SYS_USB_CONNECTED)
                    return PLUGIN_USB_CONNECTED;
                break;
        }
    }
}
