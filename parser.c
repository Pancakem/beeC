#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tokenizer.c"

enum node_kind {
  nd_add,
  nd_sub,
  nd_mul,
  nd_div,
  nd_eq,
  nd_ne,
  nd_lt,
  nd_le,
  nd_assign,
  nd_addr,
  nd_deref,
  nd_ret,
  nd_if,
  nd_while,
  nd_for,
  nd_sizeof,
  nd_block,
  nd_func_call,
  nd_expr_stmt,
  nd_var,
  nd_num,
  nd_null
};

enum type_kind { ty_char, ty_int, ty_ptr, ty_array };

struct typ {
  enum type_kind kind;
  struct typ *base;
  int array_size;
};

struct va {
  char *name;
  struct typ *ty;
  bool is_local;
  char *contents;
  int content_length;
  int offset;
};

struct var_list {
  struct var_list *next;
  struct va *v;
};

struct node {
  enum node_kind kind;
  struct node *next;
  struct typ *ty;
  struct token *tok;
  struct node *lhs;
  struct node *rhs;
  struct node *cond;
  struct node *then;
  struct node *els;
  struct node *init;
  struct node *inc;
  struct node *body;
  char *func_name;
  struct node *args;
  struct va *v;
  int val;
};

struct fun {
  struct fun *next;
  char *name;
  struct var_list *params;
  struct node *node;
  struct var_list *locals;
  int stack_size;
};

struct program {
  struct var_list *globals;
  struct fun *fns;
};

struct var_list *locals;
struct var_list *globals;

int label_count = 0;

struct va *find_var(struct token *tok);
struct node *new_unary(enum node_kind k, struct node *n, struct token *tok);
struct node *new_binary(enum node_kind k, struct node *lhs, struct node *rhs,
                        struct token *tok);
struct node *new_number(int v, struct token *tok);
struct node *new_var(struct va *v, struct token *tok);
char *new_label();
struct va *push_var(char *name, struct typ *ty, bool is_local);
struct node *primary();
struct node *func_args();
struct node* post_fix();

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

  tok = consume_indent();
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
  if (consume(")") != NULL) return NULL;
  struct node *h = assign();
  cur = h;
  for (consume(",") != NULL ) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return h;
}

struct node* post_fix() {
  struct node *n = primary();
  for (struct token *tok = consume("["); tok != NULL; tok = consume("[")) {
    struct node *exp = new_binary(nd_add, n, expr(), tok());
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

