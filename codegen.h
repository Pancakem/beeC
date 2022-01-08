#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

extern int label_seq;
extern char *func_name;
extern char *arg_reg1[];
extern char *arg_reg8[];

void gen_addr(node_t *nd);
void gen_lval(node_t *nd);
void load(typ_t *ty);
void store(typ_t *ty);
void gen(node_t *n);
void emit_data(program_t *prog);
void load_arg(va_t *v, int idx);
void emit_text(program_t *prog);
void codegen(program_t *prog);

#endif /* CODEGEN_H */
