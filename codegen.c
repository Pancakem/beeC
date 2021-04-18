#include <stdio.h>
#include "codegen.h"

int label_seq = 0;
char *func_name;
char *arg_reg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *arg_reg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};


void gen_addr(struct node *nd) {
  struct va *v;
  switch(nd->kind) {
  case nd_var:
    v = nd->v;
    if(v->is_local) {
      printf("  lea rax, [rbp-%d]\n", v->offset);
      printf("  push rax\n");      
    }else printf("  push offset %s\n", v->name);

    return;
  case nd_deref:
    gen(nd->lhs);
    return;
  }

  error_tok(nd->tok, "not an lvalue");
}

void gen_lval(struct node *nd) {
  if(nd->ty->kind == ty_array)
    error_tok(nd->tok, "not an lvalue");

  gen_addr(nd);  
}
