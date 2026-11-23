/*
 * V5F-G3M disposable GitHub-hosted CI ONLY. Linux uinput pointer plus
 * two SOFTWARE keyboards (one hot-plugged and removed during real callbacks).
 * This program NEVER runs on the user workstation. Does not call wlroots,
 * move cursor, send wheel axes, or grant positive hardware trust.
 */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

static bool ci_guard(void) {
    const char *c=getenv("CI"), *g=getenv("GITHUB_ACTIONS");
    const char *r=getenv("RUNNER_ENVIRONMENT"), *t=getenv("RUNNER_TEMP");
    const char *s=getenv("PA_G3K_KERNEL_ACTOR");
    const char *m=getenv("PA_G3M_KERNEL_ACTOR");
    return c && strcmp(c,"true")==0 &&
        g && strcmp(g,"true")==0 &&
        r && strcmp(r,"github-hosted")==0 &&
        t && strcmp(t,"/home/runner/work/_temp")==0 &&
        s && strcmp(s,"1")==0 &&
        m && strcmp(m,"1")==0 && geteuid()==0;
}
static bool send_event(int fd, unsigned short type,
        unsigned short code, int value) {
    const struct input_event e = {.type=type,.code=code,.value=value};
    return write(fd,&e,sizeof(e))==(ssize_t)sizeof(e);
}
static void brief_pause(void) {
    const struct timespec ts={.tv_sec=0,.tv_nsec=80000000L};
    (void)nanosleep(&ts,NULL);
}
static int create_second_keyboard(void) {
    int second=open("/dev/uinput",O_WRONLY|O_NONBLOCK|O_CLOEXEC);
    if(second<0) {perror("PA_G3M_SECOND_OPEN");return -1;}
    if(ioctl(second,UI_SET_EVBIT,EV_KEY)<0){
        perror("PA_G3M_SECOND_EV_KEY");goto fail;
    }
    for (unsigned code=KEY_ESC;code<=KEY_SPACE;++code) {
        if(ioctl(second,UI_SET_KEYBIT,code)<0) {
            perror("PA_G3M_SECOND_KEY_BITS");goto fail;
        }
    }
    if(ioctl(second,UI_SET_KEYBIT,KEY_SCROLLLOCK)<0) {
        perror("PA_G3M_SECOND_SCROLL_BIT");goto fail;
    }
    struct uinput_setup setup={0};
    setup.id.bustype=BUS_USB; /* fake software bus claim, NOT provenance */
    setup.id.vendor=0x1234u;
    setup.id.product=0x5a3cu;
    setup.id.version=1u;
    snprintf(setup.name,sizeof(setup.name),
        "PA-G3M-SECOND-SYNTHETIC-KEYBOARD");
    if(ioctl(second,UI_DEV_SETUP,&setup)<0 ||
       ioctl(second,UI_DEV_CREATE)<0) {
        perror("PA_G3M_SECOND_CREATE");goto fail;
    }
    return second;
fail:
    (void)close(second);
    return -1;
}

