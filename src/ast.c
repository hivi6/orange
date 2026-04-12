#include "ast.h"
#include "error.h"
#include "common.h"

// ========================================
// helper declaration
// ========================================

static token_t *g_tokens;

static void print_ast_helper(ast_t *ast, char *tree, int index);
static void print_token(token_t *token);

static void init(token_t *tokens);
static char is_eof();
static token_t *token_at(int offset);
static void token_skip(int inc);

static ast_t *expr();
static ast_t *add_expr();
static ast_t *primary_expr();

static ast_t *malloc_ast(int kind, const char *filepath, const char *source,
	pos_t start, pos_t end);
static ast_t *malloc_ast_literal(token_t *literal);
static ast_t *malloc_ast_binary(ast_t *left, token_t *op, ast_t *right);

// ========================================
// ast.h - definition
// ========================================

ast_t *parse(token_t *tokens) {
	init(tokens);

	ast_t *ast = expr();
	if (!is_eof()) {
		token_t *token = token_at(0);
		eprintf(token->filepath, token->source, token->start, token->end,
			"Unexpected token, expected EOF");
		exit(1);
	}

	return ast;
}

void print_ast(ast_t *ast) {
	printf("AST\n");
	char tree[AST_PRINT_DEPTH] = {};
	print_ast_helper(ast, tree, 0);
}

// ========================================
// helper definition
// ========================================

static void print_ast_helper(ast_t *ast, char *tree, int index) {
	if (index + 1 >= AST_PRINT_DEPTH) {
		printf("VERY LONG AST DEPTH, NOT PRINTING THE REST...\n");
		return;
	}

	for (int i = 0; i < index; i++) {
		if (tree[i]) printf("|  ");
		else printf("   ");
	}

	tree[index+1] = 1;

	switch (ast->kind) {
	case AST_LITERAL:
		printf("+- AST_LITERAL (");
		print_token(ast->literal);
		printf(")\n");
		tree[index+1] = 0;

		break;
	case AST_BINARY:
		printf("+- AST_BINARY (");
		print_token(ast->op);
		printf(")\n");

		print_ast_helper(ast->left, tree, index+1);
		tree[index+1] = 0;
		print_ast_helper(ast->right, tree, index+1);
		break;
	}
}

static void print_token(token_t *token) {
	printf("%s | '", token_kind_str(token->kind));
	for (int i = token->start.index; i < token->end.index; i++) {
		printf("%c", token->source[i]);
	}
	printf("'");
}

static void init(token_t *tokens) {
	g_tokens = tokens;
}

static char is_eof() {
	return g_tokens == NULL || g_tokens->kind == TK_EOF;
}

static token_t *token_at(int offset) {
	token_t *res = g_tokens;
	while (offset > 0 && res) {
		res = res->next;
		offset--;
	}
	return res;
}

static void token_skip(int inc) {
	while (g_tokens && inc > 0) {
		g_tokens = g_tokens->next;
		inc--;
	}
}

static ast_t *expr() {
	return add_expr();
}

static ast_t *add_expr() {
	ast_t *left = primary_expr();
	while (token_at(0)->kind == TK_PLUS || token_at(0)->kind == TK_DASH) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = primary_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *primary_expr() {
	token_t *token = token_at(0);

	switch (token_at(0)->kind) {
	case TK_INT_LITERAL:
	case TK_CHAR_LITERAL:
	case TK_STR_LITERAL: {
		token_skip(1);
		return malloc_ast_literal(token);
	}
	}

	eprintf(token->filepath, token->source, token->start, token->end,
		"Unexpected token");
	exit(1);
}

static ast_t *malloc_ast(int kind, const char *filepath, const char *source,
	pos_t start, pos_t end) {
	ast_t *res = malloc(sizeof(ast_t));
	res->kind = kind;
	res->filepath = filepath;
	res->source = source;
	res->start = start;
	res->end = end;
	return res;
}

static ast_t *malloc_ast_literal(token_t *literal) {
	ast_t *res = malloc_ast(AST_LITERAL, literal->filepath, literal->source,
		literal->start, literal->end);
	res->literal = literal;
	return res;
}

static ast_t *malloc_ast_binary(ast_t *left, token_t *op, ast_t *right) {
	ast_t *res = malloc_ast(AST_BINARY, left->filepath, left->source,
		left->start, right->end);
	res->left = left;
	res->op = op;
	res->right = right;
	return res;
}

