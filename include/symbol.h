#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"

struct symbol_t {
	const char *name;
	type_t *type;
	struct symbol_t *next;
};

typedef struct symbol_t symbol_t;

#endif // SYMBOL_H

