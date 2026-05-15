#include "util.h"
#include "common.h"

// ========================================
// util.h - definition
// ========================================

void sbuilder_init(sbuilder_t *s) {
	s->elems = NULL;
	s->len = s->cap = 0;
}

void sbuilder_free(sbuilder_t *s) {
	free(s->elems);
	sbuilder_init(s);
}

void sbuilder_reserve(sbuilder_t *s, int new_cap) {
	if (new_cap <= s->cap) return;

	s->elems = realloc(s->elems, new_cap * sizeof(char));
	s->cap = new_cap;
}

void sbuilder_appendf(sbuilder_t *s, const char *format, ...) {
	va_list args;
	va_start(args, format);
	sbuilder_appendvf(s, format, args);
	va_end(args);
}

void sbuilder_appendvf(sbuilder_t *s, const char *format, va_list arg) {
	va_list prev = arg;
	int len = vsnprintf(NULL, 0, format, arg);
	sbuilder_reserve(s, s->cap + len + 2);

	arg = prev;
	vsprintf(s->elems + s->len, format, arg);
	s->len += len;
}

void sbuilder_build(sbuilder_t *s, char **out) {
	char *temp = malloc((s->len + 1) * sizeof(char));
	for (int i = 0; i < s->len; i++) temp[i] = s->elems[i];
	temp[s->len] = 0;
	*out = temp;
}

