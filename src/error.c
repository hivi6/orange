#include "error.h"
#include "common.h"

// ========================================
// error.h - definition
// ========================================

void eprintf(const char *filepath, const char *src, pos_t start, pos_t end,
	const char *format, ...) {

	// Print the line meta data like filepath, start line and start column
	fprintf(stderr, "%s:%d:%d: ", filepath, start.line, start.column);
	
	// Print the formatted error message
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");

	// Calculate where the line starts
	int line_start = start.index;
	while (line_start - 1 >= 0 && src[line_start - 1] != '\n') {
		line_start--;
	}

	// Show marking at the top of the line marking the error
	// Something like this: vvvvvvvvvvvvvvvvv
	//                      This is the error
	fprintf(stderr, "%-8s|%8s", "", "");
	for (int i = line_start; src[i] && src[i] != '\n'; i++) {
		char ch = src[i];
		char error_sign = ' ';
		if (start.index <= i && i < end.index) {
			error_sign = 'v';
		}

		// Also take care of tabs; if there is a tab
		// You have to make sure you are aligned with the error
		// And not dependent on the terminal
		char buffer[64] = {};
		sprintf(buffer, "%c", error_sign);
		if (ch == '\t') {
			for (int i = 0; i < ERROR_TAB_INDENT_SIZE; i++) {
				buffer[i] = error_sign;
			}
			buffer[ERROR_TAB_INDENT_SIZE] = '\0';
		}

		fprintf(stderr, "%s", buffer);
	}
	fprintf(stderr, "\n");

	// Now print each line where error occurs
	int index = line_start;
	for (int line = start.line; line <= end.line; line++) {
		// Print the line number
		fprintf(stderr, "%-8d>%8s", line, "");

		// Print the line (make sure to take care of tabs)
		while (src[index] && src[index] != '\n') {
			char ch = src[index];
			if (ch == '\t') {
				for (int i = 0; i < ERROR_TAB_INDENT_SIZE; i++)
					fprintf(stderr, " ");
			}
			else {
				fprintf(stderr, "%c", ch);
			}
			index++;
		}

		fprintf(stderr, "\n");

		if (src[index]) {
			index++;
		}
	}
	fprintf(stderr, "%-8s|\n", "");
}

