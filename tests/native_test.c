#include "native_app.h"
#include <assert.h>
#include <stdio.h>
static u32 screen[SCREEN_W*SCREEN_H];
int main(void) {
    NativeApp a;
    app_init(&a,123);
    assert(a.state==0&&!a.quit);
    assert(app_input(&a,"k u 218"));
    assert(a.state==1&&app_animated(&a));
    int old=a.paddle[1];
    assert(app_input(&a,"w -50"));
    assert(a.paddle[1]>old);
    for(int i=0;i<200;i++)app_tick(&a,17);
    assert(a.x!=0&&a.y>=PLAY_TOP*RES&&a.y<=PLAY_BOTTOM*RES);
    assert(app_input(&a,"k u 170")&&a.state==2);
    assert(!app_animated(&a));
    assert(app_input(&a,"k u 218")&&a.state==1);
    assert(app_input(&a,"k u 170")&&a.state==2);
    assert(app_input(&a,"k u 170")&&a.state==0);
    assert(app_input(&a,"k u 170")&&a.quit);
    app_init(&a,456);
    assert(app_input(&a,"w -50")&&a.choice==1);
    assert(app_input(&a,"k u 218")&&a.state==1&&a.game==1);
    assert(a.length==4&&a.food>=0);
    int oldhead=a.head;
    app_tick(&a,200);
    assert(a.head!=oldhead);
    assert(app_input(&a,"k u 172"));
    assert(a.queued==0);
    app_tick(&a,200);
    assert(a.dir==0);
    assert(app_input(&a,"k u 170")&&a.state==2);
    assert(app_input(&a,"k u 170")&&a.state==0);
    assert(app_input(&a,"k u 218")&&a.game==1);
    app_init(&a,321);
    app_render(&a,screen,SCREEN_H);
    assert((screen[0]&0xff000000u)==0xff000000u);
    assert((screen[(SCREEN_W/2)*SCREEN_H+SCREEN_H-1-130]&0xff000000u)==0xff000000u);
    unsigned lit=0;
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++) {
        assert((screen[i]&0xff000000u)==0xff000000u);
        if((screen[i]&0x00ffffffu)!=0x000c1522u)lit++;
    }
    assert(lit>100);
    /* Menu text must occupy multiple contiguous pixel rows, not the old
     * malformed 3x5 glyph lookup's fragmentary marks. */
    unsigned ink=0;
    for(int x=60;x<310;x++)for(int y=15;y<42;y++)
        if((screen[x*SCREEN_H+SCREEN_H-1-y]&0x00ffffffu)!=0x000c1522u)ink++;
    assert(ink>400);
    app_init(&a,789);
    assert(app_input(&a,"k u 173")&&a.choice==1);
    assert(app_input(&a,"k u 173")&&a.choice==2);
    assert(app_input(&a,"k u 218")&&a.game==2&&a.brick_lives==3);
    int paddle=a.brick_paddle;
    assert(app_input(&a,"w -60")&&a.brick_paddle!=paddle);
    for(int i=0;i<80&&a.state==1;i++)app_tick(&a,17);
    app_init(&a,222);
    for(int i=0;i<3;i++)assert(app_input(&a,"k u 173"));
    assert(a.choice==3);
    assert(app_input(&a,"k u 218")&&a.game==3&&a.space_lives==3);
    int angle=a.ship_angle;
    assert(app_input(&a,"w -60")&&a.ship_angle!=angle);
    assert(app_input(&a,"k u 218"));
    assert(app_input(&a,"k u 173"));
    int startx=a.ship_x,starty=a.ship_y;
    for(int i=0;i<20&&a.state==1;i++)app_tick(&a,17);
    assert(a.ship_x!=startx||a.ship_y!=starty);
    app_render(&a,screen,SCREEN_H);
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++)assert((screen[i]&0xff000000u)==0xff000000u);
    app_init(&a,901);
    for(int i=0;i<4;i++)assert(app_input(&a,"k u 173"));
    assert(a.choice==4);
    assert(app_input(&a,"k u 218")&&a.game==4);
    assert(!app_animated(&a));
    assert(a.tiles[a.puzzle_blank]==0);
    int blank=a.puzzle_blank;
    for(int i=0;i<16;i++)if((i/4==blank/4&&(i+1==blank||blank+1==i))||i+4==blank||blank+4==i) {
        a.puzzle_cursor=i;break;
    }
    assert(app_input(&a,"k u 218")&&a.puzzle_blank!=blank&&a.puzzle_moves==1);
    app_render(&a,screen,SCREEN_H);
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++)assert((screen[i]&0xff000000u)==0xff000000u);
    app_init(&a,902);
    for(int i=0;i<5;i++)assert(app_input(&a,"k u 173"));
    assert(a.choice==5);
    assert(app_input(&a,"k u 218")&&a.game==5&&!a.mine_ready);
    assert(app_input(&a,"k u 218")&&a.mine_ready&&a.revealed[0]&&!a.mines[0]);
    int count=0;
    for(int i=0;i<64;i++)count+=a.mines[i]!=0;
    assert(count==10);
    app_render(&a,screen,SCREEN_H);
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++)assert((screen[i]&0xff000000u)==0xff000000u);
    app_init(&a,781);
    for(int i=0;i<6;i++)assert(app_input(&a,"k u 173"));
    assert(a.choice==6);
    assert(app_input(&a,"k u 218")&&a.game==6);
    assert(a.reversi_player==2&&a.reversi_cpu==2);
    a.reversi_cursor=19;
    assert(app_input(&a,"k u 218"));
    assert(a.reversi_player+a.reversi_cpu>=5);
    app_render(&a,screen,SCREEN_H);
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++)assert((screen[i]&0xff000000u)==0xff000000u);
    app_init(&a,1111);
    for(int i=0;i<7;i++)assert(app_input(&a,"k u 173"));
    assert(a.choice==7);
    assert(app_input(&a,"k u 218")&&a.game==7&&!app_animated(&a));
    int flipcount=0;
    for(int i=0;i<25;i++)flipcount+=a.flips[i]!=0;
    assert(flipcount>0);
    int flipmoves=a.small_steps;
    assert(app_input(&a,"k u 218")&&a.small_steps==flipmoves+1);
    app_render(&a,screen,SCREEN_H);
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++)assert((screen[i]&0xff000000u)==0xff000000u);
    app_init(&a,2222);
    for(int i=0;i<8;i++)assert(app_input(&a,"k u 173"));
    assert(a.choice==8);
    assert(app_input(&a,"k u 218")&&a.game==8&&!app_animated(&a));
    int filled=0;for(int i=0;i<16;i++)filled+=a.powers[i]!=0;
    assert(filled==2);
    assert(app_input(&a,"k u 218"));
    filled=0;for(int i=0;i<16;i++)filled+=a.powers[i]!=0;
    assert(filled>=2);
    app_render(&a,screen,SCREEN_H);
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++)assert((screen[i]&0xff000000u)==0xff000000u);
    app_init(&a,335);
    assert(app_input(&a,"w -13")&&a.choice==0);
    assert(app_input(&a,"w -16")&&a.choice==1);
    assert(app_input(&a,"k d 218")&&a.game==1);
    assert(!app_input(&a,"k d 218"));
    assert(!app_input(&a,"k u 218"));
    assert(a.game==1&&a.state==1);
    assert(app_input(&a,"k u 170")&&a.state==2);
    assert(app_input(&a,"k d 218")&&a.state==1);
    assert(!app_input(&a,"k u 218"));
    app_init(&a,445);
    assert(app_input(&a,"t d 75 114")&&a.game==0);
    assert(!app_input(&a,"t u 75 114"));
    app_init(&a,667);
    for(int i=0;i<9;i++)assert(app_input(&a,"k u 173"));
    assert(a.choice==9);
    assert(app_input(&a,"k d 218")&&a.game==9);
    assert(!app_input(&a,"k u 218"));
    assert(a.maze_pos==0);
    /* Every generated maze has a route between the entrance and exit. */
    int queue[165]={0},seen[165]={0},n=1,first=0;queue[0]=0;seen[0]=1;
    while(first<n){
        int cell=queue[first++],x=cell%15,y=cell/15;
        const int off[4]={-15,1,15,-1};
        const int bits[4]={1,2,4,8};
        for(int dir=0;dir<4;dir++){
            int other=cell+off[dir];
            if(!(a.maze_cells[cell]&bits[dir]))continue;
            if(other<0||other>=165)continue;
            if((dir==1&&x==14)||(dir==3&&x==0))continue;
            if((dir==0&&y==0)||(dir==2&&y==10))continue;
            if(!seen[other]){seen[other]=1;queue[n++]=other;}
        }
    }
    assert(seen[164]);
    app_init(&a,668);
    for(int i=0;i<10;i++)assert(app_input(&a,"k u 173"));
    assert(app_input(&a,"k u 218")&&a.game==10);
    assert(a.code_attempt==0);
    a.code_cursor=4;
    assert(app_input(&a,"k u 218")&&a.code_attempt==1);
    app_init(&a,669);
    for(int i=0;i<11;i++)assert(app_input(&a,"k u 173"));
    assert(app_input(&a,"k u 218")&&a.game==11);
    assert(a.dice_rolls==1);
    a.dice_cursor=5;
    assert(app_input(&a,"k u 218")&&a.dice_rolls==2);
    app_render(&a,screen,SCREEN_H);
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++)assert((screen[i]&0xff000000u)==0xff000000u);
    puts("Native 12-game logic and Q2 Arcade/Flappybird input tests passed");
    return 0;
}
