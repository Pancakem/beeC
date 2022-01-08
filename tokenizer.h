#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>

typedef enum { tk_reserved = 0, tk_ident, tk_str, tk_num, tk_eof } token_kind_t;

typedef struct {
  token_kind_t kind;
  struct token *next;
  int val;
  char *str;
  unsigned long len;
  char *contents;
  unsigned long content_length;
} token_t;

extern char *filename;
extern char *inpt;
extern int pos;
extern token_t *t;

void error_at(char *loc, char *f);
void error_tok(token_t *tok, char *f);
bool peek(char *s);
token_t *consume(char *op);
token_t *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
token_t *new_token(token_kind_t k, token_t *cur, char *str, unsigned long len);
bool starts_with(char *str, char *op);
char *starts_with_reserved(char *str);
bool is_reserved(char c);
char get_escape_char(char c);
token_t *read_str_literal(token_t *cur, char *p);
token_t *tokenize(char *p);

void free_token(token_t *value);

#endif /* TOKENIZER_H */
