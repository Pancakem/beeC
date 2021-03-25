#include "type.h"

struct typ* char_type() {
  struct typ tp = {.kind = ty_char};
  return &tp;
}

struct typ* int_type() {
  struct typ tp = {.kind = ty_int};
  return &tp;
}

struct typ* pointer_to(struct typ* b) {
  struct typ tp = {.kind = ty_ptr, .base = b};
  return &tp;
}

struct typ* array_of(struct typ* b, int s) {
  struct typ tp = {.kind = ty_array, .base = b, .array_size = s};
  return &tp;
}

int size_of(struct typ* ty) {
  switch(ty->kind) {
  case ty_char:
    return 1;
  case ty_int:
  case ty_ptr:
    return 8;
  }
  return size_of(ty->base) * ty->array_size;  
}

void visit(struct node *n) {
  if (n == NULL) return;

  visit(n->lhs);
  visit(n->rhs);
  visit(n->cond);
  visit(n->then);
  visit(n->els);
  visit(n->init);
  visit(n->inc);

  struct node *b = n->body;
  while(b != NULL) {
    visit(b);
    b = b->next;    
  }

  struct node *a = n->args;
  while(a != NULL) {
    visit(a);
    a = a->next;
  }

  switch(n->kind) {
  case nd_mul:
  case nd_div:
  case nd_eq:
  case nd_ne:
  case nd_lt:
  case nd_le:
  case nd_func_call:
  case nd_num:
    n->ty = int_type();
    return;
  case nd_var:
    n->ty = n->v->ty;
    return;
  case nd_add:
    if (n->rhs->ty->base != NULL) {
      struct node* tmp = n->lhs;
      n->lhs = n->rhs;
      n->rhs = tmp;
    }
    if (n->rhs->ty->base != NULL) error_tok(n->tok, "invalid pointer arithmetic operands");
    n->ty = n->lhs->ty;
    return;
  case nd_sub:
    if (n->rhs->ty->base != NULL) error_tok(n->tok, "invalid pointer arithmetic operands");
    n->ty = n->lhs->ty;
    return;

  case nd_assign:
    n->ty = n->lhs->ty;
    return;
  case nd_addr:
    if (n->lhs->ty->kind == ty_array) n->ty = pointer_to(n->lhs->ty->base);
    else n->ty = pointer_to(n->lhs->ty);
    return;
  case nd_deref:
    if (n->lhs->ty->base == NULL) error_tok(n->tok, "invalid pointer dereference");
    n->ty = n->rhs->ty->base;
    return;

  case nd_sizeof:
    n->kind = nd_num;
    n->ty = int_type();
    n->val = size_of(n->lhs->ty);
    n->lhs = NULL;
    return; 
  }
}

void add_type(struct program* p) {
  struct fun* fn = p->fns;

  while (fn != NULL) {
    struct node *n = fn->node;
    while(n != NULL) {
      visit(n);
      n = n->next;      
    }
    fn = fn->next;    
  }
  
}


