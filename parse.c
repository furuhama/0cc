/*
 * Tokenizer & Parser
 *
 * 1. Tokenize Input
 * 2. Generate vector of Tokens
 * 3. Parse Tokens
 * 4. Generate vector of Nodes
 */

/*
 * Supported syntax:
 *
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
 * mul: unary
 * mul: mul `*` unary
 * mul: mul `/` unary
 *
 * unary: term
 * unary: `+` term
 * unary: `-` term
 *
 * term: num
 * term: ident
 * term `(` assign `)`
 *
 */

#include "0cc.h"

/* Token */

// Token type
enum {
    TK_NUM = 256, // Integer token
    TK_IDENT, // Identifier token
    TK_RETURN, // Keyword `return` token
    TK_EOF, // End of File token
};

// Token
typedef struct {
    int type; // type of token
    int value; // value of TK_NUM type token
    char *name; // value of TK_IDENT type token
    char *input; // string of token to display error messages
} Token;

// Token initializer
Token *new_token(int type, int value, char *name, char* input) {
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->name = name;
    token->input = input;
    return token;
}

/* Utils */

// Error notifier
noreturn void error(char* message, char* input) {
    fprintf(stderr, message, input);
    exit(1);
}

// check the char can be a part of Identifier
int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
        ('A' <= c && c <= 'Z') ||
        ('0' <= c && c <= '9') ||
        ('0' == '_');
}

// get current token by position
Token *current_token(int pos) {
    return (Token *)tokens->data[pos];
}

/* Variables */

int pos = 0;

/* Prototypes */

Node *stmt();
Node *assign();
Node *expr();
Node *mul();
Node *unary();
Node *term();
Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
Node *new_node_ident(char*);

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
            int i = 0;
            char *pp = p;
            while (is_alnum(*pp)) {
                i++;
                pp++;
            }
            char *ident = strndup(p, i);

            Token *tk = new_token(TK_IDENT, 0, ident, p);
            vec_push(tokens, (void *)tk);
            p++;
            continue;
        }

        fprintf(stderr, "Can't tokenize: %s\n", p);
        exit(1);
    }

    vec_push(tokens, (void *)new_token(TK_EOF, 0, NULL, p));
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

Node *new_node_ident(char *name) {
    Node *node = malloc(sizeof(Node));
    node->type = NODE_IDENT;
    node->name = name;
    return node;
}

/* Token parser */

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
    Node *lhs = unary();

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

Node *unary() {
    if (current_token(pos)->type == '+') {
        pos++;
        return term();
    }
    if (current_token(pos)->type == '-') {
        pos++;
        return new_node('-', new_node_num(0), term());
    }

    return term();
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
        Node *node = assign();

        if (current_token(pos)->type != ')') {
            error("Unexpected token, expect ')' but given token is: %s", current_token(pos)->input);
        }

        pos++;
        return node;
    }
    error("Unexpected token, expect '(' or number or ident but given token is: %s", current_token(pos)->input);
}
