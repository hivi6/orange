#include "codegen.h"
#include "common.h"
#include "util.h"
#include "error.h"
#include "type.h"

// ========================================
// helper declaration
// ========================================

static sbuilder_t g_code;

static void init();

static void gen_prog(ast_t *ast);

static void gen_inst(int indent, const char *format, ...);
static void gen_allocate(ast_t *ast);

// ========================================
// codegen.h - definition
// ========================================

/*
RULES: (for self)

Based on the working on citrus VM:
- r0 is used for instruction pointing
- r1 is used for stack pointing

As we don't have a blob space in citrus VM, we have to store the 
global variables also in the stack. We also need a base pointer for each
function call
- r2 we will be using for base pointer for stack
- r3 we will be using the base pointer for the start of global space in stack
*/
char *codegen(ast_t *ast) {
	init();
	gen_prog(ast);
	
	char *res = NULL;
	sbuilder_build(&g_code, &res);
	return res;
}

// ========================================
// helper declaration
// ========================================

static void init() {
	sbuilder_init(&g_code);
}

static void gen_prog(ast_t *ast) {
	gen_inst(0, "_start:");

	// calculate the space that needs to be allocate for variables and contants
	gen_allocate(ast);

	gen_inst(1, "INST_HLT");
}

static void gen_inst(int indent, const char *format, ...) {
	if (indent) {
		sbuilder_appendf(&g_code, "    ");
	}

	va_list args;
	va_start(args, format);
	sbuilder_appendvf(&g_code, format, args);
	va_end(args);

	sbuilder_appendf(&g_code, "\n");
}

static void gen_allocate(ast_t *ast, char is_global) {
	scope_t *scope = ast->scope;

	// allocate space for all the symbols
	int stack_size = 0;
	for (symbol_t *head = scope->symbols; head; head = head->next) {
		if (head->type->kind != TYPE_FUNCTION) {
			stack_size += head->type->size;
		}
	}

	gen_inst(1, "# Allocate space for variables");
	gen_inst(1, "INST_LOAD_CONST r10 %d", stack_size);
	gen_inst(1, "INST_LOAD       r2 r1 8");
	gen_inst(1, "INST_SUB        r1 r1 r10");
	gen_inst(1, "");
}

