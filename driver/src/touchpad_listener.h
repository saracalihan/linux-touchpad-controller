#ifndef TOUCHPAD_LISTENER_H
#define TOUCHPAD_LISTENER_H

#include "common.h"
#include <fcntl.h>
#include <linux/input.h>

void setup_touchpad(const char* dev);
void* touchpad_event_thread(void* arg);
void cleanup_touchpad(void);

#endif // TOUCHPAD_LISTENER_H