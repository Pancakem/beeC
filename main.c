#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"
#include "type.h"
#include "tokenizer.h"
#include "parser.h"

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
  char* buffer = read_file(filename);
  inpt = (char *)malloc(sizeof(char) * strlen(buffer));
  strcpy(inpt, buffer);
  t = tokenize(buffer);
  struct program *p = prog();
  free_all(t);
  add_type(p);
  for (struct fun *fn = p->fns; fn != NULL; fn = fn->next) {
    int ot = 0;
    for (struct var_list *vl = fn->locals; vl != NULL; vl = vl->next) {
      struct va *v = vl->v;
      ot += size_of(v->ty);
      vl->v->offset = ot;      
    }
    fn->stack_size = align_to(ot, 8);    
  }
  
  codegen(p);
  free(p);
  return 0;  
}
