/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native compact adaptations of Rockbox Flipit (Vicentini Martin, 2002)
 * and 2048 (Franklin Wei, 2014; original concept Gabriele Cirulli).
 * GPL-2.0-or-later upstream; this implementation GPL-3.0-or-later.
 * 2048 stores tile exponents (2^n), avoiding large tile allocations.
 */
#include "native_app.h"
#include "font.h"
enum { OVER=3, FLIP=7, TWENTY48=8 };
static unsigned next(NativeApp*a){a->seed=a->seed*1664525u+1013904223u;return a->seed>>16;}
static void toggle(NativeApp*a,int i){
    if(i<0||i>=25)return;
    int x=i%5,y=i/5;
    a->flips[i]^=1;
    if(x>0)a->flips[i-1]^=1;
    if(x<4)a->flips[i+1]^=1;
    if(y>0)a->flips[i-5]^=1;
    if(y<4)a->flips[i+5]^=1;
}
static void spawn(NativeApp*a){
    int empty[16],count=0;
    for(int i=0;i<16;i++)if(!a->powers[i])empty[count++]=i;
    if(count){
        int cell=empty[next(a)%(unsigned)count];
        a->powers[cell]=(next(a)%10==0)?2:1;
    }
}
void small_begin(NativeApp*a){
    a->wheel_remainder=0;a->small_steps=0;a->small_cursor=0;
    if(a->game==FLIP){
        for(int i=0;i<25;i++)a->flips[i]=0;
        for(int n=0;n<28;n++)toggle(a,(int)(next(a)%25));
        a->small_cursor=12;
    } else {
        for(int i=0;i<16;i++)a->powers[i]=0;
        a->small_score=0;spawn(a);spawn(a);
    }
}
static int has_moves(const NativeApp*a){
    for(int i=0;i<16;i++){
        if(!a->powers[i])return 1;
        if(i%4!=3&&a->powers[i]==a->powers[i+1])return 1;
        if(i/4!=3&&a->powers[i]==a->powers[i+4])return 1;
    }
    return 0;
}
/* dir: 0 left, 1 up, 2 right, 3 down; compress then merge once. */
static void slide(NativeApp*a,int dir){
    int moved=0;
    for(int lane=0;lane<4;lane++){
        unsigned char old[4],buf[4]={0},out[4]={0};
        for(int j=0;j<4;j++){
            int row=(dir==0||dir==2)?lane:(dir==1?j:3-j);
            int col=(dir==1||dir==3)?lane:(dir==0?j:3-j);
            old[j]=a->powers[row*4+col];
        }
        int n=0;
        for(int j=0;j<4;j++)if(old[j])buf[n++]=old[j];
        int w=0;
        for(int j=0;j<n;j++){
            if(j+1<n&&buf[j]==buf[j+1]){
                unsigned char e=buf[j]+1;
                out[w++]=e;
                if(e<30)a->small_score+=1<<e;
                j++;
            }else out[w++]=buf[j];
        }
        for(int j=0;j<4;j++){
            int row=(dir==0||dir==2)?lane:(dir==1?j:3-j);
            int col=(dir==1||dir==3)?lane:(dir==0?j:3-j);
            if(a->powers[row*4+col]!=out[j])moved=1;
            a->powers[row*4+col]=out[j];
        }
    }
    if(moved){a->small_steps++;spawn(a);}
    if(!has_moves(a))a->state=OVER;
}
void small_wheel(NativeApp*a,int amount){
    a->wheel_remainder-=amount;
    int mod=a->game==FLIP?25:4;
    while(a->wheel_remainder>=40){a->small_cursor=(a->small_cursor+1)%mod;a->wheel_remainder-=40;}
    while(a->wheel_remainder<=-40){a->small_cursor=(a->small_cursor+mod-1)%mod;a->wheel_remainder+=40;}
}
void small_button(NativeApp*a,int key){
    int mod=a->game==FLIP?25:4;
    if(key==172||key==222)a->small_cursor=(a->small_cursor+mod-1)%mod;
    else if(key==173||key==223)a->small_cursor=(a->small_cursor+1)%mod;
    else if(key==218){
        if(a->game==FLIP){
            toggle(a,a->small_cursor);a->small_steps++;
            int on=0;for(int i=0;i<25;i++)on+=a->flips[i]!=0;
            if(!on)a->state=OVER;
        } else slide(a,a->small_cursor);
    }
}
void small_touch(NativeApp*a,int x,int y){
    if(a->game==FLIP){
        if(x<55||x>=320||y<69||y>=284)return;
        int col=(x-55)/53,row=(y-69)/43;
        a->small_cursor=row*5+col;small_button(a,218);
    }else{
        /* Four large directional buttons below the 4x4 board: no swipe parser. */
        if(y>=286&&y<318&&x>=31&&x<343){
            a->small_cursor=(x-31)/78;
            if(a->small_cursor>3)a->small_cursor=3;
            slide(a,a->small_cursor);
        }
    }
}
static void box(u32*d,unsigned stride,int x,int y,int w,int h,u32 color){
    if(x<0){w+=x;x=0;}
    if(y<0){h+=y;y=0;}
    if(x+w>SCREEN_W)w=SCREEN_W-x;
    if(y+h>SCREEN_H)h=SCREEN_H-y;
    if(w<=0||h<=0)return;
    for(int xx=x;xx<x+w;xx++)for(int yy=y;yy<y+h;yy++)
        d[xx*stride+SCREEN_H-1-yy]=0xff000000u|color;
}
static void label(u32*d,unsigned stride,int x,int y,const char*str,u32 color){
    for(;*str;str++){
        unsigned c=(unsigned char)*str;
        if(c>='a'&&c<='z')c-=32;
        unsigned k=c>=32&&c<=90?c-32:0;
        int width=f18_adv[k];
        for(int row=0;row<17;row++)for(int col=0;col<width;col++){
            unsigned alpha=f18_px[f18_off[k]+row*width+col];
            int xx=x+col,yy=y+row;
            if(!alpha||xx<0||xx>=SCREEN_W||yy<0||yy>=SCREEN_H)continue;
            u32 *at=&d[xx*stride+SCREEN_H-1-yy],bg=*at;
            unsigned r=(((bg>>16)&255u)*(255-alpha)+((color>>16)&255u)*alpha)/255u;
            unsigned g=(((bg>>8)&255u)*(255-alpha)+((color>>8)&255u)*alpha)/255u;
            unsigned b=((bg&255u)*(255-alpha)+(color&255u)*alpha)/255u;
            *at=0xff000000u|(r<<16)|(g<<8)|b;
        }
        x+=width;
    }
}
static void number(u32*d,unsigned s,int x,int y,unsigned value,u32 color){
    char b[12];int n=0;
    do{b[n++]=(char)('0'+value%10);value/=10;}while(value&&n<10);
    while(n--){char digit[2]={b[n],0};label(d,s,x,y,digit,color);x+=f18_adv[b[n]-32];}
}
void small_render(const NativeApp*a,u32*d,unsigned stride){
    if(a->game==FLIP){
        label(d,stride,162,48,"FLIPIT",0x4dc5e8);
        for(int i=0;i<25;i++){
            int x=55+(i%5)*53,y=69+(i/5)*43;
            box(d,stride,x,y,50,40,a->flips[i]?0x5ea9bf:0x26374d);
            if(i==a->small_cursor){box(d,stride,x,y,50,2,0xf8d56d);box(d,stride,x,y,2,40,0xf8d56d);}
            if(a->flips[i])box(d,stride,x+13,y+8,24,24,0xb9e4ee);
        }
        label(d,stride,102,290,"MOVES",0xe8f4ff);
        number(d,stride,175,290,(unsigned)a->small_steps,0x4dc5e8);
    }else{
        label(d,stride,171,48,"2048",0x4dc5e8);
        static const u32 colors[]={0x202d3e,0x526d85,0x4c98a5,0x59ada2,0xd0ac6c,0xc98961,0xc66c71,0x9578a7};
        for(int i=0;i<16;i++){
            int x=55+(i%4)*67,y=68+(i/4)*53;
            unsigned v=a->powers[i];
            box(d,stride,x,y,64,50,colors[v>7?7:v]);
            if(v){
                unsigned num=v<30?1u<<v:0;
                number(d,stride,x+(num>=1000?12:num>=100?17:num>=10?23:29),y+16,num,0xffffff);
            }
        }
        static const char*dir[]={"LEFT","UP","RIGHT","DOWN"};
        for(int i=0;i<4;i++){
            int x=31+i*78;
            box(d,stride,x,286,75,30,i==a->small_cursor?0x347f97:0x243b51);
            label(d,stride,x+10,292,dir[i],0xe8f4ff);
        }
    }
}
