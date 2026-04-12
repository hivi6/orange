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
static ast_t *assign_expr();
static ast_t *ternary_expr();
static ast_t *lor_expr();
static ast_t *land_expr();
static ast_t *bor_expr();
static ast_t *bxor_expr();
static ast_t *band_expr();
static ast_t *equality_expr();
static ast_t *relational_expr();
static ast_t *shift_expr();
static ast_t *add_expr();
static ast_t *mul_expr();
static ast_t *prefix_expr();
static ast_t *postfix_expr();
static ast_t *primary_expr();

static ast_t *malloc_ast(int kind, const char *filepath, const char *source,
	pos_t start, pos_t end);
static ast_t *malloc_ast_literal(token_t *literal);
static ast_t *malloc_ast_binary(ast_t *left, token_t *op, ast_t *right);
static ast_t *malloc_ast_ternary(ast_t *left, ast_t *mid, ast_t *right);
static ast_t *malloc_ast_prefix(token_t *op, ast_t *right);
static ast_t *malloc_ast_postfix(ast_t *left, token_t *op);
static ast_t *malloc_ast_group(token_t *lparen, ast_t *group, token_t *rparen);

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

	case AST_TERNARY:
		printf("+- AST_TERNARY\n");

		print_ast_helper(ast->left, tree, index+1);
		print_ast_helper(ast->mid, tree, index+1);
		tree[index+1] = 0;
		print_ast_helper(ast->right, tree, index+1);
		break;

	case AST_PREFIX:
		printf("+- AST_PREFIX(");
		print_token(ast->op);
		printf(")\n");

		tree[index+1] = 0;
		print_ast_helper(ast->right, tree, index+1);
		break;

	case AST_POSTFIX:
		printf("+- AST_POSTFIX(");
		print_token(ast->op);
		printf(")\n");

		tree[index+1] = 0;
		print_ast_helper(ast->left, tree, index+1);
		break;

	case AST_GROUP:
		printf("+- AST_GROUP\n");
		
		tree[index+1] = 0;
		print_ast_helper(ast->left, tree, index+1);
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
	return assign_expr();
}

static ast_t *assign_expr() {
	ast_t *left = ternary_expr();
	while (token_at(0)->kind == TK_EQUAL) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = ternary_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *ternary_expr() {
	ast_t *left = lor_expr();
	
	if (token_at(0)->kind == TK_QUESTION) {
		token_skip(1); // skip ?
		ast_t *mid = ternary_expr();
		
		token_t *colon = token_at(0);
		if (colon->kind != TK_COLON) {
			eprintf(colon->filepath, colon->source, colon->start, colon->end,
				"Expected ':' in ternary operation");
			exit(1);
		}
		token_skip(1); // skip :
		ast_t *right = ternary_expr();

		left = malloc_ast_ternary(left, mid, right);
	}

	return left;
}

static ast_t *lor_expr() {
	ast_t *left = land_expr();
	while (token_at(0)->kind == TK_PIPE_PIPE) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = land_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *land_expr() {
	ast_t *left = bor_expr();
	while (token_at(0)->kind == TK_AMPERSAND_AMPERSAND) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = bor_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *bor_expr() {
	ast_t *left = bxor_expr();
	while (token_at(0)->kind == TK_PIPE) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = bxor_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *bxor_expr() {
	ast_t *left = band_expr();
	while (token_at(0)->kind == TK_CARET) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = band_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *band_expr() {
	ast_t *left = equality_expr();
	while (token_at(0)->kind == TK_AMPERSAND) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = equality_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *equality_expr() {
	ast_t *left = relational_expr();
	while (token_at(0)->kind == TK_EQUAL_EQUAL || token_at(0)->kind == TK_BANG_EQUAL) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = relational_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *relational_expr() {
	ast_t *left = shift_expr();
	while (token_at(0)->kind == TK_LCHEVRON || token_at(0)->kind == TK_LCHEVRON_EQUAL ||
		token_at(0)->kind == TK_RCHEVRON || token_at(0)->kind == TK_RCHEVRON_EQUAL) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = shift_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *shift_expr() {
	ast_t *left = add_expr();
	while (token_at(0)->kind == TK_LCHEVRON_LCHEVRON || token_at(0)->kind == TK_RCHEVRON_RCHEVRON) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = add_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *add_expr() {
	ast_t *left = mul_expr();
	while (token_at(0)->kind == TK_PLUS || token_at(0)->kind == TK_DASH) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = mul_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *mul_expr() {
	ast_t *left = prefix_expr();
	while (token_at(0)->kind == TK_STAR || token_at(0)->kind == TK_FSLASH ||
		token_at(0)->kind == TK_MOD) {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = prefix_expr();
		left = malloc_ast_binary(left, op, right);
	}
	return left;
}

static ast_t *prefix_expr() {
	switch (token_at(0)->kind) {
	case TK_PLUS:
	case TK_DASH:
	case TK_PLUS_PLUS:
	case TK_DASH_DASH:
	case TK_BANG:
	case TK_TILDE:
	case TK_STAR:
	case TK_AMPERSAND:
	case TK_SIZEOF_KEYWORD: {
		token_t *op = token_at(0);
		token_skip(1);
		ast_t *right = prefix_expr();
		return malloc_ast_prefix(op, right);
	}
	}

	return postfix_expr();
}

static ast_t *postfix_expr() {
	ast_t *left = primary_expr();

	token_t *op = token_at(0);
	if (op->kind == TK_PLUS_PLUS || op->kind == TK_DASH_DASH) {
		token_skip(1);
		return malloc_ast_postfix(left, op);
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

	case TK_LPAREN: {
		token_t *lparen = token_at(0);
		token_skip(1); // skip (
		
		ast_t *group = expr();

		token_t *rparen = token_at(0);
		if (rparen->kind != TK_RPAREN) {
			eprintf(rparen->filepath, rparen->source, rparen->start, rparen->end,
				"Expected ')'");
			exit(1);
		}
		token_skip(1); // skip )

		return malloc_ast_group(lparen, group, rparen);
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

static ast_t *malloc_ast_ternary(ast_t *left, ast_t *mid, ast_t *right) {
	ast_t *res = malloc_ast(AST_TERNARY, left->filepath, left->source,
		left->start, right->end);
	res->left = left;
	res->mid = mid;
	res->right = right;
	return res;
}

static ast_t *malloc_ast_prefix(token_t *op, ast_t *right) {
	ast_t *res = malloc_ast(AST_PREFIX, op->filepath, op->source,
		op->start, right->end);
	res->op = op;
	res->right = right;
	return res;
}

static ast_t *malloc_ast_postfix(ast_t *left, token_t *op) {
	ast_t *res = malloc_ast(AST_POSTFIX, op->filepath, op->source,
		op->start, op->end);
	res->left = left;
	res->op = op;
	return res;
}

static ast_t *malloc_ast_group(token_t *lparen, ast_t *group, token_t *rparen) {
	ast_t *res = malloc_ast(AST_GROUP, group->filepath, group->source,
		lparen->start, rparen->end);
	res->left = group;
	return res;
}