int main(void) {
    if (!ci_guard()) {
        fputs("BLOCKED_PA_G3K_NOT_GITHUB_HOSTED_ROOT_TEST\n",stderr);
        return 2;
    }
    int fd=-1,keyboard_fd=-1,second_fd=-1,rc=1;
    bool created=false,keyboard_created=false,second_created=false;
    char name[128]={0};
    fd=open("/dev/uinput",O_WRONLY|O_NONBLOCK|O_CLOEXEC);
    if(fd<0) {perror("G3K open /dev/uinput");goto done;}
    if(ioctl(fd,UI_SET_EVBIT,EV_KEY)<0 ||
       ioctl(fd,UI_SET_EVBIT,EV_REL)<0 ||
       ioctl(fd,UI_SET_KEYBIT,BTN_LEFT)<0 ||
       ioctl(fd,UI_SET_KEYBIT,BTN_MIDDLE)<0 ||
       ioctl(fd,UI_SET_KEYBIT,BTN_RIGHT)<0 ||
       ioctl(fd,UI_SET_RELBIT,REL_X)<0 ||
       ioctl(fd,UI_SET_RELBIT,REL_Y)<0 ||
       ioctl(fd,UI_SET_RELBIT,REL_WHEEL)<0){
        perror("G3K set mouse capabilities");goto done;
    }
    struct uinput_setup setup={0};
    setup.id.bustype=BUS_USB; /* Deliberate synthetic bus claim. */
    setup.id.vendor=0x1234u;
    setup.id.product=0x5a3au;
    setup.id.version=1u;
    snprintf(setup.name,sizeof(setup.name),
        "PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL");
    if(ioctl(fd,UI_DEV_SETUP,&setup)<0 || ioctl(fd,UI_DEV_CREATE)<0){
        perror("G3K create kernel input");goto done;
    }
    created=true;
    if(ioctl(fd,UI_GET_SYSNAME(sizeof(name)),name)<0 ||
       strncmp(name,"input",5)!=0){
        perror("G3K kernel sysname");goto done;
    }
    /* Separate real Linux software keyboard, NOT genuine physical HID. */
    keyboard_fd=open("/dev/uinput",O_WRONLY|O_NONBLOCK|O_CLOEXEC);
    if(keyboard_fd<0 || ioctl(keyboard_fd,UI_SET_EVBIT,EV_KEY)<0){
        perror("PA_G3L_SYNTHETIC_KEYBOARD_OPEN_FAILED");goto done;
    }
    for (unsigned code=KEY_ESC;code<=KEY_SPACE;++code){
        if(ioctl(keyboard_fd,UI_SET_KEYBIT,code)<0){
            perror("PA_G3L_KEYBOARD_CAPABILITIES_FAILED");goto done;
        }
    }
    if(ioctl(keyboard_fd,UI_SET_KEYBIT,KEY_SCROLLLOCK)<0){
        perror("PA_G3L_SCROLLLOCK_CAPABILITY_FAILED");goto done;
    }
    struct uinput_setup keyboard_setup={0};
    keyboard_setup.id.bustype=BUS_USB; /* NOT genuine hardware. */
    keyboard_setup.id.vendor=0x1234u;
    keyboard_setup.id.product=0x5a3bu;
    keyboard_setup.id.version=1u;
    snprintf(keyboard_setup.name,sizeof(keyboard_setup.name),
        "PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE");
    if(ioctl(keyboard_fd,UI_DEV_SETUP,&keyboard_setup)<0 ||
       ioctl(keyboard_fd,UI_DEV_CREATE)<0){
        perror("PA_G3L_KEYBOARD_CREATE_FAILED");goto done;
    }
    keyboard_created=true;
    printf("PA_G3K_KERNEL_UINPUT_READY input=%s fake_bus_USB=1\n",name);
    fflush(stdout);
    char command[32]={0};
    /* First press/release while XDG target is mapped.
     * Second press/release only after test has destroyed that target.
     * Both events remain the known-synthetic kernel source. */
    for (int round = 1; round <= 4; ++round) {
        if (round==3) {
            if(!fgets(command,sizeof(command),stdin) ||
               strcmp(command,"plug\n")!=0) {
                fputs("PA_G3M_PLUG_COMMAND_REQUIRED\n",stderr);goto done;
            }
            second_fd=create_second_keyboard();
            if(second_fd<0)goto done;
            second_created=true;
            puts("PA_G3M_SECOND_KEYBOARD_PLUGGED_PASS");
            fflush(stdout);
            if(!fgets(command,sizeof(command),stdin) ||
               strcmp(command,"second_key\n")!=0) {
                fputs("PA_G3M_SECOND_KEY_COMMAND_REQUIRED\n",stderr);goto done;
            }
            if(!send_event(second_fd,EV_KEY,KEY_SCROLLLOCK,1) ||
               !send_event(second_fd,EV_SYN,SYN_REPORT,0)) {
                fputs("PA_G3M_SECOND_KEY_PRESS_FAILED\n",stderr);goto done;
            }
            brief_pause();
            if(!send_event(second_fd,EV_KEY,KEY_SCROLLLOCK,0) ||
               !send_event(second_fd,EV_SYN,SYN_REPORT,0)) {
                fputs("PA_G3M_SECOND_KEY_RELEASE_FAILED\n",stderr);goto done;
            }
            brief_pause();
            puts("PA_G3M_SECOND_KERNEL_SCROLLLOCK_SENT_PASS");
            fflush(stdout);
        }
        if (round==4) {
            if(!fgets(command,sizeof(command),stdin) ||
               strcmp(command,"unplug\n")!=0) {
                fputs("PA_G3M_UNPLUG_COMMAND_REQUIRED\n",stderr);goto done;
            }
            if(!second_created || ioctl(second_fd,UI_DEV_DESTROY)<0) {
                perror("PA_G3M_SECOND_DESTROY_FAILED");goto done;
            }
            second_created=false;
            if(close(second_fd)<0) {
                perror("PA_G3M_SECOND_CLOSE_FAILED");goto done;
            }
            second_fd=-1;
            puts("PA_G3M_SECOND_KEYBOARD_UNPLUGGED_PASS");
            fflush(stdout);
        }
        if (round==3){
            if(!fgets(command,sizeof(command),stdin) ||
               strcmp(command,"lock\n")!=0){
                fputs("PA_G3L_SCROLL_LOCK_COMMAND_REQUIRED\n",stderr);
                goto done;
            }
            if(!send_event(keyboard_fd,EV_KEY,KEY_SCROLLLOCK,1) ||
               !send_event(keyboard_fd,EV_SYN,SYN_REPORT,0)){
                fputs("PA_G3L_SCROLL_LOCK_PRESS_FAILED\n",stderr);
                goto done;
            }
            brief_pause();
            if(!send_event(keyboard_fd,EV_KEY,KEY_SCROLLLOCK,0) ||
               !send_event(keyboard_fd,EV_SYN,SYN_REPORT,0)){
                fputs("PA_G3L_SCROLL_LOCK_RELEASE_FAILED\n",stderr);
                goto done;
            }
            brief_pause();
            puts("PA_G3L_REAL_KERNEL_SCROLL_LOCK_KEY_TOGGLED_PASS press=1 release=1 axis=0");
            fflush(stdout);
        }
        if(!fgets(command,sizeof(command),stdin) ||
           strcmp(command,"click\n")!=0){
            fputs("PA_G3K_ACTOR_REFUSED_UNEXPECTED_INPUT\n",stderr);
            goto done;
        }
        if(!send_event(fd,EV_KEY,BTN_MIDDLE,1) ||
           !send_event(fd,EV_SYN,SYN_REPORT,0)){
            fputs("PA_G3K_KERNEL_MIDDLE_PRESS_SEND_FAILED\n",stderr);
            goto done;
        }
        brief_pause();
        if(!send_event(fd,EV_KEY,BTN_MIDDLE,0) ||
           !send_event(fd,EV_SYN,SYN_REPORT,0)){
            fputs("PA_G3K_KERNEL_MIDDLE_RELEASE_SEND_FAILED\n",stderr);
            goto done;
        }
        printf("PA_G3K_KERNEL_MIDDLE_SENT round=%d pressed=1 released=1 axes=0\n",round);
        fflush(stdout);
    }
    if(!fgets(command,sizeof(command),stdin) ||
       strcmp(command,"stop\n")!=0){
        fputs("PA_G3K_ACTOR_STOP_EXPECTED\n",stderr);
        goto done;
    }
    rc=0;
done:
    if(second_created && ioctl(second_fd,UI_DEV_DESTROY)<0) {
        perror("PA_G3M_SECOND_FINAL_DESTROY_FAILED");rc=1;
    }
    if(second_fd>=0 && close(second_fd)<0) {
        perror("PA_G3M_SECOND_FINAL_CLOSE_FAILED");rc=1;
    }
    if(keyboard_created && ioctl(keyboard_fd,UI_DEV_DESTROY)<0){
        perror("PA_G3L_KEYBOARD_DESTROY_FAILED");rc=1;
    }
    if(keyboard_fd>=0 && close(keyboard_fd)<0){
        perror("PA_G3L_KEYBOARD_CLOSE_FAILED");rc=1;
    }
    if(created && ioctl(fd,UI_DEV_DESTROY)<0){
        perror("PA_G3K_UINPUT_DESTROY_FAILED");
        rc=1;
    }
    if(fd>=0 && close(fd)<0){
        perror("PA_G3K_UINPUT_CLOSE_FAILED");
        rc=1;
    }
    if(rc==0)puts("PA_G3K_KERNEL_UINPUT_DESTROYED_PASS");
    if(rc==0)puts("PA_G3L_SYNTHETIC_KEYBOARD_DESTROYED_PASS");
    fflush(stdout);
    return rc;
}
