#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#define TP_SLOTS_COUNT 5
#define DATA_LEN 27
#define EVENT_BUFFER_SIZE 1000
#define DEBUG_FMT "slots[%d]->id: %05d, x: %04d, y: %04d, time: %lld\n"
#define DATA_FMT "%d%05d%04d%04d%lld"
#define RECV_DATA_LEN 10
#define CONTROLLER_VALUE_LEN 32
/*
  String ControllerCommand Frame:

  | Controller | size |  value |
  |   1        |  4   |  [size]|

                       Mouse Value -> | event(1) | key(1) |
                    Keyboard Value -> | event(1) | key(2) |

*/

typedef struct {
    int controller; // 1 mouse, 2 keyboard
    int size; // size of value
    char value[CONTROLLER_VALUE_LEN];
} ControllerCommand;

/*
  String Data Frame:

  | index | id |  x | y | time |
  |   1   |  5 |  4 | 4 |   13 |

*/

typedef struct {
    int id;
    int x;
    int y;
    long long time; // milisecond
} Slot;

typedef struct {
    Slot slots[TP_SLOTS_COUNT];
    int slot_index;
    long long timestamp;
} TouchpadFrame;

typedef struct {
    TouchpadFrame frames[EVENT_BUFFER_SIZE];
    int head;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} EventBuffer;

// Global variables
extern volatile EventBuffer event_buffer;
extern volatile bool running;

// Event buffer functions
void init_event_buffer(void);
void add_frame_to_buffer(TouchpadFrame *frame);
bool get_frame_from_buffer(TouchpadFrame *frame);
void cleanup_event_buffer(void);

#endif // COMMON_H