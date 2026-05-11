#include "analyze.h"
#include "token.h"
#include "type.h"
#include "error.h"
#include "common.h"
#include "scope.h"

// ========================================
// helper declaration
// ========================================

static scope_t *g_global_scope;

static void assert_ast_kind(ast_t *ast, int kind, const char *message);

static void prog(ast_t *ast);

static type_t *get_type_specifier(ast_t *ast, ast_t *prog);

static void create_struct(ast_t *ast);
static void define_struct(ast_t *ast, ast_t *prog);
static void create_function(ast_t *ast, ast_t *prog);
static void create_var_decl(ast_t *ast, ast_t *prog);

static char is_numeric_type(type_t *t1);
static char is_equivalent_type(type_t *t1, type_t *t2);
static type_t *expr(ast_t *ast, scope_t *scope);

// ========================================
// analyze.h - definition
// ========================================

void analyze(ast_t *ast) {
	// create primitive types
	create_type(TYPE_PRIMITIVE, "u8")->size = 1;
	create_type(TYPE_PRIMITIVE, "u16")->size = 2;
	create_type(TYPE_PRIMITIVE, "u32")->size = 4;
	create_type(TYPE_PRIMITIVE, "u64")->size = 8;
	create_type(TYPE_VOID, "void")->size = 0;

	// create pointer type
	type_t *str_type = create_type(TYPE_POINTER, "*u8");
	str_type->type.array.base_type = get_type("u8");
	str_type->type.array.counts = 1;

	prog(ast);
}

// ========================================
// helper definition
// ========================================

static void assert_ast_kind(ast_t *ast, int kind, const char *message) {
	if (ast->kind != kind) {
		eprintf(ast->filepath, ast->source, ast->start, ast->end, message);
		exit(1);
	}
}

static void prog(ast_t *ast) {
	assert_ast_kind(ast, AST_PROG, "Expected AST_PROG ast");

	// create the global scope
	g_global_scope = create_scope(NULL);
	ast->scope = g_global_scope;

	// first go through all the struct declaration for type creation
	for (int i = 0; i < ast->ast.prog.argc; i++) {
		ast_t *decl = ast->ast.prog.argv[i];
		if (decl->kind == AST_STRUCT_DECL) create_struct(decl);
	}

	// Go through all the struct, but now for struct definition
	for (int i = 0; i < ast->ast.prog.argc; i++) {
		ast_t *decl = ast->ast.prog.argv[i];
		if (decl->kind == AST_STRUCT_DECL) define_struct(decl, ast);
	}

	// Go through all the functions, and create type
	for (int i = 0; i < ast->ast.prog.argc; i++) {
		ast_t *decl = ast->ast.prog.argv[i];
		if (decl->kind == AST_FUNCTION_DECL) create_function(decl, ast);
	}

	// Go through all the global variable declaration, should be sequencial
	for (int i = 0; i < ast->ast.prog.argc; i++) {
		ast_t *decl = ast->ast.prog.argv[i];
		if (decl->kind == AST_VAR_DECL) create_var_decl(decl, ast);
	}
}

static void create_struct(ast_t *ast) {
	assert_ast_kind(ast, AST_STRUCT_DECL, "Expected AST_STRUCT_DECL ast; create_struct");

	token_t *id = ast->ast.struct_decl.identifier;
	const char *name = token_lexical_str(id);
	type_t *type = get_type(name);
	if (type != NULL) {
		eprintf(id->filepath, id->source, id->start, id->end,
			"Struct '%s' is already defined, cannot redefine", name);
		exit(1);
	}

	// Set the size to -1 for now, signifying that the structure definition is not started
	// size = -2, meaning structure definition has started but not completed
	// size >= 0, meaning structure definition is completed
	create_type(TYPE_STRUCTURE, name)->size = -1;
}

