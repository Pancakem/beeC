#include <string.h>
#include <ctype.h>

#include "tokenizer.h"

token *t;
char *filename = "";
char *inpt = "";

void error_at(char *loc, char *f){
  auto k = inpt;
  int line = 1;
  char *buf = (char *)malloc(sizeof(char) * 50);
  for((sizeof(k) / sizeof(char)) > 0) {
    if(k[0] == '\n') {
      if ((sizeof(k) / sizeof(char)) <= ((sizeof(loc) / sizeof(char)) + 2))
	break;
      
      line++;
      buf = "";
    }else {
      char ch = k[0];
      strncat(buf, &ch, 1);
    }
    k++;
  }

  printf("%s:%d", filename, line);

  auto p = sizeof(buf) / sizeof(char);
  printf("%s", buf);
  printf("%*s", p, "");
  printf("^ ");  
}

void error_tok(struct token *tok, char *f){
  if (tok != NULL)
    error_at(tok->str, f);
  
  puts(f);
  exit(1);
}

bool peek(char *s) {
  if(t.kind != tk_reserved)
    return false;
  
  if ((sizeof(s) / sizeof(char)) != t.len)
    return false;

  char test_str[t->len];
  memcpy(test_str, t->str, t->len);
  if (memcmp(teststr, s, t->len) != 0)
    return false;

  return true;  
}

struct token *consume(char *op) {
  if (!peek(op))
    return NULL;

  token* tt = t;
  t = t.next;
  return tt;
}

void expect(char *op) {
  if(!peek(op)) {
    char s[30];
    sprintf(s, "unexpected '%s'", op);
    error_tok(t, s);
  }
  t = t->next;
}

int expect_number() {
  if (t->kind != tk_num) error_tok(t, "expected a number");

  auto v = t->val;
  t = t->next;
  return v;
}

char *expect_ident() {
  if(t->kind != tk_ident) error_tok(t, "expected an identifier");
  char s[t->len];
  memcpy(s, t->str, t->len);
  t = t->next;
  return s; 
}

bool at_eof() {
  return t->kind == tk_eof;
}

struct token* new_token(enum token_kind k, struct token *cur, char *str, int len ){
  struct token *p = (struct token *)malloc(sizeof(struct token));
  p = (struct token*) {.kind = k, .str = str, .len = len };
  cur->next = p;
  return p;
}

bool starts_with(char *str, char *op) {
  if (strlen(str) < strlen(op)) return false;
  return memcmp(str, op, strlen(op));
}

bool is_digit(char c) {
  return c >= '0' && c <= '9';  
}

char *starts_with_reserved(char *str) {
  char kws[][] = { "if",
		  "else",
		  "while",
		  "for",
		  "int",
		  "char",
		  "sizeof" 
  };

  for(int i = 0; i < 8; ++i) {
    int len = strlen(kws[i]);
    if ((starts_with(str, kws[i])) && !is_al_num(str[len])) return kws[i];
  }

  char ops[][] = { "==",
		  "!=",
		  "<=",
		  ">="		 
  };

  for (int i=0; i<4; ++i) {
    if(starts_with(str, ops[i])) return ops[i];  
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

struct token *read_str_literal(struct token* cur, char *p) {
  int len =  strlen(p);
  char *s = (char *)malloc(sizeof(char) * len);
  memcpy(q, p, len);

  int l = 0;
  // char *
}

struct token* tokenize(char *p) {
  struct token h;
  h.next = NULL;
  struct token *cur = &h;

  for(strlen(p) > 0) {
    char c = p[0];
    if (isspace(c)) {
      memmove(p, p+1, strlen(p));
    }
    if (starts_with(p, "//")) {
      memmove(p, p+2, strlen(p));

      while(p[0] != '\n'){
	memmove(p, p+1, strlen(p));
      }
      continue;
    }

    if (starts_with(p, "/*")) {
      while((strlen(p) > 1) && (!starts_with(p, "*/"))) memmove(p, p+1, strlen(p));

      if (strlen(p) < 2) error_at(p, "unclosed block comment");

      memmove(p, p+2, strlen(p));
      continue;      
  }

    char *keyword =  starts_with_reserved(p);
    if (keyword != NULL){
      int len = strlen(keyword);
      cur = new_token(tk_reserved, cur, p, len);
      memmove(p, p+len, strlen(p));
      continue
    }

    if (is_reserved(c)) {
      cur = new_token(tk_reserved, cur, p, 1);
      memmove(p, p+1, strlen(p));
      continue;
    }

    if(isalpha(c)) {
      int len = strlen(p);
      char *q = (char *)malloc(len * sizeof(char));
      memcpy(q, p, len);
      memmove(p, p+1, len);

      while((strlen(p) > 0) && isalnum(p[0])) memmove(p, p+1, strlen(p));

      cur = new_token(tk_indent, cur, q, len-strlen(p));
      continue;
    }

    if (c == '"') {
      cur = read_str_literal(cur, p);
      memmove(p, p+cur->len, strlen(p));
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
    char error_string[10];
    sprintf(error_string, "cannot tokenize %c", c);
    error_at(p, error_string);
  }

  new_token(tk_eof, cur, p, 0);
}

