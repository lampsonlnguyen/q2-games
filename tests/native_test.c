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
    for(int i=0;i<20&&a.state==1;i++)app_tick(&a,17);
    app_render(&a,screen,SCREEN_H);
    for(unsigned i=0;i<SCREEN_W*SCREEN_H;i++)assert((screen[i]&0xff000000u)==0xff000000u);
    puts("Native Pong, Snake, BrickMania, Spacerocks and rendering tests passed");
    return 0;
}
