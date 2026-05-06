#include "scope.h"
#include "common.h"

// ========================================
// scope.h - definition
// ========================================

scope_t *create_scope(scope_t *parent) {
	scope_t *scope = malloc(sizeof(scope_t));
	scope->parent = parent;
	scope->symbols = NULL;
	return scope;
}

symbol_t *get_symbol(scope_t *scope, const char *name) {
	for (scope_t *temp_scope = scope; temp_scope; temp_scope = temp_scope->parent) {
		for (symbol_t *temp_symbol = temp_scope->symbols; temp_symbol; temp_symbol = temp_symbol->next) {
			if (strcmp(temp_symbol->name, name) == 0) {
				return temp_symbol;
			}
		}
	}
	return NULL;
}

symbol_t *create_symbol(scope_t *scope, const char *name, type_t *type) {
	for (symbol_t *temp_symbol = scope->symbols; temp_symbol; temp_symbol = temp_symbol->next) {
		if (strcmp(temp_symbol->name, name) == 0) {
			return NULL;
		}
	}

	symbol_t *res = malloc(sizeof(symbol_t));
	res->name = name;
	res->type = type;
	res->next = NULL;

	if (scope->symbols == NULL) scope->symbols = res;
	else {
		symbol_t *head = scope->symbols;
		while (head->next != NULL) head = head->next;
		head->next = res;
	}

	return res;
}

