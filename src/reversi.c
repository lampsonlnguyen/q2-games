/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native Reversi rules and basic computer opponent, adapted in gameplay from
 * Rockbox apps/plugins/reversi (GPL-2.0-or-later). No Rockbox runtime.
 */
#include "native_app.h"
#include "font.h"
enum {OVER=3};
static int playable(const NativeApp*a,int pos,int who,int capture){
    if(pos<0||pos>=64||a->reversi[pos])return 0;
    const int dirx[]={-1,0,1,-1,1,-1,0,1};
    const int diry[]={-1,-1,-1,0,0,1,1,1};
    int count=0,x=pos%8,y=pos/8;
    for(int d=0;d<8;d++){
        int xx=x+dirx[d],yy=y+diry[d],found=0;
        while(xx>=0&&xx<8&&yy>=0&&yy<8&&a->reversi[yy*8+xx]==3-who){
            found++;xx+=dirx[d];yy+=diry[d];
        }
        if(found&&xx>=0&&xx<8&&yy>=0&&yy<8&&a->reversi[yy*8+xx]==who) {
            count+=found;
            if(capture)for(int j=1;j<=found;j++) {
                int px=x+j*dirx[d],py=y+j*diry[d];
                /* Mutable board supplied by the caller after validating. */
                ((NativeApp*)a)->reversi[py*8+px]=(unsigned char)who;
            }
        }
    }
    if(count&&capture)((NativeApp*)a)->reversi[pos]=(unsigned char)who;
    return count;
}
static int has_move(const NativeApp*a,int who){
    for(int i=0;i<64;i++)if(playable(a,i,who,0))return 1;
    return 0;
}
static void score(NativeApp*a){
    a->reversi_player=a->reversi_cpu=0;
    for(int i=0;i<64;i++){
        a->reversi_player+=a->reversi[i]==1;
        a->reversi_cpu+=a->reversi[i]==2;
    }
}
void reversi_begin(NativeApp*a){
    for(int i=0;i<64;i++)a->reversi[i]=0;
    a->reversi[3*8+3]=2;a->reversi[4*8+4]=2;
    a->reversi[3*8+4]=1;a->reversi[4*8+3]=1;
    a->reversi_cursor=19;a->wheel_remainder=0;score(a);
}
static void reversi_pick(NativeApp*a,int cell){
    if(!playable(a,cell,1,0))return;
    playable(a,cell,1,1);
    if(has_move(a,2)){
        int best=-1000,move=-1;
        for(int i=0;i<64;i++){
            int captured=playable(a,i,2,0);
            if(!captured)continue;
            int edge=((i%8==0)||(i%8==7)||(i/8==0)||(i/8==7));
            int corner=((i%8==0||i%8==7)&&(i/8==0||i/8==7));
            int score0=captured*4+edge*8+corner*100;
            if(score0>best){best=score0;move=i;}
        }
        if(move>=0)playable(a,move,2,1);
    }
    score(a);
    if(!has_move(a,1)&&!has_move(a,2))a->state=OVER;
}
void reversi_wheel(NativeApp*a,int delta){
    a->wheel_remainder-=delta;
    while(a->wheel_remainder>=30){a->reversi_cursor=(a->reversi_cursor+1)%64;a->wheel_remainder-=30;}
    while(a->wheel_remainder<=-30){a->reversi_cursor=(a->reversi_cursor+63)%64;a->wheel_remainder+=30;}
}
void reversi_button(NativeApp*a,int code){
    if(code==172||code==222)a->reversi_cursor=(a->reversi_cursor+63)%64;
    else if(code==173||code==223)a->reversi_cursor=(a->reversi_cursor+1)%64;
    else if(code==218)reversi_pick(a,a->reversi_cursor);
}
void reversi_touch(NativeApp*a,int x,int y){
    if(x<59||x>=315||y<65||y>=289)return;
    a->reversi_cursor=((y-65)/28)*8+(x-59)/32;
    reversi_pick(a,a->reversi_cursor);
}
static void box(u32*d,unsigned s,int x,int y,int w,int h,u32 color){
    if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}
    if(x+w>SCREEN_W)w=SCREEN_W-x;
    if(y+h>SCREEN_H)h=SCREEN_H-y;
    if(w<=0||h<=0)return;
    for(int xx=x;xx<x+w;xx++)for(int yy=y;yy<y+h;yy++)d[xx*s+SCREEN_H-1-yy]=0xff000000u|color;
}
static void label(u32*d,unsigned s,int x,int y,const char*t,u32 c){
    for(;*t;t++){
        int glyph=(unsigned char)*t-32;
        if(glyph<0||glyph>58)glyph=0;
        int width=f18_adv[glyph];
        for(int yy=0;yy<17;yy++)for(int xx=0;xx<width;xx++){
            unsigned alpha=f18_px[f18_off[glyph]+yy*width+xx];
            int px=x+xx,py=y+yy;
            if(!alpha||px<0||py<0||px>=SCREEN_W||py>=SCREEN_H)continue;
            u32*at=&d[px*s+SCREEN_H-1-py],old=*at;
            unsigned rr=(((old>>16)&255u)*(255-alpha)+((c>>16)&255u)*alpha)/255u;
            unsigned gg=(((old>>8)&255u)*(255-alpha)+((c>>8)&255u)*alpha)/255u;
            unsigned bb=((old&255u)*(255-alpha)+(c&255u)*alpha)/255u;
            *at=0xff000000u|(rr<<16)|(gg<<8)|bb;
        }
        x+=width;
    }
}
static void number(u32*d,unsigned s,int x,int y,int n,u32 color){
    char b[4];int count=0;
    do{b[count++]=(char)('0'+n%10);n/=10;}while(n&&count<3);
    while(count--){char t[2]={b[count],0};label(d,s,x,y,t,color);x+=f18_adv[b[count]-32];}
}
void reversi_render(const NativeApp*a,u32*d,unsigned s){
    label(d,s,148,48,"REVERSI",0x4dc5e8);
    for(int i=0;i<64;i++){
        int x=59+(i%8)*32,y=65+(i/8)*28;
        box(d,s,x,y,30,26,i==a->reversi_cursor?0x50b5a3:0x245347);
        int piece=a->reversi[i];
        if(piece){
            box(d,s,x+7,y+5,17,17,piece==1?0xf2f2e9:0x152333);
            box(d,s,x+9,y+7,13,13,piece==1?0xffffff:0x0a1019);
        } else if(playable(a,i,1,0)){
            box(d,s,x+13,y+11,5,5,0x85bcb3);
        }
    }
    label(d,s,98,295,"YOU",0xe8f4ff);
    number(d,s,145,295,a->reversi_player,0x4dc5e8);
    label(d,s,213,295,"CPU",0xe8f4ff);
    number(d,s,262,295,a->reversi_cpu,0x4dc5e8);
}
