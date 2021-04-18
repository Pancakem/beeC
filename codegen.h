#include "parser.h"
#include "tokenizer.h"

void gen_addr(struct node *nd);
void gen_lval(struct node *nd);
void load(struct typ* ty);
void store(struct typ* ty);
void gen(struct node *n);
void emit_data(struct program *prog);
void load_arg(struct va* v, int idx);
void emit_text(struct program* prog);
void codegen(struct program* prog);

