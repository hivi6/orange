#ifndef AST_H
#define AST_H

#include "pos.h"
#include "token.h"

#define AST_PRINT_DEPTH 1024

enum {
	AST_LITERAL_EXPR,
	AST_VAR_EXPR,
	AST_GROUP_EXPR,
	AST_BINARY_EXPR,
};

struct ast_t {
	int kind;
	const char *filepath;
	const char *source;
	pos_t start;
	pos_t end;

	union {
		// AST_LITERAL_EXPR
		struct {
			token_t *token;
		} literal_expr;

		// AST_VAR_EXPR
		struct {
			token_t *token;
		} var_expr;

		// AST_GROUP_EXPR
		struct {
			struct ast_t *expr;
		} group_expr;

		// AST_BINARY_EXPR
		struct {
			struct ast_t *left;
			token_t *op;
			struct ast_t *right;
		} binary_expr;
	} ast;
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

