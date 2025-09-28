#include "common.h"

EventBuffer buffer;
Slot last_moves[TP_SLOTS_COUNT] = {0};

volatile bool running = true;

void init_event_buffer(void) {
    pthread_mutex_init(&buffer.mutex, NULL);
    pthread_cond_init(&buffer.cond, NULL);
    buffer.head = 0;
    buffer.count = 0;
}


void _get_frame_from_buffer(TouchpadFrame *frame){
    int read_index = (buffer.head - 1 + EVENT_BUFFER_SIZE) % EVENT_BUFFER_SIZE;
    *frame = buffer.frames[read_index];
}

void add_frame_to_buffer(TouchpadFrame *frame) {
    pthread_mutex_lock(&buffer.mutex);

    Slot cur_slot = frame->slots[frame->slot_index]; 
    if(cur_slot.id != -1){
        TouchpadFrame last_frame;
        _get_frame_from_buffer(&last_frame);
        last_moves[frame->slot_index] = last_frame.slots[frame->slot_index]; 
    }
    buffer.frames[buffer.head] = *frame;
    buffer.head = (buffer.head + 1) % EVENT_BUFFER_SIZE;
    if(buffer.count < EVENT_BUFFER_SIZE){
        buffer.count++;
    }
    pthread_cond_broadcast(&buffer.cond);
    pthread_mutex_unlock(&buffer.mutex);
}


bool get_frame_from_buffer(TouchpadFrame *frame) {
    pthread_mutex_lock(&buffer.mutex);
    pthread_cond_wait(&buffer.cond, &buffer.mutex);

    _get_frame_from_buffer(frame);

    pthread_mutex_unlock(&buffer.mutex);
    return true;
}

void cleanup_event_buffer(void) {
    printf("cleanup_event_buffer starting\n");
    pthread_mutex_destroy(&buffer.mutex);
    pthread_cond_destroy(&buffer.cond);
    printf("cleanup_event_buffer done\n");
}

int get_frame_count(){
   return buffer.count;
}

Slot* get_last_move(int i){
    if(i>= TP_SLOTS_COUNT){
        printf("[ERROR]: get_last_move unboundery '%d'\n",  i);
        return NULL;
    }

    if(last_moves[i].id == 0){
        return NULL;
    }
    return &last_moves[i];
}
