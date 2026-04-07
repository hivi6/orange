#ifndef ERROR_H
#define ERROR_H

#include "pos.h"

#define ERROR_TAB_INDENT_SIZE 8

/**
 * Print error
 *
 * Params:
 *     filepath  filepath where the error occurred
 *     src       source code of the file where the error occurred
 *     start     start position of the error (inclusive)
 *     end       end position of the error (exclusive)
 *     format    error format that needs to be printed
 *     ...       variable arguments for the error message
 */
void eprintf(const char *filepath, const char *src, pos_t start, pos_t end,
	const char *format, ...);

#endif // ERROR_H

