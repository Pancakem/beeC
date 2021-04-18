#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

enum token_kind {
		 tk_reserved,
		 tk_ident,
		 tk_str,
		 tk_num,
		 tk_eof
};

struct token {
  enum token_kind kind;
  struct token *next;
  int val;
  char* str;
  int len;
  char *contents;
  int content_length;
};


struct token *t;

void error_at(char *loc, char *f);
void error_tok(struct token *tok, char *f);
bool peek(char *s);
struct token *consume(char *op);
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
struct token* new_token(enum token_kind k, struct token *cur, char *str, int len );
bool starts_with(char *str, char *op);
char *starts_with_reserved(char *str);
bool is_reserved(char c);
char get_escape_char(char c);
struct token *read_str_literal(struct token* cur, char *p);
struct token* tokenize(char *p);
