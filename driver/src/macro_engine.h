#ifndef MACRO_ENGINE_H
#define MACRO_ENGINE_H

#include "common.h"

#define MAX_QUERY_VALUES 10
#define MAX_QUERY_CHILDREN 10

// Query Operator Types
typedef enum {
    OP_AND,
    OP_OR,
    OP_GTE,
    OP_LTE,
    OP_ADD,
    OP_SUB,
    OP_DIV,
    OP_MULT,
    OP_DOUBLE_TAP,
    OP_ON_BOTTOM,
    OP_ON_RIGHT,
    OP_ON_LEFT,
    OP_GET_WIDTH,
    OP_GET_HEIGHT
} QueryOperator;


typedef enum {
    QV_INT,
    QV_NODE
} QueryValueType;

struct QueryNode;

typedef struct {
    QueryValueType type;
    union {
        int v_int;
        struct QueryNode* v_node;
    };
} QueryValue;

typedef struct QueryNode {
    QueryOperator operator;
    QueryValue values[MAX_QUERY_VALUES];
    int value_count;
} QueryNode;

typedef struct{
    Slot slot;
    Slot* old_slot;
    TouchpadFrame frame;
} QueryCtx;


typedef struct {
    QueryNode* condition;
    char* command;
} MacroDefinition;

void init_macro_engine();
void cleanup_macro_engine();
void* macro_engine_thread(void* arg);

int evaluate_query(QueryNode* node, QueryCtx* ctx);
QueryNode* create_query_node(QueryOperator op);
void add_query_value(QueryNode* node, int value);
void add_query_child(QueryNode* parent, QueryNode* child);
void free_query_node(QueryNode* node);

#endif // MACRO_ENGINE_H
