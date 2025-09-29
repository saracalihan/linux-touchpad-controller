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
    OP_DOUBLE_TAP,
    OP_ON_BOTTOM,
    OP_ON_RIGHT,
    OP_ON_LEFT,
    OP_HOLD
} QueryOperator;

// Forward declaration
struct QueryNode;

// Query Node Structure
typedef struct QueryNode {
    QueryOperator operator;
    int values[MAX_QUERY_VALUES];
    int value_count;
    struct QueryNode* children[MAX_QUERY_CHILDREN];
    int child_count;
} QueryNode;

typedef struct{
    Slot slot;
    Slot* old_slot;
    TouchpadFrame frame;
} QueryCtx;

// Macro Definition Structure
typedef struct {
    QueryNode* condition;
    char* command;
} MacroDefinition;

void init_macro_engine();
void cleanup_macro_engine();
void* macro_engine_thread(void* arg);

bool evaluate_query(QueryNode* node, QueryCtx* ctx);
QueryNode* create_query_node(QueryOperator op);
void add_query_value(QueryNode* node, int value);
void add_query_child(QueryNode* parent, QueryNode* child);
void free_query_node(QueryNode* node);

#endif // MACRO_ENGINE_H
