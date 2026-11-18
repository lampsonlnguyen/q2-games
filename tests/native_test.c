#include "native_app.h"
#include <assert.h>
#include <stdio.h>
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
    puts("Native Pong input and physics tests passed");
    return 0;
}