static void define_struct(ast_t *ast, ast_t *prog) {
	assert_ast_kind(ast, AST_STRUCT_DECL, "Expected AST_STRUCT_DECL ast; define_struct");

	token_t *id = ast->ast.struct_decl.identifier;
	char *name = token_lexical_str(id);
	type_t *type = get_type(name);
	if (type == NULL) {
		eprintf(id->filepath, id->source, id->start, id->end,
			"Struct '%s' is not declared! IMPOSSIBLE", name);
		exit(1);
	}

	// size is non-negative; meaning the definition is complete
	if (type->size >= 0) {
		free(name);
		return;
	}

	// by default size = -1, meaning definition hasn't started
	// size = -2, meaning definition has started
	// So, if definition has started and we are again visiting the struct
	// that means, we have recursive struct used, and so that connot be processed
	if (type->size == -2) {
		eprintf(id->filepath, id->source, id->start, id->end,
			"Struct '%s' is recursive struct, and so cannot be resolved", name);
		exit(1);
	}

	// start type definition; so setting size to -2
	type->size = -2;

	int size = 0;
	int fields_cnt = ast->ast.struct_decl.fields_cnt;
	ast_t **fields = ast->ast.struct_decl.fields;
	ast_t **field_types = ast->ast.struct_decl.types;
	for (int i = 0; i < fields_cnt; i++) {
		token_t *field_token = fields[i]->ast.var_expr.token;
		ast_t *field_type_specifier = field_types[i];
		char *field_name = token_lexical_str(field_token);
		type_t *field_type = get_type_specifier(field_type_specifier, prog);

		// check for duplicate field_name
		for (int j = 0; j < type->type.structure.field_counts; j++) {
			if (strcmp(field_name, type->type.structure.field_names[j]) == 0) {
				eprintf(field_token->filepath, field_token->source, field_token->start,
					field_token->end, "Duplicate field name in struct '%s'", name);
				exit(1);
			}
		}

		if (field_type->size < 0) {
			eprintf(field_type_specifier->filepath, field_type_specifier->source,
				field_type_specifier->start, field_type_specifier->end,
				"Impossible, cannot be field_type of negative size!");
			exit(1);
		}
		size += field_type->size;

		type->type.structure.field_counts++;
		type->type.structure.field_names = realloc(type->type.structure.field_names,
			type->type.structure.field_counts * sizeof(char*));
		type->type.structure.field_types = realloc(type->type.structure.field_types,
			type->type.structure.field_counts * sizeof(type_t *));
		type->type.structure.field_names[type->type.structure.field_counts-1] = field_name;
		type->type.structure.field_types[type->type.structure.field_counts-1] = field_type;
	}
	type->size = size;

	free(name);
}

static type_t *get_type_specifier(ast_t *ast, ast_t *prog) {
	assert_ast_kind(ast, AST_TYPE_SPECIFIER, "Expected AST_TYPE_SPECIFIER ast");

	char *type_lexical = token_lexical_str(ast->ast.type_specifier.type_name);
	type_t *base_type = get_type(type_lexical);
	if (base_type == NULL) {
		token_t *token = ast->ast.type_specifier.type_name;
		eprintf(token->filepath, token->source, token->start, token->end,
			"No such type defined");
		exit(1);
	}

	// Get how many pointers
	for (int i = 0; i < ast->ast.type_specifier.pointer_cnt; i++) {
		type_t *pointer = calloc(sizeof(type_t), 1);
		pointer->kind = TYPE_POINTER;
		pointer->size = 8;
		pointer->type.array.base_type = base_type;
		pointer->type.array.counts = 1;
		base_type = pointer;
	}

	// time to resolve the base_type
	if (base_type->size < 0) {
		for (int i = 0; i < prog->ast.prog.argc; i++) {
			ast_t *decl = prog->ast.prog.argv[i];
			if (decl->kind == AST_STRUCT_DECL) {
				token_t *id = decl->ast.struct_decl.identifier;
				char *name = token_lexical_str(id);
				if (strcmp(type_lexical, name) == 0) {
					define_struct(decl, prog);
				}
				free(name);
			}
		}
	}

	// Get all the array information
	for (int i = 0; i < ast->ast.type_specifier.argc; i++) {
		ast_t *index = ast->ast.type_specifier.argv[i];
		char *index_lexical = token_lexical_str(index->ast.literal_expr.token);
		int index_cnt = atoi(index_lexical);
		
		type_t *pointer = calloc(sizeof(type_t), 1);
		pointer->kind = TYPE_ARRAY;
		pointer->size = base_type->size * index_cnt;
		pointer->type.array.base_type = base_type;
		pointer->type.array.counts = index_cnt;
		base_type = pointer;
	}

	return base_type;
}

