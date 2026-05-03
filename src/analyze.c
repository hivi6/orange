#include "analyze.h"
#include "token.h"
#include "type.h"
#include "error.h"
#include "common.h"

// ========================================
// helper declaration
// ========================================

static void assert_ast_kind(ast_t *ast, int kind, const char *message);

static void prog(ast_t *ast);

static void create_struct(ast_t *ast);
static void define_struct(ast_t *ast, ast_t *prog);
static type_t *get_field_type(ast_t *ast, ast_t *prog);

// ========================================
// analyze.h - definition
// ========================================

void analyze(ast_t *ast) {
	// create primitive types
	create_type(TYPE_PRIMITIVE, "u8")->size = 1;
	create_type(TYPE_PRIMITIVE, "u16")->size = 2;
	create_type(TYPE_PRIMITIVE, "u32")->size = 4;
	create_type(TYPE_PRIMITIVE, "u64")->size = 8;

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
		type_t *field_type = get_field_type(field_type_specifier, prog);

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

static type_t *get_field_type(ast_t *ast, ast_t *prog) {
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

