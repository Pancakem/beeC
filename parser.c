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
    if (strlen(v->name) == tok->len && strncmp(tok->str, v->name, tok->len)) {
      return v;
    }

    vl = vl->next;
  }
  vl = globals;
  while(vl != NULL) {
    struct va *v = vl->v;
    if (strlen(v->name) == tok->len && strncmp(tok->str, v->name, tok->len)) {
      return v;
    }
    vl = vl->next;
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
  struct token *tok = (struct token*) malloc(sizeof(struct token));
  if ((tok = consume(")")) != NULL) return NULL;
  struct node *h = assign();
  struct node *cur = h;
  tok = consume(",");
  while(tok != NULL) {
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
  if(consume("+") != NULL)
    return unary();

  struct token *tok = (struct token*) malloc(sizeof(struct token));
  if((tok = consume("-")) != NULL)
    return new_binary(nd_sub, new_number(0, tok), unary(), tok);
  if((tok = consume("&")) != NULL)
    return new_unary(nd_addr, unary(), tok);
  if((tok = consume("*")) != NULL)
    return new_unary(nd_deref, unary(), tok);

  return post_fix();
}
 

struct node *mul() {
  struct node *n = unary();
  struct token *tok = (struct token*) malloc(sizeof(struct token));
  while(true) {
    if((tok = consume("*")) != NULL)
      n = new_binary(nd_mul, n, unary(), tok);
    else if((tok = consume("/")) != NULL)
      n = new_binary(nd_div, n, unary(), tok);
    else
      return n;
  }
  
}

struct node *add() {
  struct node *n = mul();
  struct token *tok = (struct token*) malloc(sizeof(struct token));
  while(true) {
    if((tok = consume("+")) != NULL)
      n = new_binary(nd_add, n, mul(), tok);
    else if((tok = consume("-")) != NULL)
      n = new_binary(nd_sub, n, mul(), tok);
    else
      return n;
  }
  
}

struct node *relational() {
  struct node *n = add();
  struct token *tok = (struct token*) malloc(sizeof(struct token));
  while(true) {
    if((tok = consume("<")) != NULL)
      n = new_binary(nd_lt, n, add(), tok);
    else if((tok = consume("<=")) != NULL)
      n = new_binary(nd_le, n, add(), tok);
    else if((tok = consume(">")) != NULL)
      n = new_binary(nd_lt, n, add(), tok); // gt?
    else if((tok = consume(">=")) != NULL)
      n = new_binary(nd_le, n, add(), tok); // ge?
    else
      return n;
  }
}

struct node *equality() {
  struct node *n = relational();
  struct token *tok = (struct token*) malloc(sizeof(struct token));
  while(true) {
    if((tok = consume("==")) != NULL)
      n = new_binary(nd_eq, n, relational(), tok);
    else if((tok = consume("!=")) != NULL)
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
  struct token *tok = (struct token*) malloc(sizeof(struct token));
  if((tok = consume("return")) != NULL) {
    struct node *n = new_unary(nd_ret, expr(), tok);
    expect(";");
    return n;    
  }
  
  if((tok = consume("if")) != NULL) {
    struct node *n = (struct node*) malloc(sizeof(struct node));
    n->kind = nd_if;
    n->tok = tok;
    expect("(");
    n->cond = expr();
    expect(")");
    n->then = stmt();
    if (consume("else") != NULL)
      n->els = stmt();
    return n;    
  }

  if((tok = consume("while")) != NULL) {
    struct node *n = (struct node*) malloc(sizeof(struct node));
    n->kind = nd_while;
    n->tok = tok;
    expect("(");
    n->cond = expr();
    expect(")");
    n->then = stmt();
    return n;
  }

  if((tok = consume("for")) != NULL) {
    struct node *n = (struct node*) malloc(sizeof(struct node));
    n->kind = nd_for;
    n->tok = tok;
    expect("(");
    if(consume(";") == NULL) {
      n->init = read_expr_stmt();
      expect(";");
    }
    if(consume(";") == NULL) {
      n->cond = read_expr_stmt();
      expect(";");
    }
    if(consume(")") == NULL) {
      n->inc = read_expr_stmt();
      expect(")");
    }
    n->then = stmt();
    return n;
  }

  if((tok = consume("{")) != NULL) {
    struct node *h = (struct node*) malloc(sizeof(struct node));
    struct node *current = h;
    while(consume("}") == NULL) {
      current->next = stmt();
      current = current->next;
    }
    struct node *n = (struct node*) malloc(sizeof(struct node));
    n->body = h->next;
    return n;
  }

  if (is_type_name()) return declaration();

  struct node *n = read_expr_stmt();
  expect(";");
  return n;  
}

bool is_type_name() {
  return peek("char") || peek("int");
}

struct node *read_expr_stmt() {
  struct token *tt = t;
  return new_unary(nd_expr_stmt, expr(), tt);
}

struct node *declaration() {
  struct token* tok = t;
  struct typ *ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  struct va* v = push_var(name, ty, true);
  if (consume(";") != NULL) {
    struct node* nd = (struct node*)malloc(sizeof(struct node));
    nd->kind = nd_null;
    nd->tok = tok;
    return nd;   
  }
  expect("=");
  struct node *lhs = (struct node*)malloc(sizeof(struct node));;
  lhs->kind = nd_var;
  lhs->tok = tok;
  lhs->v = v;
  struct node *rhs = expr();
  expect(";");
  struct node *n = new_binary(nd_assign, lhs, rhs, tok);
  return new_unary(nd_expr_stmt, n, tok);
}

void global_var() {
  struct typ* ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);
  expect(";");
  push_var(name, ty, false);
}

struct fun *function() {
  locals = NULL;
  base_type();
  struct fun* fn = (struct fun*)malloc(sizeof(struct fun));
  fn->name = expect_ident();
  expect("(");
  fn->params = read_func_params();
  expect("{");
  struct node *h = (struct fun*)malloc(sizeof(struct node));
  struct node *current = h;
  while(consume("}") == NULL) {
    current->next = stmt();
    current = current->next;
  }
  fn->node = h->next;
  fn->locals = locals;
  return fn;
}

struct typ *base_type() {
  struct typ* ty = (struct typ*) malloc(sizeof(struct typ));
  if (consume("char") != NULL)
    ty = char_type();
  else {
    expect("int");
    ty = int_type();    
  }
  while(consume("*") == NULL)
    ty = pointer_to(ty);
  return ty;
}

struct typ *read_type_suffix(struct typ* b) {
  if (consume("[") == NULL) return b;
  int arr_size = expect_number();
  expect("]");
  b = read_type_suffix(b);
  return array_of(b, arr_size);

}

struct var_list* read_func_param() {
  struct typ* ty = base_type();
  char *name = expect_ident();
  ty = read_type_suffix(ty);

  struct var_list *res = (struct var_list*) malloc(sizeof(struct var_list));
  res->v = push_var(name, ty, true);
  return res;
}

struct var_list* read_func_params() {
  if(consume("(") != NULL) return NULL;
  
  struct var_list* h = read_func_param();
  struct var_list* current = h;
  
  while(consume(")") == NULL) {
    expect(",");
    current->next = read_func_param();
    current = current->next;
  }
  return h;
}

bool is_function() {
  struct token* tok = t;
  base_type();
  bool f = (consume_ident() != NULL && consume("(") != NULL );
  t = tok;
  return f;
}

struct program *prog() {
  struct fun* h = (struct fun*)malloc(sizeof(struct fun));
  struct fun* current = h;
  
  globals = NULL;
  while(!at_eof()) {
    if (is_function()) {
      current->next = function();
      current = current->next;      
    }else global_var();  
  }

  struct program *pg = (struct program*) malloc(sizeof(struct program));
  pg->globals = globals;
  pg->fns = h->next;
  return pg;
}

