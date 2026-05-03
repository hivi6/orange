#include "type.h"
#include "common.h"

// ========================================
// helper declaration
// ========================================

static type_t *g_head;
static type_t *g_tail;

void print_type_info_helper(type_t *type, int indent, int print_struct_members);

// ========================================
// type.h - definition
// ========================================

type_t *create_type(int kind, const char *name) {
	type_t *res = calloc(sizeof(type_t), 1);
	res->kind = kind;
	res->name = name;
	if (g_head == NULL) g_head = g_tail = res;
	else {
		g_tail->next = res;
		g_tail = res;
	}
	return res;
}

type_t *get_type(const char *name) {
	for (type_t *temp = g_head; temp; temp = temp->next) {
		if (strcmp(name, temp->name) == 0) return temp;
	}
	return NULL;
}

void append_type(int *argc, type_t ***list, type_t *type) {
	(*argc)++;
	*list = realloc(*list, sizeof(type_t *) * (*argc));
	(*list)[*argc - 1] = type;
}

type_t *all_types() {
	return g_head;
}

void print_type_info(type_t *type) {
	print_type_info_helper(type, 0, 1);
}

// ========================================
// helper definition
// ========================================

void print_type_info_helper(type_t *type, int indent, int print_struct_members) {
	const char *kind_str[] = {"TYPE_PRIMITIVE", "TYPE_POINTER", "TYPE_ARRAY", 
		"TYPE_STRUCTURE", "TYPE_FUNCTION"};

	char *indent_str = malloc((indent * 4 + 1) * sizeof(char));
	for (int i = 0; i < indent * 4; i++) indent_str[i] = ' ';
	indent_str[indent * 4] = 0;

	if (type->kind != TYPE_POINTER && type->kind != TYPE_ARRAY) {
		printf("%stype_name: %s\n", indent_str, type->name);
	}
	printf("%skind: %s(%d)\n", indent_str, kind_str[type->kind], type->kind);
	printf("%ssize: %lld\n", indent_str, type->size);

	if (type->kind == TYPE_STRUCTURE && print_struct_members) {
		printf("%smembers:\n", indent_str);
		
		for (int i = 0; i < type->type.structure.field_counts; i++) {
			printf("%s    field_name: %s\n", indent_str, type->type.structure.field_names[i]);
			print_type_info_helper(type->type.structure.field_types[i], indent+1, 0);
			if (i < type->type.structure.field_counts-1) printf("\n");
		}
	}
	else if (type->kind == TYPE_ARRAY || type->kind == TYPE_POINTER) {
		printf("%scount: %d\n", indent_str, type->type.array.counts);
		printf("%sbase_type:\n", indent_str);
		print_type_info_helper(type->type.array.base_type, indent+1, print_struct_members);
	}
	else if (type->kind == TYPE_FUNCTION) {
		if (type->type.function.return_type) {
			printf("%sreturn_type:\n", indent_str);
			print_type_info_helper(type->type.function.return_type, indent+1, 0);
		}

		if (type->type.function.param_counts) {
			printf("%sparams:\n", indent_str);
			for (int i = 0; i < type->type.function.param_counts; i++) {
				printf("%s    param_name: %s\n", indent_str, type->type.function.param_names[i]);
				print_type_info_helper(type->type.function.param_types[i], indent+1, 0);
				if (i < type->type.function.param_counts-1) printf("\n");
			}
		}
	}

	free(indent_str);
}

