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
static void append_ast(int *argc, ast_t ***argv, ast_t *arg);

static ast_t *malloc_ast(int kind, const char *filepath, const char *source,
	pos_t start, pos_t end);
static ast_t *malloc_ast_literal_expr(token_t *token);
static ast_t *malloc_ast_var_expr(token_t *token);
static ast_t *malloc_ast_group_expr(token_t *lparen, ast_t *expr, token_t *rparen);
static ast_t *malloc_ast_binary_expr(ast_t *left, token_t *op, ast_t *right);
static ast_t *malloc_ast_ternary_expr(ast_t *left, ast_t *mid, ast_t *right);
static ast_t *malloc_ast_prefix_expr(token_t *op, ast_t *right);
static ast_t *malloc_ast_postfix_expr(ast_t *left, token_t *op);
static ast_t *malloc_ast_array_access_expr(ast_t *left, token_t *lbracket, ast_t *index, 
	token_t *rbracket);
static ast_t *malloc_ast_member_access_expr(ast_t *left, token_t *op, token_t *member);
static ast_t *malloc_ast_function_call_expr(ast_t *left);
static ast_t *malloc_ast_expr_stmt(ast_t *expr, token_t *semicolon);
static ast_t *malloc_ast_return_stmt(token_t *return_keyword, ast_t *expr, token_t *semicolon);
static ast_t *malloc_ast_continue_stmt(token_t *continue_keyword, token_t *semicolon);
static ast_t *malloc_ast_break_stmt(token_t *break_keyword, token_t *semicolon);
static ast_t *malloc_ast_defer_stmt(token_t *defer_keyword, ast_t *expr, token_t *semicolon);
static ast_t *malloc_ast_while_stmt(token_t *while_keyword, ast_t *expr, ast_t *stmt);
static ast_t *malloc_ast_if_stmt(token_t *if_keyword, ast_t *expr, ast_t *true_stmt, ast_t *false_stmt);
static ast_t *malloc_ast_block_stmt(token_t *lbrace);
static ast_t *malloc_ast_type_specifier(token_t *type_name);
static ast_t *malloc_ast_var_stmt(token_t *var_keyword, token_t *identifier, ast_t *type, 
	ast_t *expr, token_t *semicolon);
static ast_t *malloc_ast_struct_decl(token_t *struct_keyword, token_t *struct_name);

static ast_t *decl();
static ast_t *struct_decl();

static ast_t *type_specifier();

static ast_t *stmt();
static ast_t *var_stmt();
static ast_t *block_stmt();
static ast_t *if_stmt();
static ast_t *while_stmt();
static ast_t *defer_stmt();
static ast_t *break_stmt();
static ast_t *continue_stmt();
static ast_t *return_stmt();
static ast_t *expr_stmt();

static ast_t *expr();
static ast_t *assign_expr();
static ast_t *ternary_expr();
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
static ast_t *prefix_expr();
static ast_t *postfix_expr();
static ast_t *primary_expr();

// ========================================
// ast.h - definition
// ========================================

