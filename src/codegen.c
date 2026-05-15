#include "codegen.h"
#include "common.h"
#include "util.h"
#include "error.h"

// ========================================
// helper declaration
// ========================================

static sbuilder_t g_code;

static void init();

static void gen_inst(int indent, const char *format, ...);
static void gen_prog(ast_t *ast);

// ========================================
// codegen.h - definition
// ========================================

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
	gen_inst(0, "INST_HALT");
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

