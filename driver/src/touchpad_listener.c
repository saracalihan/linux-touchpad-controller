#include "touchpad_listener.h"
#include "tcp_server.h"

static int touchpad_fd = -1;

void init_touchpad(const char* dev) {
    touchpad_fd = open(dev, O_RDONLY);
    if (touchpad_fd < 0) {
        perror("touchpad open");
        exit(EXIT_FAILURE);
    }
    printf("Touchpad açıldı: %s\n", dev);
}

void* touchpad_event_thread(void* arg) {
    (void)arg;
    struct input_event ev;
    int current_slot = 0;
    TouchpadFrame current_frame = {0};

    // Initialize current frame
    for (int i = 0; i < TP_SLOTS_COUNT; i++) {
        current_frame.slots[i].id = -1;
    }

    printf("Touchpad event listener started\n");

    while (running) {
        ssize_t n = read(touchpad_fd, &ev, sizeof(ev));
        if (n <= 0) {
            if (n < 0 && errno != EINTR) {
                perror("touchpad read error");
            }
            continue;
        }

        if (ev.type == EV_ABS) {
            switch (ev.code) {
                case ABS_MT_SLOT:
                    current_slot = ev.value;
                    break;
                case ABS_MT_TRACKING_ID:
                    current_frame.slots[current_slot].id = ev.value;
                    if (ev.value == -1) {
                        printf("Slot %d released\n", current_slot);
                    } else {
                        printf("Slot %d assigned id %d\n", current_slot, ev.value);
                    }
                    break;
                case ABS_MT_POSITION_X:
                    current_frame.slots[current_slot].x = ev.value;
                    break;
                case ABS_MT_POSITION_Y:
                    current_frame.slots[current_slot].y = ev.value;
                    break;
            }
            current_frame.slots[current_slot].time = (long long)ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000;
        } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
            current_frame.timestamp = (long long)ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000;
            current_frame.is_sync_report = true;

            printf("Frame:\n");
            for (int i = 0; i < TP_SLOTS_COUNT; i++) {
                if (current_frame.slots[i].id >= 0 || 1 == 1) {
                    printf(DEBUG_FMT,
                        i, current_frame.slots[i].id, current_frame.slots[i].x, 
                        current_frame.slots[i].y, current_frame.slots[i].time);
                }
            }

            add_frame_to_buffer(&current_frame);
            send_frame_to_client(&current_frame);
        }
    }

    printf("Touchpad event listener stopped\n");
    return NULL;
}

void cleanup_touchpad(void) {
    if (touchpad_fd > 0) {
        close(touchpad_fd);
        touchpad_fd = -1;
    }
}