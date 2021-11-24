#include "parser.h"
#include "tokenizer.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int label_count = 0;
struct var_list *locals;
struct var_list *globals;

void init_node(struct node *init_node, enum node_kind kind, struct node *next,
               struct typ *ty, struct token *tok, struct node *lhs,
               struct node *rhs, struct node *cond, struct node *then,
               struct node *els, struct node *init, struct node *inc,
               struct node *body, char *func_name, struct node *args,
               struct va *v, int val) {

  init_node->kind = kind;
  init_node->next = next;
  init_node->ty = ty;
  init_node->tok = tok;
  init_node->lhs = lhs;
  init_node->rhs = rhs;
  init_node->cond = cond;
  init_node->then = then;
  init_node->els = els;
  init_node->init = init;
  init_node->inc = inc;
  init_node->body = body;
  init_node->func_name = func_name;
  init_node->args = args;
  init_node->v = v;
  init_node->val = val;
}

void init_va(struct va *v, char *name, struct typ *ty, bool is_local,
             char *contents, int content_len, int offset) {
  v->name = name;
  v->ty = ty;
  v->is_local = is_local;
  v->contents = contents;
  v->content_length = content_len;
  v->offset = offset;
}

void init_type(struct typ *ty, enum type_kind kind, struct typ *base,
               int array_size) {
  ty->kind = kind;
  ty->base = base;
  ty->array_size = array_size;
}

void init_var_list(struct var_list *vl, struct var_list *next, struct va *v) {
  vl->next = next;
  vl->v = v;
}

void init_function(struct fun *fn, struct fun *next, char *name,
                   struct var_list *params, struct node *nde,
                   struct var_list *locals, int stack_size) {
  fn->next = next;
  fn->name = name;
  fn->params = params;
  fn->node = nde;
  fn->locals = locals;
  fn->stack_size = stack_size;
}

struct va *find_var(struct token *tok) {
  struct var_list *vl = locals;
  struct va *v;
  while (vl != NULL) {
    v = vl->v;
    if (strlen(v->name) == tok->len && strncmp(tok->str, v->name, tok->len)) {
      return v;
    }

    vl = vl->next;
  }
  vl = globals;
  while (vl != NULL) {
    v = vl->v;
    if (strlen(v->name) == tok->len && strncmp(tok->str, v->name, tok->len)) {
      return v;
    }
    vl = vl->next;
  }

  return NULL;
}

struct node *new_unary(enum node_kind k, struct node *n, struct token *tok) {
  struct node *nd = (struct node *)malloc(sizeof(struct node));
  init_node(nd, k, NULL, NULL, tok, n, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, 0);
  return nd;
}

struct node *new_binary(enum node_kind k, struct node *lhs, struct node *rhs,
                        struct token *tok) {
  struct node *n = (struct node *)malloc(sizeof(struct node));
  init_node(n, k, NULL, NULL, tok, lhs, rhs, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, 0);
  return n;
}

struct node *new_number(int v, struct token *tok) {
  struct node *n = (struct node *)malloc(sizeof(struct node));
  init_node(n, nd_num, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, NULL, v);
  return n;
}

struct node *new_var(struct va *v, struct token *tok) {
  struct node *n = (struct node *)malloc(sizeof(struct node));
  init_node(n, nd_var, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, v, 0);
  return n;
}

char *new_label() {
  char *name = (char *)malloc(sizeof(char) * 20);
  sprintf(name, ".L.data.%d", label_count);
  return name;
}

