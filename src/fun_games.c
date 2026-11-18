/* SPDX-License-Identifier: GPL-3.0-or-later
 * Small native game adaptations inspired by Rockbox jackpot.c (Kevin
 * Ferrare/Pierre Delore, 2007) and robotfindskitten.c (Leonard Richardson,
 * 1997/2000). Original Rockbox sources GPL-2.0-or-later.
 * Zero heap, no original art/data files, Q2 wheel/Center/touch controls.
 */
#include "native_app.h"
#include "font.h"
enum {OVER=3,JACKPOT=12,ROBOT=13,GRID_W=15,GRID_H=9};
static unsigned random_next(NativeApp*a){a->seed=a->seed*1664525u+1013904223u;return a->seed>>16;}
static int in_range(int n,int low,int high){return n>=low&&n<high;}
static void jackpot_spin(NativeApp*a){
    if(a->jackpot_spins>=10){a->state=OVER;return;}
    for(int i=0;i<3;i++)a->jackpot_reel[i]=(unsigned char)(random_next(a)%9);
    int x=a->jackpot_reel[0],y=a->jackpot_reel[1],z=a->jackpot_reel[2];
    int points=x==y&&y==z?9:(x==y||y==z||x==z)?2:0;
    a->jackpot_score+=points;
    a->jackpot_last=points;
    a->jackpot_spins++;
    if(a->jackpot_spins>=10)a->state=OVER;
}
static void robot_init(NativeApp*a){
    for(int i=0;i<GRID_W*GRID_H;i++)a->kitten_board[i]=0;
    a->robot_pos=(GRID_H/2)*GRID_W+GRID_W/2;
    a->robot_heading=1;a->robot_steps=0;a->robot_message=0;
    int kitty;
    do{kitty=(int)(random_next(a)%(GRID_W*GRID_H));}
    while(kitty==a->robot_pos);
    a->kitten_board[kitty]=1;
    for(int i=0;i<24;i++){
        int cell=(int)(random_next(a)%(GRID_W*GRID_H));
        if(cell==a->robot_pos||a->kitten_board[cell])continue;
        a->kitten_board[cell]=2;
    }
}
void fun_begin(NativeApp*a){
    a->wheel_remainder=0;
    if(a->game==JACKPOT){
        a->jackpot_score=0;a->jackpot_spins=0;a->jackpot_last=0;
        for(int i=0;i<3;i++)a->jackpot_reel[i]=(unsigned char)(random_next(a)%9);
    }else robot_init(a);
}
static void robot_advance(NativeApp*a){
    int old=a->robot_pos;
    int x=old%GRID_W,y=old/GRID_W;
    if(a->robot_heading==0)y--;
    else if(a->robot_heading==1)x++;
    else if(a->robot_heading==2)y++;
    else x--;
    if(!in_range(x,0,GRID_W)||!in_range(y,0,GRID_H))return;
    a->robot_pos=y*GRID_W+x;
    a->robot_steps++;
    if(a->kitten_board[a->robot_pos]==1){a->state=OVER;return;}
    if(a->kitten_board[a->robot_pos]==2){
        a->robot_message=1+(int)(random_next(a)%5);
        a->kitten_board[a->robot_pos]=0;
    }else a->robot_message=0;
}
void fun_wheel(NativeApp*a,int delta){
    if(a->game==JACKPOT)return; /* The single-button game needs no wheel. */
    a->wheel_remainder-=delta;
    while(a->wheel_remainder>=28){a->robot_heading=(a->robot_heading+1)&3;a->wheel_remainder-=28;}
    while(a->wheel_remainder<=-28){a->robot_heading=(a->robot_heading+3)&3;a->wheel_remainder+=28;}
}
void fun_button(NativeApp*a,int code){
    if(a->game==JACKPOT){if(code==218)jackpot_spin(a);return;}
    if(code==172||code==222)a->robot_heading=(a->robot_heading+3)&3;
    else if(code==173||code==223)a->robot_heading=(a->robot_heading+1)&3;
    else if(code==218)robot_advance(a);
}
void fun_touch(NativeApp*a,int x,int y){
    if(a->game==JACKPOT){if(y>=210)jackpot_spin(a);return;}
    if(y>=283){
        a->robot_heading=x<93?3:x<187?0:x<280?2:1;
        robot_advance(a);
    }else if(x>=37&&x<337&&y>=70&&y<250){
        int target=((y-70)/20)*GRID_W+(x-37)/20;
        int old=a->robot_pos;
        int diff=target-old;
        if(diff==1&&old%GRID_W!=GRID_W-1)a->robot_heading=1;
        else if(diff==-1&&old%GRID_W!=0)a->robot_heading=3;
        else if(diff==GRID_W)a->robot_heading=2;
        else if(diff==-GRID_W)a->robot_heading=0;
        else return;
        robot_advance(a);
    }
}
static void fill(u32*d,unsigned s,int x,int y,int w,int h,u32 c){
    if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}
    if(x+w>SCREEN_W)w=SCREEN_W-x;
    if(y+h>SCREEN_H)h=SCREEN_H-y;
    if(w<=0||h<=0)return;
    for(int xx=x;xx<x+w;xx++)for(int yy=y;yy<y+h;yy++)
        d[xx*s+SCREEN_H-1-yy]=0xff000000u|c;
}
static void label(u32*d,unsigned s,int x,int y,const char*t,u32 c){
    for(;*t;t++){
        unsigned k=(unsigned char)*t;
        if(k>='a'&&k<='z')k-=32;
        k=k>=32&&k<=90?k-32:0;
        int w=f18_adv[k];
        for(int yy=0;yy<17;yy++)for(int xx=0;xx<w;xx++){
            unsigned alpha=f18_px[f18_off[k]+yy*w+xx];
            int px=x+xx,py=y+yy;
            if(!alpha||px<0||px>=SCREEN_W||py<0||py>=SCREEN_H)continue;
            u32 *at=&d[px*s+SCREEN_H-1-py],old=*at;
            unsigned r=(((old>>16)&255)*(255-alpha)+((c>>16)&255)*alpha)/255;
            unsigned g=(((old>>8)&255)*(255-alpha)+((c>>8)&255)*alpha)/255;
            unsigned b=((old&255)*(255-alpha)+(c&255)*alpha)/255;
            *at=0xff000000u|(r<<16)|(g<<8)|b;
        }
        x+=w;
    }
}
static void number(u32*d,unsigned s,int x,int y,int value,u32 c){
    char b[12];int n=0;
    do{b[n++]=(char)('0'+value%10);value/=10;}while(value&&n<10);
    while(n--){char t[2]={b[n],0};label(d,s,x,y,t,c);x+=f18_adv[b[n]-32];}
}
void fun_render(const NativeApp*a,u32*d,unsigned s){
    if(a->game==JACKPOT){
        label(d,s,155,48,"JACKPOT",0x4dc5e8);
        static const u32 color[9]={0xd7a460,0x51ada4,0xbb7b9c,0x84a8c7,
            0xd5cc71,0xc67d64,0x7b98ca,0xa1c580,0xd8a26d};
        for(int i=0;i<3;i++){
            int x=45+i*97;
            fill(d,s,x,100,90,96,0x22354a);
            fill(d,s,x+7,107,76,81,color[a->jackpot_reel[i]]);
            number(d,s,x+39,140,(int)a->jackpot_reel[i]+1,0x17283b);
        }
        label(d,s,75,213,"SCORE",0xe8f4ff);
        number(d,s,153,213,a->jackpot_score,0x4dc5e8);
        label(d,s,215,213,"SPINS",0xe8f4ff);
        number(d,s,292,213,10-a->jackpot_spins,0xf0c174);
        fill(d,s,88,255,200,49,0x326f87);
        label(d,s,157,270,"SPIN",0xe8f4ff);
    }else{
        label(d,s,113,48,"FIND THE KITTEN",0x4dc5e8);
        for(int i=0;i<GRID_W*GRID_H;i++){
            int x=37+(i%GRID_W)*20,y=70+(i/GRID_W)*20;
            fill(d,s,x,y,19,19,(i&1)?0x1c3041:0x24394c);
            if(a->kitten_board[i]==2){
                fill(d,s,x+7,y+7,6,6,0xdcbf75);
            }
            if(i==a->robot_pos){
                fill(d,s,x+3,y+3,14,14,0x4dc5e8);
                fill(d,s,x+6,y+7,3,3,0x0c1522);
                fill(d,s,x+12,y+7,3,3,0x0c1522);
            }
            if(a->state==OVER&&a->kitten_board[i]==1)
                fill(d,s,x+3,y+4,14,12,0xfa8c9d);
        }
        static const char* const notes[]={"","A TINY COMPASS","A LOST SOCK","A PAPER MOON","A SMALL RADIO","AN OLD TOASTER"};
        label(d,s,99,257,notes[a->robot_message],0xe8f4ff);
        static const char*const dirs[]={"LEFT","UP","DOWN","RIGHT"};
        for(int i=0;i<4;i++){
            int x=11+i*92;
            fill(d,s,x,285,88,31,a->robot_heading==(i==0?3:i==1?0:i==2?2:1)?0x397f93:0x22364a);
            label(d,s,x+10,292,dirs[i],0xe8f4ff);
        }
    }
}
