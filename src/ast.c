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
static ast_t *malloc_ast_group_expr(token_t *lparen, ast_t *expr, token_t *rparen);
static ast_t *malloc_ast_binary_expr(ast_t *left, token_t *op, ast_t *right);

static ast_t *expr();
static ast_t *lor_expr();
static ast_t *land_expr();
static ast_t *bor_expr();
static ast_t *bxor_expr();
static ast_t *band_expr();
static ast_t *equality_expr();
static ast_t *relation_expr();
static ast_t *shift_expr();
static ast_t *add_expr();
static ast_t *mul_expr();
static ast_t *primary_expr();

// ========================================
// ast.h - definition
// ========================================

ast_t *parse(token_t *tokens) {
	init(tokens);
	ast_t *ast = expr();
	if (token_at(0)->kind != TK_EOF) {
		token_t *token = token_at(0);
		eprintf(token->filepath, token->source, token->start, token->end,
			"Expected EOF but got some token!");
		exit(1);
	}

	return ast;
}

void print_ast(ast_t *ast) {
	char depth[AST_PRINT_DEPTH] = {};
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

	for (int i = 0; i < index+1; i++) {
		if (i == index || depth[i]) printf("|  ");
		else printf("   ");
	}
	printf("\n");
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

	case AST_GROUP_EXPR: {
		printf("+- AST_GROUP_EXPR\n");
		depth[index+1] = 0;
		print_ast_helper(ast->ast.group_expr.expr, depth, index+1);
		break;
	}

	case AST_BINARY_EXPR: {
		printf("+- AST_BINARY_EXPR(");
		print_token(ast->ast.binary_expr.op);
		printf(")\n");

		print_ast_helper(ast->ast.binary_expr.left, depth, index+1);
		depth[index+1] = 0;
		print_ast_helper(ast->ast.binary_expr.right, depth, index+1);
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
	printf("%s | '", token_kind_str(token->kind));
	for (int i = token->start.index; i < token->end.index; i++) {
		printf("%c", token->source[i]);
	}
	printf("'");
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

static ast_t *malloc_ast_group_expr(token_t *lparen, ast_t *expr, token_t *rparen) {
	ast_t *ast = malloc_ast(AST_GROUP_EXPR, expr->filepath, expr->source,
		lparen->start, rparen->end);
	ast->ast.group_expr.expr = expr;
	return ast;
}

static ast_t *malloc_ast_binary_expr(ast_t *left, token_t *op, ast_t *right) {
	ast_t *ast = malloc_ast(AST_BINARY_EXPR, left->filepath, left->source,
		left->start, right->end);
	ast->ast.binary_expr.left = left;
	ast->ast.binary_expr.op = op;
	ast->ast.binary_expr.right = right;
	return ast;
}

static ast_t *expr() {
	return lor_expr();
}

static ast_t *lor_expr() {
	ast_t *left = land_expr();
	while (token_at(0)->kind == TK_PIPE_PIPE) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = land_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *land_expr() {
	ast_t *left = bor_expr();
	while (token_at(0)->kind == TK_AMPERSAND_AMPERSAND) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = bor_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *bor_expr() {
	ast_t *left = bxor_expr();
	while (token_at(0)->kind == TK_PIPE) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = bxor_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *bxor_expr() {
	ast_t *left = band_expr();
	while (token_at(0)->kind == TK_CARET) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = band_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *band_expr() {
	ast_t *left = equality_expr();
	while (token_at(0)->kind == TK_AMPERSAND) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = equality_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *equality_expr() {
	ast_t *left = relation_expr();
	while (token_at(0)->kind == TK_EQUAL_EQUAL || token_at(0)->kind == TK_BANG_EQUAL) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = relation_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *relation_expr() {
	ast_t *left = shift_expr();
	while (token_at(0)->kind == TK_LCHEVRON || token_at(0)->kind == TK_LCHEVRON_EQUAL || 
		token_at(0)->kind == TK_RCHEVRON || token_at(0)->kind == TK_RCHEVRON_EQUAL) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = shift_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *shift_expr() {
	ast_t *left = add_expr();
	while (token_at(0)->kind == TK_LCHEVRON_LCHEVRON || token_at(0)->kind == TK_RCHEVRON_RCHEVRON) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = add_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *add_expr() {
	ast_t *left = mul_expr();
	while (token_at(0)->kind == TK_PLUS || token_at(0)->kind == TK_DASH) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = mul_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
}

static ast_t *mul_expr() {
	ast_t *left = primary_expr();
	while (token_at(0)->kind == TK_STAR || token_at(0)->kind == TK_FSLASH ||
		token_at(0)->kind == TK_MOD) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = primary_expr();

		left = malloc_ast_binary_expr(left, op, right);
	}
	return left;
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
	case TK_LPAREN: {
		token_t *lparen = token_at(0);
		token_skip(1); // skip (

		ast_t *ast = expr();
		token_t *rparen = token_at(0);
		if (rparen->kind != TK_RPAREN) {
			eprintf(rparen->filepath, rparen->source, ast->start, rparen->end,
				"Expected ')' at the end of expression");
			exit(1);
		}
		token_skip(1); // skip )

		return malloc_ast_group_expr(lparen, ast, rparen);
	}
	}

	token_t *token = token_at(0);
	eprintf(token->filepath, token->source, token->start, token->end, "Unexpected token");
	exit(1);
}

