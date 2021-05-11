#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *file_path) {
  FILE *fle = fopen(file_path, "r");
  if (!fle) {
    printf("failed to open the file");
    exit(1);
  }
  fseek(fle, 0L, SEEK_END);
  long size = ftell(fle);
  fseek(fle, 0L, SEEK_SET);
  char *buffer = (char *)malloc(sizeof(char) * size);
  fread(buffer, size, 0, fle);
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
}
