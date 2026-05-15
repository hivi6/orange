#ifndef UTIL_H
#define UTIL_H

struct sbuilder_t {
	char *elems;
	int len;
	int cap;
};

typedef struct sbuilder_t sbuilder_t;

/**
 * Initialize the string builder
 *
 * params:
 *     s  pointer to string builder
 */
void sbuilder_init(sbuilder_t *s);

/**
 * Free the string builder
 *
 * params:
 *     s  pointer to string builder
 */
void sbuilder_free(sbuilder_t *s);

/**
 * Reserve a given space of memory
 *
 * params:
 *     s        pointer to string builder
 *     new_cap  new capacity of the string builder
 */
void sbuilder_reserve(sbuilder_t *s, int new_cap);

/**
 * Append given string format to the end of the string builder
 *
 * params:
 *     s       pointer to string builder
 *     format  format for the append string
 *     ...     variable arguments for the append(like printf)
 */
void sbuilder_appendf(sbuilder_t *s, const char *format, ...);

/**
 * Create a new memory space with the content of the string builder
 *
 * params:
 *     s    pointer to string builder
 *     out  output of the new memory space
 */
void sbuilder_build(sbuilder_t *s, char **out);

#endif /* UTIL_H */

