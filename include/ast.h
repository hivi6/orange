#ifndef AST_H
#define AST_H

#include "pos.h"
#include "token.h"
#include "scope.h"

#define AST_PRINT_DEPTH 1024

enum {
	AST_LITERAL_EXPR,
	AST_VAR_EXPR,
	AST_GROUP_EXPR,
	AST_BINARY_EXPR,
	AST_TERNARY_EXPR,
	AST_PREFIX_EXPR,
	AST_POSTFIX_EXPR,
	AST_ARRAY_ACCESS_EXPR,
	AST_MEMBER_ACCESS_EXPR,
	AST_FUNCTION_CALL_EXPR,

	AST_EXPR_STMT,
	AST_RETURN_STMT,
	AST_CONTINUE_STMT,
	AST_BREAK_STMT,
	AST_DEFER_STMT,
	AST_WHILE_STMT,
	AST_IF_STMT,
	AST_BLOCK_STMT,
	AST_VAR_STMT,

	AST_TYPE_SPECIFIER,

	AST_STRUCT_DECL,
	AST_FUNCTION_DECL,
	AST_VAR_DECL,

	AST_PROG,
};

struct ast_t {
	int kind;
	const char *filepath;
	const char *source;
	pos_t start;
	pos_t end;

	// keep track of the current scope (prog, function_decl, block_stmt)
	scope_t *scope;

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

		// AST_TERNARY_EXPR
		struct {
			struct ast_t *left;
			struct ast_t *mid;
			struct ast_t *right;
		} ternary_expr;

		// AST_PREFIX_EXPR
		struct {
			token_t *op;
			struct ast_t *right;
		} prefix_expr;

		// AST_POSTFIX_EXPR
		struct {
			struct ast_t *left;
			token_t *op;
		} postfix_expr;

		// AST_ARRAY_ACCESS_EXPR
		struct {
			struct ast_t *left;
			struct ast_t *index;
		} array_access_expr;

		// AST_MEMBER_ACCESS_EXPR
		struct {
			struct ast_t *left;
			token_t *op;
			token_t *member;
		} member_access_expr;

		// AST_FUNCTION_CALL_EXPR
		struct {
			struct ast_t *left;

			int argc;
			struct ast_t **argv;
		} function_call_expr;

		// AST_EXPR_STMT
		struct {
			struct ast_t *expr;
		} expr_stmt;

		// AST_RETURN_STMT
		struct {
			struct ast_t *expr;
		} return_stmt;

		// AST_DEFER_STMT
		struct {
			struct ast_t *expr;
		} defer_stmt;

		// AST_WHILE_STMT
		struct {
			struct ast_t *expr;
			struct ast_t *stmt;
		} while_stmt;

		// AST_IF_STMT
		struct {
			struct ast_t *expr;
			struct ast_t *true_stmt;
			struct ast_t *false_stmt;
		} if_stmt;

		// AST_BLOCK_STMT
		struct {
			int argc;
			struct ast_t **argv; // statement array
		} block_stmt;

		// AST_VAR_STMT
		struct {
			token_t *identifier;
			struct ast_t *type;
			struct ast_t *expr;
		} var_stmt;

		// AST_TYPE_SPECIFIER
		struct {
			token_t *type_name;

			int pointer_cnt;

			// array sizes (but only AST_LITERAL_EXPR, and only TK_INT_LITERAL)
			int argc;
			struct ast_t **argv;
		} type_specifier;

		// AST_STRUCT_DECL
		struct {
			// struct name
			token_t *identifier;

			// struct fields (but only AST_VAR_EXPR, and only TK_IDENTIFIER)
			int fields_cnt;
			struct ast_t **fields;

			// type of each struct fields (but only AST_TYPE_SPECIFIER)
			int types_cnt;
			struct ast_t **types;
		} struct_decl;

		// AST_FUNCTION_DECL
		struct {
			// function name
			token_t *identifier;

			// function parameter names (but only AST_VAR_EXPR, only TK_IDENTIFIER)
			int params_cnt;
			struct ast_t **params;

			// type of each function parameters (but only AST_TYPE_SPECIFIER)
			int types_cnt;
			struct ast_t **types;
			
			// function return type (but only AST_TYPE_SPECIFIER)
			struct ast_t *return_type;

			// function body (but only AST_BLOCK_STMT)
			struct ast_t *body;
		} function_decl;

		// AST_PROG
		struct {
			// Number of declarations
			int argc;
			struct ast_t **argv;
		} prog;
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

