#ifndef SCOPE_H
#define SCOPE_H

#include "symbol.h"

struct scope_t {
	struct scope_t *parent;
	symbol_t *symbols;
};

typedef struct scope_t scope_t;

/**
 * Create a new scope and append it to the global scope
 *
 * params:
 *     parent  parent scope
 *
 * returns:
 *     pointer to the scope
 */
scope_t *create_scope(scope_t *parent);

/**
 * Get the symbol pointer from the given scope or the parent scope
 *
 * params:
 *     name  symbol name
 *
 * returns:
 *     symbol pointer
 */
symbol_t *get_symbol(scope_t *scope, const char *name);

/**
 * Create a new symbol of a given type in the given scope
 *
 * params:
 *     scope  scope where the symbol will be created
 *     name   name of the symbol
 *     type   type of the symbol
 *
 * returns:
 *     symbol pointer
 */
symbol_t *create_symbol(scope_t *scope, const char *name, type_t *type);

#endif // SCOPE_H

