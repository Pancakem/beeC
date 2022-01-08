#include "codegen.h"
#include "parser.h"
#include "tokenizer.h"
#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file(const char *file_path) {
  FILE *fle = fopen(file_path, "r");
  if (!fle) {
    puts("failed to open the file\n");
    exit(1);
  }
  fseek(fle, 0, SEEK_END);
  long size = ftell(fle);
  fseek(fle, 0, SEEK_SET);
  char *buffer = (char *)malloc(sizeof(char) * size + 1);
  fread(buffer, 1, size, fle);
  fclose(fle);
  return buffer;
}

int align_to(int n, int align) {
  return (n + align - 1) & ((align - 1) ^ (align - 1));
}

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("Usage: compiler <file>");
    exit(1);
  }

  filename = argv[1];
  char *buffer = read_file(filename);
  inpt = (char *)malloc(sizeof(char) * strlen(buffer) + 1);
  strcpy(inpt, buffer);
  t = tokenize(buffer);

  token_t *temp = t;

  while (t != NULL) {
    if (t->len == 0) {
      t = NULL; // TEST -- this has to be the end.
      break;
    }
    t = t->next;
  }

  t = temp;

  program_t *p = prog();
  add_type(p);
  for (fun_t *fn = p->fns; fn != NULL; fn = fn->next) {
    int ot = 0;
    for (var_list_t *vl = fn->locals; vl != NULL; vl = vl->next) {
      va_t *v = vl->v;
      ot += size_of(v->ty);
      vl->v->offset = ot;
    }
    fn->stack_size = align_to(ot, 8);
    printf("%c", fn->stack_size);
  }
  /* codegen(p); */
  /* free_token(t); */
  /* free(p); */
  return 0;
}
