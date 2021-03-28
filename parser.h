#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "tokenizer.h"

enum node_kind;
enum type_kind;
struct typ;
struct va;
struct var_list;
struct node;
struct fun;
struct program;

struct var_list *locals;
struct var_list *globals;

struct va* find_var(struct token *tok);
struct node *new_unary(enum node_kind k, struct node *n, struct token *tok);
struct node *new_binary(enum node_kind k, struct node *lhs, struct node *rhs, struct token *tok);
struct node *new_number(int v, struct token *tok);
struct node* new_var(struct va *v, struct token *tok);
char *new_label();
struct va* push_var(char *name, struct typ *ty, bool is_local);
struct node* primary();
