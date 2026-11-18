/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native, fixed-memory adaptations inspired by Rockbox sliding_puzzle.c
 * (Vicentini Martin, 2002) and minesweeper.c (Antoine Cellerier, 2004-2006).
 * Original Rockbox plugins are GPL-2.0-or-later; this code is GPL-3.0-or-later.
 */
#include "native_app.h"
#include "font.h"
enum { GAME=1, OVER=3, SLIDE=4, MINES=5 };
static unsigned random_next(NativeApp *a) {
    a->seed=a->seed*1664525u+1013904223u;
    return a->seed>>16;
}
static int neighbor(int a,int b,int width) {
    return (a/width==b/width && (a+1==b||b+1==a)) || a+width==b || b+width==a;
}
static void slide_swap(NativeApp *a,int i) {
    int old=a->puzzle_blank;
    a->tiles[old]=a->tiles[i];a->tiles[i]=0;a->puzzle_blank=i;
}
static void slide_begin(NativeApp *a) {
    for(int i=0;i<15;i++)a->tiles[i]=(unsigned char)(i+1);
    a->tiles[15]=0;a->puzzle_blank=15;a->puzzle_cursor=0;a->puzzle_moves=0;
    int previous=-1;
    for(int step=0;step<180;step++) {
        int choices[4],n=0;
        for(int i=0;i<16;i++)
            if(neighbor(i,a->puzzle_blank,4)&&i!=previous)choices[n++]=i;
        int old=a->puzzle_blank;
        slide_swap(a,choices[random_next(a)%(unsigned)n]);
        previous=old;
    }
}
static void slide_pick(NativeApp *a,int cell){
    if(cell<0||cell>=16||!neighbor(cell,a->puzzle_blank,4))return;
    slide_swap(a,cell);a->puzzle_moves++;
    for(int i=0;i<15;i++)if(a->tiles[i]!=(unsigned char)(i+1))return;
    if(a->tiles[15]==0)a->state=OVER;
}
static int mines_neighbors(const NativeApp *a,int i){
    int count=0,x=i%8,y=i/8;
    for(int yy=y-1;yy<=y+1;yy++)for(int xx=x-1;xx<=x+1;xx++)
        if(xx>=0&&xx<8&&yy>=0&&yy<8&&(xx!=x||yy!=y))
            count+=a->mines[yy*8+xx]!=0;
    return count;
}
static void mines_begin(NativeApp *a){
    for(int i=0;i<64;i++)a->mines[i]=a->flags[i]=a->revealed[i]=0;
    a->mine_cursor=0;a->mine_ready=0;a->mine_left=54;
}
static void mines_seed(NativeApp *a,int safe) {
    int planted=0;
    while(planted<10) {
        int i=(int)(random_next(a)%64);
        if(i==safe||a->mines[i])continue;
        a->mines[i]=1;planted++;
    }
    a->mine_ready=1;
}
static void mines_open(NativeApp *a,int cell){
    if(cell<0||cell>=64||a->flags[cell]||a->revealed[cell])return;
    if(!a->mine_ready)mines_seed(a,cell);
    if(a->mines[cell]){a->revealed[cell]=1;a->state=OVER;return;}
    int queue[64],head=0,tail=0;
    a->revealed[cell]=1;queue[tail++]=cell;a->mine_left--;
    while(head<tail){
        int i=queue[head++];
        if(mines_neighbors(a,i))continue;
        int x=i%8,y=i/8;
        for(int yy=y-1;yy<=y+1;yy++)for(int xx=x-1;xx<=x+1;xx++){
            if(xx<0||xx>=8||yy<0||yy>=8)continue;
            int j=yy*8+xx;
            if(!a->revealed[j]&&!a->flags[j]&&!a->mines[j]){
                a->revealed[j]=1;queue[tail++]=j;a->mine_left--;
            }
        }
    }
    if(a->mine_left==0)a->state=OVER;
}
void puzzle_begin(NativeApp *a){
    a->wheel_remainder=0;
    if(a->game==SLIDE)slide_begin(a);
    else mines_begin(a);
}
void puzzle_wheel(NativeApp *a,int delta){
    a->wheel_remainder-=delta;
    int *cursor=a->game==SLIDE?&a->puzzle_cursor:&a->mine_cursor;
    int n=a->game==SLIDE?16:64;
    while(a->wheel_remainder>=30){*cursor=(*cursor+1)%n;a->wheel_remainder-=30;}
    while(a->wheel_remainder<=-30){*cursor=(*cursor+n-1)%n;a->wheel_remainder+=30;}
}
void puzzle_button(NativeApp *a,int code){
    if(a->game==SLIDE){
        if(code==172||code==222)a->puzzle_cursor=(a->puzzle_cursor+15)%16;
        else if(code==173||code==223)a->puzzle_cursor=(a->puzzle_cursor+1)%16;
        else if(code==218)slide_pick(a,a->puzzle_cursor);
    }else{
        if(code==172||code==222)a->flags[a->mine_cursor]^=!a->revealed[a->mine_cursor];
        else if(code==173||code==223)a->mine_cursor=(a->mine_cursor+1)%64;
        else if(code==218)mines_open(a,a->mine_cursor);
    }
}
void puzzle_touch(NativeApp *a,int x,int y){
    if(a->game==SLIDE){
        if(x<43||x>=331||y<64||y>=280)return;
        int col=(x-43)/72,row=(y-64)/54;
        int cell=row*4+col;
        a->puzzle_cursor=cell;slide_pick(a,cell);
    }else{
        if(x<59||x>=315||y<65||y>=289)return;
        int col=(x-59)/32,row=(y-65)/28;
        int cell=row*8+col;
        a->mine_cursor=cell;mines_open(a,cell);
    }
}
static void box(u32 *d,unsigned s,int x,int y,int w,int h,u32 c){
    if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
    if(x+w>SCREEN_W)w=SCREEN_W-x;if(y+h>SCREEN_H)h=SCREEN_H-y;
    if(w<=0||h<=0)return;
    for(int xx=x;xx<x+w;xx++)for(int yy=y;yy<y+h;yy++)
        d[xx*s+SCREEN_H-1-yy]=c|0xff000000u;
}
static void text(u32*d,unsigned s,int x,int y,const char*t,u32 color){
    for(;*t;t++){
        unsigned ch=(unsigned char)*t;
        if(ch>='a'&&ch<='z')ch-=32;
        unsigned idx=ch>=32&&ch<=90?ch-32:0;
        int width=f18_adv[idx];
        for(int row=0;row<17;row++)for(int col=0;col<width;col++){
            unsigned alpha=f18_px[f18_off[idx]+row*width+col];
            int px=x+col,py=y+row;
            if(!alpha||px<0||py<0||px>=SCREEN_W||py>=SCREEN_H)continue;
            u32 *p=&d[px*s+SCREEN_H-1-py],old=*p;
            unsigned r=(((old>>16)&255)*(255-alpha)+((color>>16)&255)*alpha)/255;
            unsigned g=(((old>>8)&255)*(255-alpha)+((color>>8)&255)*alpha)/255;
            unsigned b=((old&255)*(255-alpha)+(color&255)*alpha)/255;
            *p=0xff000000u|(r<<16)|(g<<8)|b;
        }
        x+=width;
    }
}
static void number(u32*d,unsigned s,int x,int y,int value,u32 color){
    char buf[12];int n=0;
    do {buf[n++]=(char)('0'+value%10);value/=10;}while(value&&n<10);
    while(n--){char tmp[2]={buf[n],0};text(d,s,x,y,tmp,color);x+=f18_adv[buf[n]-32];}
}
void puzzle_render(const NativeApp *a,u32*d,unsigned stride){
    if(a->game==SLIDE){
        text(d,stride,120,47,"SLIDING PUZZLE",0x4dc5e8);
        for(int i=0;i<16;i++){
            int x=43+(i%4)*72,y=64+(i/4)*54;
            if(!a->tiles[i]){box(d,stride,x,y,69,51,0x111c2e);continue;}
            box(d,stride,x,y,69,51,i==a->puzzle_cursor?0x3c7590:0x293b53);
            if(i==a->puzzle_cursor)box(d,stride,x,y,69,2,0x9feaff);
            int n=a->tiles[i];number(d,stride,x+(n>9?24:30),y+17,n,0xe8f4ff);
        }
        text(d,stride,105,286,"MOVES",0xe8f4ff);
        number(d,stride,172,286,a->puzzle_moves,0x4dc5e8);
    }else{
        text(d,stride,125,46,"MINESWEEPER",0x4dc5e8);
        for(int i=0;i<64;i++){
            int x=59+(i%8)*32,y=65+(i/8)*28;
            u32 tile=a->revealed[i]?0x26364a:0x4d647e;
            if(i==a->mine_cursor)tile=0x26768f;
            box(d,stride,x,y,30,26,tile);
            if(a->revealed[i]){
                if(a->mines[i])text(d,stride,x+10,y+5,"X",0xff7869);
                else {int n=mines_neighbors(a,i);if(n)number(d,stride,x+10,y+5,n,0xe8f4ff);}
            }else if(a->flags[i])text(d,stride,x+10,y+5,"F",0xf4c46b);
        }
        text(d,stride,72,296,"PREV FLAG  CENTER OPEN",0xe8f4ff);
    }
}
