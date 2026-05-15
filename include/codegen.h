#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

/**
 * Generate the assembly code for citrus VM
 *
 * params:
 *    ast  the final ast of the source code
 *
 * returns:
 *    char pointer to the final assembly code
 *
 * memory: User responsibility to free
 */
char *codegen(ast_t *ast);

#endif /* CODEGEN_H */

