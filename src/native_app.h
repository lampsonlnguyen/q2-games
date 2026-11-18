/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef Q2_NATIVE_APP_H
#define Q2_NATIVE_APP_H
#include <stdint.h>
enum { SCREEN_W=375, SCREEN_H=320, PLAY_TOP=48, PLAY_BOTTOM=278, PADDLE_H=45, PADDLE_W=7, BALL_SZ=10, RES=100, SNAKE_W=20, SNAKE_H=13, SNAKE_CAP=260 };
typedef uint32_t u32;
typedef struct {
    int x,y,vx,vy;
    int paddle[2],score[2],cpu[2];
    unsigned seed,accumulator;
    int state,quit,wheel_remainder,choice,game;
    uint16_t snake[SNAKE_CAP];
    int head,tail,length,dir,queued,food,snake_score,snake_accum;
} NativeApp;
void app_init(NativeApp *a,unsigned seed);
int app_input(NativeApp *a,const char *message);
int app_tick(NativeApp *a,unsigned elapsed_ms);
int app_animated(const NativeApp *a);
void app_render(const NativeApp *a,u32 *pixels,unsigned stride);
#endif
