#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stddef.h>

inline bool is_null(void *value) {
  return value == NULL;
}

#endif /* UTIL_H */
