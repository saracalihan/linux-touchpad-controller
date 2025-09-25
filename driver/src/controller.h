#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "common.h"

#define DEVICE_NAME_PREFIX "as-touchpad"

typedef enum {
    CT_MOUSE=1,
    CT_KEYBOARD,
} ControllerType;

typedef enum {
    CT_PRESS=1,
    CT_RELEASE,
    CT_CLICK,
    CT_MOVE,
} ControllerEvent;

typedef enum {
    MOUSE_LEFT=1,
    MOUSE_RIGTH
} MouseKey;

void init_controllers();
void cleanup_controllers();

void mouse_press(int code);
void mouse_release(int code);
void mouse_click(int code);

int exec_command(ControllerCommand c);

#endif // CONTROLLER_H