ast_t *parse(token_t *tokens) {
	init(tokens);
	ast_t *ast = decl();
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

	case AST_TERNARY_EXPR: {
		printf("+- AST_TERNARY_EXPR\n");
		
		print_ast_helper(ast->ast.ternary_expr.left, depth, index+1);
		print_ast_helper(ast->ast.ternary_expr.mid, depth, index+1);
		depth[index+1] = 0;
		print_ast_helper(ast->ast.ternary_expr.right, depth, index+1);
		break;
	}

	case AST_PREFIX_EXPR: {
		printf("+- AST_PREFIX_EXPR(");
		print_token(ast->ast.prefix_expr.op);
		printf(")\n");

		depth[index+1] = 0;
		print_ast_helper(ast->ast.prefix_expr.right, depth, index+1);
		break;
	}

	case AST_POSTFIX_EXPR: {
		printf("+- AST_POSTFIX_EXPR(");
		print_token(ast->ast.postfix_expr.op);
		printf(")\n");

		depth[index+1] = 0;
		print_ast_helper(ast->ast.postfix_expr.left, depth, index+1);
		break;
	}

	case AST_ARRAY_ACCESS_EXPR: {
		printf("+- AST_ARRAY_ACCESS_EXPR\n");

		print_ast_helper(ast->ast.array_access_expr.left, depth, index+1);
		depth[index+1] = 0;
		print_ast_helper(ast->ast.array_access_expr.index, depth, index+1);
		break;
	}

	case AST_MEMBER_ACCESS_EXPR: {
		printf("+- AST_MEMBER_ACCESS_EXPR(");
		print_token(ast->ast.member_access_expr.op);
		printf(", ");
		print_token(ast->ast.member_access_expr.member);
		printf(")\n");

		depth[index+1] = 0;
		print_ast_helper(ast->ast.member_access_expr.left, depth, index+1);
		break;
	}

	case AST_FUNCTION_CALL_EXPR: {
		printf("+- AST_FUNCTION_CALL_EXPR(ARGUMENTS: %d)\n",
			ast->ast.function_call_expr.argc);

		if (ast->ast.function_call_expr.argc == 0) depth[index+1] = 0;
		print_ast_helper(ast->ast.function_call_expr.left, depth, index+1);

		for (int i = 0; i < ast->ast.function_call_expr.argc; i++) {
			if (i == ast->ast.function_call_expr.argc-1) {
				depth[index+1] = 0;
			}
			print_ast_helper(ast->ast.function_call_expr.argv[i], depth, index+1);
		}
		break;
	}

	case AST_EXPR_STMT: {
		printf("+- AST_EXPR_STMT\n");
		
		depth[index+1] = 0;
		print_ast_helper(ast->ast.expr_stmt.expr, depth, index+1);
		break;
	}

	case AST_RETURN_STMT: {
		printf("+- AST_RETURN_STMT\n");

		depth[index+1] = 0;
		if (ast->ast.return_stmt.expr) {
			print_ast_helper(ast->ast.return_stmt.expr, depth, index+1);
		}
		break;
	}

	case AST_CONTINUE_STMT: {
		printf("+- AST_CONTINUE_STMT\n");
		break;
	}

	case AST_BREAK_STMT: {
		printf("+- AST_BREAK_STMT\n");
		break;
	}

	case AST_DEFER_STMT: {
		printf("+- AST_DEFER_STMT\n");

		depth[index+1] = 0;
		print_ast_helper(ast->ast.defer_stmt.expr, depth, index+1);
		break;
	}

	case AST_WHILE_STMT: {
		printf("+- AST_WHILE_STMT\n");

		print_ast_helper(ast->ast.while_stmt.expr, depth, index+1);
		depth[index+1] = 0;
		print_ast_helper(ast->ast.while_stmt.stmt, depth, index+1);
		break;
	}

	case AST_IF_STMT: {
		printf("+- AST_IF_STMT\n");

		print_ast_helper(ast->ast.if_stmt.expr, depth, index+1);
		if (ast->ast.if_stmt.false_stmt == NULL) depth[index+1] = 0;
		print_ast_helper(ast->ast.if_stmt.true_stmt, depth, index+1);
		depth[index+1] = 0;
		if (ast->ast.if_stmt.false_stmt) {
			print_ast_helper(ast->ast.if_stmt.false_stmt, depth, index+1);
		}
		break;
	}

	case AST_BLOCK_STMT: {
		printf("+- AST_BLOCK_STMT\n");

		for (int i = 0; i < ast->ast.block_stmt.argc; i++) {
			if (i == ast->ast.block_stmt.argc-1) {
				depth[index+1] = 0;
			}
			print_ast_helper(ast->ast.block_stmt.argv[i], depth, index+1);
		}
		break;
	}

	case AST_VAR_STMT: {
		printf("+- AST_VAR_STMT(");
		print_token(ast->ast.var_stmt.identifier);
		printf(")\n");

		if (ast->ast.var_stmt.type) print_ast_helper(ast->ast.var_stmt.type, depth, index+1);
		if (ast->ast.var_stmt.expr) {
			depth[index+1] = 0;
			print_ast_helper(ast->ast.var_stmt.expr, depth, index+1);
		}
		break;
	}

	case AST_TYPE_SPECIFIER: {
		printf("+- AST_TYPE_SPECIFIER(");
		for (int i = ast->start.index; i < ast->end.index; i++) {
			printf("%c", ast->source[i]);
		}
		printf(")\n");
		break;
	}

	case AST_STRUCT_DECL: {
		printf("+- AST_STRUCT_DECL\n");

		for (int i = 0; i < ast->ast.struct_decl.fields_cnt; i++) {
			print_ast_helper(ast->ast.struct_decl.fields[i], depth, index+1);
			print_ast_helper(ast->ast.struct_decl.types[i], depth, index+1);
		}

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

static void append_ast(int *argc, ast_t ***argv, ast_t *arg) {
	(*argc)++;
	*argv = realloc(*argv, sizeof(ast_t *) * (*argc));
	(*argv)[*argc - 1] = arg;
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

static ast_t *malloc_ast_ternary_expr(ast_t *left, ast_t *mid, ast_t *right) {
	ast_t *ast = malloc_ast(AST_TERNARY_EXPR, left->filepath, left->source,
		left->start, right->end);
	ast->ast.ternary_expr.left = left;
	ast->ast.ternary_expr.mid = mid;
	ast->ast.ternary_expr.right = right;
	return ast;
}

static ast_t *malloc_ast_prefix_expr(token_t *op, ast_t *right) {
	ast_t *ast = malloc_ast(AST_PREFIX_EXPR, op->filepath, op->source,
		op->start, right->end);
	ast->ast.prefix_expr.op = op;
	ast->ast.prefix_expr.right = right;
	return ast;
}

static ast_t *malloc_ast_postfix_expr(ast_t *left, token_t *op) {
	ast_t *ast = malloc_ast(AST_POSTFIX_EXPR, left->filepath, left->source,
		left->start, op->end);
	ast->ast.postfix_expr.left = left;
	ast->ast.postfix_expr.op = op;
	return ast;
}

static ast_t *malloc_ast_array_access_expr(ast_t *left, token_t *lbracket, ast_t *index, 
	token_t *rbracket) {
	ast_t *ast = malloc_ast(AST_ARRAY_ACCESS_EXPR, left->filepath, left->source,
		left->start, rbracket->end);
	ast->ast.array_access_expr.left = left;
	ast->ast.array_access_expr.index = index;
	return ast;
}

static ast_t *malloc_ast_member_access_expr(ast_t *left, token_t *op, token_t *member) {
	ast_t *ast = malloc_ast(AST_MEMBER_ACCESS_EXPR, left->filepath, left->source,
		left->start, member->end);
	ast->ast.member_access_expr.left = left;
	ast->ast.member_access_expr.op = op;
	ast->ast.member_access_expr.member = member;
	return ast;
}

static ast_t *malloc_ast_function_call_expr(ast_t *left) {
	ast_t *ast = malloc_ast(AST_FUNCTION_CALL_EXPR, left->filepath, left->source,
		left->start, left->end);
	ast->ast.function_call_expr.left = left;
	ast->ast.function_call_expr.argc = 0;
	ast->ast.function_call_expr.argv = NULL;
	return ast;
}

static ast_t *malloc_ast_expr_stmt(ast_t *expr, token_t *semicolon) {
	ast_t *ast = malloc_ast(AST_EXPR_STMT, expr->filepath, expr->source,
		expr->start, semicolon->end);
	ast->ast.expr_stmt.expr = expr;
	return ast;
}

static ast_t *malloc_ast_return_stmt(token_t *return_keyword, ast_t *expr, token_t *semicolon) {
	ast_t *ast = malloc_ast(AST_RETURN_STMT, return_keyword->filepath, return_keyword->source,
		return_keyword->start, semicolon->end);
	ast->ast.return_stmt.expr = expr;
	return ast;
}

static ast_t *malloc_ast_continue_stmt(token_t *continue_keyword, token_t *semicolon) {
	ast_t *ast = malloc_ast(AST_CONTINUE_STMT, continue_keyword->filepath, semicolon->source,
		continue_keyword->start, semicolon->end);
	return ast;
}

static ast_t *malloc_ast_break_stmt(token_t *break_keyword, token_t *semicolon) {
	ast_t *ast = malloc_ast(AST_BREAK_STMT, break_keyword->filepath, break_keyword->source,
		break_keyword->start, semicolon->end);
	return ast;
}

static ast_t *malloc_ast_defer_stmt(token_t *defer_keyword, ast_t *expr, token_t *semicolon) {
	ast_t *ast = malloc_ast(AST_DEFER_STMT, defer_keyword->filepath, defer_keyword->source,
		defer_keyword->start, semicolon->end);
	ast->ast.defer_stmt.expr = expr;
	return ast;
}

static ast_t *malloc_ast_while_stmt(token_t *while_keyword, ast_t *expr, ast_t *stmt) {
	ast_t *ast = malloc_ast(AST_WHILE_STMT, while_keyword->filepath, while_keyword->source,
		while_keyword->start, stmt->end);
	ast->ast.while_stmt.expr = expr;
	ast->ast.while_stmt.stmt = stmt;
	return ast;
}

static ast_t *malloc_ast_if_stmt(token_t *if_keyword, ast_t *expr, ast_t *true_stmt, ast_t *false_stmt) {
	pos_t end = true_stmt->end;
	if (false_stmt) end = false_stmt->end;

	ast_t *ast = malloc_ast(AST_IF_STMT, if_keyword->filepath, if_keyword->source,
		if_keyword->start, end);
	ast->ast.if_stmt.expr = expr;
	ast->ast.if_stmt.true_stmt = true_stmt;
	ast->ast.if_stmt.false_stmt = false_stmt;
	return ast;
}

static ast_t *malloc_ast_block_stmt(token_t *lbrace) {
	ast_t *ast = malloc_ast(AST_BLOCK_STMT, lbrace->filepath, lbrace->source,
		lbrace->start, lbrace->end);
	return ast;
}

static ast_t *malloc_ast_type_specifier(token_t *type_name) {
	ast_t *ast = malloc_ast(AST_TYPE_SPECIFIER, type_name->filepath, type_name->source,
		type_name->start, type_name->end);
	ast->ast.type_specifier.type_name = type_name;
	return ast;
}

static ast_t *malloc_ast_var_stmt(token_t *var_keyword, token_t *identifier, ast_t *type, 
	ast_t *expr, token_t *semicolon) {
	ast_t *ast = malloc_ast(AST_VAR_STMT, var_keyword->filepath, var_keyword->source,
		var_keyword->start, semicolon->end);
	ast->ast.var_stmt.identifier = identifier;
	ast->ast.var_stmt.type = type;
	ast->ast.var_stmt.expr = expr;
	return ast;
}

static ast_t *malloc_ast_struct_decl(token_t *struct_keyword, token_t *struct_name) {
	ast_t *ast = malloc_ast(AST_STRUCT_DECL, struct_keyword->filepath, struct_keyword->source,
		struct_keyword->start, struct_name->end);
	ast->ast.struct_decl.identifier = struct_name;
	return ast;
}

static ast_t *decl() {
	if (token_at(0)->kind == TK_STRUCT_KEYWORD) return struct_decl();
	
	token_t *token = token_at(0);
	eprintf(token->filepath, token->source, token->start, token->end,
		"Unexpected token in decl");
	exit(1);
}

static ast_t *struct_decl() {
	token_t *struct_keyword = token_at(0);
	token_skip(1); // skip struct keyword

	token_t *identifier = token_at(0);
	if (identifier->kind != TK_IDENTIFIER) {
		eprintf(identifier->filepath, identifier->source, struct_keyword->start, identifier->end,
			"Expected identifier after struct keyword");
		exit(1);
	}
	token_skip(1); // skip struct identifier

	token_t *lbrace = token_at(0);
	if (lbrace->kind != TK_LBRACE) {
		eprintf(lbrace->filepath, lbrace->source, struct_keyword->start, lbrace->end,
			"Expected '{' after struct identifier");
		exit(1);
	}
	token_skip(1); // skip {

	ast_t *ast = malloc_ast_struct_decl(struct_keyword, identifier);

	while (token_at(0)->kind != TK_RBRACE) {
		ast_t *field = primary_expr();
		if (field->kind != AST_VAR_EXPR || field->ast.var_expr.token->kind != TK_IDENTIFIER) {
			eprintf(field->filepath, field->source, field->start, field->end,
				"Expected identifier field in struct");
			exit(1);
		}

		token_t *colon = token_at(0);
		if (colon->kind != TK_COLON) {
			eprintf(colon->filepath, colon->source, field->start, colon->end,
				"Expected ':' after identifier field");
			exit(1);
		}
		token_skip(1); // skip :

		ast_t *type = type_specifier();

		token_t *semicolon = token_at(0);
		if (semicolon->kind != TK_SEMICOLON) {
			eprintf(semicolon->filepath, semicolon->source, field->start, semicolon->end,
				"Expected ';' at the end of field declaration");
			exit(1);
		}
		token_skip(1); // skip ;

		append_ast(&(ast->ast.struct_decl.fields_cnt), &(ast->ast.struct_decl.fields), field);
		append_ast(&(ast->ast.struct_decl.types_cnt), &(ast->ast.struct_decl.types), type);
	}

	token_t *rbrace = token_at(0);
	if (token_at(0)->kind != TK_RBRACE) {
		token_t *token = token_at(0);
		eprintf(token->filepath, token->source, struct_keyword->start, token->end,
			"Expected '}' at the end of struct declaration");
		exit(1);
	}
	token_skip(1);

	ast->end = rbrace->end;
	return ast;
}

static ast_t *type_specifier() {
	int pointer_count = 0;

	pos_t start;
	if (token_at(0)->kind == TK_STAR) start = token_at(0)->start;
	while (token_at(0)->kind == TK_STAR) {
		pointer_count++;
		token_skip(1);
	}

	token_t *identifier = token_at(0);
	if (identifier->kind != TK_IDENTIFIER) {
		eprintf(identifier->filepath, identifier->source, identifier->start, identifier->end,
			"Expected an identifier in type_specifier");
		exit(1);
	}
	token_skip(1); // skip identifier;

	ast_t *type_spec = malloc_ast_type_specifier(identifier);
	if (pointer_count) {
		type_spec->ast.type_specifier.pointer_cnt = pointer_count;
		type_spec->start = start;
	}

	while (token_at(0)->kind == TK_LBRACKET) {
		token_skip(1); // skip [

		ast_t *array_size = primary_expr();
		if (array_size->kind != AST_LITERAL_EXPR || 
			array_size->ast.literal_expr.token->kind != TK_INT_LITERAL) {
			eprintf(array_size->filepath, array_size->source, array_size->start, array_size->end,
				"Expected only int literal as array size");
			exit(1);
		}

		append_ast(&(type_spec->ast.type_specifier.argc), &(type_spec->ast.type_specifier.argv), array_size);
		
		token_t *rbracket = token_at(0);
		if (rbracket->kind != TK_RBRACKET) {
			eprintf(rbracket->filepath, rbracket->source, rbracket->start, rbracket->end,
				"Expected ']' after array size");
			exit(1);
		}
		token_skip(1);

		type_spec->end = rbracket->end;
	}

	return type_spec;
}

static ast_t *stmt() {
	if (token_at(0)->kind == TK_VAR_KEYWORD) return var_stmt();
	if (token_at(0)->kind == TK_LBRACE) return block_stmt();
	if (token_at(0)->kind == TK_IF_KEYWORD) return if_stmt();
	if (token_at(0)->kind == TK_WHILE_KEYWORD) return while_stmt();
	if (token_at(0)->kind == TK_DEFER_KEYWORD) return defer_stmt();
	if (token_at(0)->kind == TK_BREAK_KEYWORD) return break_stmt();
	if (token_at(0)->kind == TK_CONTINUE_KEYWORD) return continue_stmt();
	if (token_at(0)->kind == TK_RETURN_KEYWORD) return return_stmt();
	return expr_stmt();
}

static ast_t *var_stmt() {
	token_t *var_keyword = token_at(0);
	token_skip(1); // skip var keyword

	token_t *identifier = token_at(0);
	if (identifier->kind != TK_IDENTIFIER) {
		eprintf(identifier->filepath, identifier->source, var_keyword->start, identifier->end,
			"Expected identifier after var keyword");
		exit(1);
	}
	token_skip(1); // skip identifier

	ast_t *type = NULL;
	if (token_at(0)->kind == TK_COLON) {
		token_skip(1); // skip :
		type = type_specifier();
	}

	ast_t *var_expr = NULL;
	if (token_at(0)->kind == TK_EQUAL) {
		token_skip(1); // skip =
		var_expr = expr();
	}

	token_t *semicolon = token_at(0);
	if (semicolon->kind != TK_SEMICOLON) {
		eprintf(semicolon->filepath, semicolon->source, var_keyword->start, semicolon->end,
			"Expected ';' at the end of var stmt");
		exit(1);
	}
	token_skip(1);

	return malloc_ast_var_stmt(var_keyword, identifier, type, var_expr, semicolon);
}

static ast_t *block_stmt() {
	token_t *lbrace = token_at(0);
	token_skip(1); // skip {

	ast_t *res = malloc_ast_block_stmt(lbrace);

	while (token_at(0)->kind != TK_EOF && token_at(0)->kind != TK_RBRACE) {
		ast_t *ast = stmt();
		append_ast(&(res->ast.block_stmt.argc), &(res->ast.block_stmt.argv), ast);
	}

	if (token_at(0)->kind != TK_RBRACE) {
		token_t *token = token_at(0);
		eprintf(token->filepath, token->source, res->start, token->end,
			"Expected '}' at the end of block stmt");
		exit(1);
	}
	token_t *rbrace = token_at(0);
	token_skip(1);
	res->end = rbrace->end;

	return res;
}

static ast_t *if_stmt() {
	token_t *if_keyword = token_at(0);
	token_skip(1); // skip if keyword

	if (token_at(0)->kind != TK_LPAREN) {
		token_t *token = token_at(0);
		eprintf(token->filepath, token->source, if_keyword->start, token->end,
			"Expected '(' after if keyword");
		exit(1);
	}
	token_skip(1); // skip (

	ast_t *if_expr = expr();

	if (token_at(0)->kind != TK_RPAREN) {
		token_t *token = token_at(0);
		eprintf(token->filepath, token->source, if_keyword->start, token->end,
			"Expected ')' after if expr");
		exit(1);
	}
	token_skip(1); // skip )

	ast_t *true_stmt = stmt();
	ast_t *false_stmt = NULL;

	if (token_at(0)->kind == TK_ELSE_KEYWORD) {
		token_skip(1); // skip else keyword
		false_stmt = stmt();
	}

	return malloc_ast_if_stmt(if_keyword, if_expr, true_stmt, false_stmt);
}

static ast_t *while_stmt() {
	token_t *while_keyword = token_at(0);
	token_skip(1); // skip while keyword

	if (token_at(0)->kind != TK_LPAREN) {
		token_t *token = token_at(0);
		eprintf(token->filepath, token->source, while_keyword->start, token->end,
			"Expected '(' after while keyword");
		exit(1);
	}
	token_skip(1); // skip (

	ast_t *while_expr = expr();

	if (token_at(0)->kind != TK_RPAREN) {
		token_t *token = token_at(0);
		eprintf(token->filepath, token->source, while_keyword->start, token->end,
			"Expected ')' after while expr");
		exit(1);
	}
	token_skip(1); // skip )

	ast_t *while_stmt = stmt();

	return malloc_ast_while_stmt(while_keyword, while_expr, while_stmt);
}

static ast_t *defer_stmt() {
	token_t *defer_keyword = token_at(0);
	token_skip(1); // skip defer keyword

	ast_t *defer_expr = expr();

	token_t *semicolon = token_at(0);
	if (semicolon->kind != TK_SEMICOLON) {
		eprintf(semicolon->filepath, semicolon->source, defer_keyword->start, semicolon->end,
			"Expected ';' at the end of defer stmt");
		exit(1);
	}
	token_skip(1); // skip ;

	return malloc_ast_defer_stmt(defer_keyword, defer_expr, semicolon);
}

static ast_t *break_stmt() {
	token_t *break_keyword = token_at(0);
	token_skip(1); // skip break keyword

	token_t *semicolon = token_at(0);
	if (semicolon->kind != TK_SEMICOLON) {
		eprintf(semicolon->filepath, semicolon->source, break_keyword->start, semicolon->end,
			"Expected ';' at the end of break stmt");
		exit(1);
	}
	token_skip(1); // skip ;

	return malloc_ast_break_stmt(break_keyword, semicolon);
}

static ast_t *continue_stmt() {
	token_t *continue_keyword = token_at(0);
	token_skip(1); // skip continue keyword

	token_t *semicolon = token_at(0);
	if (semicolon->kind != TK_SEMICOLON) {
		eprintf(semicolon->filepath, semicolon->source, continue_keyword->start, semicolon->end,
			"Expected ';' at the end of continue stmt");
		exit(1);
	}
	token_skip(1); // skip ;

	return malloc_ast_continue_stmt(continue_keyword, semicolon);
}

static ast_t *return_stmt() {
	token_t *return_keyword = token_at(0);
	token_skip(1); // skip return keyword

	ast_t *return_expr = NULL;
	if (token_at(0)->kind != TK_SEMICOLON) {
		return_expr = expr();
	}

	token_t *semicolon = token_at(0);
	if (semicolon->kind != TK_SEMICOLON) {
		eprintf(semicolon->filepath, semicolon->source, return_keyword->start, semicolon->end,
			"Expected ';' at the end of return stmt");
		exit(1);
	}
	token_skip(1); // skip ;

	return malloc_ast_return_stmt(return_keyword, return_expr, semicolon);
}

static ast_t *expr_stmt() {
	ast_t *ast = expr();
	
	token_t *semicolon = token_at(0);
	if (semicolon->kind != TK_SEMICOLON) {
		eprintf(semicolon->filepath, semicolon->source, ast->start, semicolon->end,
			"Expected ';' at the end of expr stmt");
		exit(1);
	}
	token_skip(1); // skip ;

	return malloc_ast_expr_stmt(ast, semicolon);
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

		left = malloc_ast_binary_expr(left, op, right);
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
				"Expected ':'");
			exit(1);
		}
		token_skip(1); // skip :
		ast_t *right = ternary_expr();
		
		left = malloc_ast_ternary_expr(left, mid, right);
	}

	return left;
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
	ast_t *left = prefix_expr();
	while (token_at(0)->kind == TK_STAR || token_at(0)->kind == TK_FSLASH ||
		token_at(0)->kind == TK_MOD) {
		token_t *op = token_at(0);
		token_skip(1);

		ast_t *right = prefix_expr();

		left = malloc_ast_binary_expr(left, op, right);
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
		return malloc_ast_prefix_expr(op, right);
	}
	}

	return postfix_expr();
}

