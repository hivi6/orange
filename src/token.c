#include "token.h"
#include "error.h"
#include "common.h"

// ========================================
// helper declaration
// ========================================

static const char *g_filepath;
static const char *g_source;
static int g_source_length;
static token_t *g_head;
static token_t *g_tail;
static pos_t g_prev_pos;
static pos_t g_cur_pos;

static void init(const char *filepath);
static char is_eof();
static void generate_token();
static char *read_file(const char *filepath);
static char is_whitespace(char ch);
static char char_at(int offset);
static void char_skip(int inc);
static void token_append(int kind);
static char keyword_kind();
static void char_literal_skip();
static void escape_character_skip();

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
	init(filepath);

	while (!is_eof()) {
		generate_token();
	}

	return g_head;
}

static void init(const char *filepath) {
	g_filepath = filepath;
	g_source = read_file(filepath);
	g_source_length = strlen(g_source);
	g_head = NULL;
	g_tail = NULL;
	g_prev_pos = POS_INIT;
	g_cur_pos = POS_INIT;
}

static char is_eof() {
	return g_cur_pos.index >= g_source_length;
}

static void generate_token() {
	g_prev_pos = g_cur_pos;

	if (is_whitespace(char_at(0))) {
		char_skip(1);
		return;
	}

	int skip = 1;
	int kind = -1;
	if (char_at(0) == '&' && char_at(1) == '&') skip = 2, kind = TK_AMPERSAND_AMPERSAND;
	else if (char_at(0) == '&') skip = 1, kind = TK_AMPERSAND;
	else if (char_at(0) == '!' && char_at(1) == '=') skip = 2, kind = TK_BANG_EQUAL;
	else if (char_at(0) == '!') skip = 1, kind = TK_BANG;
	else if (char_at(0) == '^') skip = 1, kind = TK_CARET;
	else if (char_at(0) == ':') skip = 1, kind = TK_COLON;
	else if (char_at(0) == ',') skip = 1, kind = TK_COMMA;
	else if (char_at(0) == '-' && char_at(1) == '-') skip = 2, kind = TK_DASH_DASH;
	else if (char_at(0) == '-' && char_at(1) == '>') skip = 2, kind = TK_DASH_RCHEVRON;
	else if (char_at(0) == '-') skip = 1, kind = TK_DASH;
	else if (char_at(0) == '.') skip = 1, kind = TK_DOT;
	else if (char_at(0) == '=' && char_at(1) == '=') skip = 2, kind = TK_EQUAL_EQUAL;
	else if (char_at(0) == '=') skip = 1, kind = TK_EQUAL;
	else if (char_at(0) == '/') skip = 1, kind = TK_FSLASH;
	else if (char_at(0) == '{') skip = 1, kind = TK_LBRACE;
	else if (char_at(0) == '[') skip = 1, kind = TK_LBRACKET;
	else if (char_at(0) == '<' && char_at(1) == '=') skip = 2, kind = TK_LCHEVRON_EQUAL;
	else if (char_at(0) == '<' && char_at(1) == '<') skip = 2, kind = TK_LCHEVRON_LCHEVRON;
	else if (char_at(0) == '<') skip = 1, kind = TK_LCHEVRON;
	else if (char_at(0) == '(') skip = 1, kind = TK_LPAREN;
	else if (char_at(0) == '%') skip = 1, kind = TK_MOD;
	else if (char_at(0) == '|' && char_at(1) == '|') skip = 2, kind = TK_PIPE_PIPE;
	else if (char_at(0) == '|') skip = 1, kind = TK_PIPE;
	else if (char_at(0) == '+' && char_at(1) == '+') skip = 2, kind = TK_PLUS_PLUS;
	else if (char_at(0) == '+') skip = 1, kind = TK_PLUS;
	else if (char_at(0) == '?') skip = 1, kind = TK_QUESTION;
	else if (char_at(0) == '}') skip = 1, kind = TK_RBRACE;
	else if (char_at(0) == ']') skip = 1, kind = TK_RBRACKET;
	else if (char_at(0) == '>' && char_at(1) == '=') skip = 2, kind = TK_RCHEVRON_EQUAL;
	else if (char_at(0) == '>' && char_at(1) == '>') skip = 2, kind = TK_RCHEVRON_RCHEVRON;
	else if (char_at(0) == '>') skip = 1, kind = TK_RCHEVRON;
	else if (char_at(0) == ')') skip = 1, kind = TK_RPAREN;
	else if (char_at(0) == ';') skip = 1, kind = TK_SEMICOLON;
	else if (char_at(0) == '*') skip = 1, kind = TK_STAR;
	else if (char_at(0) == '~') skip = 1, kind = TK_TILDE;
	else if (isalpha(char_at(0)) || char_at(0) == '_') {
		skip = 0;
		while (isalnum(char_at(0)) || char_at(0) == '_') char_skip(1);
		kind = keyword_kind();
	}
	else if (isdigit(char_at(0))) {
		skip = 0;

		if (char_at(0) == '0') char_skip(1);
		else while (isdigit(char_at(0))) char_skip(1);
		
		kind = TK_INT_LITERAL;
	}
	else if (char_at(0) == '\'') {
		skip = 0;
		char_literal_skip();
		kind = TK_CHAR_LITERAL;
	}

	if (kind != -1) {
		char_skip(skip);
		token_append(kind);
		return;
	}

	char_skip(1);
	eprintf(g_filepath, g_source, g_prev_pos, g_cur_pos, "Unknown character");
	exit(1);
}

