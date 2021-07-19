#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

extern int label_seq;
char *func_name;
extern char *arg_reg1[];
extern char *arg_reg8[];

void gen_addr(struct node *nd);
void gen_lval(struct node *nd);
void load(struct typ *ty);
void store(struct typ *ty);
void gen(struct node *n);
void emit_data(struct program *prog);
void load_arg(struct va *v, int idx);
void emit_text(struct program *prog);
void codegen(struct program *prog);


#endif /* CODEGEN_H */
