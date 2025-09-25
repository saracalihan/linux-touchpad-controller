#include "common.h"

EventBuffer event_buffer = {0};
volatile bool running = true;

void init_event_buffer(void) {
    pthread_mutex_init(&event_buffer.mutex, NULL);
    pthread_cond_init(&event_buffer.cond, NULL);
    event_buffer.write_index = 0;
    event_buffer.read_index = 0;
    event_buffer.count = 0;
}

void add_frame_to_buffer(TouchpadFrame *frame) {
    pthread_mutex_lock(&event_buffer.mutex);

    event_buffer.frames[event_buffer.write_index] = *frame;
    event_buffer.write_index = (event_buffer.write_index + 1) % EVENT_BUFFER_SIZE;

    if (event_buffer.count < EVENT_BUFFER_SIZE) {
        event_buffer.count++;
    } else {
        event_buffer.read_index = (event_buffer.read_index + 1) % EVENT_BUFFER_SIZE;
    }

    pthread_cond_broadcast(&event_buffer.cond);
    pthread_mutex_unlock(&event_buffer.mutex);
}

bool get_frame_from_buffer(TouchpadFrame *frame) {
    pthread_mutex_lock(&event_buffer.mutex);

    if (event_buffer.count == 0) {
        pthread_mutex_unlock(&event_buffer.mutex);
        return false;
    }

    *frame = event_buffer.frames[event_buffer.read_index];
    event_buffer.read_index = (event_buffer.read_index + 1) % EVENT_BUFFER_SIZE;
    event_buffer.count--;

    pthread_mutex_unlock(&event_buffer.mutex);
    return true;
}

void cleanup_event_buffer(void) {
    pthread_mutex_destroy(&event_buffer.mutex);
    pthread_cond_destroy(&event_buffer.cond);
}