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

/*
 * Global variables
 */

// Take an array for tokens (max: 100)
Token tokens[100];

// Position of token parser
int pos = 0;

/*
 * Functions
 */

/* Prototype declarations */

Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
Node *expr();
Node *mul();
Node *term();
noreturn void error(char*, char*);

/* Tokenizer (Raw source code parser) */

void tokenize(char *p) {
    int i = 0;
    while (*p) {
        // Trim spaces
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Tokenize operators
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            tokens[i].type = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        // Tokenize digits
        if (isdigit(*p)) {
            tokens[i].type = TK_NUM;
            tokens[i].input = p;
            tokens[i].value = strtol(p, &p, 10);
            i++;
            continue;
        }

        fprintf(stderr, "Can't tokenize: %s\n", p);
        exit(1);
    }

    tokens[i].type = TK_EOF;
    tokens[i].input = p;
}

// Error notifier
noreturn void error(char* message, char* input) {
    fprintf(stderr, message, input);
    exit(1);
}

/* Node initializers */

Node *new_node(int op, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->type = op; // MEMO: ここの右辺は type じゃなくて op じゃないかな
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
 * mul:  term
 * mul:  term `*` mul
 * mul:  term `/` mul
 * term: number
 * term: `(` expr `)`
 */

Node *expr() {
    Node *lhs = mul();

    if (tokens[pos].type == '+') {
        pos++;
        return new_node('+', lhs, expr());
    }
    if (tokens[pos].type == '-') {
        pos++;
        return new_node('-', lhs, expr());
    }

    return lhs;
}

Node *mul() {
    Node *lhs = term();

    if (tokens[pos].type == '*') {
        pos++;
        return new_node('*', lhs, mul());
    }
    if (tokens[pos].type == '/') {
        pos++;
        return new_node('/', lhs, mul());
    }

    return lhs;
}

Node *term() {
    if (tokens[pos].type == TK_NUM) {
        return new_node_num(tokens[pos++].value);
    }
    if (tokens[pos].type == '(') {
        pos++;
        Node *node = expr();

        if (tokens[pos].type != ')') {
            error("Unexpected token, expect ')' but given token is: %s", tokens[pos].input);
        }

        pos++;
        return node;
    }
    error("Unexpected token, expect '(' or number but given token is: %s", tokens[pos].input);
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

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments.\n");
        return 1;
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
