/*
  Run `sudo evtest` command for device's `type`, `code` datas 
*/

#include "controller.h"

int mouse_fd = -1;

int init_mouse(){
    int fd_uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd_uinput < 0) {
        perror("open uinput");
        return 1;
    }

    // Sol click eventini destekle
    ioctl(fd_uinput, UI_SET_EVBIT, EV_KEY);
    ioctl(fd_uinput, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd_uinput, UI_SET_EVBIT, EV_SYN);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0x1;
    usetup.id.product = 0x1;
    strcpy(usetup.name, DEVICE_NAME_PREFIX"-mouse");

    ioctl(fd_uinput, UI_DEV_SETUP, &usetup);
    ioctl(fd_uinput, UI_DEV_CREATE);
    return fd_uinput;
}

void init_controllers(){
    mouse_fd = init_mouse();
    sleep(1);
}

void cleanup_mouse(){
    ioctl(mouse_fd, UI_DEV_DESTROY);
    close(mouse_fd);
}

void cleanup_controllers(){
    cleanup_mouse();
}

/*
    BTN_RIGHT, BTN_LEFT
*/
void mouse_press(int code){
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);

    ev.type = EV_KEY;
    ev.code = code;
    ev.value = 1;
    write(mouse_fd, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(mouse_fd, &ev, sizeof(ev));
}

/*
    BTN_RIGHT, BTN_LEFT
*/
void mouse_release(int code){
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);

    ev.type = EV_KEY;
    ev.code = code;
    ev.value = 0;
    write(mouse_fd, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(mouse_fd, &ev, sizeof(ev));
}

void mouse_click(int code){
    mouse_press(code);
    usleep(20000); // 20ms
    mouse_release(code);
}

// TODO: mouse scroll. type 2 (EV_REL), code 8 (REL_WHEEL), value -1/+1

int exec_command(ControllerCommand c){
    printf("value: '%s'\n", c.value);
    switch(c.controller){
        case CT_MOUSE:
            char ek[2] = {c.value[0], 0};
            char bk[2] = {c.value[1], 0};
            int event = atoi(&ek);
            int button = atoi(&bk);
            printf("event: %d, button: %d\n", event, button);

            if(button != MOUSE_LEFT && button != MOUSE_RIGTH){
                printf("[CONTROLLER ERROR]: exec_command MouseKey value:'%d' MOUSE_LEFT:%d, MOUSE_RIGTH:%d'\n", button, MOUSE_LEFT, MOUSE_RIGTH);
                return 0;
            }

            switch(event){
                case CT_PRESS:
                    mouse_press(button == MOUSE_LEFT ? BTN_LEFT : BTN_RIGHT);
                    break;
                case CT_RELEASE:
                    mouse_release(button == MOUSE_LEFT ? BTN_LEFT : BTN_RIGHT);
                    break;
                case CT_CLICK:
                    mouse_click(button == MOUSE_LEFT ? BTN_LEFT : BTN_RIGHT);
                    break;
                default:
                    printf("[CONTROLLER ERROR]: exec_command ControllerEvent value:'%'. CT_PRESS:%d, CT_RELEASE:%d, CT_CLICK:%d'\n", atoi(&c.value[0]),CT_PRESS,CT_RELEASE, CT_CLICK);
                    return 0;
            }
            break;
        case CT_KEYBOARD:
            printf("NOT IMPLEMENTED YET: KEYBOARD CONTROLLER\n");
            break;
        default:
            printf("[CONTROLLER ERROR]: unknown command controller: '%d'", c.controller);
            return 0;
    }
    return 1;
}
