/* SPDX-License-Identifier: GPL-3.0-or-later
 * Compact independent implementations of three small Rockbox game concepts:
 * Maze (maze.c), Codebuster (codebuster.c), Dice (dice.c).
 * Uses only fixed arrays and integers, no Rockbox plugin runtime.
 */
#include "native_app.h"
#include "font.h"
enum {GAME=1,OVER=3,MAZE=9,CODEBUSTER=10,DICE=11,MAZE_W=15,MAZE_H=11};
static unsigned rand32(NativeApp*a) {a->seed=a->seed*1664525u+1013904223u;return a->seed>>16;}
static const int sx[4]={0,1,0,-1},sy[4]={-1,0,1,0};
static const unsigned char bit[4]={1,2,4,8};
static void maze_begin(NativeApp*a){
    unsigned char seen[MAZE_W*MAZE_H]={0};
    unsigned char stack[MAZE_W*MAZE_H];
    for(int i=0;i<MAZE_W*MAZE_H;i++)a->maze_cells[i]=0;
    int top=0;stack[0]=0;seen[0]=1;
    while(top>=0){
        int cell=stack[top],x=cell%MAZE_W,y=cell/MAZE_W;
        int opts[4],n=0;
        for(int d=0;d<4;d++){
            int xx=x+sx[d],yy=y+sy[d];
            if(xx>=0&&xx<MAZE_W&&yy>=0&&yy<MAZE_H&&!seen[yy*MAZE_W+xx])opts[n++]=d;
        }
        if(!n){top--;continue;}
        int dir=opts[rand32(a)%(unsigned)n];
        int other=(y+sy[dir])*MAZE_W+x+sx[dir];
        a->maze_cells[cell]|=bit[dir];
        a->maze_cells[other]|=bit[(dir+2)%4];
        seen[other]=1;stack[++top]=(unsigned char)other;
    }
    a->maze_pos=0;a->maze_dir=1;a->maze_steps=0;a->wheel_remainder=0;
}
static void maze_move(NativeApp*a){
    int current=a->maze_pos,dir=a->maze_dir;
    if(!(a->maze_cells[current]&bit[dir]))return;
    a->maze_pos+=sx[dir]+sy[dir]*MAZE_W;
    a->maze_steps++;
    if(a->maze_pos==MAZE_W*MAZE_H-1)a->state=OVER;
}
static void code_begin(NativeApp*a){
    a->code_cursor=0;a->code_attempt=0;a->wheel_remainder=0;
    for(int i=0;i<4;i++){a->code_secret[i]=1+(unsigned char)(rand32(a)%6);a->code_guess[i]=1;}
    for(int i=0;i<8;i++){a->code_exact[i]=0;a->code_partial[i]=0;}
}
static void code_submit(NativeApp*a){
    if(a->code_attempt>=8){a->state=OVER;return;}
    int exact=0,partial=0,secret[7]={0},guess[7]={0};
    for(int i=0;i<4;i++){
        int x=a->code_secret[i],y=a->code_guess[i];
        a->code_history[a->code_attempt][i]=a->code_guess[i];
        if(x==y)exact++;
        else{secret[x]++;guess[y]++;}
    }
    for(int value=1;value<=6;value++)
        partial+=secret[value]<guess[value]?secret[value]:guess[value];
    a->code_exact[a->code_attempt]=(unsigned char)exact;
    a->code_partial[a->code_attempt]=(unsigned char)partial;
    a->code_attempt++;
    if(exact==4||a->code_attempt==8)a->state=OVER;
}
static void dice_roll(NativeApp*a){
    if(a->dice_rolls>=3)return;
    for(int i=0;i<5;i++)if(!a->dice_held[i])
        a->dice_value[i]=1+(unsigned char)(rand32(a)%6);
    a->dice_rolls++;
    if(a->dice_rolls==3) {
        int count[7]={0},best=0;
        for(int i=0;i<5;i++)count[a->dice_value[i]]++;
        for(int i=1;i<=6;i++)if(count[i]>best)best=count[i];
        a->dice_score+=best*best;
        a->dice_round++;
        if(a->dice_round==5){a->state=OVER;return;}
        a->dice_rolls=0;
        for(int i=0;i<5;i++)a->dice_held[i]=0;
    }
}
static void dice_begin(NativeApp*a){
    a->dice_cursor=0;a->dice_round=0;a->dice_score=0;a->dice_rolls=0;
    a->wheel_remainder=0;
    for(int i=0;i<5;i++){a->dice_held[i]=0;a->dice_value[i]=1;}
    dice_roll(a);
}
void tiny_begin(NativeApp*a){
    if(a->game==MAZE)maze_begin(a);
    else if(a->game==CODEBUSTER)code_begin(a);
    else dice_begin(a);
}
void tiny_wheel(NativeApp*a,int movement){
    a->wheel_remainder-=movement;
    int *cursor=a->game==MAZE?&a->maze_dir:a->game==CODEBUSTER?&a->code_cursor:&a->dice_cursor;
    int n=a->game==MAZE?4:a->game==CODEBUSTER?5:6;
    while(a->wheel_remainder>=28){*cursor=(*cursor+1)%n;a->wheel_remainder-=28;}
    while(a->wheel_remainder<=-28){*cursor=(*cursor+n-1)%n;a->wheel_remainder+=28;}
}
void tiny_button(NativeApp*a,int key){
    int *cursor=a->game==MAZE?&a->maze_dir:a->game==CODEBUSTER?&a->code_cursor:&a->dice_cursor;
    int n=a->game==MAZE?4:a->game==CODEBUSTER?5:6;
    if(key==172||key==222){*cursor=(*cursor+n-1)%n;return;}
    if(key==173||key==223){*cursor=(*cursor+1)%n;return;}
    if(key!=218)return;
    if(a->game==MAZE)maze_move(a);
    else if(a->game==CODEBUSTER) {
        if(a->code_cursor==4)code_submit(a);
        else a->code_guess[a->code_cursor]=a->code_guess[a->code_cursor]%6+1;
    } else if(a->dice_cursor==5)dice_roll(a);
    else a->dice_held[a->dice_cursor]^=1;
}
void tiny_touch(NativeApp*a,int x,int y){
    if(a->game==MAZE){
        if(y>=282){
            a->maze_dir=(x<94?3:x<187?0:x<280?2:1);
            maze_move(a);
        }
    }else if(a->game==CODEBUSTER){
        if(y>=268){a->code_cursor=4;code_submit(a);}
        else if(y>=205&&y<265&&x>=45&&x<333) {
            a->code_cursor=(x-45)/72;
            a->code_guess[a->code_cursor]=a->code_guess[a->code_cursor]%6+1;
        }
    }else{
        if(y>258){a->dice_cursor=5;dice_roll(a);}
        else if(y>=112&&y<200&&x>=33&&x<343){
            a->dice_cursor=(x-33)/62;
            a->dice_held[a->dice_cursor]^=1;
        }
    }
}
static void rect(u32*d,unsigned s,int x,int y,int w,int h,u32 color){
    if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
    if(x+w>SCREEN_W)w=SCREEN_W-x;
    if(y+h>SCREEN_H)h=SCREEN_H-y;
    if(w<=0||h<=0)return;
    for(int xx=x;xx<x+w;xx++)for(int yy=y;yy<y+h;yy++)
        d[xx*s+SCREEN_H-1-yy]=color|0xff000000u;
}
static void word(u32*d,unsigned s,int x,int y,const char*t,u32 c){
    for(;*t;t++){
        unsigned ch=(unsigned char)*t;
        if(ch>='a'&&ch<='z')ch-=32;
        unsigned idx=ch>=32&&ch<=90?ch-32:0,wd=f18_adv[idx];
        for(int row=0;row<17;row++)for(unsigned col=0;col<wd;col++){
            unsigned alpha=f18_px[f18_off[idx]+row*wd+col];
            int px=x+(int)col,py=y+row;
            if(!alpha||px<0||px>=SCREEN_W||py<0||py>=SCREEN_H)continue;
            u32*at=&d[px*s+SCREEN_H-1-py],old=*at;
            unsigned r=(((old>>16)&255)*(255-alpha)+((c>>16)&255)*alpha)/255;
            unsigned g=(((old>>8)&255)*(255-alpha)+((c>>8)&255)*alpha)/255;
            unsigned b=((old&255)*(255-alpha)+(c&255)*alpha)/255;
            *at=0xff000000u|(r<<16)|(g<<8)|b;
        }
        x+=(int)wd;
    }
}
static void number(u32*d,unsigned s,int x,int y,int v,u32 c){
    char b[12];int n=0;
    do{b[n++]=(char)('0'+v%10);v/=10;}while(v&&n<10);
    while(n--){char p[2]={b[n],0};word(d,s,x,y,p,c);x+=f18_adv[b[n]-32];}
}
void tiny_render(const NativeApp*a,u32*d,unsigned s){
    if(a->game==MAZE){
        word(d,s,169,47,"MAZE",0x4dc5e8);
        for(int cell=0;cell<MAZE_W*MAZE_H;cell++){
            int x=37+(cell%MAZE_W)*20,y=60+(cell/MAZE_W)*20;
            unsigned walls=a->maze_cells[cell];
            rect(d,s,x,y,20,20,0x26374a);
            if(!(walls&1))rect(d,s,x,y,20,2,0xe2e8f1);
            if(!(walls&8))rect(d,s,x,y,2,20,0xe2e8f1);
            if(!(walls&2))rect(d,s,x+18,y,2,20,0xe2e8f1);
            if(!(walls&4))rect(d,s,x,y+18,20,2,0xe2e8f1);
        }
        rect(d,s,37+14*20+6,60+10*20+6,8,8,0x51dca2);
        rect(d,s,37+(a->maze_pos%MAZE_W)*20+6,60+(a->maze_pos/MAZE_W)*20+6,8,8,0xffb871);
        static const char* const dirs[]={"UP","RIGHT","DOWN","LEFT"};
        for(int i=0;i<4;i++){
            int x=12+i*92;
            rect(d,s,x,287,87,30,i==a->maze_dir?0x336d88:0x17283b);
            word(d,s,x+8,295,dirs[i],0xe8f4ff);
        }
    } else if(a->game==CODEBUSTER){
        word(d,s,135,48,"CODEBUSTER",0x4dc5e8);
        for(int row=0;row<a->code_attempt;row++){
            int y=70+row*17;
            for(int col=0;col<4;col++)
                number(d,s,68+col*35,y,a->code_history[row][col],0xe8f4ff);
            word(d,s,231,y,"E",0x51dca2);
            number(d,s,245,y,a->code_exact[row],0x51dca2);
            word(d,s,276,y,"P",0xfabf71);
            number(d,s,290,y,a->code_partial[row],0xfabf71);
        }
        for(int col=0;col<4;col++){
            int x=45+col*72,y=206;
            rect(d,s,x,y,67,54,a->code_cursor==col?0x346f8a:0x27394e);
            number(d,s,x+28,y+17,a->code_guess[col],0xe8f4ff);
        }
        rect(d,s,100,272,175,35,a->code_cursor==4?0x397f93:0x293b53);
        word(d,s,157,281,"CHECK",0xe8f4ff);
    } else {
        word(d,s,165,50,"DICE",0x4dc5e8);
        word(d,s,91,79,"ROUND",0xe8f4ff);number(d,s,162,79,a->dice_round+1,0x4dc5e8);
        word(d,s,224,79,"ROLL",0xe8f4ff);number(d,s,273,79,a->dice_rolls,0x4dc5e8);
        for(int i=0;i<5;i++){
            int x=33+i*62;
            rect(d,s,x,112,58,85,a->dice_cursor==i?0x37738d:0x293b53);
            number(d,s,x+23,140,a->dice_value[i],0xe8f4ff);
            if(a->dice_held[i])word(d,s,x+7,174,"HOLD",0xf9d47a);
        }
        word(d,s,126,225,"SCORE",0xe8f4ff);
        number(d,s,205,225,a->dice_score,0x4dc5e8);
        rect(d,s,92,268,191,40,a->dice_cursor==5?0x327a8e:0x27394e);
        word(d,s,151,279,"ROLL",0xe8f4ff);
    }
}
