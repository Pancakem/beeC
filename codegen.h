#include "tokenizer.h"

int label_seq = 0;
char *func_name;
char *arg_reg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *arg_reg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_addr(struct node *nd);
void gen_lval(struct node *n);
void load(struct typ* ty);
void store(struct typ* ty);
void gen(struct node *n);
void emit_data(struct program *prog);
void load_arg(struct va* v, int idx);
void emit_text(struct program* prog);
void codegen(struct program* prog);

