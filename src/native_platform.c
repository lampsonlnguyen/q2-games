/* SPDX-License-Identifier: GPL-3.0-or-later
 * Q2 card runtime using the Linux/MIPS system-call ABI and framebuffer contract.
 * Startup/syscall pattern adapted from DiamondBond's Buddy/q2-fidget card.
 */
#include "native_app.h"
long sc(long n,long a,long b,long c,long d,long e,long f);
__asm__(".text\n.set noreorder\n.globl sc\nsc:\n"
        "move $2, $4\nmove $4, $5\nmove $5, $6\nmove $6, $7\n"
        "lw $7, 16($29)\nlw $8, 20($29)\nlw $9, 24($29)\n"
        "addiu $29, $29, -32\nsw $8, 16($29)\nsw $9, 20($29)\n"
        "syscall\naddiu $29, $29, 32\n"
        "beqz $7, 1f\nnop\nsubu $2, $0, $2\n1: jr $31\nnop\n"
        ".globl __start\n__start:\n"
        "lw $4, 0($29)\naddiu $5, $29, 4\nli $8, -8\nand $29, $29, $8\n"
        "addiu $29, $29, -32\njal q2_main\nnop\nmove $4, $2\nli $2, 4246\nsyscall\n1: b 1b\nnop\n.set reorder\n");

enum { SYS_READ=4003,SYS_WRITE=4004,SYS_OPEN=4005,SYS_CLOSE=4006,
       SYS_UNLINK=4010,SYS_RENAME=4038,SYS_IOCTL=4054,SYS_MMAP=4090,
       SYS_MUNMAP=4091,SYS_FSYNC=4118,SYS_BIND=4169,SYS_RECV=4175,
       SYS_SOCKET=4183,SYS_POLL=4188,SYS_CLOCK_GETTIME=4263,
       FB_VSCREEN=0x4600,FB_FSCREEN=0x4602,FB_PAN=0x4606,
       O_WRONLY=1,O_RDWR=2,O_CREAT=0x100,O_TRUNC=0x200,O_CLOEXEC=0x80000,
       MSG_DONTWAIT=0x40,FRAME_MS=17 };
#define INPUT_SOCKET "/tmp/q2-games.input.sock"
#define SAVE_FILE "/mnt/mmc/.arcade/highscores"
#define SAVE_TEMP "/mnt/mmc/.arcade/highscores.new"
#define SAVE_MAGIC 0x51324143u
static NativeApp app;
static unsigned now_ms(void) {
    long time[2]={0};
    sc(SYS_CLOCK_GETTIME,1,(long)time,0,0,0,0);
    return (unsigned)time[0]*1000u+(unsigned)time[1]/1000000u;
}
static int bind_input(void) {
    struct { unsigned short family; char path[108]; } addr={1,INPUT_SOCKET};
    int fd=(int)sc(SYS_SOCKET,1,1,0,0,0,0);
    if(fd<0)return -1;
    sc(SYS_UNLINK,(long)INPUT_SOCKET,0,0,0,0,0);
    if(!sc(SYS_BIND,fd,(long)&addr,sizeof addr,0,0,0))return fd;
    sc(SYS_CLOSE,fd,0,0,0,0,0);
    return -1;
}
int q2_main(void) {
    unsigned var[40],fix[17];
    int fb=(int)sc(SYS_OPEN,(long)"/dev/fb0",O_RDWR|O_CLOEXEC,0,0,0,0);
    if(fb<0)return 1;
    if(sc(SYS_IOCTL,fb,FB_VSCREEN,(long)var,0,0,0) ||
       sc(SYS_IOCTL,fb,FB_FSCREEN,(long)fix,0,0,0)) {
        sc(SYS_CLOSE,fb,0,0,0,0,0);return 1;
    }
    unsigned stride=fix[11]/sizeof(u32),front=var[5];
    if(var[0]!=SCREEN_H||var[1]!=SCREEN_W||var[4]||var[6]!=32||
       var[8]!=16||fix[11]%sizeof(u32)||stride<SCREEN_H) {
        sc(SYS_CLOSE,fb,0,0,0,0,0);return 1;
    }
    unsigned rows=fix[5]/fix[11];
    if(rows>var[3])rows=var[3];
    if(rows<SCREEN_W||front>rows-SCREEN_W) {
        sc(SYS_CLOSE,fb,0,0,0,0,0);return 1;
    }
    long mapped=sc(SYS_MMAP,0,(long)fix[5],3,1,fb,0);
    if(mapped<0 && mapped>=-4095) {
        sc(SYS_CLOSE,fb,0,0,0,0,0);return 1;
    }
    u32 *pixels=(u32*)mapped;
    int input=bind_input();
    if(input<0) {
        sc(SYS_MUNMAP,mapped,(long)fix[5],0,0,0,0);
        sc(SYS_CLOSE,fb,0,0,0,0,0);return 1;
    }
    app_init(&app,now_ms());
    unsigned visible=front;
    int can_pan=rows>=2*SCREEN_W && (front==0||front==SCREEN_W);
    struct {int fd;short events,revents;} poll_input={input,1,0};
    unsigned previous=now_ms(),deadline=previous;
    int redraw=1;
    while(!app.quit) {
        unsigned now=now_ms();
        int animated=app_animated(&app);
        if((animated&&(int)(deadline-now)<=0)||redraw) {
            unsigned elapsed=now-previous;
            if(elapsed>2*FRAME_MS)elapsed=2*FRAME_MS;
            previous=now;
            deadline=now+FRAME_MS;
            if(animated)app_tick(&app,elapsed);
            unsigned target=can_pan?(visible?0:SCREEN_W):visible;
            app_render(&app,pixels+target*stride,stride);
            if(can_pan){
                var[5]=target;
                if(!sc(SYS_IOCTL,fb,FB_PAN,(long)var,0,0,0))
                    visible=target;
                else {
                    can_pan=0;
                    app_render(&app,pixels+visible*stride,stride);
                }
            }
            redraw=0;
        }
        animated=app_animated(&app);
        int remaining=(int)(deadline-now_ms());
        if(remaining<0)remaining=0;
        long ready=sc(SYS_POLL,(long)&poll_input,1,animated?remaining:-1,0,0,0);
        if(ready==0||ready==-4)continue;
        if(ready<0||!(poll_input.revents&1))break;
        char message[64];
        long n=sc(SYS_RECV,input,(long)message,sizeof message-1,MSG_DONTWAIT,0,0);
        if(n>0){
            message[n]=0;
            if(app_input(&app,message))redraw=1;
        }
    }
    if(visible!=front){
        var[5]=front;
        sc(SYS_IOCTL,fb,FB_PAN,(long)var,0,0,0);
    }
    sc(SYS_CLOSE,input,0,0,0,0,0);
    sc(SYS_UNLINK,(long)INPUT_SOCKET,0,0,0,0,0);
    sc(SYS_MUNMAP,mapped,(long)fix[5],0,0,0,0);
    sc(SYS_CLOSE,fb,0,0,0,0,0);
    return app.quit?0:1;
}
