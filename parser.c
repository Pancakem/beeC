#include "parser.h"

var_list *locals;
var_list *globals;
int label_count = 0;

struct va* find_var(token *tok) {
  auto vl = locals;
  while(vl != NULL) {
    auto v = vl.v;
    // if ()
      
  }
  return NULL;
}

struct node *new_unary(node_kind k, node *n, token *tok) {
  return &node{ .kind = k, .lhs = n, .tok = tok};
}

struct node *new_binary(node_kind k, node *lhs, node *rhs, token *tok) {
  return &node{ .kind = k, .lhs = lhs, .rhs = rhs, .tok = tok };
}

struct node *new_number(int v, token *tok) {
  return &node{ .kind = nd_num, .val = v, .tok = tok };
}

struct node* new_var(va *v, token *tok) {
  return &node{ .kind = nd_var, .v = v, .tok = tok };
}

char *new_label() {
  char *name = (char*) malloc(sizeof(char) * 20) ;
  sprintf(name, ".L.data.%d", label_count);
  return name;
}

struct va* push_var(char *name, typ *ty, bool is_local) {
  struct va *v = &va{ .name = name, .ty = ty, .is_local = is_local };
  struct var_list *vl = &var_list{ .v = v};

  if (is_local) {
    vl.next = locals;
    locals = vl;
  }else {
    vl.next = globals;
    globals = vl;
  }
  return v;
}

struct node* primary() {
  if (consume("(") == NULL) {
    auto n = expr();
    expect(")");
    return n;
  }

  auto tok = consume("sizeof");
  if (tok != NULL) {
    return new_unary(nd_size_of, unary(), tok);
  }

  tok = consume_indent();
  if(tok != NULL) {
    if(consume("(") == NULL) {
      return &node{ .kind = nd_func_call, .funcname = tok.str, .args:func_args(), .tok = tok };
    }

    auto v = find_var(tok);
    if (v == NULL) {
      error_tok(tok, "undefined variable");
    }
    return new_var(v, tok);
  }

  tok = t;

  if (tok.kind == tk_str){
    t = t.next;
    auto ty = array_of(char_type(), tok->content_length);
    auto v = push_var(new_label(), ty, false);
    v.contents = tok->contents;
    v.content_length = tok->content_length;
    return new_var(v, tok);
  }

  if (tok.kind != tk_num) {
    error_tok(tok, "expected expression");
  }

  return new_number(expected_number(), tok)   ;
}
