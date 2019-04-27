/*
 * 0cc
 *
 * Compile process:
 *
 * 1. Tokenize input string
 *    (Trim extra spaces, Set type & value for each tokens)
 *
 * 2. Create Abstract Syntax Tree (= AST)
 *    (Create nodes by syntax rules)
 *
 * 3. Generate assembly codes by consuming AST
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

/*
 * Structs
 */

/* Token */

enum {
    TK_NUM = 256, // Integer token
    TK_EOF, // End of File token
};

typedef struct {
    int type; // type of token
    int value; // value of TK_NUM type token
    char *input; // string of token to display error messages
} Token;

/* Token initializers */

Token *new_token(int type, int value, char* input) {
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->input = input;
    return token;
}

/* Node (of Abstract Syntax Tree) */

enum {
    NODE_NUM = 256, // Integer node
};

typedef struct Node {
    int type; // Operator or ND_NUM
    struct Node *lhs;
    struct Node *rhs;
    int value; // value for ND_NUM
} Node;

/* Vector */

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

/* Vector functions */

Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));

    int default_capacity = 16;

    vec->data = malloc(sizeof(void *) * default_capacity);
    vec->capacity = default_capacity;
    vec->len = 0;

    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len] = elem;
    vec->len++;
}

/* Prototype declarations */

Token *current_token(int);
Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
Node *expr();
Node *mul();
Node *term();
noreturn void error(char*, char*);

/*
 * Global variables
 */

// Take a vector for tokens
Vector *tokens;

// Position of token parser
int pos = 0;

/* Tokenizer (Raw source code parser) */

void tokenize(char *p) {
    while (*p) {
        // Trim spaces
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Tokenize operators
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            Token *tk = new_token(*p, 0, p);
            vec_push(tokens, (void *)tk);
            p++;
            continue;
        }

        // Tokenize digits
        if (isdigit(*p)) {
            Token *tk = new_token(TK_NUM, strtol(p, &p, 10), p);
            vec_push(tokens, (void *)tk);
            continue;
        }

        fprintf(stderr, "Can't tokenize: %s\n", p);
        exit(1);
    }

    vec_push(tokens, (void *)new_token(TK_EOF, 0, p));
}

// Error notifier
noreturn void error(char* message, char* input) {
    fprintf(stderr, message, input);
    exit(1);
}

/* Node initializers */

Node *new_node(int op, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->type = op;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int value) {
    Node *node = malloc(sizeof(Node));
    node->type = NODE_NUM;
    node->value = value;
    return node;
}

/* Token parser */

/*
 * expr: mul
 * expr: mul `+` expr
 * expr: mul `-` expr
 *
 * mul:  term
 * mul:  term `*` mul
 * mul:  term `/` mul
 *
 * term: number
 * term: `(` expr `)`
 */

Token *current_token(int pos) {
    return (Token *)tokens->data[pos];
}

Node *expr() {
    Node *lhs = mul();

    if (current_token(pos)->type == '+') {
        pos++;
        return new_node('+', lhs, expr());
    }
    if (current_token(pos)->type == '-') {
        pos++;
        return new_node('-', lhs, expr());
    }

    return lhs;
}

Node *mul() {
    Node *lhs = term();

    if (current_token(pos)->type == '*') {
        pos++;
        return new_node('*', lhs, mul());
    }
    if (current_token(pos)->type == '/') {
        pos++;
        return new_node('/', lhs, mul());
    }

    return lhs;
}

Node *term() {
    if (current_token(pos)->type == TK_NUM) {
        return new_node_num(current_token(pos++)->value);
    }
    if (current_token(pos)->type == '(') {
        pos++;
        Node *node = expr();

        if (current_token(pos)->type != ')') {
            error("Unexpected token, expect ')' but given token is: %s", current_token(pos)->input);
        }

        pos++;
        return node;
    }
    error("Unexpected token, expect '(' or number but given token is: %s", current_token(pos)->input);
}

/* Assembly generator */

void generate(Node *node) {
    if (node->type == NODE_NUM) {
        printf("    push %d\n", node->value);
        return;
    }

    generate(node->lhs);
    generate(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->type) {
    case '+':
        printf("    add rax, rdi\n");
        break;
    case '-':
        printf("    sub rax, rdi\n");
        break;
    case '*':
        printf("    mul rdi\n");
        break;
    case '/':
        printf("    mov rdx, 0\n");
        printf("    div rdi\n");
    }

    printf("    push rax\n");
}

/* Test code */

void expect(int line, int expected, int actual) {
    if (expected == actual) {
        return;
    }

    fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);

    exit(1);
}

void runtest() {
    Vector *vec = new_vector();

    expect(__LINE__, 0, vec->len);

    for (long i = 0; i < 100; i++) {
        vec_push(vec, (void *)i);
    }

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);

    printf("OK\n");
}

/* main */

int main(int argc, char **argv) {
    tokens = new_vector();

    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments.\n");
        return 1;
    }

    if (strcmp(argv[1], "-test") == 0) {
        runtest();

        return 0;
    }

    tokenize(argv[1]);
    Node* node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global _main\n");
    printf("_main:\n");

    generate(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
