#include "parser.h"
#include "tokenizer.h"
#include "type.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int label_count = 0;
var_list_t *locals;
var_list_t *globals;

void init_node(node_t *init_node, node_kind_t kind, node_t *next, typ_t *ty,
               token_t *tok, node_t *lhs, node_t *rhs, node_t *cond,
               node_t *then, node_t *els, node_t *init, node_t *inc,
               node_t *body, char *func_name, node_t *args, va_t *v, int val) {

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

void init_va(va_t *v, char *name, typ_t *ty, bool is_local, char *contents,
             int content_len, int offset) {
  v->name = name;
  v->ty = ty;
  v->is_local = is_local;
  v->contents = contents;
  v->content_length = content_len;
  v->offset = offset;
}

void init_type(typ_t *ty, type_kind_t kind, typ_t *base, int array_size) {
  ty->kind = kind;
  ty->base = base;
  ty->array_size = array_size;
}

void init_var_list(var_list_t *vl, var_list_t *next, va_t *v) {
  vl->next = next;
  vl->v = v;
}

void init_function(fun_t *fn, fun_t *next, char *name, var_list_t *params,
                   node_t *nde, var_list_t *locals, int stack_size) {
  fn->next = next;
  fn->name = name;
  fn->params = params;
  fn->node = nde;
  fn->locals = locals;
  fn->stack_size = stack_size;
}

va_t *find_var(token_t *tok) {
  var_list_t *vl = locals;
  va_t *v;
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

node_t *new_unary(node_kind_t k, node_t *n, token_t *tok) {
  node_t *nd = (node_t *)malloc(sizeof(node_t));
  init_node(nd, k, NULL, NULL, tok, n, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, 0);
  return nd;
}

node_t *new_binary(node_kind_t k, node_t *lhs, node_t *rhs, token_t *tok) {
  node_t *n = (node_t *)malloc(sizeof(node_t));
  init_node(n, k, NULL, NULL, tok, lhs, rhs, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, 0);
  return n;
}

node_t *new_number(int v, token_t *tok) {
  node_t *n = (node_t *)malloc(sizeof(node_t));
  init_node(n, nd_num, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, NULL, v);
  return n;
}

node_t *new_var(va_t *v, token_t *tok) {
  node_t *n = (node_t *)malloc(sizeof(node_t));
  init_node(n, nd_var, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, v, 0);
  return n;
}

char *new_label() {
  char *name = (char *)malloc(sizeof(char) * 20);
  sprintf(name, ".L.data.%d", label_count);
  return name;
}

va_t *push_var(char *name, typ_t *ty, bool is_local) {
  va_t *v = (va_t *)malloc(sizeof(va_t));
  init_va(v, name, ty, is_local, NULL, 0, 0);

  var_list_t vl = {.v = v};

  if (is_local) {
    vl.next = locals;
    locals = &vl;
  } else {
    vl.next = globals;
    globals = &vl;
  }
  return v;
}

node_t *primary() {
  if (consume("(") != NULL) {
    node_t *n = expr();
    expect(")");
    return n;
  }

  token_t *tok = consume("sizeof");
  if (tok != NULL) {
    return new_unary(nd_sizeof, unary(), tok);
  }

  tok = consume_ident();
  if (tok != NULL) {
    if (consume("(") == NULL) {
      node_t *n = (node_t *)malloc(sizeof(node_t));
      init_node(n, nd_func_call, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, tok->str, func_args(), NULL, 0);
      return n;
    }

    va_t *v = find_var(tok);
    if (v == NULL) {
      error_tok(tok, "undefined variable");
    }
    return new_var(v, tok);
  }

  tok = t;

  if (tok->kind == tk_str) {
    t = t->next;
    typ_t *cty = char_type();
    typ_t *ty = array_of(cty, tok->content_length);
    va_t *v = push_var(new_label(), ty, false);
    v->contents = tok->contents;
    v->content_length = tok->content_length;
    return new_var(v, tok);
  } else if (tok->kind == tk_ident) {
    t = t->next;
    return relational();
  } else if (tok->kind == tk_reserved) {
    t = t->next;
    return relational();
  }

  if (tok->kind != tk_num) {
    error_tok(tok, "expected expression");
  }

  return new_number(expect_number(), tok);
}

node_t *func_args() {
  token_t *tok;
  if ((tok = consume(")")) != NULL)
    return NULL;

  node_t *h = assign();
  node_t *cur = h;
  tok = consume(",");
  while (tok != NULL) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return h;
}

node_t *post_fix() {
  node_t *n = primary();
  for (token_t *tok = consume("["); tok != NULL; tok = consume("[")) {
    node_t *exp = new_binary(nd_add, n, expr(), tok);
    expect("]");
    n = new_unary(nd_deref, exp, tok);
  }
  return n;
}

node_t *unary() {
  if (consume("+") != NULL)
    return unary();

  token_t *tok;
  if ((tok = consume("-")) != NULL)
    return new_binary(nd_sub, new_number(0, tok), unary(), tok);
  if ((tok = consume("&")) != NULL)
    return new_unary(nd_addr, unary(), tok);
  if ((tok = consume("*")) != NULL)
    return new_unary(nd_deref, unary(), tok);

  return post_fix();
}

node_t *mul() {
  node_t *n = unary();
  token_t *tok;
  while (true) {
    if ((tok = consume("*")) != NULL)
      n = new_binary(nd_mul, n, unary(), tok);
    else if ((tok = consume("/")) != NULL)
      n = new_binary(nd_div, n, unary(), tok);
    else
      return n;
  }
}

node_t *add() {
  node_t *n = mul();
  token_t *tok;
  while (true) {
    if ((tok = consume("+")) != NULL)
      n = new_binary(nd_add, n, mul(), tok);
    else if ((tok = consume("-")) != NULL)
      n = new_binary(nd_sub, n, mul(), tok);
    else
      return n;
  }
}

node_t *relational() {
  node_t *n = add();
  token_t *tok;
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

node_t *equality() {
  node_t *n = relational();
  token_t *tok;
  while (true) {
    if ((tok = consume("==")) != NULL)
      n = new_binary(nd_eq, n, relational(), tok);
    else if ((tok = consume("!=")) != NULL)
      n = new_binary(nd_ne, n, relational(), tok);
    else
      return n;
  }
}

node_t *assign() {
  node_t *n = equality();
  token_t *tok = consume("=");
  if (tok != NULL)
    n = new_binary(nd_assign, n, assign(), tok);

  return n;
}

node_t *expr() { return assign(); }

node_t *stmt() {
  token_t *tok;
  if ((tok = consume("return")) != NULL) {
    node_t *n = new_unary(nd_ret, expr(), tok);
    expect(";");
    return n;
  }

  if ((tok = consume("if")) != NULL) {
    node_t *n = (node_t *)malloc(sizeof(node_t));
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
    node_t *n = (node_t *)malloc(sizeof(node_t));
    init_node(n, nd_while, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
              NULL, NULL, NULL, NULL, NULL, 0);
    expect("(");
    n->cond = expr();
    expect(")");
    n->then = stmt();
    return n;
  }

  if ((tok = consume("for")) != NULL) {
    node_t *n = (node_t *)malloc(sizeof(node_t));
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
    node_t h;
    node_t *current = &h;
    while (consume("}") == NULL) {
      current->next = stmt();
      current = current->next;
    }
    node_t *n = (node_t *)malloc(sizeof(node_t));
    init_node(n, nd_block, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
              NULL, h.next, NULL, NULL, NULL, 0);
    return n;
  }

  if (is_type_name())
    return declaration();

  node_t *n = read_expr_stmt();
  expect(";");
  return n;
}

bool is_type_name() { return peek("char") || peek("int"); }

node_t *read_expr_stmt() {
  token_t *tt = t;
  return new_unary(nd_expr_stmt, expr(), tt);
}

node_t *declaration() {
  token_t *tok = t;
  typ_t *ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  va_t *v = push_var(name, ty, true);
  if (consume(";") != NULL) {
    node_t *nd = (node_t *)malloc(sizeof(node_t));
    init_node(nd, nd_null, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
              NULL, NULL, NULL, NULL, NULL, 0);
    return nd;
  }
  expect("=");
  node_t *lhs = (node_t *)malloc(sizeof(node_t));
  init_node(lhs, nd_var, NULL, NULL, tok, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, NULL, 0);
  node_t *rhs = expr();
  expect(";");
  node_t *n = new_binary(nd_assign, lhs, rhs, tok);
  return new_unary(nd_expr_stmt, n, tok);
}

void global_var() {
  typ_t *ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  expect(";");
  push_var(name, ty, false);
}

fun_t *function() {
  locals = NULL;
  base_type();
  fun_t *fn = (fun_t *)malloc(sizeof(fun_t));
  init_function(fn, NULL, expect_ident(), NULL, NULL, NULL, NULL);
  expect("(");
  fn->params = read_func_params();
  expect("{");
  node_t h;
  node_t *current = &h;
  while (consume("}") == NULL) {
    current->next = stmt();
    current = current->next;
  }
  fn->node = h.next;
  fn->locals = locals;
  return fn;
}

typ_t *base_type() {
  typ_t *ty = (typ_t *)malloc(sizeof(typ_t));
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

typ_t *read_type_suffix(typ_t *b) {
  if (consume("[") == NULL)
    return b;
  int arr_size = expect_number();
  expect("]");
  b = read_type_suffix(b);
  return array_of(b, arr_size);
}

var_list_t *read_func_param() {
  typ_t *ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);

  var_list_t *res = (var_list_t *)malloc(sizeof(var_list_t));
  init_var_list(res, NULL, push_var(name, ty, true));
  return res;
}

var_list_t *read_func_params() {
  if (consume(")") != NULL)
    return NULL;

  var_list_t *h = read_func_param();
  var_list_t *current = h;

  while (consume(")") == NULL) {
    expect(",");
    current->next = read_func_param();
    current = current->next;
  }
  return h;
}

bool is_function() {
  token_t *tok = t;
  base_type();
  bool is_ident = consume_ident() != NULL;
  bool is_func_open = consume("(") != NULL;
  t = tok;
  return is_ident && is_func_open;
}

program_t *prog() {
  fun_t h;
  fun_t *current = &h;

  globals = NULL;
  while (!at_eof()) {
    if (is_function()) {
      current->next = function();
      current = current->next;
      printf("Found function %s\n", current->next->name);
    } else
      global_var();
  }

  program_t *pg = (program_t *)malloc(sizeof(program_t));
  pg->globals = globals;
  pg->fns = h.next;
  return pg;
}
