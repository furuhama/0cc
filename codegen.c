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

    if (node->type == NODE_EQ) {
        generate(node->lhs);
        generate(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzx rax, al\n"); // In Linux, use `movzb` instead.
        printf("    push rax\n");
        return;
    }

    if (node->type == NODE_NE) {
        generate(node->lhs);
        generate(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzx rax, al\n"); // In Linux, use `movzb` instead.
        printf("    push rax\n");
        return;
    }

    if (node->type == NODE_LE) {
        generate(node->lhs);
        generate(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzx rax, al\n"); // In Linux, use `movzb` instead.
        printf("    push rax\n");
        return;
    }

    if (node->type == NODE_LT) {
        generate(node->lhs);
        generate(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzx rax, al\n"); // In Linux, use `movzb` instead.
        printf("    push rax\n");
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

    if (node->type == NODE_IF) {
        condition_count++;
        // Copy `condition_count` value, because this variable is not closed to this function
        // and there is possibility of nested condition conrtol (e.x. if (true) if (false) stmt else stmt;)
        // In such case, `condition_count` can't be stable value.
        int label = condition_count;

        generate(node->lhs);
        Node *if_body = node->rhs;

        if (if_body->rhs != NULL) {
            // `if` ~ `else`
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lelse%d\n", label);
            generate(if_body->lhs);
            printf("    jmp .Lend%d\n", label);
            printf(".Lelse%d:\n", label);
            generate(if_body->rhs);
            printf(".Lend%d:\n", label);
            return;
        } else {
            // `if` ~
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", label);
            generate(if_body->lhs);
            printf(".Lend%d:\n", label);
            printf("    push rax\n");
            return;
        }
    }

    if (node->type == NODE_BLOCK) {
        for (int i = 0; i < node->stmts->len; i++) {
            Node *item = (Node *)(node->stmts->data[i]);
            generate(item);
        }

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
