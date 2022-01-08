#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include <stdbool.h>

typedef enum {
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
} node_kind_t;

typedef enum { ty_char, ty_int, ty_ptr, ty_array } type_kind_t;

typedef struct typ {
  type_kind_t kind;
  struct typ *base;
  int array_size;
} typ_t;

typedef struct {
  char *name;
  typ_t *ty;
  bool is_local;
  char *contents;
  int content_length;
  int offset;
} va_t;

typedef struct var_list {
  struct var_list *next;
  va_t *v;
} var_list_t;

typedef struct node {
  node_kind_t kind;
  struct node *next;
  struct typ *ty;
  token_t *tok;
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
  va_t *v;
  int val;
} node_t;

typedef struct fun {
  struct fun *next;
  char *name;
  struct var_list *params;
  struct node *node;
  struct var_list *locals;
  int stack_size;
} fun_t;

typedef struct {
  struct var_list *globals;
  fun_t *fns;
} program_t;

// struct var_list *locals;
// struct var_list *globals;

extern int label_count;

void init_node(node_t *init_node, node_kind_t kind, node_t *next, typ_t *ty,
               token_t *tok, node_t *lhs, node_t *rhs, node_t *cond,
               node_t *then, node_t *els, node_t *init, node_t *inc,
               node_t *body, char *func_name, node_t *args, va_t *v, int val);

void init_va(va_t *v, char *name, typ_t *ty, bool is_local, char *contents,
             int content_len, int offset);

void init_type(typ_t *ty, type_kind_t kind, typ_t *base, int array_size);

void init_var_list(var_list_t *vl, var_list_t *next, va_t *v);

void init_function(fun_t *fn, fun_t *next, char *name, var_list_t *params,
                   node_t *node, var_list_t *locals, int stack_size);

va_t *find_var(token_t *tok);
node_t *new_unary(node_kind_t k, node_t *n, token_t *tok);
node_t *new_binary(node_kind_t k, node_t *lhs, node_t *rhs, token_t *tok);
node_t *new_number(int v, token_t *tok);
node_t *new_var(va_t *v, token_t *tok);
char *new_label();
va_t *push_var(char *name, typ_t *ty, bool is_local);
node_t *primary();
node_t *func_args();
node_t *post_fix();
node_t *unary();
node_t *mul();
node_t *add();
node_t *relational();
node_t *equality();
node_t *assign();
node_t *expr();
node_t *stmt();
bool is_type_name();
node_t *read_expr_stmt();
node_t *declaration();
void global_var();
fun_t *function();
typ_t *base_type();
typ_t *read_type_suffix(typ_t *b);
var_list_t *read_func_params();
bool is_function();
program_t *prog();

#endif /* PARSER_H */
