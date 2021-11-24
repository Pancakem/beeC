#include "codegen.h"
#include "type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int label_seq = 0;
char *arg_reg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *arg_reg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *func_name = "";

void gen_addr(struct node *nd) {
  struct va *v = (struct va *)malloc(sizeof(struct va));
  switch (nd->kind) {
  case nd_var:
    v = nd->v;
    if (v->is_local) {
      printf("  lea rax, [rbp-%d]\n", v->offset);
      printf("  push rax\n");
    } else
      printf("  push offset %s\n", v->name);

    free(v);
    return;
  case nd_deref:
    gen(nd->lhs);
    free(v);
    return;
  }
  free(v);
  error_tok(nd->tok, "not an lvalue");
}

void gen_lval(struct node *nd) {
  if (nd->ty->kind == ty_array)
    error_tok(nd->tok, "not an lvalue");

  gen_addr(nd);
}

void load(struct typ *ty) {
  printf(" pop rax\n");
  if (size_of(ty) == 1)
    puts(" movsx rax, byte ptr [rax]\n");
  else
    puts(" mov rax, [rax]\n");
  puts(" push rax\n");
}

void store(struct typ *ty) {
  puts(" pop rdi\n");
  puts(" pop rax\n");
  if (size_of(ty) == 1)
    puts(" mov [rax], dil\n");
  else
    puts(" mov [rax], rdi\n");
  puts(" push rdi\n");
}

void gen(struct node *n) {
  int seq = 0;
  int c = 0;
  switch (n->kind) {
  case nd_null:
    return;
  case nd_num:
    printf(" push %d\n", n->val);
    break;
  case nd_expr_stmt:
    gen(n->lhs);
    printf(" add rsp, 8\n");
    return;
  case nd_var:
    gen_addr(n);
    if (n->ty->kind != ty_array)
      load(n->ty);
    return;
  case nd_assign:
    gen_lval(n->lhs);
    gen(n->rhs);
    store(n->ty);
    return;
  case nd_addr:
    gen_addr(n->lhs);
    return;
  case nd_deref:
    gen(n->lhs);
    if (n->ty->kind != ty_array)
      load(n->ty);
    return;
  case nd_if:
    seq = label_seq;
    label_seq++;
    if (n->els != NULL) {
      gen(n->cond);
      puts(" pop rax\n");
      puts(" cmp rax, 0\n");
      printf(" je .Lelse%d\n", seq);
      gen(n->then);
      printf(" jmp .Lend%d\n", seq);
      printf(".Lelse%d:\n", seq);
      gen(n->els);
      printf(".Lend%d:\n", seq);
    } else {
      gen(n->cond);
      puts(" pop rax\n");
      puts(" cmp rax, 0\n");
      printf(" je .Lend%d\n", seq);
      gen(n->then);
      printf(".Lend%d:\n", seq);
    }
    return;
  case nd_while:
    seq = label_seq;
    label_seq++;
    printf(".Lbegin%d:\n", seq);
    gen(n->cond);
    puts(" pop rax\n");
    puts(" cmp rax, 0\n");
    printf(" je .Lend%d\n", seq);
    gen(n->then);
    printf(" jmp .Lbegin%d\n", seq);
    printf(".Lend%d:\n", seq);
    return;
  case nd_for:
    seq = label_seq;
    label_seq++;
    if (n->init != NULL)
      gen(n->init);
    printf(".Lbegin%d\n", seq);
    if (n->cond != NULL) {
      gen(n->cond);
      puts(" pop rax\n");
      puts(" cmp rax, 0\n");
      printf(" je .Lend%d\n", seq);
    }
    gen(n->then);
    if (n->inc != NULL)
      gen(n->inc);
    printf(" jmp .Lbegin%d\n", seq);
    printf(".Lend%d:\n", seq);
    return;
  case nd_block:
    for (struct node *b = n->body; b != NULL; b = b->next)
      gen(b);
    return;
  case nd_func_call:
    c = 0;
    for (struct node *arg = n->args; arg != NULL; arg = arg->next) {
      gen(arg);
      c++;
    }
    while (c > 0) {
      c--;
      printf(" pop %s\n", arg_reg8[c]);
    }
    seq = label_seq;
    label_seq++;
    printf(" mov rax, rsp\n");
    printf(" and rax, 15\n");
    printf(" jnz .Lcall%d\n", seq);
    printf(" mov rax, 0\n");
    printf(" call %s\n", n->func_name);
    printf(" jmp .Lend%d\n", seq);
    printf(".Lcall%d:\n", seq);
    printf(" sub rsp, 8\n");
    printf(" mov rax, 0\n");
    printf(" call %s\n", n->func_name);
    printf(" add rsp, 8\n");
    printf(".Lend%d:\n", seq);
    printf(" push rax\n");
    return;
  case nd_ret:
    gen(n->lhs);
    puts(" pop rax\n");
    printf(" jmp .Lreturn.%s\n", n->func_name);
    return;
  }
  gen(n->lhs);
  gen(n->rhs);
  puts(" pop rdi\n");
  puts(" pop rax\n");

  switch (n->kind) {
  case nd_add:
    if (n->ty->base != NULL)
      printf(" imul rdi, %d\n", size_of(n->ty->base));
    printf(" add rax, rdi\n");
    break;
  case nd_sub:
    if (n->ty->base != NULL)
      printf(" imul rdi, %d\n", size_of(n->ty->base));
    printf(" sub rax, rdi\n");
    break;
  case nd_mul:
    printf(" imul rax, rdi\n");
    break;
  case nd_div:
    printf(" cqo\n");
    printf(" idiv rdi\n");
    break;
  case nd_eq:
    printf(" cmp rax, rdi\n");
    printf(" sete al\n");
    printf(" movzb rax, al\n");
    break;
  case nd_ne:
    printf(" cmp rax, rdi\n");
    printf(" setne al\n");
    printf(" movzb rax, al\n");
    break;
  case nd_lt:
    printf(" cmp rax, rdi\n");
    printf(" setl al\n");
    printf(" movzb rax, al\n");
    break;
  case nd_le:
    printf(" cmp rax, rdi\n");
    printf(" setle al\n");
    printf(" movzb rax, al\n");
    break;
  }
  printf(" push rax\n");
}

