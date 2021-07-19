#include "parser.h"
#include "tokenizer.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int label_count = 0;

struct va *find_var(struct token *tok) {
  struct var_list *vl = locals;
  while (vl != NULL) {
    struct va *v = vl->v;
    // TODO!!
    if (strlen(v->name) == tok->len) {
      return v;
    }
  }
  return NULL;
}

struct node *new_unary(enum node_kind k, struct node *n, struct token *tok) {
  struct node *nd = (struct node *)malloc(sizeof(struct node));
  nd->kind = k;
  nd->lhs = n;
  nd->tok = tok;
  return nd;
}

struct node *new_binary(enum node_kind k, struct node *lhs, struct node *rhs,
                        struct token *tok) {
  struct node *n = (struct node *)malloc(sizeof(struct node));
  n->kind = k;
  n->lhs = lhs;
  n->rhs = rhs;
  n->tok = tok;
  return n;
}

struct node *new_number(int v, struct token *tok) {
  struct node *n = (struct node *)malloc(sizeof(struct node));
  n->kind = nd_num;
  n->val = v;
  n->tok = tok;
  return n;
}

struct node *new_var(struct va *v, struct token *tok) {
  struct node *n = (struct node*)malloc(sizeof(struct node));
  n->kind = nd_var;
  n->v = v;
  n->tok = tok;
  return n;
}

char *new_label() {
  char *name = (char *)malloc(sizeof(char) * 20);
  sprintf(name, ".L.data.%d", label_count);
  return name;
}

struct va *push_var(char *name, struct typ *ty, bool is_local) {
  struct va *v = (struct va *)malloc(sizeof(struct va));
  v->name = name;
  v->ty = ty;
  v->is_local = is_local;

  struct var_list vl = {.v = v};

  if (is_local) {
    vl.next = locals;
    locals = &vl;
  } else {
    vl.next = globals;
    globals = &vl;
  }
  return v;
}

struct node *primary() {
  if (consume("(") == NULL) {
    struct node *n = expr();
    expect(")");
    return n;
  }

  struct token *tok = consume("sizeof");
  if (tok != NULL) {
    return new_unary(nd_sizeof, unary(), tok);
  }

  tok = consume_ident();
  if (tok != NULL) {
    if (consume("(") == NULL) {
      struct node *n = (struct node*)malloc(sizeof(struct node));
      n->kind = nd_func_call;
      n->func_name = tok->str;
      n->args = func_args();
      n->tok = tok;
      return n;
    }

    struct va *v = find_var(tok);
    if (v == NULL) {
      error_tok(tok, "undefined variable");
    }
    return new_var(v, tok);
  }

  tok = t;

  if (tok->kind == tk_str) {
    t = t->next;
    struct typ *cty = char_type();
    struct typ *ty = array_of(cty, tok->content_length);
    struct va *v = push_var(new_label(), ty, false);
    v->contents = tok->contents;
    v->content_length = tok->content_length;
    return new_var(v, tok);
  }

  if (tok->kind != tk_num) {
    error_tok(tok, "expected expression");
  }

  return new_number(expect_number(), tok);
}

struct node *func_args() {
  struct token *tk;
  if ((tk = consume(")")) != NULL) return NULL;
  struct node *h = assign();
  struct node *cur = h;
  for (tk = consume(","); tk != NULL; ) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return h;
}

struct node* post_fix() {
  struct node *n = primary();
  for (struct token *tok = consume("["); tok != NULL; tok = consume("[")) {
    struct node *exp = new_binary(nd_add, n, expr(), tok);
    expect("]");
    n = new_unary(nd_deref, exp, tok);
  }
  return n;  
}

struct node *unary() {
  
}

struct node *mul() {
  
}

struct node *add() {
  
}

struct node *relational() {
  
}

struct node *equality() {
  
}

struct node *assign() {
  
}

struct node *expr() {
  
}

struct node *stmt() {
}

bool is_type_name() {}

struct node *read_expr_stmt() {
  
}

struct node *declaration() {
  
}

void global_var() {}

struct fun *function() {}

struct typ *base_type() {}

struct typ *read_type_suffix(struct typ* b) {}

struct var_list* read_func_params() {}

bool is_function() {}

struct program *prog() {}

