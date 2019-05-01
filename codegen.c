/*
 * Assembly Code Generator
 */

#include "0cc.h"

void prefix();
void prologue();
void epilogue();
void generate(Node *);
void gen_lval(Node *);

/* Assembly generator */

void codegen(Vector *nodes) {
    prefix();

    prologue();

    // nodes's last element is EOF node, and we will ignore it
    for (int i = 0; i < nodes->len - 1; i++) {
        Node *node = (Node *)nodes->data[i];

        generate(node);

        printf("    pop rax\n");
    }

    epilogue();
}

void gen_lval(Node *node) {
    if (node->type != NODE_IDENT) {
        error("Left value of assinment is not variable", NULL);
    }

    long offset = (long)map_get(vars, node->name);

    printf("    mov rax, rbp\n");
    printf("    sub rax, %ld\n", offset);
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

void prologue() {
    int total_vars = vars->keys->len;
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", total_vars * 8);
}

void epilogue() {
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}

void prefix() {
    printf(".intel_syntax noprefix\n");
    printf(".global _main\n");
    printf("_main:\n");
}