void emit_data(struct program *p) {
  puts(".data\n");
  for (struct var_list *vl = p->globals; vl != NULL; vl = vl->next) {
    struct va *v = vl->v;
    printf("%s:\n", v->name);
    if (v->contents != NULL) {
      for (int i = 0; i < strlen(v->contents); ++i)
        printf(" .byte %d\n", v->contents[i]);
    } else
      printf(" .zero %d\n", size_of(v->ty));
  }
}

void load_arg(struct va *v, int idx) {
  int sz = size_of(v->ty);
  if (sz == 1)
    printf(" mov [rbp-%d], %s\n", v->offset, arg_reg1[idx]);
  else
    printf(" mov [rbp-%d], %s\n", v->offset, arg_reg8[idx]);
}

void emit_text(struct program *prog) {
  puts(".text\n");
  for (struct fun *fn = prog->fns; fn != NULL; fn = fn->next) {
    printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);
    func_name = fn->name;
    printf(" push rbp\n");
    printf(" mov rbp, rsp\n");
    printf(" sub rsp, %d\n", fn->stack_size);
    int i = 0;
    for (struct var_list *vl = fn->params; vl != NULL; vl = vl->next) {
      load_arg(vl->v, i);
      i++;
    }
    for (struct node *n = fn->node; n != NULL; n = n->next) {
      gen(n);
    }
    printf(".Lreturn.%s:\n", func_name);
    puts(" mov rsp, rbp\n");
    puts(" pop rbp\n");
    puts(" ret\n");
  }
}

void codegen(struct program *prog) {
  puts(".intel_syntax noprefix\n");
  emit_data(prog);
  emit_text(prog);
}
