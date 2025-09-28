#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "macro_engine.h"
#include "common.h"

// USER CONSTANTS
#define WIDTH 1500
#define HEIGHT 1000

// SYSTEM CONSTANTS
#define DOUBLE_TAP_MS 300
#define DOUBLE_TAP_AREA 100
#define LOWER_SECTION_HEIGHT 100

// TODO: change it to dhe native Command type
#define COMMAND_LEFT_CLICK "1000231"
#define COMMAND_RIGHT_CLICK "1000232"

static pthread_t macro_thread;

bool double_tap(Slot* slot, Slot* oldSlot){
    return oldSlot->id != slot->id && abs(slot->x - oldSlot->x) < DOUBLE_TAP_AREA && abs(slot->y - oldSlot->y) < DOUBLE_TAP_AREA && ( slot->time - oldSlot->time) <= DOUBLE_TAP_MS;
}

bool on_bottom(Slot* slot){
    return slot->y > HEIGHT - LOWER_SECTION_HEIGHT;
}

bool on_right(Slot* slot){
    return slot->x > WIDTH/2;
}

bool on_left(Slot* slot){
    return slot->x < WIDTH/2;
}

void init_macro_engine(){
        if (pthread_create(&macro_thread, NULL, macro_engine_thread, NULL) != 0) {
        perror("init_macro_engine pthread_create");
        exit(EXIT_FAILURE);
    }
}

void cleanup_macro_engine(){
    printf("cleanup_macro_engine starting...\n");
    pthread_cancel(macro_thread);
    pthread_join(macro_thread, NULL);
    printf("cleanup_macro_engine done\n");

}

void* macro_engine_thread(void* arg){
    printf("macro engine started\n");
    while(1){
        TouchpadFrame frame;
        get_frame_from_buffer(&frame);
        Slot slot = frame.slots[frame.slot_index];
        Slot *oldSlot = get_last_move(frame.slot_index);

        if(oldSlot){
            if(slot.id != -1){
                if(double_tap(&slot, oldSlot)){
                    if(on_right(&slot) && on_bottom(&slot)){
                        exec_str_command(COMMAND_RIGHT_CLICK);
                        printf("[MACRO] Right click\n");
                    } else{
                        exec_str_command(COMMAND_LEFT_CLICK);
                        printf("[MACRO] Double tap\n");
                    }
                    continue;
                }
            }
        }

    }
    return NULL;
}

