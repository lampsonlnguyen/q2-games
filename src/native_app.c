/* SPDX-License-Identifier: GPL-3.0-or-later
 * Port of Rockbox Pong physics: paddle collision, offset-dependent bounce,
 * jitter, score reset and CPU tracking adapted from:
 * apps/plugins/pong.c (C) 2004 Daniel Stenberg, GPL-2.0-or-later.
 * Q2 input / menu implementation is new.
 */
#include "native_app.h"

enum { MENU=0, GAME=1, PAUSE=2, OVER=3, PONG=0, SNAKE=1, BRICKS=2, SPACE=3 };
static unsigned rnd(NativeApp *a) { a->seed=a->seed*1664525u+1013904223u; return a->seed>>16; }
static int clamp(int v,int lo,int hi) { return v<lo?lo:v>hi?hi:v; }
static int field_h(void) { return PLAY_BOTTOM-PLAY_TOP; }
static void serve(NativeApp *a,int direction) {
    a->x=(SCREEN_W/2)*RES;
    a->y=(PLAY_TOP+field_h()/2)*RES;
    a->vx=direction*560;
    a->vy=(rnd(a)&1?1:-1)*380;
}
void app_init(NativeApp *a,unsigned seed) {
    *a=(NativeApp){0};
    a->seed=seed?seed:1u;
    a->paddle[0]=a->paddle[1]=PLAY_TOP+(field_h()-PADDLE_H)/2;
    a->cpu[0]=a->cpu[1]=1;
    a->state=MENU;
    a->game=PONG;
    serve(a,1);
}
static void snake_begin(NativeApp *a);
static void begin(NativeApp *a) {
    a->game=a->choice;
    if(a->game==SNAKE) { snake_begin(a); a->state=GAME; return; }
    if(a->game>=BRICKS) { extra_begin(a); a->state=GAME; return; }
    a->score[0]=a->score[1]=0;
    a->cpu[0]=1; a->cpu[1]=0;
    a->paddle[0]=a->paddle[1]=PLAY_TOP+(field_h()-PADDLE_H)/2;
    a->accumulator=0;
    serve(a,(rnd(a)&1)?1:-1);
    a->state=GAME;
}
static void move(NativeApp *a,int delta) {
    a->paddle[1]=clamp(a->paddle[1]+delta,PLAY_TOP,PLAY_BOTTOM-PADDLE_H);
}
static void collision(NativeApp *a,int side) {
    int info=(a->y/RES+BALL_SZ/2)-(a->paddle[side]+PADDLE_H/2);
    a->vx=-a->vx;
    a->x+=side?-PADDLE_W*RES/4:PADDLE_W*RES/4;
    /* Rockbox Pong's original paddle-angle reflection logic. */
    if(a->vy>0) {
        if(info>0) a->vy+=info*RES/3;
        else if(info<0) a->vy=info*RES/2;
    } else {
        if(info>0) a->vy=info*RES/2;
        else if(info<0) a->vy+=info*RES/3;
    }
    a->vy+=(int)(rnd(a)%21)-10;
    a->vy=clamp(a->vy,-1400,1400);
}
static void step(NativeApp *a) {
    for(int side=0;side<2;side++) if(a->cpu[side]) {
        int x=a->x/RES;
        if((side==0&&x<SCREEN_W*3/8)||(side==1&&x>SCREEN_W*5/8)) {
            int middle=a->paddle[side]+PADDLE_H/2;
            int ball=a->y/RES;
            int direction=ball>middle?6:ball<middle?-6:0;
            a->paddle[side]=clamp(a->paddle[side]+direction,PLAY_TOP,PLAY_BOTTOM-PADDLE_H);
        }
    }
    a->x+=a->vx;
    a->y+=a->vy;
    if(a->y<PLAY_TOP*RES) {a->y=PLAY_TOP*RES;a->vy=-a->vy;}
    if(a->y>(PLAY_BOTTOM-BALL_SZ)*RES) {a->y=(PLAY_BOTTOM-BALL_SZ)*RES;a->vy=-a->vy;}
    int by=a->y/RES;
    if(by+BALL_SZ>a->paddle[0]&&by<a->paddle[0]+PADDLE_H&&
       a->vx<0&&a->x<=(17+PADDLE_W)*RES) collision(a,0);
    else if(by+BALL_SZ>a->paddle[1]&&by<a->paddle[1]+PADDLE_H&&
            a->vx>0&&a->x+BALL_SZ*RES>=(SCREEN_W-17-PADDLE_W)*RES) collision(a,1);
    else if(a->x<17*RES) {a->score[1]++;serve(a,1);}
    else if(a->x>(SCREEN_W-17-BALL_SZ)*RES) {a->score[0]++;serve(a,-1);}
}
int app_animated(const NativeApp *a) {return a->state==GAME;}
static void snake_step(NativeApp *a);
int app_tick(NativeApp *a,unsigned elapsed_ms) {
    if(!app_animated(a))return 0;
    if(a->game>=BRICKS) { extra_tick(a,elapsed_ms); return 1; }
    if(a->game==SNAKE) {
        a->snake_accum+=(int)elapsed_ms;
        int pace=185-a->snake_score*3;
        if(pace<95)pace=95;
        for(int i=0;i<3&&a->snake_accum>=pace;i++) { a->snake_accum-=pace; snake_step(a); if(a->state!=GAME)break; }
        return 1;
    }
    if(elapsed_ms>50)elapsed_ms=50;
    a->accumulator+=elapsed_ms*60;
    for(int i=0;i<4&&a->accumulator>=1000;i++) { a->accumulator-=1000; step(a); }
    return 1;
}
static int parse(const char **ptr) {
    const char *p=*ptr;
    int sign=1;
    while(*p==' ')p++;
    if(*p=='-'){sign=-1;p++;}
    if(*p<'0'||*p>'9')return 999999;
    int v=0;
    while(*p>='0'&&*p<='9'){if(v>100000)return 999999;v=v*10+*p++-'0';}
    *ptr=p;
    return sign*v;
}
/* Portable Rockbox Snake-inspired gameplay:
 * ring-ordered body, apple spawning, no 180 degree reversals, and wall/body death.
 * Itai Shaked's Rockbox snake.c (2002) uses an age-grid approach;
 * this implementation keeps its own fixed-size ring to avoid heap allocation.
 */
