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
    TK_IDENT, // Identifier token
    TK_RETURN, // Keyword `return` token
    TK_EOF, // End of File token
};

typedef struct {
    int type; // type of token
    int value; // value of TK_NUM type token
    char name; // value of TK_IDENT type token
    char *input; // string of token to display error messages
} Token;

/* Token initializers */

Token *new_token(int type, int value, char name, char* input) {
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->name = name;
    token->input = input;
    return token;
}

/* Node (of Abstract Syntax Tree) */

enum {
    NODE_NUM = 256, // Integer node
    NODE_IDENT, // Identifier node
    NODE_RETURN, // `return` node
};

typedef struct Node {
    int type; // Operator or NODE enum
    struct Node *lhs;
    struct Node *rhs;
    int value; // value for NODE_NUM
    char name; // value for NODE_IDENT
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

/* Util */

// check the char can be a part of Identifier
int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
        ('A' <= c && c <= 'Z') ||
        ('0' <= c && c <= '9') ||
        ('0' == '_');
}

/* Prototype declarations */

Token *current_token(int);
Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
Node *new_node_ident(char);
void program();
Node *stmt();
Node *assign();
Node *expr();
Node *mul();
Node *term();
noreturn void error(char*, char*);

/*
 * Global variables
 */

Vector *tokens;
Vector *nodes;

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
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';' || *p == '=') {
            Token *tk = new_token(*p, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p++;
            continue;
        }

        // Tokenize digits
        if (isdigit(*p)) {
            Token *tk = new_token(TK_NUM, strtol(p, &p, 10), NULL, p);
            vec_push(tokens, (void *)tk);
            continue;
        }

        // `return`
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            Token *tk = new_token(TK_RETURN, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p += 6;
            continue;
        }

        // Tokenize Identifiers
        if ('a' <= *p && *p <= 'z') {
            Token *tk = new_token(TK_IDENT, 0, *p, p);
            vec_push(tokens, (void *)tk);
            p++;
            continue;
        }

        fprintf(stderr, "Can't tokenize: %s\n", p);
        exit(1);
    }

    vec_push(tokens, (void *)new_token(TK_EOF, 0, NULL, p));
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

Node *new_node_ident(char name) {
    Node *node = malloc(sizeof(Node));
    node->type = NODE_IDENT;
    node->name = name;
    return node;
}

/* Token parser */

/*
 * program: ε
 * program: stmt program
 *
 * stmt: assign `;`
 * stmt: `return` assign `;`
 *
 * assign: expr
 * assign: expr `=` assign
 *
 * expr: mul
 * expr: expr `+` mul
 * expr: expr `-` mul
 *
 * mul: term
 * mul: mul `*` term
 * mul: mul `/` term
 *
 * term: num
 * term: ident
 * term `(` assign `)`
 */

Token *current_token(int pos) {
    return (Token *)tokens->data[pos];
}

void program() {
    while (current_token(pos)->type != TK_EOF) {
        vec_push(nodes, (void *)stmt());
    }

    vec_push(nodes, NULL);
}

Node *stmt() {
    Node *node;

    if (current_token(pos)->type == TK_RETURN) {
        pos++;
        node = malloc(sizeof(Node));
        node->type = NODE_RETURN;
        node->lhs = assign();
    } else {
        node = assign();
    }

    if (current_token(pos)->type == ';') {
        pos++;
        return node;
    }

    error("Unexpected token, expect ';' but given token is: %s", current_token(pos)->input);
}

Node *assign() {
    Node *lhs = expr();

    if (current_token(pos)->type == '=') {
        pos++;
        return new_node('=', lhs, assign());
    }

    return lhs;
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
    if (current_token(pos)->type == TK_IDENT) {
        return new_node_ident(current_token(pos++)->name);
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
    error("Unexpected token, expect '(' or number or ident but given token is: %s", current_token(pos)->input);
}

/* Assembly generator */

void gen_lval(Node *node) {
    if (node->type != NODE_IDENT) {
        error("Left value of assinment is not variable", NULL);
    }

    int offset = ('z' - node->name + 1) * 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n");
}

void generate(Node *node) {
    if (node->type == NODE_RETURN) {
        generate(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    }

    if (node->type == NODE_NUM) {
        printf("    push %d\n", node->value);
        return;
    }

    if (node->type == NODE_IDENT) {
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    }

    if (node->type == '=') {
        gen_lval(node->lhs);
        generate(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
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
    nodes = new_vector();

    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments.\n");
        return 1;
    }

    if (strcmp(argv[1], "-test") == 0) {
        runtest();

        return 0;
    }

    // Tokenize input

    tokenize(argv[1]);

    // Convert tokens to nodes

    program();

    // Generate Assembly

    printf(".intel_syntax noprefix\n");
    printf(".global _main\n");
    printf("_main:\n");

    // Prologue: take places for 26 characters
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    // nodes's last element is EOF node, and we will ignore it
    for (int i = 0; i < nodes->len - 1; i++) {
        Node *node = (Node *)nodes->data[i];

        generate(node);

        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}
