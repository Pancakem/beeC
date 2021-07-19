#ifndef TYPE_H
#define TYPE_H

#include "parser.h"
#include <stddef.h>
#include <stdlib.h>


struct typ *char_type();
struct typ *int_type();
struct typ *pointer_to(struct typ *b);
struct typ *array_of(struct typ *b, int s);
int size_of(struct typ *ty);




#endif /* TYPE_H */