static void create_function(ast_t *ast, ast_t *prog) {
	assert_ast_kind(ast, AST_FUNCTION_DECL, "Expected AST_FUNCTION_DECL ast");

	token_t *id = ast->ast.function_decl.identifier;
	char *function_name = token_lexical_str(id);
	if (get_type(function_name) != NULL) {
		eprintf(id->filepath, id->source, id->start, id->end,
			"Function with '%s' name already defined", function_name);
		exit(1);
	}

	type_t *type = create_type(TYPE_FUNCTION, function_name);

	// create a symbol for function in the global scope for function call
	create_symbol(g_global_scope, function_name, type);

	// create a function scope with parent as the global scope, for params and function body
	scope_t *function_scope = create_scope(g_global_scope);
	ast->scope = function_scope;

	long long size = 0;
	type_t *return_type = get_type("void");
	if (ast->ast.function_decl.return_type) {
		return_type = get_type_specifier(ast->ast.function_decl.return_type, prog);
		size = return_type->size;
	}

	for (int i = 0; i < ast->ast.function_decl.params_cnt; i++) {
		token_t *param_token = ast->ast.function_decl.params[i]->ast.var_expr.token;
		char *param_name = token_lexical_str(param_token);
		type_t *param_type = get_type_specifier(ast->ast.function_decl.types[i], prog);

		// check for duplicate parameter name
		for (int j = 0; j < type->type.function.param_counts; j++) {
			if (strcmp(param_name, type->type.function.param_names[j]) == 0) {
				eprintf(param_token->filepath, param_token->source, param_token->start,
					param_token->end, 
					"Duplicate parameter name in function '%s'", function_name);
				exit(1);
			}
		}

		// make the param part of the function scope
		create_symbol(function_scope, param_name, param_type);

		type->type.function.param_counts++;
		type->type.function.param_names = realloc(type->type.function.param_names,
			type->type.function.param_counts * sizeof(char*));
		type->type.function.param_types = realloc(type->type.function.param_types,
			type->type.function.param_counts * sizeof(type_t*));
		type->type.function.param_names[type->type.function.param_counts-1] = param_name;
		type->type.function.param_types[type->type.function.param_counts-1] = param_type;
	}

	type->type.function.return_type = return_type;
	type->size = size;
}

static void create_var_decl(ast_t *ast, ast_t *prog) {
	assert_ast_kind(ast, AST_VAR_DECL, "Expected AST_VAR_DECL ast");

	ast->scope = g_global_scope;

	token_t *id = ast->ast.var_stmt.identifier;
	char *name = token_lexical_str(id);
	if (get_symbol(ast->scope, name) != NULL) {
		eprintf(id->filepath, id->source, id->start, id->end,
			"Cannot redefine global variable/function");
		exit(1);
	}
	if (get_type(name) != NULL) {
		eprintf(id->filepath, id->source, id->start, id->end,
			"Cannot redefine a symbol as variable from struct");
		exit(1);
	}
	type_t *type = get_type("u32");
	if (ast->ast.var_stmt.type != NULL) {
		type = get_type_specifier(ast->ast.var_stmt.type, prog);
	}

	if (ast->ast.var_stmt.expr != NULL) {
		ast_t *e = ast->ast.var_stmt.expr;
		type_t *expr_type = expr(e, g_global_scope);
		if (!is_equivalent_type(type, expr_type)) {
			eprintf(e->filepath, e->source, e->start, e->end,
				"expression type is not equivalent to the type provided in declaration");
			exit(1);
		}
	}

	create_symbol(g_global_scope, name, type);
}

static char is_equivalent_type(type_t *t1, type_t *t2) {
	if (is_numeric_type(t1) && is_numeric_type(t2)) return 1;
	return t1 == t2;
}

static char is_numeric_type(type_t *t1) {
	return t1->kind == TYPE_POINTER || t1->kind == TYPE_PRIMITIVE || t1->kind == TYPE_ARRAY;
}

