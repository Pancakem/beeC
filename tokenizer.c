#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

char *filename = "";
char *inpt = "";

enum token_kind { tk_reserved, tk_ident, tk_str, tk_num, tk_eof };

struct token {
  enum token_kind kind;
  struct token *next;
  int val;
  char *str;
  int len;
  char *contents;
  int content_length;
};

struct token *t;


void error_at(char *loc, char *f);
void error_tok(struct token *tok, char *f);
bool peek(char *s);
struct token *consume(char *op);
struct token *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
struct token *new_token(enum token_kind k, struct token *cur, char *str,
                        int len);
bool starts_with(char *str, char *op);
char *starts_with_reserved(char *str);
bool is_reserved(char c);
char get_escape_char(char c);
struct token *read_str_literal(struct token *cur, char *p);
struct token *tokenize(char *p);


void error_at(char *loc, char *f) {
  char *k = inpt;
  int line = 1;
  char *buf = (char *)malloc(sizeof(char) * 50);
  while (strlen(k) > 0) {
    if (k[0] == '\n') {
      if (strlen(k) <= strlen(loc) + 2)
        break;

      line++;
      buf = "";
    } else {
      char ch = k[0];
      strncat(buf, &ch, 1);
    }
    k++;
  }

  printf("%s:%d", filename, line);

  int p = strlen(buf);
  printf("%s", buf);
  printf("%*s", p, "");
  printf("^ ");
}

void error_tok(struct token *tok, char *f) {
  if (tok != NULL)
    error_at(tok->str, f);

  puts(f);
  exit(1);
}

bool peek(char *s) {
  if (t->kind != tk_reserved)
    return false;

  if (strlen(s) != t->len)
    return false;

  char test_str[t->len];
  memcpy(test_str, t->str, t->len);
  if (memcmp(test_str, s, t->len) != 0)
    return false;

  return true;
}

struct token *consume(char *op) {
  if (!peek(op))
    return NULL;

  struct token *tt = (struct token*)malloc(sizeof(struct token));
  tt= t;
  t = t->next;
  return tt;
}

struct token* consume_ident() {
  if (t->kind != tk_ident) return NULL;
  struct token *tt = (struct token*)malloc(sizeof(struct token));
  tt = t;
  t = t->next;
  return tt;  
}

void expect(char *op) {
  if (!peek(op)) {
    char s[11 + strlen(op)];
    sprintf(s, "unexpected '%s'", op);
    error_tok(t, s);
  }
  t = t->next;
}

int expect_number() {
  if (t->kind != tk_num)
    error_tok(t, "expected a number");

  int v = t->val;
  t = t->next;
  return v;
}

char *expect_ident() {
  if (t->kind != tk_ident)
    error_tok(t, "expected an identifier");
  char *s = (char *)malloc(sizeof(char) * t->len);
  memcpy(s, t->str, t->len);
  t = t->next;
  return s;
}

bool at_eof() { return t->kind == tk_eof; }

struct token *new_token(enum token_kind k, struct token *cur, char *str,
                        int len) {
  struct token *p = (struct token*) malloc(sizeof(struct token));
  p->kind = k;
  p->str = str;
  p->len = len;
  cur->next = p;
  return p;
}

bool starts_with(char *str, char *op) {
  if (strlen(str) < strlen(op))
    return false;
  return memcmp(str, op, strlen(op));
}

char *starts_with_reserved(char *str) {
  char *kws[] = {"if", "else", "while", "for", "int", "char", "sizeof"};

  for (int i = 0; i < 8; ++i) {
    int len = strlen(kws[i]);
    if ((starts_with(str, kws[i])) && !isalnum(str[len]))
      return kws[i];
  }

  char *ops[] = {"==", "!=", "<=", ">="};

  for (int i = 0; i < 4; ++i) {
    if (starts_with(str, ops[i]))
      return ops[i];
  }
  return NULL;
}

bool is_reserved(char c) {
  switch (c) {
  case '+':
  case '-':
  case '*':
  case '/':
  case '(':
  case ')':
  case '<':
  case '>':
  case ';':
  case '=':
  case '{':
  case '}':
  case ',':
  case '&':
  case '[':
  case ']':
    return true;
  default:
    return false;
  }
}

char get_escape_char(char c) {
  switch (c) {
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 't':
    return '\t';
  case 'n':
    return '\n';
  case 'v':
    return '\v';
  case 'f':
    return '\f';
  case 'r':
    return '\r';
  case 'e':
    return 27;
  case '0':
    return 0;
  }
  return c;
}

struct token *read_str_literal(struct token *cur, char *p) {
  int len = strlen(p);
  char *s = (char *)malloc(sizeof(char) * len);
  memcpy(s, p, len);

  int l = 0;
  char *r = (char *)malloc(sizeof(char) * strlen(p));
  while (1) {
    char c = p[0];

    if (l == 1024)
      error_at(p, "string literal too large");
    if (c == 0)
      error_at(p, "unclosed string literal");
    if (c == '"')
      break;
    if (c == '\\') {
      memmove(p, p + 1, strlen(p));
      char t = get_escape_char(p[0]);
      strncat(r, &t, 1);
      memmove(p, p + 1, strlen(p));
    } else {
      strncat(r, &c, 1);
      memmove(p, p + 1, strlen(p));
    }
  }

  struct token *tok = new_token(tk_str, cur, s, strlen(s) - strlen(p) + 1);
  char c = 0;
  strncat(r, &c, 1);
  tok->contents = r;
  tok->content_length = strlen(tok->contents);
  return tok;
}

struct token *tokenize(char *p) {
  struct token h;
  h.next = NULL;
  struct token *cur = &h;

  while (strlen(p) > 0) {
    char c = p[0];
    if (isspace(c)) {
      memmove(p, p + 1, strlen(p));
    }
    if (starts_with(p, "//")) {
      memmove(p, p + 2, strlen(p));

      while (p[0] != '\n') {
        memmove(p, p + 1, strlen(p));
      }
      continue;
    }

    if (starts_with(p, "/*")) {
      while ((strlen(p) > 1) && (!starts_with(p, "*/")))
        memmove(p, p + 1, strlen(p));

      if (strlen(p) < 2)
        error_at(p, "unclosed block comment");

      memmove(p, p + 2, strlen(p));
      continue;
    }

    char *keyword = starts_with_reserved(p);
    if (keyword != NULL) {
      int len = strlen(keyword);
      cur = new_token(tk_reserved, cur, p, len);
      memmove(p, p + len, strlen(p));
      continue;
    }

    if (is_reserved(c)) {
      cur = new_token(tk_reserved, cur, p, 1);
      memmove(p, p + 1, strlen(p));
      continue;
    }

    if (isalpha(c)) {
      int len = strlen(p);
      char *q = (char *)malloc(len * sizeof(char));
      memcpy(q, p, len);
      memmove(p, p + 1, len);

      while ((strlen(p) > 0) && isalnum(p[0]))
        memmove(p, p + 1, strlen(p));

      cur = new_token(tk_ident, cur, q, len - strlen(p));
      continue;
    }

    if (c == '"') {
      cur = read_str_literal(cur, p);
      memmove(p, p + cur->len, strlen(p));
      continue;
    }

    if (isdigit(c)) {
      cur = new_token(tk_num, cur, p, 0);
      int len = strlen(p);
      int v = atoi(p);

      cur->val = v;
      cur->len = len - strlen(p);
      continue;
    }
    char error_string[20];
    sprintf(error_string, "cannot tokenize %c", c);
    error_at(p, error_string);
  }

  new_token(tk_eof, cur, p, 0);
  return h.next;
}
