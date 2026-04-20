#include "ast.h"
#include "common.h"
#include "error.h"

// ========================================
// helper declaration
// ========================================

static token_t *g_tokens;
static const char *g_filepath;
static const char *g_source;

static void init(token_t *tokens);
static token_t *token_at(int index);
static void token_skip(int inc);
static void print_ast_helper(ast_t *ast, char *depth, int index);
static void print_token(token_t *token);

static ast_t *malloc_ast(int kind, const char *filepath, const char *source,
	pos_t start, pos_t end);
static ast_t *malloc_ast_literal_expr(token_t *token);
static ast_t *malloc_ast_var_expr(token_t *token);

static ast_t *expr();
static ast_t *primary_expr();

// ========================================
// ast.h - definition
// ========================================

ast_t *parse(token_t *tokens) {
	init(tokens);
	ast_t *ast = expr();
	return ast;
}

void print_ast(ast_t *ast) {
	char depth[AST_PRINT_DEPTH];
	printf("AST\n");
	print_ast_helper(ast, depth, 0);
}

// ========================================
// helper declaration
// ========================================

static void init(token_t *tokens) {
	g_tokens = tokens;
}

static token_t *token_at(int index) {
	token_t *head = NULL;
	for (head = g_tokens; index > 0 && head->kind != TK_EOF; head = head->next, index--);
	return head;
}

static void token_skip(int inc) {
	while (g_tokens->kind != TK_EOF && inc) {
		g_tokens = g_tokens->next;
		inc--;
	}
}

static void print_ast_helper(ast_t *ast, char *depth, int index) {
	if (index >= AST_PRINT_DEPTH) {
		printf("Print stack too deep\n");
		return;
	}

	for (int i = 0; i < index; i++) {
		if (depth[i]) printf("|  ");
		else printf("   ");
	}

	depth[index+1] = 1;
	switch (ast->kind) {
	case AST_LITERAL_EXPR: {
		printf("+- AST_LITERAL_EXPR(");
		print_token(ast->ast.literal_expr.token);
		printf(")\n");
		depth[index+1] = 0;
		break;
	}
	case AST_VAR_EXPR: {
		printf("+- AST_VAR_EXPR(");
		print_token(ast->ast.literal_expr.token);
		printf(")\n");
		depth[index+1] = 0;
		break;
	}
	default: {
		printf("\n");
		eprintf(ast->filepath, ast->source, ast->start, ast->end,
			"What is this ast that needs printing? Probably no print definition(%d)", ast->kind);
		exit(1);
	}
	}
}

static void print_token(token_t *token) {
	printf("%s | ", token_kind_str(token->kind));
	for (int i = token->start.index; i < token->end.index; i++) {
		printf("%c", token->source[i]);
	}
}

static ast_t *malloc_ast(int kind, const char *filepath, const char *source,
	pos_t start, pos_t end) {
	ast_t *ast = calloc(sizeof(ast_t), 1);
	ast->kind = kind;
	ast->filepath = filepath;
	ast->source = source;
	ast->start = start;
	ast->end = end;
	return ast;
}

static ast_t *malloc_ast_literal_expr(token_t *token) {
	ast_t *ast = malloc_ast(AST_LITERAL_EXPR, token->filepath, token->source, 
		token->start, token->end);
	ast->ast.literal_expr.token = token;
	return ast;
}

static ast_t *malloc_ast_var_expr(token_t *token) {
	ast_t *ast = malloc_ast(AST_VAR_EXPR, token->filepath, token->source, 
		token->start, token->end);
	ast->ast.var_expr.token = token;
	return ast;
}

static ast_t *expr() {
	return primary_expr();
}

static ast_t *primary_expr() {
	switch (token_at(0)->kind) {
	case TK_INT_LITERAL:
	case TK_STR_LITERAL:
	case TK_CHAR_LITERAL: {
		token_t *token = token_at(0);
		token_skip(1);
		return malloc_ast_literal_expr(token);
	}
	case TK_IDENTIFIER: {
		token_t *token = token_at(0);
		token_skip(1);
		return malloc_ast_var_expr(token);
	}
	}

	token_t *token = token_at(0);
	eprintf(token->filepath, token->source, token->start, token->end, "Unexpected token");
	exit(1);
}

