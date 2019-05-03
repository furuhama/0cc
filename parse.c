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
 * program: sentence program
 *
 * sentence: stmt `;`
 * sentence: `{` block_items `}`
 *
 * block_items: stmt `;`
 * block_items: stmt `;` block_items
 *
 * stmt: assign
 * stmt: `if` `(` assign `)` stmt
 * stmt: `if` `(` assign `)` stmt `else` stmt
 * stmt: `return` assign
 *
 * assign: equality
 * assign: equality `=` assign
 *
 * equality: relational
 * equality: equality `==` relational
 * equality: equality `!=` relational
 *
 * relational: expr
 * relational: relational `<=` expr
 * relational: relational `<` expr
 * relational: relational `>=` expr
 * relational: relational `>` expr
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
    TK_EQ, // Equal
    TK_NE, // Not Equal
    TK_LT, // Less Than
    TK_LE, // Less than or Equal to
    TK_GT, // Greater Than
    TK_GE, // Greater than or Equal to
    TK_IF, // Keyword `if` token
    TK_ELSE, // Keyword `else` token
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

Node *sentence();
Node *block_items();
Node *stmt();
Node *assign();
Node *equality();
Node *relational();
Node *expr();
Node *mul();
Node *unary();
Node *term();
Node *new_node(int, Node*, Node*);
Node *new_node_num(int);
Node *new_node_ident(char*);
Node *new_node_if(Node *, Node *, Node *);
void dump_tokens();

/* Tokenizer (Raw source code parser) */

void tokenize(char *p) {
    while (*p) {
        // Trim spaces
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strncmp(p, "==", 2) == 0) {
            Token *tk = new_token(TK_EQ, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p += 2;
            continue;
        }

        if (strncmp(p, "!=", 2) == 0) {
            Token *tk = new_token(TK_NE, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p += 2;
            continue;
        }

        if (strncmp(p, "<=", 2) == 0) {
            Token *tk = new_token(TK_LE, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p += 2;
            continue;
        }

        if (strncmp(p, ">=", 2) == 0) {
            Token *tk = new_token(TK_GE, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p += 2;
            continue;
        }

        if (*p == '<') {
            Token *tk = new_token(TK_LT, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p++;
            continue;
        }

        if (*p == '>') {
            Token *tk = new_token(TK_GT, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p++;
            continue;
        }

        // Tokenize operators
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';' || *p == '=' || *p == '{' || *p == '}') {
            Token *tk = new_token(*p, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p++;
            continue;
        }

        // Tokenize digits
        if (isdigit(*p)) {
            char *input = p;
            Token *tk = new_token(TK_NUM, strtol(p, &p, 10), NULL, input);
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

        // `if`
        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            Token *tk = new_token(TK_IF, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p += 2;
            continue;
        }

        // `else`
        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            Token *tk = new_token(TK_ELSE, 0, NULL, p);
            vec_push(tokens, (void *)tk);
            p += 4;
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
            p += i;
            continue;
        }

        error("Can't tokenize: %s\n", p);
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

Node *new_node_if(Node *cond, Node *if_body, Node *else_body) {
    Node *node = malloc(sizeof(Node));
    node->type = NODE_IF;
    node->lhs = cond;

    node->rhs = malloc(sizeof(Node));
    node->rhs->type = NODE_IF_BODY;
    node->rhs->lhs = if_body;
    node->rhs->rhs = else_body;

    return node;
}

/* Token parser */

void program() {
    while (current_token(pos)->type != TK_EOF) {
        vec_push(nodes, (void *)sentence());
    }

    vec_push(nodes, NULL);
}

Node *sentence() {
    Node *node;

    if (current_token(pos)->type == '{') {
        // block is given
        pos++;

        node = block_items();
    } else {
        // normal statement is given
        node = stmt();

        if (current_token(pos)->type != ';') {
            error("Unexpected token, expect ';' but given token is: %s\n", current_token(pos)->input);
        }
        pos++;
    }

    return node;
}

Node *block_items() {
    Node *node = malloc(sizeof(Node));
    Vector *items = new_vector();

    while (current_token(pos)->type != '}') {
        Node *item = stmt();
        vec_push(items, (void *)item);

        if (current_token(pos)->type != ';') {
            error("Unexpected token, expect ';' but given token is: %s\n", current_token(pos)->input);
        }
        pos++;
    }
    pos++;

    node->type = NODE_BLOCK;
    node->stmts = items;

    return node;
}

Node *stmt() {
    Node *node;

    if (current_token(pos)->type == TK_RETURN) {
        pos++;

        node = malloc(sizeof(Node));
        node->type = NODE_RETURN;
        node->lhs = assign();
    } else if (current_token(pos)->type == TK_IF) {
        pos++;

        if (current_token(pos)->type != '(') {
            error("Unexpected token, expect '(', but given token is: %s\n", current_token(pos)->input);
        }
        pos++;

        Node *cond = assign();

        if (current_token(pos)->type != ')') {
            error("Unexpected token, expect ')', but given token is: %s\n", current_token(pos)->input);
        }
        pos++;

        Node *if_body = stmt();
        Node *else_body = NULL;

        // Read ahead current position to set else body
        if (tokens->len >= pos && current_token(pos)->type == TK_ELSE) {
            pos++;
            else_body = stmt();
        }

        node = new_node_if(cond, if_body, else_body);
    } else {
        node = assign();
    }

    return node;
}

Node *assign() {
    Node *lhs = equality();

    if (current_token(pos)->type == '=') {
        pos++;
        return new_node('=', lhs, assign());
    }

    return lhs;
}

Node *equality() {
    Node *lhs = relational();

    if (current_token(pos)->type == TK_EQ) {
        pos++;
        return new_node(NODE_EQ, lhs, equality());
    }
    if (current_token(pos)->type == TK_NE) {
        pos++;
        return new_node(NODE_NE, lhs, equality());
    }

    return lhs;
}

Node *relational() {
    Node *lhs = expr();

    if (current_token(pos)->type == TK_LE) {
        pos++;
        return new_node(NODE_LE, lhs, relational());
    }
    if (current_token(pos)->type == TK_LT) {
        pos++;
        return new_node(NODE_LT, lhs, relational());
    }
    if (current_token(pos)->type == TK_GE) {
        pos++;
        return new_node(NODE_LE, relational(), lhs); // Reverse left and right hand sides
    }
    if (current_token(pos)->type == TK_GT) {
        pos++;
        return new_node(NODE_LT, relational(), lhs); // Reverse left and right hand sides
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
        // Set ident to `vars` Map, if it does not exist in `vars` yet
        if ((long)map_get(vars, current_token(pos)->name) == 0) {
            long offset = (vars->keys->len + 1) * 8;
            map_push(vars, current_token(pos)->name, (void *)offset);
        }

        return new_node_ident(current_token(pos++)->name);
    }
    if (current_token(pos)->type == '(') {
        pos++;
        Node *node = assign();

        if (current_token(pos)->type != ')') {
            error("Unexpected token, expect ')' but given token is: %s\n", current_token(pos)->input);
        }

        pos++;
        return node;
    }
    error("Unexpected token, expect '(' or number or ident but given token is: %s\n", current_token(pos)->input);
}

// Debug
void dump_tokens() {
    for (int i = 0; i < tokens->len; i++) {
        Token *cur = (Token *)tokens->data[i];
        printf("# type: %d, value: %d, name: %s, input: %s\n", cur->type, cur->value, cur->name, cur->input);
    }
}
