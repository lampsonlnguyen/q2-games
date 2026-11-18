/* SPDX-License-Identifier: GPL-3.0-or-later
 * Compact native adaptations of the gameplay in Rockbox brickmania.c
 * (Ben Basha et al., 2005-2010) and spacerocks.c (Mat Holton, 2006).
 * GPL-2.0-or-later upstream; this combined port is GPL-3.0-or-later.
 * Fixed arrays, integer physics and no Rockbox runtime or allocation.
 */
#include "native_app.h"
enum { GAME=1, OVER=3, BRICKS=2, SPACE=3 };
static int clamp(int n,int lo,int hi){return n<lo?lo:n>hi?hi:n;}
static int abs_i(int n){return n<0?-n:n;}
static unsigned roll(NativeApp *a){a->seed=a->seed*1664525u+1013904223u;return a->seed>>16;}
static const int dx[16]={0,2,3,3,4,3,3,2,0,-2,-3,-3,-4,-3,-3,-2};
static const int dy[16]={-4,-3,-3,-2,0,2,3,3,4,3,3,2,0,-2,-3,-3};
static void ball_reset(NativeApp *a){
    a->bx=a->brick_paddle+23; a->by=253; a->bvx=(roll(a)&1)?3:-3; a->bvy=-3;
}
static void brick_wave(NativeApp *a){
    for(int i=0;i<BRICK_ROWS*BRICK_COLS;i++)a->bricks[i]=1+(unsigned char)((i/BRICK_COLS)%3);
    a->brick_left=BRICK_ROWS*BRICK_COLS;
    ball_reset(a);
}
static void space_wave(NativeApp *a){
    for(int i=0;i<ROCK_MAX;i++){
        a->rocks[i].alive=1; a->rocks[i].size= i<4?16:10;
        a->rocks[i].x=(int)(roll(a)%325)+25;
        a->rocks[i].y=(int)(roll(a)%185)+70;
        a->rocks[i].vx=(roll(a)&1)?1:-1;
        a->rocks[i].vy=(roll(a)&1)?1:-1;
        /* Don't spawn directly on the ship. */
        if(abs_i(a->rocks[i].x-a->ship_x)<38&&abs_i(a->rocks[i].y-a->ship_y)<38)a->rocks[i].x=30+i*45;
    }
}
void extra_begin(NativeApp *a){
    if(a->game==BRICKS){
        a->accumulator=0;
        a->brick_paddle=159; a->brick_score=0; a->brick_lives=3; brick_wave(a);
    }else{
        a->space_score=0;a->space_lives=3;a->space_accum=0;
        a->wheel_remainder=0;
        a->ship_x=187;a->ship_y=175;a->ship_vx=0;a->ship_vy=0;
        a->ship_invuln=70;a->ship_angle=0;a->space_cooldown=0;
        for(int i=0;i<SHOT_MAX;i++)a->shots[i].ttl=0;
        space_wave(a);
    }
}
static void brick_step(NativeApp *a){
    a->bx+=a->bvx;a->by+=a->bvy;
    if(a->bx<25){a->bx=25;a->bvx=abs_i(a->bvx);}
    if(a->bx>343){a->bx=343;a->bvx=-abs_i(a->bvx);}
    if(a->by<55){a->by=55;a->bvy=abs_i(a->bvy);}
    if(a->bvy>0&&a->by+6>=273&&a->by<=283&&a->bx+6>=a->brick_paddle&&a->bx<=a->brick_paddle+52){
        int hit=a->bx+3-(a->brick_paddle+26);
        a->bvy=-abs_i(a->bvy);
        a->bvx=clamp(hit/6,-5,5);
        if(a->bvx==0)a->bvx=hit<0?-1:1;
        a->by=267;
    }
    if(a->by>307){
        if(--a->brick_lives<=0){a->state=OVER;return;}
        ball_reset(a);return;
    }
    if(a->by>=70&&a->by<70+BRICK_ROWS*21){
        int col=(a->bx+3-24)/33,row=(a->by+3-70)/21;
        if(col>=0&&col<BRICK_COLS&&row>=0&&row<BRICK_ROWS){
            int id=row*BRICK_COLS+col;
            if(a->bricks[id]){
                a->bricks[id]--;
                if(!a->bricks[id])a->brick_left--;
                a->brick_score+=10;
                a->bvy=-a->bvy;
                if(a->brick_left==0)brick_wave(a);
            }
        }
    }
}
static void space_step(NativeApp *a){
    if(a->space_cooldown>0)a->space_cooldown--;
    if(a->ship_invuln>0)a->ship_invuln--;
    a->ship_x+=a->ship_vx;a->ship_y+=a->ship_vy;
    if(a->ship_x<24)a->ship_x=346;
    if(a->ship_x>346)a->ship_x=24;
    if(a->ship_y<85)a->ship_y=275;
    if(a->ship_y>275)a->ship_y=85;
    for(int i=0;i<SHOT_MAX;i++){
        if(a->shots[i].ttl<=0)continue;
        a->shots[i].x+=a->shots[i].vx; a->shots[i].y+=a->shots[i].vy;
        if(a->shots[i].x<18||a->shots[i].x>356||a->shots[i].y<55||a->shots[i].y>285)a->shots[i].ttl=0;
        else a->shots[i].ttl--;
    }
    int alive=0;
    for(int i=0;i<ROCK_MAX;i++){
        if(!a->rocks[i].alive)continue;
        alive++;
        a->rocks[i].x+=a->rocks[i].vx; a->rocks[i].y+=a->rocks[i].vy;
        if(a->rocks[i].x<24)a->rocks[i].x=350;
        if(a->rocks[i].x>350)a->rocks[i].x=24;
        if(a->rocks[i].y<60)a->rocks[i].y=270;
        if(a->rocks[i].y>270)a->rocks[i].y=60;
        int x=a->rocks[i].x,y=a->rocks[i].y,rad=a->rocks[i].size;
        if(a->ship_invuln==0&&abs_i(x-a->ship_x)<rad+7&&abs_i(y-a->ship_y)<rad+7){
            if(--a->space_lives<=0){a->state=OVER;return;}
            a->ship_x=187;a->ship_y=175;a->ship_vx=0;a->ship_vy=0;
            a->ship_invuln=75;
        }
        for(int j=0;j<SHOT_MAX;j++){
            if(a->shots[j].ttl<=0)continue;
            if(abs_i(a->shots[j].x-x)<rad&&abs_i(a->shots[j].y-y)<rad){
                a->shots[j].ttl=0;a->rocks[i].alive=0;
                a->space_score+=rad==16?20:50;
                if(rad==16){
                    /* Break a large rock into a smaller moving fragment. */
                    for(int k=0;k<ROCK_MAX;k++)if(!a->rocks[k].alive&&k!=i){
                        a->rocks[k].alive=1;a->rocks[k].size=8;
                        a->rocks[k].x=x;a->rocks[k].y=y;
                        a->rocks[k].vx=-a->rocks[i].vx;
                        a->rocks[k].vy=(roll(a)&1)?2:-2;
                        break;
                    }
                }
                break;
            }
        }
    }
    if(alive==0)space_wave(a);
}
void extra_tick(NativeApp *a,unsigned elapsed_ms){
    if(a->game==BRICKS) {
        a->accumulator+=elapsed_ms*60;
        for(int i=0;i<4&&a->accumulator>=1000;i++){
            a->accumulator-=1000;brick_step(a);if(a->state!=GAME)break;
        }
    }else{
        a->space_accum+=(int)elapsed_ms;
        for(int i=0;i<4&&a->space_accum>=25;i++){
            a->space_accum-=25;space_step(a);
        }
    }
}
void extra_wheel(NativeApp *a,int delta){
    if(a->game==BRICKS)a->brick_paddle=clamp(a->brick_paddle-delta/2,25,299);
    else{
        a->wheel_remainder-=delta;
        while(a->wheel_remainder>=30){a->ship_angle=(a->ship_angle+1)&15;a->wheel_remainder-=30;}
        while(a->wheel_remainder<=-30){a->ship_angle=(a->ship_angle+15)&15;a->wheel_remainder+=30;}
    }
}
void extra_button(NativeApp *a,int code){
    if(a->game==BRICKS){
        if(code==172||code==222)a->brick_paddle=clamp(a->brick_paddle-15,25,299);
        else if(code==173||code==223)a->brick_paddle=clamp(a->brick_paddle+15,25,299);
        return;
    }
    if(code==172||code==222){
        /* Rockbox-like hyperspace: jump to a safe random sector. */
        a->ship_x=45+(int)(roll(a)%270);a->ship_y=95+(int)(roll(a)%155);
        a->ship_invuln=40;return;
    }
    if(code==173||code==223){
        a->ship_vx=clamp(a->ship_vx+dx[a->ship_angle]/2,-5,5);
        a->ship_vy=clamp(a->ship_vy+dy[a->ship_angle]/2,-5,5);
        return;
    }
    if(code!=218||a->space_cooldown)return;
    for(int i=0;i<SHOT_MAX;i++)if(a->shots[i].ttl<=0){
        a->shots[i].x=a->ship_x+dx[a->ship_angle]*4;
        a->shots[i].y=a->ship_y+dy[a->ship_angle]*4;
        a->shots[i].vx=dx[a->ship_angle]*2;
        a->shots[i].vy=dy[a->ship_angle]*2;
        a->shots[i].ttl=42;a->space_cooldown=5;break;
    }
}
static void box(u32 *d,unsigned stride,int x,int y,int w,int h,u32 c){
    if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
    if(x+w>SCREEN_W)w=SCREEN_W-x;
    if(y+h>SCREEN_H)h=SCREEN_H-y;
    if(w<=0||h<=0)return;
    for(int px=x;px<x+w;px++)for(int py=y;py<y+h;py++)
        d[px*stride+SCREEN_H-1-py]=c|0xff000000u;
}
static void segment(u32*d,unsigned stride,int x0,int y0,int x1,int y1,u32 color){
    int sx=x0<x1?1:-1,sy=y0<y1?1:-1,dx0=abs_i(x1-x0),dy0=-abs_i(y1-y0),err=dx0+dy0;
    for(;;){
        box(d,stride,x0,y0,2,2,color);
        if(x0==x1&&y0==y1)break;
        int e=2*err;
        if(e>=dy0){err+=dy0;x0+=sx;}
        if(e<=dx0){err+=dx0;y0+=sy;}
    }
}
void extra_render(const NativeApp *a,u32 *d,unsigned stride){
    if(a->game==BRICKS){
        for(int i=0;i<BRICK_ROWS*BRICK_COLS;i++)if(a->bricks[i]){
            int x=24+(i%BRICK_COLS)*33,y=70+(i/BRICK_COLS)*21;
            u32 color=a->bricks[i]==3?0xdf725b:a->bricks[i]==2?0xe3b258:0x55b5a7;
            box(d,stride,x,y,30,16,color);
            box(d,stride,x+2,y+2,26,2,0xeff4e9);
        }
        box(d,stride,a->brick_paddle,275,52,7,0x4dc5e8);
        box(d,stride,a->bx,a->by,7,7,0xffffff);
        for(int life=0;life<a->brick_lives;life++)box(d,stride,27+life*18,298,11,4,0xe3b258);
    }else{
        box(d,stride,19,70,337,219,0x15263a);
        for(int i=0;i<ROCK_MAX;i++)if(a->rocks[i].alive){
            int x=a->rocks[i].x,y=a->rocks[i].y,r=a->rocks[i].size;
            segment(d,stride,x-r,y-3,x-r/2,y-r,0xb3bdd0);
            segment(d,stride,x-r/2,y-r,x+r/2,y-r+2,0xb3bdd0);
            segment(d,stride,x+r/2,y-r+2,x+r,y+3,0xb3bdd0);
            segment(d,stride,x+r,y+3,x+r/3,y+r,0xb3bdd0);
            segment(d,stride,x+r/3,y+r,x-r,y+4,0xb3bdd0);
        }
        for(int life=0;life<a->space_lives;life++)box(d,stride,25+life*16,297,9,5,0x4dc5e8);
        for(int i=0;i<SHOT_MAX;i++)if(a->shots[i].ttl>0)
            box(d,stride,a->shots[i].x,a->shots[i].y,4,4,0xf7dd77);
        int a0=a->ship_angle, left=(a0+11)&15,right=(a0+5)&15;
        int x=a->ship_x,y=a->ship_y;
        u32 ship_color=a->ship_invuln>0?0x74a8bc:0x4dc5e8;
        segment(d,stride,x+dx[a0]*4,y+dy[a0]*4,x+dx[left]*3,y+dy[left]*3,ship_color);
        segment(d,stride,x+dx[a0]*4,y+dy[a0]*4,x+dx[right]*3,y+dy[right]*3,ship_color);
        segment(d,stride,x+dx[left]*3,y+dy[left]*3,x+dx[right]*3,y+dy[right]*3,ship_color);
    }
}