static ast_t *postfix_expr() {
	ast_t *left = primary_expr();

	char keepGoing = 1;
	while (keepGoing) {
		switch (token_at(0)->kind) {
		case TK_PLUS_PLUS:
		case TK_DASH_DASH: {
			token_t *op = token_at(0);
			token_skip(1);
			left = malloc_ast_postfix_expr(left, op);
			break;
		}

		case TK_LBRACKET: {
			token_t *lbracket = token_at(0);
			token_skip(1); // skip [

			ast_t *index = expr();
			token_t *rbracket = token_at(0);
			if (rbracket->kind != TK_RBRACKET) {
				eprintf(rbracket->filepath, rbracket->source, left->start, rbracket->end,
					"Expected ']'");
				exit(1);
			}
			token_skip(1); // skip ]

			left = malloc_ast_array_access_expr(left, lbracket, index, rbracket);
			break;
		}

		case TK_DOT:
		case TK_DASH_RCHEVRON: {
			token_t *op = token_at(0);
			token_skip(1); // skip . or ->

			token_t *member = token_at(0);
			if (member->kind != TK_IDENTIFIER) {
				eprintf(op->filepath, op->source, op->start, member->end,
					"Expected member identifier");
				exit(1);
			}
			token_skip(1); // skip identifier

			left = malloc_ast_member_access_expr(left, op, member);
			break;
		}

		case TK_LPAREN: {
			token_skip(1); // skip (
			left = malloc_ast_function_call_expr(left);

			while (token_at(0)->kind != TK_EOF && token_at(0)->kind != TK_RPAREN) {
				ast_t *arg = expr();
				append_ast(&(left->ast.function_call_expr.argc), 
					&(left->ast.function_call_expr.argv), arg);
				if (token_at(0)->kind == TK_COMMA) {
					token_skip(1); // skip ,
				}
			}

			token_t *token = token_at(0);
			if (token_at(0)->kind != TK_RPAREN) {
				eprintf(token->filepath, token->source, token->start, token->end,
					"Expected ')' at the end of function call");
				exit(1);
			}
			token_skip(1); // skip )
			
			left->end = token->end;
			break;
		}

		default:
			keepGoing = 0;
			break;
		}
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