struct va *push_var(char *name, struct typ *ty, bool is_local) {
  struct va *v = (struct va *)malloc(sizeof(struct va));
  init_va(v, name, ty, is_local, NULL, 0, 0);

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
  if (consume("(") != NULL) {
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
      struct node *n = (struct node *)malloc(sizeof(struct node));
      init_node(n, nd_func_call, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, tok->str, func_args(), NULL, 0);
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
  struct token *tok;
  if ((tok = consume(")")) != NULL)
    return NULL;

  struct node *h = assign();
  struct node *cur = h;
  tok = consume(",");
  while (tok != NULL) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return h;
}

struct node *post_fix() {
  struct node *n = primary();
  for (struct token *tok = consume("["); tok != NULL; tok = consume("[")) {
    struct node *exp = new_binary(nd_add, n, expr(), tok);
    expect("]");
    n = new_unary(nd_deref, exp, tok);
  }
  return n;
}

struct node *unary() {
  if (consume("+") != NULL)
    return unary();

  struct token *tok;
  if ((tok = consume("-")) != NULL)
    return new_binary(nd_sub, new_number(0, tok), unary(), tok);
  if ((tok = consume("&")) != NULL)
    return new_unary(nd_addr, unary(), tok);
  if ((tok = consume("*")) != NULL)
    return new_unary(nd_deref, unary(), tok);

  return post_fix();
}

struct node *mul() {
  struct node *n = unary();
  struct token *tok;
  while (true) {
    if ((tok = consume("*")) != NULL)
      n = new_binary(nd_mul, n, unary(), tok);
    else if ((tok = consume("/")) != NULL)
      n = new_binary(nd_div, n, unary(), tok);
    else
      return n;
  }
}

struct node *add() {
  struct node *n = mul();
  struct token *tok;
  while (true) {
    if ((tok = consume("+")) != NULL)
      n = new_binary(nd_add, n, mul(), tok);
    else if ((tok = consume("-")) != NULL)
      n = new_binary(nd_sub, n, mul(), tok);
    else
      return n;
  }
}

struct node *relational() {
  struct node *n = add();
  struct token *tok;
  while (true) {
    if ((tok = consume("<")) != NULL)
      n = new_binary(nd_lt, n, add(), tok);
    else if ((tok = consume("<=")) != NULL)
      n = new_binary(nd_le, n, add(), tok);
    else if ((tok = consume(">")) != NULL)
      n = new_binary(nd_lt, n, add(), tok); // gt?
    else if ((tok = consume(">=")) != NULL)
      n = new_binary(nd_le, n, add(), tok); // ge?
    else
      return n;
  }
}

struct node *equality() {
  struct node *n = relational();
  struct token *tok;
  while (true) {
    if ((tok = consume("==")) != NULL)
      n = new_binary(nd_eq, n, relational(), tok);
    else if ((tok = consume("!=")) != NULL)
      n = new_binary(nd_ne, n, relational(), tok);
    else
      return n;
  }
}

struct node *assign() {
  struct node *n = equality();
  struct token *tok = consume("=");
  if (tok != NULL)
    n = new_binary(nd_assign, n, assign(), tok);

  return n;
}

struct node *expr() {
  return assign();
}

struct node *stmt() {
  struct token *tok;
  if ((tok = consume("return")) != NULL) {
    struct node *n = new_unary(nd_ret, expr(), tok);
    expect(";");
    return n;
  }

  if ((tok = consume("if")) != NULL) {
    struct node *n = (struct node *)malloc(sizeof(struct node));
    init_node(n, nd_if, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
              NULL, NULL, NULL, NULL, NULL, 0);
    expect("(");
    n->cond = expr();
    expect(")");
    n->then = stmt();
    if (consume("else") != NULL)
      n->els = stmt();
    return n;
  }

  if ((tok = consume("while")) != NULL) {
    struct node *n = (struct node *)malloc(sizeof(struct node));
    init_node(n, nd_while, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
              NULL, NULL, NULL, NULL, NULL, 0);
    expect("(");
    n->cond = expr();
    expect(")");
    n->then = stmt();
    return n;
  }

  if ((tok = consume("for")) != NULL) {
    struct node *n = (struct node *)malloc(sizeof(struct node));
    init_node(n, nd_for, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
              NULL, NULL, NULL, NULL, NULL, 0);

    expect("(");
    if (consume(";") == NULL) {
      n->init = read_expr_stmt();
      expect(";");
    }
    if (consume(";") == NULL) {
      n->cond = read_expr_stmt();
      expect(";");
    }
    if (consume(")") == NULL) {
      n->inc = read_expr_stmt();
      expect(")");
    }
    n->then = stmt();
    return n;
  }