static type_t *expr(ast_t *ast, scope_t *scope) {
	ast->scope = scope;

	if (ast->kind == AST_LITERAL_EXPR) {
		switch (ast->ast.literal_expr.token->kind) {
		case TK_INT_LITERAL:
			return get_type("u32");
		case TK_CHAR_LITERAL:
			return get_type("u8");
		case TK_STR_LITERAL:
			return get_type("*u8");
		}
	}
	else if (ast->kind == AST_VAR_EXPR) {
		token_t *var_token = ast->ast.var_expr.token;
		char *name = token_lexical_str(var_token);
		symbol_t *symbol = get_symbol(scope, name);
		if (symbol == NULL) {
			eprintf(var_token->filepath, var_token->source, var_token->start, var_token->end,
				"No such symbol defined");
			exit(1);
		}
		ast->is_lvalue = 1;
		return symbol->type;
	}
	else if (ast->kind == AST_GROUP_EXPR) {
		type_t *type = expr(ast->ast.group_expr.expr, scope);
		ast->is_lvalue = ast->ast.group_expr.expr->is_lvalue;
		return type;
	}
	else if (ast->kind == AST_BINARY_EXPR) {
		type_t *ltype = expr(ast->ast.binary_expr.left, scope);
		type_t *rtype = expr(ast->ast.binary_expr.right, scope);
		if (ast->ast.binary_expr.op->kind == TK_EQUAL && !ast->ast.binary_expr.left->is_lvalue) {
			ast_t *left = ast->ast.binary_expr.left;
			eprintf(left->filepath, left->source, left->start, left->end,
				"Expected an lvalue");
			exit(1);
		}
		ast->is_lvalue = ast->ast.binary_expr.left->is_lvalue;
		
		if (!is_equivalent_type(ltype, rtype)) {
			token_t *op = ast->ast.binary_expr.op;
			eprintf(op->filepath, op->source, op->start, op->end,
				"Unsupported operation for left and right expression");
			exit(1);
		}

		if (ltype->kind == TYPE_POINTER || ltype->kind == TYPE_ARRAY) return ltype;
		if (rtype->kind == TYPE_POINTER || rtype->kind == TYPE_ARRAY) return rtype;

		if (ltype->size > rtype->size) return ltype;
		return rtype;
	}
	else if (ast->kind == AST_TERNARY_EXPR) {
		type_t *ltype = expr(ast->ast.ternary_expr.left, scope);
		type_t *mtype = expr(ast->ast.ternary_expr.mid, scope);
		type_t *rtype = expr(ast->ast.ternary_expr.right, scope);

		if (!is_equivalent_type(mtype, rtype)) {
			eprintf(ast->filepath, ast->source, ast->start, ast->end,
				"Unsupported type for mid and right expression");
			exit(1);
		}

		if (!is_numeric_type(ltype)) {
			eprintf(ast->filepath, ast->source, ast->start, ast->end,
				"Expected numeric expression in condition");
			exit(1);
		}
		
		if (mtype->size > rtype->size) return mtype;
		return rtype;
	}
	else if (ast->kind == AST_PREFIX_EXPR) {
		token_t *op = ast->ast.prefix_expr.op;
		ast_t *right = ast->ast.prefix_expr.right;

		// for sizeof, it should also handle struct types instead of just expressions
		if (op->kind == TK_SIZEOF_KEYWORD) {
			ast_t *right = ast->ast.prefix_expr.right;
			while (right->kind == AST_GROUP_EXPR) right = right->ast.group_expr.expr;
			if (right->kind == AST_VAR_EXPR) {
				char *name = token_lexical_str(right->ast.var_expr.token);
				if (get_type(name) == NULL) {
					free(name);
					return get_type("u64");
				}
				free(name);
			}
		}
	
		type_t *rtype = expr(right, scope);
		if ((op->kind == TK_PLUS_PLUS || op->kind == TK_DASH_DASH || 
			op->kind == TK_AMPERSAND) && !right->is_lvalue) {
			eprintf(right->filepath, right->source, right->start, right->end,
				"Expected lvalue");
			exit(1);
		}

		if (op->kind == TK_SIZEOF_KEYWORD) return get_type("u64");
		if (op->kind == TK_AMPERSAND) {
			type_t *pointer_type = create_type(TYPE_POINTER, NULL);
			pointer_type->type.array.base_type = rtype;
			pointer_type->type.array.counts = 1;
			return pointer_type;
		}
		if (op->kind == TK_STAR) {
			if (rtype->kind != TYPE_POINTER) {
				eprintf(right->filepath, right->source, right->start, right->end,
					"Expected pointer");
				exit(1);
			}
			return rtype->type.array.base_type;
		}
		if (!is_numeric_type(rtype)) {
			eprintf(right->filepath, right->source, right->start, right->end,
				"Expected numeric type for the given prefix operation");
			exit(1);
		}

		return rtype;
	}
	else if (ast->kind == AST_POSTFIX_EXPR) {
		ast_t *left = ast->ast.postfix_expr.left;
		type_t *ltype = expr(ast->ast.postfix_expr.left, scope);
		
		if (!is_numeric_type(ltype)) {
			eprintf(left->filepath, left->source, left->start, left->end,
				"Expected numeric type for the given postfix operation");
			exit(1);
		}

		if (!left->is_lvalue) {
			eprintf(left->filepath, left->source, left->start, left->end,
				"Expected lvalue");
			exit(1);
		}

		return ltype;
	}
	else if (ast->kind == AST_ARRAY_ACCESS_EXPR) {
		ast_t *left = ast->ast.array_access_expr.left;
		ast_t *index = ast->ast.array_access_expr.index;

		type_t *ltype = expr(left, scope);
		type_t *itype = expr(index, scope);

		if (itype->kind != TYPE_PRIMITIVE) {
			eprintf(index->filepath, index->source, index->start, index->end,
				"Expected numeric index expression");
			exit(1);
		}
		if (ltype->kind != TYPE_POINTER && ltype->kind != TYPE_ARRAY) {
			eprintf(left->filepath, left->source, left->start, left->end,
				"Expected pointer or array type");
			exit(1);
		}

		ast->is_lvalue = 1;
		return ltype->type.array.base_type;
	}
	else if (ast->kind == AST_MEMBER_ACCESS_EXPR) {
		ast_t *left = ast->ast.member_access_expr.left;
		token_t *op = ast->ast.member_access_expr.op;
		token_t *member = ast->ast.member_access_expr.member;

		type_t *ltype = expr(left, scope);
		type_t *stype = NULL;
		if (op->kind == TK_DASH_RCHEVRON && 
			(ltype->kind == TYPE_POINTER || ltype->kind == TYPE_ARRAY)) {
			stype = ltype->type.array.base_type;
		}
		if (op->kind == TK_DOT) {
			stype = ltype;
		}

		if (stype == NULL || stype->kind != TYPE_STRUCTURE) {
			eprintf(left->filepath, left->source, left->start, left->end,
				"Expected a struct or a struct pointer");
			exit(1);
		}

		char *name = token_lexical_str(member);
		for (int i = 0; i < stype->type.structure.field_counts; i++) {
			if (strcmp(name, stype->type.structure.field_names[i]) == 0) {
				free(name);
				return stype->type.structure.field_types[i];
			}
		}
		
		eprintf(member->filepath, member->source, member->start, member->end,
			"Not a valid member field");
		exit(1);
	}
	else if (ast->kind == AST_FUNCTION_CALL_EXPR) {
		ast_t *left = ast->ast.function_call_expr.left;
		type_t *ltype = expr(left, scope);

		if (ltype->kind != TYPE_FUNCTION) {
			eprintf(left->filepath, left->source, left->start, left->end,
				"Expected a function");
			exit(1);
		}

		if (ltype->type.function.param_counts != ast->ast.function_call_expr.argc) {
			eprintf(ast->filepath, ast->source, ast->start, ast->end,
				"Unmatched parameter counts");
			exit(1);
		}

		for (int i = 0; i < ast->ast.function_call_expr.argc; i++) {
			ast_t *arg = ast->ast.function_call_expr.argv[i];
			type_t *atype = expr(arg, scope);
			if (!is_equivalent_type(atype, ltype->type.function.param_types[i])) {
				eprintf(arg->filepath, arg->source, arg->start, arg->end,
					"Unmatched parameter type");
				exit(1);
			}
		}

		return ltype->type.function.return_type;
	}
	
	eprintf(ast->filepath, ast->source, ast->start, ast->end,
		"Unexpected expr ast kind(%d)[POSTFIX: %d]", ast->kind, AST_POSTFIX_EXPR);
	exit(1);
}

