#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "tokenizer.h"

enum node_kind {
      nd_add,
      nd_sub,
      nd_mul
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
      nd_fun_call,
      nd_expr_stmt,
      nd_var,
      nd_num,
      nd_null
};

enum type_kind {
	ty_char,
	ty_int,
	ty_ptr,
	ty_array
};

struct typ {
  type_kind kind;
  typ *base;
  int array_size;
};

struct va {
  char *name;
  typ *ty;
  bool is_local;
  char *contents;
  int content_length;
  int offset;
};

struct var_list {
  var_list *next;
  va *v;
};

struct node {
  node_kind kind;
  node *next;
  typ *ty;
  token *tok;
  node *lhs;
  node *rhs;
  node *cond;
  node *then;
  node *els;
  node *init;
  node *inc;
  node *body;
  char *funcname;
  node *args;
  va *v;
  int val;
};

struct fun {
  fun *next;
  char *name;
  var_list *params;
  node *node;
  var_list *locals;
  int stack_size;
};

struct program {
  var_list *globals;
  fun *fns;
};
