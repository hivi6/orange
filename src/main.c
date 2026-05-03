#include "common.h"
#include "token.h"
#include "ast.h"
#include "analyze.h"
#include "type.h"

// ========================================
// helper declarations
// ========================================

void usage(FILE *fptr) {
	fprintf(fptr, 
		"USAGE: orange [OPTION] <filepath>\n"
		"\n"
		"ABOUT:\n"
		"    orange is a compiler for a statically typed\n"
		"    programming language for citrus VM\n"
		"\n"
		"OPTION:\n"
		"    -h, --help     This screen\n"
		"    --only-tokens  Print only the tokens\n"
		"    --only-ast     Print only the ast\n"
		"\n"
		"HINTS:\n"
		"    1. If you want to read from stdin, then make filepath == '-'\n"
		"       echo hello | orange -; this should read hello from the stdin\n");
}

// ========================================
// main
// ========================================

int main(int argc, const char **argv) {
	int usage_flag = 0, only_tokens_flag = 0, only_ast_flag = 0;
	int argi = 1;
	for (; argi < argc; argi++) {
		if (strcmp(argv[argi], "-h") == 0 || 
			strcmp(argv[argi], "--help") == 0) {
			usage_flag = 1;
		}
		else if (strcmp(argv[argi], "--only-tokens") == 0) {
			only_tokens_flag = 1;
		}
		else if (strcmp(argv[argi], "--only-ast") == 0) {
			only_ast_flag = 1;
		}
		else {
			break;
		}
	}

	if (usage_flag) {
		usage(stdout);
		exit(1);
	}

	if (argi == argc) {
		fprintf(stderr, "ERROR: No filepath found\n\n");
		usage(stderr);
		exit(1);
	}

	token_t *tokens = generate_tokens(argv[argi]);
	if (only_tokens_flag) {
		for (token_t *head = tokens; head; head = head->next) {
			printf("TOKEN_KIND: %s\n", token_kind_str(head->kind));
			printf("TOKEN_LEXICAL: ");
			for (int i = head->start.index; i < head->end.index; i++) {
				printf("%c", head->source[i]);
			}
			printf("\n\n");
		}
		return 0;
	}

	ast_t *ast = parse(tokens);
	if (only_ast_flag) {
		print_ast(ast);
		return 0;
	}

	analyze(ast);
	printf("=== TYPE INFORMATION ===\n\n");
	for (type_t *temp = all_types(); temp; temp = temp->next) {
		print_type_info(temp, 0);
		printf("\n===================\n\n");
	}

	return 0;
}

