#include <stdio.h>
#include "codegen.h"

void gen_addr(struct node *nd) {
  struct va *v;
  switch(n->kind) {
  case nd_var:
    v = n->v;
    if(v.is_local) {
      printf("  lea rax, [rbp-%d]\n", v->offset);
      printf("  push rax\n");      
    }else printf("  push offset %s\n", v->name);

    return;
  case nd_deref:
    gen(n->lhs);
    return;
  }

  error_tok(n->tok, "not an lvalue");
}

void gen_lval(struct node *n) {
  if(n->ty->kind == ty_array)
    error_tok(n->tok, "not an lvalue");

  gen_addr(n);  
}