static char *read_file(const char *filepath) {
	char is_stdin = (strcmp(filepath, "-") == 0);
	FILE *fptr = stdin;

	if (!is_stdin) fptr = fopen(filepath, "r");
	if (fptr == NULL) {
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "ERROR: read_file(\"%s\")", filepath);
		perror(buffer);
		exit(1);
	}

	int len = 1024 * 1024; // For stdin
	if (!is_stdin) {
		fseek(fptr, 0, SEEK_END);
		len = ftell(fptr);
		fseek(fptr, 0, SEEK_SET);
	}

	char *buffer = (char *) calloc(len+1, sizeof(char));
	if (buffer == NULL) {
		perror("Error in read_file while malloc");
		exit(1);
	}
	int size = fread(buffer, 1, len, fptr);
	buffer[size] = 0;

	if (!is_stdin) fclose(fptr);

	return buffer;
}

char is_whitespace(char ch) {
	return (ch == 0 || ch == '\n' || ch == ' ' || ch == '\t');
}

static char char_at(int offset) {
	int index = g_cur_pos.index + offset;
	if (index >= g_source_length) {
		return 0;
	}

	return g_source[index];
}

static void char_skip(int inc) {
	for (int i = 0; i < inc; i++) {
		if (is_eof()) break;

		if (char_at(0) == '\n') {
			g_cur_pos.line++;
			g_cur_pos.column = 0;
		}
		g_cur_pos.index++;
		g_cur_pos.column++;
	}
}

static void token_append(int kind) {
	token_t *res = malloc(sizeof(token_t));
	res->kind = kind;
	res->filepath = g_filepath;
	res->source = g_source;
	res->start = g_prev_pos;
	res->end = g_cur_pos;
	res->next = NULL;

	if (g_head == NULL) g_head = res;
	else g_tail->next = res;
	g_tail = res;
}

static char keyword_kind() {
	int len = g_cur_pos.index - g_prev_pos.index;
	const char *src = g_source + g_prev_pos.index;

	if (strlen("break") == len && strncmp(src, "break", len) == 0) return TK_BREAK_KEYWORD;
	else if (strlen("continue") == len && strncmp(src, "continue", len) == 0) return TK_CONTINUE_KEYWORD;
	else if (strlen("defer") == len && strncmp(src, "defer", len) == 0) return TK_DEFER_KEYWORD;
	else if (strlen("else") == len && strncmp(src, "else", len) == 0) return TK_ELSE_KEYWORD;
	else if (strlen("fn") == len && strncmp(src, "fn", len) == 0) return TK_FN_KEYWORD;
	else if (strlen("if") == len && strncmp(src, "if", len) == 0) return TK_IF_KEYWORD;
	else if (strlen("return") == len && strncmp(src, "return", len) == 0) return TK_RETURN_KEYWORD;
	else if (strlen("sizeof") == len && strncmp(src, "sizeof", len) == 0) return TK_SIZEOF_KEYWORD;
	else if (strlen("struct") == len && strncmp(src, "struct", len) == 0) return TK_STRUCT_KEYWORD;
	else if (strlen("var") == len && strncmp(src, "var", len) == 0) return TK_VAR_KEYWORD;
	else if (strlen("while") == len && strncmp(src, "while", len) == 0) return TK_WHILE_KEYWORD;
	return TK_IDENTIFIER;
}

static void char_literal_skip() {
	char_skip(1); // skip single quote

	if (32 <= char_at(0) && char_at(0) < 127 && char_at(0) != '\'' && char_at(0) != '\\') {
		// skip the printable character that is not single quote or backslash
		char_skip(1);
	}
	else if (char_at(0) == '\\') {
		// skip escape character
		escape_character_skip();
	}
	else {
		// no such rule, so this should cause error
		char_skip(1);
		eprintf(g_filepath, g_source, g_prev_pos, g_cur_pos, 
			"Expected printable character or starting of excape character");
		exit(1);
	}

	if (char_at(0) != '\'') {
		char_skip(1);
		eprintf(g_filepath, g_source, g_prev_pos, g_cur_pos, 
			"Expected single quote");
		exit(1);
	}

	char_skip(1); // skip single quote
}

static void escape_character_skip() {
	char_skip(1); // skip backslash

	const char *escape_char = "ntbra'\"?\\fv0";
	int is_escape_char = 0;
	for (int i = 0; escape_char[i]; i++) {
		if (escape_char[i] == char_at(0)) {
			is_escape_char = 1;
			break;
		}
	}
	if (is_escape_char) {
		char_skip(1);
		return;
	}

	char_skip(1);
	eprintf(g_filepath, g_source, g_prev_pos, g_cur_pos,
		"Unexpected escape sequence");
	exit(1);
}