  if ((tok = consume("{")) != NULL) {
    struct node h;
    struct node *current = &h;
    while (consume("}") == NULL) {
      current->next = stmt();
      current = current->next;
    }
    struct node *n = (struct node *)malloc(sizeof(struct node));
    init_node(n, nd_block, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
              NULL, h.next, NULL, NULL, NULL, 0);
    return n;
  }

  if (is_type_name())
    return declaration();

  struct node *n = read_expr_stmt();
  expect(";");
  return n;
}

bool is_type_name() { return peek("char") || peek("int"); }

struct node *read_expr_stmt() {
  struct token *tt = t;
  return new_unary(nd_expr_stmt, expr(), tt);
}

struct node *declaration() {
  struct token *tok = t;
  struct typ *ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  struct va *v = push_var(name, ty, true);
  if (consume(";") != NULL) {
    struct node *nd = (struct node *)malloc(sizeof(struct node));
    init_node(nd, nd_null, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
              NULL, NULL, NULL, NULL, NULL, 0);
    return nd;
  }
  expect("=");
  struct node *lhs = (struct node *)malloc(sizeof(struct node));
  init_node(lhs, nd_var, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, NULL, 0);
  struct node *rhs = expr();
  expect(";");
  struct node *n = new_binary(nd_assign, lhs, rhs, tok);
  return new_unary(nd_expr_stmt, n, tok);
}

void global_var() {
  struct typ *ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  expect(";");
  push_var(name, ty, false);
}

struct fun *function() {
  locals = NULL;
  base_type();
  struct fun *fn = (struct fun *)malloc(sizeof(struct fun));
  init_function(fn, NULL, expect_ident(), NULL, NULL, NULL, NULL);
  expect("(");
  fn->params = read_func_params();
  expect("{");
  struct node h;
  struct node *current = &h;
  while (consume("}") == NULL) {
    current->next = stmt();
    current = current->next;
  }
  fn->node = h.next;
  fn->locals = locals;
  return fn;
}

struct typ *base_type() {
  struct typ *ty = (struct typ *)malloc(sizeof(struct typ));
  if (consume("char") != NULL)
    ty = char_type();
  else {
    expect("int");
    ty = int_type();
  }
  while (consume("*") != NULL)
    ty = pointer_to(ty);
  return ty;
}

struct typ *read_type_suffix(struct typ *b) {
  if (consume("[") == NULL)
    return b;
  int arr_size = expect_number();
  expect("]");
  b = read_type_suffix(b);
  return array_of(b, arr_size);
}

struct var_list *read_func_param() {
  struct typ *ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);

  struct var_list *res = (struct var_list *)malloc(sizeof(struct var_list));
  init_var_list(res, NULL, push_var(name, ty, true));
  return res;
}

struct var_list *read_func_params() {
  if (consume(")") != NULL)
    return NULL;

  struct var_list *h = read_func_param();
  struct var_list *current = h;

  while (consume(")") == NULL) {
    expect(",");
    current->next = read_func_param();
    current = current->next;
  }
  return h;
}

bool is_function() {
  struct token *tok = t;
  base_type();
  bool is_ident = consume_ident() != NULL;
  bool is_func_open = consume("(") != NULL;
  t = tok;
  return is_ident && is_func_open;
}

struct program *prog() {
  struct fun h;
  struct fun *current = &h;

  globals = NULL;
  while (!at_eof()) {
    if (is_function()) {
      current->next = function();
      current = current->next;
      printf("Found function %s\n", current->next->name);
    } else
      global_var();
  }

  struct program *pg = (struct program *)malloc(sizeof(struct program));
  pg->globals = globals;
  pg->fns = h.next;
  return pg;
}
