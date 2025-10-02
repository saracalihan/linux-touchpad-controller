#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "macro_engine.h"

// USER CONSTANTS
#define WIDTH 1500
#define HEIGHT 1000

// SYSTEM CONSTANTS
#define DOUBLE_TAP_MS 300
#define DOUBLE_TAP_AREA 100
#define LOWER_SECTION_HEIGHT 200
#define HOLD_TRASHOLD_MS 400

// TODO: change it to dhe native Command type
#define COMMAND_LEFT_CLICK "1000231"
#define COMMAND_RIGHT_CLICK "1000232"

#define ERROR(...) printf("[QUERY ERROR]: "__VA_ARGS__)

static pthread_t macro_thread;

bool double_tap(Slot* slot, Slot* oldSlot){
    if(!(oldSlot && slot->id != -1)){
        return false;
    }

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

bool hold(Slot* slot, Slot* old_slot, int time){
    if(!old_slot || slot->id != -1){
        return false;
    }
    int dt = slot->time - old_slot->time;
    return time - HOLD_TRASHOLD_MS < dt && dt < time + HOLD_TRASHOLD_MS; 
}

bool evaluate_query(QueryNode* node, QueryCtx* ctx) {
    if (!node) return false;

    switch (node->operator) {
        case OP_AND: {
            if (node->child_count < 2) {
                ERROR("'and' need least 2 eperand! %d\n", node->child_count);
                return false;
            }

            for (int i = 0; i < node->child_count; i++) {
                if (!evaluate_query(node->children[i], ctx)) {
                    return false;
                }
            }
            return true;
        }

        case OP_OR: {
            if (node->child_count < 2) {
                ERROR("'and' need least 2 eperand! %d\n", node->child_count);
                return false;
            }

            for (int i = 0; i < node->child_count; i++) {
                if (evaluate_query(node->children[i], ctx)) {
                    return true;
                }
            }
            return false;
        }

        case OP_GTE: {
            if (node->value_count == 2) {
                return node->values[0] >= node->values[1];
            } else {
                ERROR("'gte' need least 2 eperand!\n");
            }
            return false;
        }

        case OP_LTE: {
            if (node->value_count == 2) {
                return node->values[0] <= node->values[1];
            } else {
                ERROR("'lte' need least 2 eperand!\n");
            }
            return false;
        }

        case OP_DOUBLE_TAP: {
            return ctx->old_slot ? double_tap(&ctx->slot, ctx->old_slot) : false;
        }

        case OP_ON_BOTTOM: {
            return on_bottom(&ctx->slot);
        }

        case OP_ON_RIGHT: {
            return on_right(&ctx->slot);
        }

        case OP_ON_LEFT: {
            return on_left(&ctx->slot);
        }

        default:
            ERROR("Invalid operator '%d'!\n", node->operator);
            return false;
    }
}

QueryNode* create_query_node(QueryOperator op) {
    QueryNode* node = malloc(sizeof(QueryNode));
    if (!node) return NULL;

    node->operator = op;
    node->value_count = 0;
    node->child_count = 0;
    memset(node->values, 0, sizeof(node->values));
    memset(node->children, 0, sizeof(node->children));

    return node;
}

void add_query_value(QueryNode* node, int value) {
    if (node && node->value_count < MAX_QUERY_VALUES) {
        node->values[node->value_count++] = value;
    }
}

void add_query_child(QueryNode* parent, QueryNode* child) {
    if (parent && child && parent->child_count < MAX_QUERY_CHILDREN) {
        parent->children[parent->child_count++] = child;
    }
}

void free_query_node(QueryNode* node) {
    if (!node) return;

    for (int i = 0; i < node->child_count; i++) {
        free_query_node(node->children[i]);
    }
    free(node);
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
    (void) arg;
    printf("macro engine started\n");

    // DEFAULT QUERIES
    // Right Click: double_tap(&slot, oldSlot) && on_right(&slot) && on_bottom(&slot)
    QueryNode* right_click_query = create_query_node(OP_AND);
    add_query_child(right_click_query, create_query_node(OP_DOUBLE_TAP));
    add_query_child(right_click_query, create_query_node(OP_ON_RIGHT));
    add_query_child(right_click_query, create_query_node(OP_ON_BOTTOM));
    // Double Tap: double_tap(&slot, oldSlot)
    QueryNode* double_tap_query = create_query_node(OP_DOUBLE_TAP);

    while(1){
        TouchpadFrame frame;
        get_frame_from_buffer(&frame);
        Slot slot = frame.slots[frame.slot_index];
        Slot *old_slot = get_last_move(frame.slot_index);
        QueryCtx ctx = {0};
        ctx.slot = slot;
        ctx.old_slot = old_slot;
        ctx.frame = frame;


        if (evaluate_query(right_click_query, &ctx)) {
            exec_str_command(COMMAND_RIGHT_CLICK);
            printf("[MACRO] Right click\n");
            continue;
        }

        if (evaluate_query(double_tap_query, &ctx)) {
            exec_str_command(COMMAND_LEFT_CLICK);
            printf("[MACRO] Double tap\n");
            continue;
        }
        // // EXAMPLE QUERY
        // // Equivalent to: slot.x >= 100 && slot.y <= 500 && on_left(&slot)
        // QueryNode* complex_query = create_query_node(OP_AND);

        // QueryNode* x_check = create_query_node(OP_GTE);
        // add_query_value(x_check, slot.x);
        // add_query_value(x_check, 100);

        // QueryNode* y_check = create_query_node(OP_LTE);
        // add_query_value(y_check, slot.y);
        // add_query_value(y_check, 500);

        // add_query_child(complex_query, x_check);
        // add_query_child(complex_query, y_check);
        // add_query_child(complex_query, create_query_node(OP_ON_LEFT));

        // if (evaluate_query(complex_query, &ctx)) {
        //     // Execute some command
        //     printf("[MACRO] Complex condition met\n");
        // }
        // free_query_node(complex_query);
    }
    free_query_node(right_click_query);
    free_query_node(double_tap_query);

    return NULL;
}

