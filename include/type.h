#ifndef TYPE_H
#define TYPE_H

enum {
	// u8, u16, u32, u64
	TYPE_PRIMITIVE,
	TYPE_POINTER,
	TYPE_ARRAY,
	TYPE_STRUCTURE,
	TYPE_FUNCTION,
};

struct type_t {
	int kind;
	const char *name;
	long long size; // in bytes
	struct type_t *next;

	union {
		// TYPE_POINTER
		// TYPE_ARRAY
		struct {
			struct type_t *base_type;
			int counts; // counts = 1 if kind = TYPE_POINTER
		} array;

		// TYPE_STRUCTURE
		struct {
			const char **field_names;
			struct type_t **field_types;
			int field_counts;
		} structure;

		// TYPE_FUNCTION
		struct {
			struct type_t *return_type;
			struct type_t **param_types;
			int param_counts;
		} function;
	} type;
};

typedef struct type_t type_t;

/**
 * Create the new type of a given kind and a given name
 *
 * params:
 *     kind  what is the "kind" of type?
 *     name  name of the type
 *
 * returns:
 *     type pointer
 */
type_t *create_type(int kind, const char *name);

/**
 * Get the type information with the given name
 *
 * params:
 *     name  name of the type
 *
 * returns:
 *     type pointer
 */
type_t *get_type(const char *name);

/**
 * Append type to a type list
 *
 * params:
 *     argc  pointer to the size of the list
 *     list  pointer to the list
 *     type  type that needs to be inserted into the list
 */
void append_type(int *argc, type_t ***list, type_t *type);

/**
 * Get the head of the type list
 *
 * returns:
 *     the head of the type list
 */
type_t *all_types();

/**
 * Print type info
 *
 * params:
 *     type    type that needs printing
 *     indent  indentation before printing a string
 */
void print_type_info(type_t *type, int indent);

#endif // TYPE_H

