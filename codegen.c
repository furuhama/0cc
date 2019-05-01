#include "0cc.h"

/* Assembly generator */

void gen_lval(Node *node) {
    if (node->type != NODE_IDENT) {
        error("Left value of assinment is not variable", NULL);
    }

    int offset = ('z' - *node->name + 1) * 8;
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
