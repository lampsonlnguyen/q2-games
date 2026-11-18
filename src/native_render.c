/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "native_app.h"
#include "font.h"
static void rect(u32 *d,unsigned stride,int x,int y,int w,int h,u32 c) {
    int x0=x<0?0:x,y0=y<0?0:y,x1=x+w>SCREEN_W?SCREEN_W:x+w,y1=y+h>SCREEN_H?SCREEN_H:y+h;
    if(x1<=x0||y1<=y0)return;
    for(int px=x0;px<x1;px++)for(int py=y0;py<y1;py++)
        d[px*stride+SCREEN_H-1-py]=c|0xff000000u;
}
/* The readable 12px/17px Liberation Sans Bold glyphs used by Q2 Arcade.
 * Anti-aliased alpha is composited against an opaque framebuffer, matching
 * the established Q2 games renderer. */
static void label(u32 *d,unsigned stride,int x,int y,const char *text,int scale,u32 color) {
    const unsigned char *adv=scale>=3?f18_adv:f12_adv;
    const unsigned short *off=scale>=3?f18_off:f12_off;
    const unsigned char *pixels=scale>=3?f18_px:f12_px;
    int height=scale>=3?17:12;
    for(;*text;text++){
        unsigned ch=(unsigned char)*text;
        if(ch>='a'&&ch<='z')ch-=32;
        unsigned idx=ch>=32&&ch<=90?ch-32:0;
        int width=adv[idx];
        for(int row=0;row<height;row++)for(int col=0;col<width;col++){
            unsigned alpha=pixels[off[idx]+row*width+col];
            if(!alpha)continue;
            int px=x+col,py=y+row;
            if(px<0||px>=SCREEN_W||py<0||py>=SCREEN_H)continue;
            u32 *at=&d[px*stride+SCREEN_H-1-py],old=*at;
            unsigned red=(((old>>16)&255u)*(255-alpha)+((color>>16)&255u)*alpha)/255u;
            unsigned green=(((old>>8)&255u)*(255-alpha)+((color>>8)&255u)*alpha)/255u;
            unsigned blue=((old&255u)*(255-alpha)+(color&255u)*alpha)/255u;
            *at=0xff000000u|(red<<16)|(green<<8)|blue;
        }
        x+=width;
    }
}
static void number(u32*d,unsigned stride,int x,int y,int n,int scale,u32 color) {
    char buf[12];int count=0;
    if(n==0)buf[count++]='0';
    while(n>0&&count<10){buf[count++]=(char)('0'+n%10);n/=10;}
    for(int i=count-1;i>=0;i--) {
        char digit[2]={buf[i],0};
        label(d,stride,x,y,digit,scale,color);
        x+=(scale>=3?f18_adv[buf[i]-32]:f12_adv[buf[i]-32]);
    }
}
void app_render(const NativeApp *a,u32*d,unsigned s) {
    const u32 bg=0x0c1522,edge=0x17283b,fg=0xe8f4ff,accent=0x4dc5e8;
    rect(d,s,0,0,SCREEN_W,SCREEN_H,bg);
    label(d,s,80,20,"Q2 GAMES",3,fg);
    if(a->state==0) {
        label(d,s,76,58,"SELECT GAME",3,fg);
        rect(d,s,47,107,281,51,a->choice==0?0x27506b:edge);
        label(d,s,111,123,"PONG",3,a->choice==0?accent:fg);
        rect(d,s,47,168,281,51,a->choice==1?0x27506b:edge);
        label(d,s,103,184,"SNAKE",3,a->choice==1?accent:fg);
        label(d,s,76,251,"WHEEL  CENTER",2,fg);
        return;
    }
    if(a->game==1){
        const int unit=14,ox=47,oy=72;
        label(d,s,80,51,"SNAKE",2,accent);
        rect(d,s,ox-4,oy-4,SNAKE_W*unit+8,SNAKE_H*unit+8,edge);
        for(int i=0;i<a->length;i++){
            int cell=a->snake[(a->tail+i)%SNAKE_CAP];
            int x=cell%SNAKE_W,y=cell/SNAKE_W;
            rect(d,s,ox+x*unit,oy+y*unit,unit-1,unit-1,i==a->length-1?accent:0x5aa5a0);
        }
        if(a->food>=0)rect(d,s,ox+(a->food%SNAKE_W)*unit+2,oy+(a->food/SNAKE_W)*unit+2,unit-5,unit-5,0xfa8464);
        label(d,s,112,283,"SCORE",2,fg);
        number(d,s,221,283,a->snake_score,2,accent);
    } else {
    rect(d,s,17,PLAY_TOP,SCREEN_W-34,PLAY_BOTTOM-PLAY_TOP,edge);
    rect(d,s,SCREEN_W/2-1,PLAY_TOP,2,PLAY_BOTTOM-PLAY_TOP,0x30415b);
    rect(d,s,17,PLAY_TOP,SCREEN_W-34,2,accent);
    rect(d,s,17,PLAY_BOTTOM-2,SCREEN_W-34,2,accent);
    rect(d,s,17,a->paddle[0],PADDLE_W,PADDLE_H,0xff9e68);
    rect(d,s,SCREEN_W-17-PADDLE_W,a->paddle[1],PADDLE_W,PADDLE_H,accent);
    rect(d,s,a->x/RES,a->y/RES,BALL_SZ,BALL_SZ,fg);
    number(d,s,151,287,a->score[0],2,fg);
    label(d,s,180,287,"-",2,fg);
    number(d,s,210,287,a->score[1],2,fg);
    }
    if(a->state==2) {
        rect(d,s,58,121,259,88,0x213247);
        label(d,s,129,136,"PAUSED",3,fg);
        label(d,s,91,177,"CENTER RESUMES",2,accent);
    }
    if(a->state==3){
        rect(d,s,58,121,259,88,0x213247);
        label(d,s,111,136,"GAME OVER",3,fg);
        label(d,s,91,177,"CENTER RESTART",2,accent);
    }
}
