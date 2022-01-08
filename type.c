#include "type.h"

typ_t *char_type() {
  typ_t *tp = (typ_t *)malloc(sizeof(typ_t));
  init_type(tp, ty_char, NULL, 0);
  return tp;
}

typ_t *int_type() {
  typ_t *tp = (typ_t *)malloc(sizeof(typ_t));
  init_type(tp, ty_int, NULL, 0);
  return tp;
}

typ_t *pointer_to(typ_t *b) {
  typ_t *tp = (typ_t *)malloc(sizeof(typ_t));
  init_type(tp, ty_ptr, b, 0);
  return tp;
}

typ_t *array_of(typ_t *b, int s) {
  typ_t *tp = (typ_t *)malloc(sizeof(typ_t));
  init_type(tp, ty_array, b, s);
  return tp;
}

int size_of(typ_t *ty) {
  switch (ty->kind) {
  case ty_char:
    return 1;
  case ty_int:
  case ty_ptr:
    return 8;
  }
  return size_of(ty->base) * ty->array_size;
}

void visit(node_t *n) {
  if (n == NULL)
    return;

  visit(n->lhs);
  visit(n->rhs);
  visit(n->cond);
  visit(n->then);
  visit(n->els);
  visit(n->init);
  visit(n->inc);

  node_t *b = n->body;
  while (b != NULL) {
    visit(b);
    b = b->next;
  }

  node_t *a = n->args;
  while (a != NULL) {
    visit(a);
    a = a->next;
  }

  node_t *tmp;
  switch (n->kind) {
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
      tmp = n->lhs;
      n->lhs = n->rhs;
      n->rhs = tmp;
    }
    if (n->rhs->ty->base != NULL)
      error_tok(n->tok, "invalid pointer arithmetic operands");
    n->ty = n->lhs->ty;
    return;
  case nd_sub:
    if (n->rhs->ty->base != NULL)
      error_tok(n->tok, "invalid pointer arithmetic operands");
    n->ty = n->lhs->ty;
    return;

  case nd_assign:
    n->ty = n->lhs->ty;
    return;
  case nd_addr:
    if (n->lhs->ty->kind == ty_array)
      n->ty = pointer_to(n->lhs->ty->base);
    else
      n->ty = pointer_to(n->lhs->ty);
    return;
  case nd_deref:
    if (n->lhs->ty->base == NULL)
      error_tok(n->tok, "invalid pointer dereference");
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

void add_type(program_t *p) {
  fun_t *fn = p->fns;

  while (fn != NULL) {
    node_t *n = fn->node;
    while (n != NULL) {
      visit(n);
      n = n->next;
    }
    fn = fn->next;
  }
}
