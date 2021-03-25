#include "parser.h"

struct typ* char_type();
struct typ* int_type();
struct typ* pointer_to(struct typ* b);
struct typ* array_of(struct typ* b, int s);
int size_of(struct typ* ty);
void visit(struct node *n);
void add_type(struct program* p);
