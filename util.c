#include "util.h"

#include <stdlib.h>
#include <string.h>

char* slice_str(const char *str, size_t start, size_t end){
  char *result = (char *)malloc(sizeof(char) * (end - start));
  strncpy(result, str + start, end - start);
  return result;
}
