#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

// veri okuma
int main() {
    // cat /proc/bus/input/devices
    const char *dev = "/dev/input/event6";
    struct input_event ev;
    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    while (1) {
        if (read(fd, &ev, sizeof(ev)) > 0) {
            printf("time %ld.%06ld type %d code %d value %d\n",
                   ev.time.tv_sec, ev.time.tv_usec,
                   ev.type, ev.code, ev.value);
        }
    }
    return 0;
}


#include <string.h>

#define MAX_SLOTS 10

// konum okuma, multi touch
int main2() {
    const char *dev = "/dev/input/event6"; // kendi cihazını yaz
    struct input_event ev;
    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int current_slot = 0;
    int slot_ids[MAX_SLOTS];
    int slot_x[MAX_SLOTS];
    int slot_y[MAX_SLOTS];
    memset(slot_ids, -1, sizeof(slot_ids));

    while (1) {
        if (read(fd, &ev, sizeof(ev)) > 0) {
            if (ev.type == EV_ABS) {
                switch (ev.code) {
                    case ABS_MT_SLOT:
                        current_slot = ev.value;
                        break;
                    case ABS_MT_TRACKING_ID:
                        slot_ids[current_slot] = ev.value;
                        if (ev.value == -1) {
                            printf("Slot %d released\n", current_slot);
                        } else {
                            printf("Slot %d assigned id %d\n", current_slot, ev.value);
                        }
                        break;
                    case ABS_MT_POSITION_X:
                        slot_x[current_slot] = ev.value;
                        break;
                    case ABS_MT_POSITION_Y:
                        slot_y[current_slot] = ev.value;
                        break;
                    case ABS_MT_PRESSURE:
                        printf("Slot %d pressure = %d\n", current_slot, ev.value);
                        break;
                }
            } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                printf("Frame:\n");
                for (int i=0; i<MAX_SLOTS; i++) {
                    if (slot_ids[i] >= 0) {
                        printf("  Finger %d -> X=%d, Y=%d\n",
                               slot_ids[i], slot_x[i], slot_y[i]);
                    }
                }
            }
        }
    }
}


#define MAX_SLOTS 10

// konum ve handle click
int main3() {
    const char *dev = "/dev/input/event6"; // kendi cihazını yaz
    struct input_event ev;
    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int current_slot = 0;
    int slot_ids[MAX_SLOTS];
    int slot_x[MAX_SLOTS];
    int slot_y[MAX_SLOTS];
    int slot_pressure[MAX_SLOTS];
    memset(slot_ids, -1, sizeof(slot_ids));

    while (1) {
        if (read(fd, &ev, sizeof(ev)) > 0) {
            if (ev.type == EV_ABS) {
                switch (ev.code) {
                    case ABS_MT_SLOT:
                        current_slot = ev.value;
                        break;
                    case ABS_MT_TRACKING_ID:
                        slot_ids[current_slot] = ev.value;
                        if (ev.value == -1) {
                            printf("Slot %d released\n", current_slot);
                        } else {
                            printf("Slot %d assigned id %d\n", current_slot, ev.value);
                        }
                        break;
                    case ABS_MT_POSITION_X:
                        slot_x[current_slot] = ev.value;
                        break;
                    case ABS_MT_POSITION_Y:
                        slot_y[current_slot] = ev.value;
                        break;
                    case ABS_MT_PRESSURE:
                        slot_pressure[current_slot] = ev.value;
                        break;
                }
            } else if (ev.type == EV_KEY) {
                // Buton eventleri
                if (ev.code == BTN_LEFT)
                    printf("Left button %s\n", ev.value ? "pressed" : "released");
                else if (ev.code == BTN_RIGHT)
                    printf("Right button %s\n", ev.value ? "pressed" : "released");
                else if (ev.code == BTN_MIDDLE)
                    printf("Middle button %s\n", ev.value ? "pressed" : "released");
                else if (ev.code == BTN_TOUCH)
                    printf("Touch %s\n", ev.value ? "down" : "up");
                else if (ev.code == BTN_TOOL_FINGER)
                    printf("Single finger detected\n");
                else if (ev.code == BTN_TOOL_DOUBLETAP)
                    printf("Two fingers detected\n");
                else if (ev.code == BTN_TOOL_TRIPLETAP)
                    printf("Three fingers detected\n");
            } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                printf("Frame:\n");
                for (int i=0; i<MAX_SLOTS; i++) {
                    if (slot_ids[i] >= 0) {
                        printf("  Finger %d -> X=%d, Y=%d, Pressure=%d\n",
                               slot_ids[i], slot_x[i], slot_y[i], slot_pressure[i]);
                    }
                }
            }
        }
    }
}


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>

// programatic click
int main() {
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
    strcpy(usetup.name, "virtual-mouse");

    ioctl(fd_uinput, UI_DEV_SETUP, &usetup);
    ioctl(fd_uinput, UI_DEV_CREATE);

    sleep(1); // cihazın hazır olmasını bekle

    // ======= Sol click bas =======
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);

    ev.type = EV_KEY;
    ev.code = BTN_LEFT;
    ev.value = 1; // bas
    write(fd_uinput, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd_uinput, &ev, sizeof(ev));

    usleep(2000000); // 20ms basılı tut

    // ======= Sol click bırak =======
    ev.type = EV_KEY;
    ev.code = BTN_LEFT;
    ev.value = 0; // bırak
    write(fd_uinput, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd_uinput, &ev, sizeof(ev));

    ioctl(fd_uinput, UI_DEV_DESTROY);
    close(fd_uinput);
    return 0;
}

// #include <stdio.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <linux/uinput.h>
// #include <string.h>
// #include <sys/ioctl.h>
// #include <sys/time.h>

// void emit5(int fd, int type, int code, int val) {
//     struct input_event ie;
//     gettimeofday(&ie.time, NULL);
//     ie.type = type;
//     ie.code = code;
//     ie.value = val;
//     write(fd, &ie, sizeof(ie));
// }

// int main5() {
//     int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
//     if (fd < 0) { perror("open"); return 1; }

//     ioctl(fd, UI_SET_EVBIT, EV_REL);
//     ioctl(fd, UI_SET_RELBIT, REL_X);
//     ioctl(fd, UI_SET_RELBIT, REL_Y);

//     ioctl(fd, UI_SET_EVBIT, EV_KEY);
//     ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

//     ioctl(fd, UI_SET_EVBIT, EV_SYN);

//     struct uinput_setup usetup;
//     memset(&usetup, 0, sizeof(usetup));
//     usetup.id.bustype = BUS_USB;
//     usetup.id.vendor = 0x1;
//     usetup.id.product = 0x2;
//     strcpy(usetup.name, "virtual-rel-mouse");

//     ioctl(fd, UI_DEV_SETUP, &usetup);
//     ioctl(fd, UI_DEV_CREATE);
//     sleep(1);

//     // Mouse'u sağa 100, aşağı 50 hareket ettir
//     emit(fd, EV_REL, REL_X, 100);
//     emit(fd, EV_REL, REL_Y, 50);
//     emit(fd, EV_SYN, SYN_REPORT, 0);

//     sleep(1);

//     ioctl(fd, UI_DEV_DESTROY);
//     close(fd);
//     return 0;
// }
