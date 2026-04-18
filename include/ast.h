#ifndef AST_H
#define AST_H

#include "pos.h"
#include "token.h"

struct ast_t {
	int kind;
	const char *filepath;
	const char *source;
	pos_t start;
	pos_t end;
};

typedef struct ast_t ast_t;

/**
 * Parse the list of tokens into the required ast
 *
 * params:
 *     tokens  head of the token list
 *
 * returns:
 *     returns the pointer to the ast
 */
ast_t *parse(token_t *tokens);

/**
 * Print the ast
 *
 * params:
 *     ast  ast that needs to be printed
 */
void print_ast(ast_t *ast);

#endif // AST_H

