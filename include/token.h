#ifndef TOKEN_H
#define TOKEN_H

#include "pos.h"

enum {
	TK_EOF = 0,

	TK_AMPERSAND,
	TK_AMPERSAND_AMPERSAND,
	TK_BANG,
	TK_BANG_EQUAL,
	TK_CARET,
	TK_COLON,
	TK_COMMA,
	TK_DASH_RCHEVRON,
	TK_DOT,
	TK_EQUAL,
	TK_EQUAL_EQUAL,
	TK_FSLASH,
	TK_LBRACE,
	TK_LBRACKET,
	TK_LCHEVRON,
	TK_LCHEVRON_EQUAL,
	TK_LCHEVRON_LCHEVRON,
	TK_LPAREN,
	TK_DASH,
	TK_DASH_DASH,
	TK_MOD,
	TK_PIPE,
	TK_PIPE_PIPE,
	TK_PLUS,
	TK_PLUS_PLUS,
	TK_QUESTION,
	TK_RBRACE,
	TK_RBRACKET,
	TK_RCHEVRON,
	TK_RCHEVRON_EQUAL,
	TK_RCHEVRON_RCHEVRON,
	TK_RPAREN,
	TK_SEMICOLON,
	TK_STAR,
	TK_TILDE,

	TK_BREAK_KEYWORD,
	TK_CONTINUE_KEYWORD,
	TK_DEFER_KEYWORD,
	TK_ELSE_KEYWORD,
	TK_FN_KEYWORD,
	TK_IF_KEYWORD,
	TK_RETURN_KEYWORD,
	TK_SIZEOF_KEYWORD,
	TK_STRUCT_KEYWORD,
	TK_VAR_KEYWORD,
	TK_WHILE_KEYWORD,

	TK_IDENTIFIER,

	TK_INT_LITERAL,
	TK_CHAR_LITERAL,
	TK_STR_LITERAL,
};

struct token_t {
	int kind;
	const char *filepath;
	const char *source;
	pos_t start;
	pos_t end;

	struct token_t *next;
};

typedef struct token_t token_t;

/**
 * Get the string alternative to the token kind number
 *
 * params:
 *     kind  enum token kind
 *
 * returns:
 *     character string alternative to the enum
 */
const char *token_kind_str(int kind);

/**
 * Get the lexical string
 *
 * params:
 *     token  the token whose lexical string needs to be created
 *
 * returns:
 *     get the pointer to the created lexical string
 */
char *token_lexical_str(token_t *token);

/**
 * Generate tokens from the given filepath
 *
 * params:
 *     filepath  filepath to the source text whose tokens are generated
 *
 * returns:
 *     head to the token list
 */
token_t *generate_tokens(const char *filepath);

#endif // TOKEN_H

