#include "token.h"
#include "common.h"

// ========================================
// token.h - definition
// ========================================

const char *token_kind_str(int kind) {
	if (kind == TK_AMPERSAND) return "TK_AMPERSAND";
	if (kind == TK_AMPERSAND_AMPERSAND) return "TK_AMPERSAND_AMPERSAND";
	if (kind == TK_BANG) return "TK_BANG";
	if (kind == TK_BANG_EQUAL) return "TK_BANG_EQUAL";
	if (kind == TK_CARET) return "TK_CARET";
	if (kind == TK_COLON) return "TK_COLON";
	if (kind == TK_COMMA) return "TK_COMMA";
	if (kind == TK_DASH_RCHEVRON) return "TK_DASH_RCHEVRON";
	if (kind == TK_DOT) return "TK_DOT";
	if (kind == TK_EQUAL) return "TK_EQUAL";
	if (kind == TK_EQUAL_EQUAL) return "TK_EQUAL_EQUAL";
	if (kind == TK_FSLASH) return "TK_FSLASH";
	if (kind == TK_LBRACE) return "TK_LBRACE";
	if (kind == TK_LBRACKET) return "TK_LBRACKET";
	if (kind == TK_LCHEVRON) return "TK_LCHEVRON";
	if (kind == TK_LCHEVRON_EQUAL) return "TK_LCHEVRON_EQUAL";
	if (kind == TK_LCHEVRON_LCHEVRON) return "TK_LCHEVRON_LCHEVRON";
	if (kind == TK_LPAREN) return "TK_LPAREN";
	if (kind == TK_DASH) return "TK_DASH";
	if (kind == TK_DASH_DASH) return "TK_DASH_DASH";
	if (kind == TK_MOD) return "TK_MOD";
	if (kind == TK_PIPE) return "TK_PIPE";
	if (kind == TK_PIPE_PIPE) return "TK_PIPE_PIPE";
	if (kind == TK_PLUS) return "TK_PLUS";
	if (kind == TK_PLUS_PLUS) return "TK_PLUS_PLUS";
	if (kind == TK_QUESTION) return "TK_QUESTION";
	if (kind == TK_RBRACE) return "TK_RBRACE";
	if (kind == TK_RBRACKET) return "TK_RBRACKET";
	if (kind == TK_RCHEVRON) return "TK_RCHEVRON";
	if (kind == TK_RCHEVRON_EQUAL) return "TK_RCHEVRON_EQUAL";
	if (kind == TK_RCHEVRON_RCHEVRON) return "TK_RCHEVRON_RCHEVRON";
	if (kind == TK_RPAREN) return "TK_RPAREN";
	if (kind == TK_SEMICOLON) return "TK_SEMICOLON";
	if (kind == TK_STAR) return "TK_STAR";
	if (kind == TK_TILDE) return "TK_TILDE";
	if (kind == TK_BREAK_KEYWORD) return "TK_BREAK_KEYWORD";
	if (kind == TK_CONTINUE_KEYWORD) return "TK_CONTINUE_KEYWORD";
	if (kind == TK_DEFER_KEYWORD) return "TK_DEFER_KEYWORD";
	if (kind == TK_ELSE_KEYWORD) return "TK_ELSE_KEYWORD";
	if (kind == TK_FN_KEYWORD) return "TK_FN_KEYWORD";
	if (kind == TK_IF_KEYWORD) return "TK_IF_KEYWORD";
	if (kind == TK_RETURN_KEYWORD) return "TK_RETURN_KEYWORD";
	if (kind == TK_SIZEOF_KEYWORD) return "TK_SIZEOF_KEYWORD";
	if (kind == TK_STRUCT_KEYWORD) return "TK_STRUCT_KEYWORD";
	if (kind == TK_VAR_KEYWORD) return "TK_VAR_KEYWORD";
	if (kind == TK_WHILE_KEYWORD) return "TK_WHILE_KEYWORD";
	if (kind == TK_IDENTIFIER) return "TK_IDENTIFIER";
	if (kind == TK_INT_LITERAL) return "TK_INT_LITERAL";
	if (kind == TK_CHAR_LITERAL) return "TK_CHAR_LITERAL";
	if (kind == TK_STR_LITERAL) return "TK_STR_LITERAL";
	return "TK_UNKNOWN";
}

token_t *generate_tokens(const char *filepath) {
	return NULL;
}

