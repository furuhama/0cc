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

#include "0cc.h"

void expect(int, int, int);
void runtest();

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

/* Test code */

void expect(int line, int expected, int actual) {
    if (expected == actual) {
        return;
    }

    fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);

    exit(1);
}

void runtest() {
    // Vector test
    Vector *vec = new_vector();

    expect(__LINE__, 0, vec->len);

    for (long i = 0; i < 100; i++) {
        vec_push(vec, (void *)i);
    }

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);

    // Map test
    Map *map = new_map();

    expect(__LINE__, 0, (long)map_get(map, "foo"));

    map_push(map, "foo", (void *)2);
    expect(__LINE__, 2, (long)map_get(map, "foo"));

    map_push(map, "bar", (void *)4);
    expect(__LINE__, 4, (long)map_get(map, "bar"));

    map_push(map, "foo", (void *)6);
    expect(__LINE__, 6, (long)map_get(map, "foo"));

    printf("OK\n");
}
