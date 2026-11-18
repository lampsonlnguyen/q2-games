/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "native_app.h"
static void rect(u32 *d,unsigned stride,int x,int y,int w,int h,u32 c) {
    int x0=x<0?0:x,y0=y<0?0:y,x1=x+w>SCREEN_W?SCREEN_W:x+w,y1=y+h>SCREEN_H?SCREEN_H:y+h;
    if(x1<=x0||y1<=y0)return;
    for(int px=x0;px<x1;px++)for(int py=y0;py<y1;py++)
        d[px*stride+SCREEN_H-1-py]=c;
}
/* Compact custom 3x5 glyphs, for game title / HUD. */
static const char glyphs[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789: -";
static const unsigned short bits[]={
0x7bed,0x7aeb,0x724f,0x7b6b,0x79cf,0x79c7,0x72ef,0x5bed,0x7497,0x349e,
0x5aed,0x4927,0x5f6d,0x5f6d,0x7b6f,0x7bc7,0x7b7c,0x7bed,0x73ce,0x7492,
0x5b6f,0x5b6a,0x5f7f,0x5aeb,0x5a92,0x79cf,
0x7b6f,0x2492,0x73e7,0x73cf,0x5bef,0x79cf,0x79ef,0x7249,0x7bef,0x7bcf,
0x000a,0x0000,0x001c };
static void label(u32*d,unsigned s,int x,int y,const char *t,int scale,u32 c) {
    while(*t) {
        char ch=*t++; unsigned idx=0;
        while(glyphs[idx]&&glyphs[idx]!=ch)idx++;
        if(glyphs[idx]) {
            unsigned short b=bits[idx];
            for(int cy=0;cy<5;cy++)for(int cx=0;cx<3;cx++)
                if(b&(1u<<(14-(cy*3+cx))))rect(d,s,x+cx*scale,y+cy*scale,scale,scale,c);
        }
        x+=4*scale;
    }
}
static void number(u32*d,unsigned s,int x,int y,int n,int scale,u32 color) {
    char buf[12];int i=0,j;
    if(n==0)buf[i++]='0';
    while(n>0&&i<10){buf[i++]=(char)('0'+n%10);n/=10;}
    for(j=i-1;j>=0;j--){char tmp[2]={buf[j],0};label(d,s,x,y,tmp,scale,color);x+=4*scale;}
}
void app_render(const NativeApp *a,u32*d,unsigned s) {
    const u32 bg=0x0c1522,edge=0x17283b,fg=0xe8f4ff,accent=0x4dc5e8;
    rect(d,s,0,0,SCREEN_W,SCREEN_H,bg);
    label(d,s,80,20,"Q2 GAMES",3,fg);
    if(a->state==0) {
        label(d,s,70,110,"ROCKBOX PONG",3,accent);
        label(d,s,86,174,"CENTER TO PLAY",2,fg);
        label(d,s,98,205,"WHEEL MOVES",2,fg);
        label(d,s,106,235,"BACK EXITS",2,fg);
        return;
    }
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
    if(a->state==2) {
        rect(d,s,58,121,259,88,0x213247);
        label(d,s,129,136,"PAUSED",3,fg);
        label(d,s,91,177,"CENTER RESUMES",2,accent);
    }
}
