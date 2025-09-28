#include "common.h"

EventBuffer buffer;
volatile bool running = true;

void init_event_buffer(void) {
    pthread_mutex_init(&buffer.mutex, NULL);
    pthread_cond_init(&buffer.cond, NULL);
    buffer.head = 0;
}

void add_frame_to_buffer(TouchpadFrame *frame) {
    pthread_mutex_lock(&buffer.mutex);

    buffer.frames[buffer.head] = *frame;
    buffer.head = (buffer.head + 1) % EVENT_BUFFER_SIZE;

    pthread_cond_broadcast(&buffer.cond);
    pthread_mutex_unlock(&buffer.mutex);
}

bool get_frame_from_buffer(TouchpadFrame *frame) {
    pthread_mutex_lock(&buffer.mutex);
    pthread_cond_wait(&buffer.cond, &buffer.mutex);

    int read_index = (buffer.head - 1 + EVENT_BUFFER_SIZE) % EVENT_BUFFER_SIZE;
    *frame = buffer.frames[read_index];

    pthread_mutex_unlock(&buffer.mutex);
    return true;
}

void cleanup_event_buffer(void) {
    printf("cleanup_event_buffer starting\n");
    pthread_mutex_destroy(&buffer.mutex);
    pthread_cond_destroy(&buffer.cond);
    printf("cleanup_event_buffer done\n");
}