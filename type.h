#ifndef TYPE_H
#define TYPE_H

#include "parser.h"
#include <stddef.h>
#include <stdlib.h>

typ_t *char_type();
typ_t *int_type();
typ_t *pointer_to(typ_t *b);
typ_t *array_of(typ_t *b, int s);
int size_of(typ_t *ty);
void add_type(program_t *p);

#endif /* TYPE_H */
