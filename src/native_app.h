/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef Q2_NATIVE_APP_H
#define Q2_NATIVE_APP_H
#include <stdint.h>
enum { SCREEN_W=375, SCREEN_H=320, PLAY_TOP=48, PLAY_BOTTOM=278, PADDLE_H=45, PADDLE_W=7, BALL_SZ=10, RES=100, SNAKE_W=20, SNAKE_H=13, SNAKE_CAP=260, BRICK_COLS=10, BRICK_ROWS=5, ROCK_MAX=7, SHOT_MAX=6, GAME_COUNT=9 };
typedef uint32_t u32;
typedef struct {
    int x,y,vx,vy;
    int paddle[2],score[2],cpu[2];
    unsigned seed,accumulator;
    int state,quit,wheel_remainder,choice,game,center_down,touch_down;
    uint16_t snake[SNAKE_CAP];
    int head,tail,length,dir,queued,food,snake_score,snake_accum;
    int bx,by,bvx,bvy,brick_paddle,brick_score,brick_lives,brick_left;
    unsigned char bricks[BRICK_COLS*BRICK_ROWS];
    int ship_x,ship_y,ship_vx,ship_vy,ship_invuln,ship_angle,space_score,space_lives,space_accum,space_cooldown;
    struct {int x,y,vx,vy,size,alive;} rocks[ROCK_MAX];
    struct {int x,y,vx,vy,ttl;} shots[SHOT_MAX];
    unsigned char tiles[16],mines[64],flags[64],revealed[64];
    int puzzle_cursor,puzzle_blank,puzzle_moves,mine_cursor,mine_ready,mine_left;
    unsigned char reversi[64];
    int reversi_cursor,reversi_player,reversi_cpu;
    unsigned char flips[25],powers[16];
    int small_cursor,small_steps,small_score;
} NativeApp;
void app_init(NativeApp *a,unsigned seed);
int app_input(NativeApp *a,const char *message);
int app_tick(NativeApp *a,unsigned elapsed_ms);
int app_animated(const NativeApp *a);
void app_render(const NativeApp *a,u32 *pixels,unsigned stride);
void extra_begin(NativeApp *a);
void extra_tick(NativeApp *a,unsigned elapsed_ms);
void extra_wheel(NativeApp *a,int delta);
void extra_button(NativeApp *a,int code);
void extra_render(const NativeApp *a,u32 *pixels,unsigned stride);
void puzzle_begin(NativeApp *a);
void puzzle_wheel(NativeApp *a,int delta);
void puzzle_button(NativeApp *a,int code);
void puzzle_touch(NativeApp *a,int x,int y);
void puzzle_render(const NativeApp *a,u32 *pixels,unsigned stride);
void reversi_begin(NativeApp *a);
void reversi_wheel(NativeApp *a,int delta);
void reversi_button(NativeApp *a,int code);
void reversi_touch(NativeApp *a,int x,int y);
void reversi_render(const NativeApp *a,u32 *pixels,unsigned stride);
void small_begin(NativeApp *a);
void small_wheel(NativeApp *a,int amount);
void small_button(NativeApp *a,int key);
void small_touch(NativeApp *a,int x,int y);
void small_render(const NativeApp *a,u32 *pixels,unsigned stride);
#endif