static void snake_food(NativeApp *a) {
    if(a->length>=SNAKE_CAP){a->state=OVER;a->food=-1;return;}
    unsigned offset=rnd(a)%SNAKE_CAP;
    for(int i=0;i<SNAKE_CAP;i++){
        int cell=(int)((offset+(unsigned)i)%SNAKE_CAP),used=0;
        for(int n=0;n<a->length;n++)
            if(a->snake[(a->tail+n)%SNAKE_CAP]==cell) {used=1;break;}
        if(!used){a->food=cell;return;}
    }
    a->state=OVER;
}
static void snake_begin(NativeApp *a) {
    a->length=4; a->tail=0; a->head=3;
    a->dir=1; a->queued=1;
    a->snake_score=0; a->snake_accum=0;
    const int mid=SNAKE_H/2;
    for(int i=0;i<4;i++)a->snake[i]=(uint16_t)(mid*SNAKE_W+5+i);
    snake_food(a);
}
static void snake_turn(NativeApp *a,int delta) {
    if(a->queued!=a->dir)return; /* limit to one turn per movement frame */
    a->queued=(a->dir+(delta>0?1:3))&3;
}
static void snake_step(NativeApp *a) {
    a->dir=a->queued;
    int old=a->snake[a->head],x=old%SNAKE_W,y=old/SNAKE_W;
    if(a->dir==0)y--;
    else if(a->dir==1)x++;
    else if(a->dir==2)y++;
    else x--;
    if(x<0||x>=SNAKE_W||y<0||y>=SNAKE_H){a->state=OVER;return;}
    int next=y*SNAKE_W+x,ate=next==a->food;
    for(int i=0;i<a->length-(ate?0:1);i++)
        if(a->snake[(a->tail+i)%SNAKE_CAP]==next){a->state=OVER;return;}
    if(!ate)a->tail=(a->tail+1)%SNAKE_CAP;
    else {a->length++;a->snake_score++;}
    a->head=(a->head+1)%SNAKE_CAP;
    a->snake[a->head]=(uint16_t)next;
    if(ate)snake_food(a);
}
int app_input(NativeApp *a,const char *message) {
    if(!message||message[1]!=' ')return 0;
    if(message[0]=='w') {
        const char *ptr=message+2;int value=parse(&ptr);
        if(value<-2000||value>2000)return 0;
        if(a->state==MENU) {
            a->wheel_remainder-=value;
            if(a->wheel_remainder>=50){if(a->choice<3)a->choice++;a->wheel_remainder=0;}
            else if(a->wheel_remainder<=-50){if(a->choice>0)a->choice--;a->wheel_remainder=0;}
        } else if(a->state==GAME&&a->game==SNAKE) {
            a->wheel_remainder-=value;
            if(a->wheel_remainder>=50){snake_turn(a,1);a->wheel_remainder=0;}
            else if(a->wheel_remainder<=-50){snake_turn(a,-1);a->wheel_remainder=0;}
        } else if(a->state==GAME&&a->game>=BRICKS)extra_wheel(a,value);
        else if(a->state==GAME)move(a,clamp(-value,-200,200));
        return 1;
    }
    if(message[3]!=' ')return 0;
    const char *ptr=message+4;int value=parse(&ptr);
    if(message[0]=='k'&&message[2]=='u') {
        if(value==170) {
            if(a->state==MENU)a->quit=1;
            else if(a->state==GAME)a->state=PAUSE;
            else a->state=MENU;
            return 1;
        }
        if(value==218) {
            if(a->state==MENU){begin(a);}
            else if(a->state==PAUSE)a->state=GAME;
            else if(a->state==OVER)begin(a);
            else if(a->state==GAME&&a->game>=BRICKS)extra_button(a,value);
            return 1;
        }
        if(value==171) {
            if(a->state==GAME)a->state=PAUSE;
            else if(a->state==PAUSE)a->state=GAME;
            return 1;
        }
        if(a->state==MENU&&(value==172||value==222||value==173||value==223)) {
            if(value==172||value==222){if(a->choice>0)a->choice--;}
            else if(a->choice<3)a->choice++;
            return 1;
        }
        if(a->state==GAME&&a->game==SNAKE&&(value==172||value==222||value==173||value==223)) {
            snake_turn(a,(value==172||value==222)?-1:1);return 1;
        }
        if(a->state==GAME&&a->game>=BRICKS&&(value==172||value==222||value==173||value==223)) {
            extra_button(a,value);return 1;
        }
        if(a->state==GAME&&(value==172||value==222)) {move(a,-14);return 1;}
        if(a->state==GAME&&(value==173||value==223)) {move(a,14);return 1;}
    }
    if(message[0]=='t'&&message[2]=='u') {
        int y=parse(&ptr);
        if(value<0||value>=SCREEN_W||y<0||y>=SCREEN_H)return 0;
        if(a->state==MENU){a->choice=y<138?PONG:y<181?SNAKE:y<224?BRICKS:SPACE;begin(a);}
        else if(a->state==GAME&&y<PLAY_TOP)a->state=PAUSE;
        else if(a->state==PAUSE)a->state=GAME;
        else if(a->state==GAME&&a->game>=BRICKS)extra_button(a,218);
        else if(a->state==GAME&&a->game==PONG) a->paddle[1]=clamp(y-PADDLE_H/2,PLAY_TOP,PLAY_BOTTOM-PADDLE_H);
        return 1;
    }
    return 0;
}
