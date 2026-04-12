#ifndef AST_H
#define AST_H

#include "token.h"

#define AST_PRINT_DEPTH 1024

enum {
	AST_LITERAL = 0,
};

struct ast_t {
	int kind;
	const char *filepath;
	const char *source;
	pos_t start;
	pos_t end;

	// literal
	token_t *literal;
};

typedef struct ast_t ast_t;

/**
 * Parse the whole token list and generate the ast
 *
 * params:
 *     tokens  list token tokens
 *
 * returns:
 *     generated ast
 */
ast_t *parse(token_t *tokens);

/**
 * Print the full ast
 *
 * params:
 *     ast  ast that needs printing
 */
void print_ast(ast_t *ast);

#endif // AST_H

