#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdnoreturn.h>

/* Structs & Enums */

// Node type
enum {
    NODE_NUM = 256, // Integer node
    NODE_IDENT, // Identifier node
    NODE_RETURN, // `return` node
};

// Node (of Abstract Syntax Tree)
typedef struct Node {
    int type; // Operator or NODE enum
    struct Node *lhs;
    struct Node *rhs;
    int value; // value for NODE_NUM
    char *name; // value for NODE_IDENT
} Node;

// Vector
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

// Map
typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

/* Prototypes */

// Variables
Vector *tokens;
Vector *nodes;
Map *vars;

// Vector fucntions
Vector *new_vector();
void vec_push(Vector *, void *);

// Map fucntions
Map *new_map();
void map_push(Map *, char *, void *);
void *map_get(Map *, char *);

// Tokenize functions
void tokenize(char *);

// Parse fucntions
void program();

// Codegen fucntions
void codegen(Vector *);

// Utils
noreturn void error(char*, char*);
