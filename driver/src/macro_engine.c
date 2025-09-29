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
#define LOWER_SECTION_HEIGHT 100

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

int _add(int x, int y){
    return x + y;
}

int _sub(int x, int y){
    return x - y;
}

int _div(int x, int y){
    if(y == 0){
        ERROR("devide by zero!");
        return 0;
    }
    return x / y;
}

int _mul(int x, int y){
    return x * y;
}

int evaluate_query(QueryNode* node, QueryCtx* ctx) {
    if (!node) return 0;

    switch (node->operator) {
        case OP_AND: {
            if (node->value_count < 2) {
                ERROR("'and' need least 2 eperand! %d\n", node->value_count);
                return 0;
            }

            for (int i = 0; i < node->value_count; i++) {
                if(node->values[i].type != QV_NODE){
                    ERROR("'and' only can 'and' QV_NODE!\n");
                    return 0;
                }
                if (!evaluate_query(node->values[i].v_node, ctx)) {
                    return 0;
                }
            }
            return 1;
        }
        case OP_OR: {
            if (node->value_count < 2) {
                ERROR("'or' need least 2 eperand! %d\n", node->value_count);
                return 0;
            }

            for (int i = 0; i < node->value_count; i++) {
                if(node->values[i].type != QV_NODE){
                    ERROR("'or' only can 'or' QV_NODE!\n");
                    return 0;
                }
                if (evaluate_query(node->values[i].v_node, ctx)) {
                    return 1;
                }
            }
            return 0;
        }
        case OP_GTE: {
            if (node->value_count == 2) {
                if(node->values[0].type != QV_INT || node->values[1].type != QV_INT){
                    ERROR("'gte' need 2 int typed value\n");
                    return 0;
                }
                return node->values[0].v_int >= node->values[1].v_int;
            } else {
                ERROR("'gte' need least 2 eperand!\n");
            }
            return 0;
        }
        case OP_LTE: {
            if (node->value_count == 2) {
                if(node->values[0].type != QV_INT || node->values[1].type != QV_INT){
                    ERROR("'lte' need 2 int typed value\n");
                    return 0;
                }
                return node->values[0].v_int <= node->values[1].v_int;
            } else {
                ERROR("'lte' need least 2 eperand!\n");
            }
            return 0;
        }
        case OP_ADD: {
            if (node->value_count >= 2) {
                int total =0;
                for(int i = 0; i < node->value_count; i++){
                    if(node->values[i].type == QV_INT){
                        total += node->values[i].v_int;
                    } else if(node->values[i].type == QV_NODE){
                        total += evaluate_query(node->values[i].v_node, ctx);
                    } else {
                        ERROR("add: '%d' type not implemented yet", node->values[i].type);
                        return 0;
                    }
                }
                return total;
            } else {
                ERROR("'add' need least 2 eperand!\n");
                return 0;
            }
        }
        case OP_DIV: {
            if (node->value_count >= 2) {
                int total = node->values[0].type == QV_NODE ? evaluate_query(node->values[0].v_node, ctx) : node->values[0].v_int ;
                printf("total: %d\n", total);
                for(int i = 1; i < node->value_count; i++){
                    if(node->values[i].type == QV_INT){
                        total /= node->values[i].v_int;
                    } else if(node->values[i].type == QV_NODE){
                        total /= evaluate_query(node->values[i].v_node, ctx);
                    } else {
                        ERROR("div: '%d' type not implemented yet", node->values[i].type);
                        return 0;
                    }
                }
                return total;
            } else {
                ERROR("'div' need least 2 eperand!\n");
                return 0;
            }
        }

        case OP_DOUBLE_TAP: {
            return ctx->old_slot ? double_tap(&ctx->slot, ctx->old_slot) : 0;
        }

        case OP_ON_BOTTOM: {
            return on_bottom(&ctx->slot) ? 1 : 0;
        }

        case OP_ON_RIGHT: {
            return on_right(&ctx->slot) ? 1 : 0;
        }

        case OP_ON_LEFT: {
            return on_left(&ctx->slot) ? 1 : 0;
        }

        case OP_GET_WIDTH: {
            return WIDTH;
        }

        case OP_GET_HEIGHT: {
            return HEIGHT;
        }

        default:
            ERROR("Invalid operator '%d'!\n", node->operator);
            return 0;
    }
}

QueryNode* create_query_node(QueryOperator op) {
    QueryNode* node = malloc(sizeof(QueryNode));
    if (!node) return NULL;

    node->operator = op;
    node->value_count = 0;
    memset(node->values, 0, sizeof(node->values));

    return node;
}

void add_query_value(QueryNode* node, int value) {
    if (node && node->value_count < MAX_QUERY_VALUES) {
        QueryValue v = {
            .type = QV_INT,
            .v_int = value,
        };
        node->values[node->value_count++] = v;
    }
}

void add_query_child(QueryNode* parent, QueryNode* child) {
    if (parent && child && parent->value_count < MAX_QUERY_CHILDREN) {
        QueryValue v = {
            .type = QV_NODE,
            .v_node = child,
        };
        parent->values[parent->value_count++] = v;
    }
}

void free_query_node(QueryNode* node) {
    if (!node) return;

    for (int i = 0; i < node->value_count; i++) {
        if(node->values[i].type != QV_NODE){
            continue;
        }
        free_query_node(node->values[i].v_node);
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

        QueryNode* x_check = create_query_node(OP_GTE);
        add_query_value(x_check, slot.x);
        QueryNode* div = create_query_node(OP_DIV);
        add_query_child(div, create_query_node(OP_GET_WIDTH));
        add_query_value(div, 2);
        add_query_value(x_check, div);

        // QueryNode* y_check = create_query_node(OP_LTE);
        // add_query_value(y_check, slot.y);
        // add_query_value(y_check, 500);

        // add_query_child(complex_query, x_check);
        // add_query_child(complex_query, y_check);
        // add_query_child(complex_query, create_query_node(OP_ON_LEFT));

        if (evaluate_query(x_check, &ctx)) {
            // Execute some command
            printf("[MACRO] Complex condition met\n");
        }
        free_query_node(x_check);
    }
    free_query_node(right_click_query);
    free_query_node(double_tap_query);

    return NULL;
}

