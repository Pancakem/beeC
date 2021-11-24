#include "tokenizer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *inpt = 0;
char *filename = 0;
int pos = 0;
struct token *t = NULL;

void error_at(char *loc, char *err_str) {
  char *k = inpt;
  int line = 1;
  while (strlen(k) > 0) {
    if (k[0] == '\n') {
      line++;
    }
    k++;
  }

  printf("%s:%d: error: %s\n", filename, line, err_str);
  printf("%s\n", loc);
  printf("^ \n");
  exit(1);
}

void error_tok(struct token *tok, char *f) {
  if (tok != NULL)
    error_at(tok->str, f);

  puts(f);
  exit(1);
}

bool peek(char *s) {
  return ((strlen(s) == t->len) && strncmp(t->str, s, t->len) == 0);
}

struct token *consume(char *op) {
  if (!peek(op))
    return NULL;

  struct token *tok = t;
  t = tok->next;
  return tok;
}

struct token *consume_ident() {
  if (t->kind != tk_ident)
    return NULL;

  struct token *tt = t;
  t = tt->next;
  return tt;
}

void expect(char *op) {
  if (!peek(op)) {
    char s[20 + strlen(op)];
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
  strncpy(s, t->str, t->len);
  t = t->next;
  return s;
}

bool at_eof() { return t->kind == tk_eof; }

struct token *new_token(enum token_kind k, struct token *cur, char *str,
                        unsigned long len) {
  struct token *p = (struct token *)malloc(sizeof(struct token));
  p->kind = k;
  p->str = str;
  p->len = len;
  p->val = 0;
  p->contents = NULL;
  p->content_length = 0;

  cur->next = p;
  return p;
}

bool starts_with(char *str, char *op) {
  return strncmp(str, op, strlen(op)) == 0;
}

bool starts_with_char(char c, char op) { return c == op; }

char *starts_with_reserved(char *str) {
  char *kws[] = {"return", "if",  "else", "while",
                 "for",    "int", "char", "sizeof"};

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
  char s[len + 1];
  strncpy(s, p, len);

  int l = 0;
  char r[strlen(p)];
  int curr_pos = 0;
  while (1) {
    char c = p[curr_pos];

    if (l == 1024)
      error_at(p + curr_pos, "string literal too large");
    if (c == 0)
      error_at(p + curr_pos, "unclosed string literal");
    if (c == '"')
      break;
    if (c == '\\') {
      char t = get_escape_char((p + curr_pos)[0]);
      strncat(r, &t, 1);
      curr_pos++;
    } else {
      strncat(r, &c, 1);
      curr_pos++;
    }
    l++;
  }

  struct token *tok =
      new_token(tk_str, cur, s, strlen(s) - strlen(p + curr_pos) + 1);
  char c = 0;
  strncat(r, &c, 1);
  tok->contents = r;
  tok->content_length = strlen(tok->contents);

  // test :
  pos += curr_pos;

  return tok;
}

struct token *tokenize(char *p) {
  struct token h;
  h.next = NULL;
  struct token *cur = &h;

  while (strlen(p + pos) > 0) {
    char c = p[pos];
    if (isspace(c)) {
      pos++;
      continue;
    }
    if (starts_with(p, "//")) {
      pos += 2;
      while (p[pos] != '\n') {
        pos++;
      }
      continue;
    }

    if (starts_with(p + pos, "/*")) {
      while ((strlen(p + pos) > 1) && (!starts_with(p + pos, "*/")))
        pos++;

      if (strlen(p + pos) < 2)
        error_at(p + pos, "unclosed block comment");

      pos += 2;
      continue;
    }

    char *keyword = starts_with_reserved(p + pos);
    if (keyword != NULL) {
      int len = strlen(keyword);
      cur = new_token(tk_reserved, cur, keyword, len);
      pos += len;
      continue;
    }

    if (is_reserved(c)) {
      char *q = (char *)malloc(sizeof(char) + 1);
      strncpy(q, p + pos, 1 + 1);
      q[1] = '\0';
      cur = new_token(tk_reserved, cur, q, 1);
      pos++;
      continue;
    }

    if (isalpha(c)) {
      int len = strlen(p + pos);
      int start_pos = pos;
      pos++;

      while ((strlen(p + pos) > 0) && isalnum((p + pos)[0]))
        pos++;

      int new_len = strlen(p + pos);
      char *q = (char *)malloc(sizeof(char) * (len - new_len + 1));
      strncpy(q, p + start_pos, len - new_len);
      q[len - new_len] = '\0';

      cur = new_token(tk_ident, cur, q, len - new_len);
      continue;
    }

    if (c == '"') {
      cur = read_str_literal(cur, p + pos);
      pos++;
      continue;
    }

    if (isdigit(c)) {
      int len = strlen(p + pos);
      int start_pos = pos;
      pos++;
      while ((strlen(p + pos) > 0) && isdigit(p[pos]))
        pos++;

      int new_len = strlen(p + pos);
      char *q = (char *)malloc(sizeof(char) * (len - new_len));
      strncpy(q, p + start_pos, len - new_len);
      q[len - new_len] = '\0';
      cur = new_token(tk_num, cur, q, 0);

      int v = atoi(q);

      cur->val = v;
      cur->len = len - new_len;
      continue;
    }
    char error_string[50];
    sprintf(error_string, "cannot tokenize %c", c);
    error_at(p + pos, error_string);
  }

  new_token(tk_eof, cur, p + pos, 0);
  return h.next;
}

void free_token(struct token *value) {
  void *anode;

  while (value != NULL) {
    anode = value;
    value = value->next;
    free(anode);
  }
}
