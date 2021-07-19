#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

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

extern int label_count;

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
struct node *unary();
struct node *mul();
struct node *add();
struct node *relational();
struct node *equality();
struct node *assign();
struct node *expr();
struct node *stmt();
bool is_type_name();
struct node *read_expr_stmt();
struct node *declaration();
void global_var();
struct fun *function();
struct typ *base_type();
struct typ *read_type_suffix(struct typ* b);
struct var_list* read_func_params();
bool is_function();
struct program *prog();


#endif /* PARSER_H */
