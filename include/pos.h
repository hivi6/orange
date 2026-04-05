#ifndef POS_H
#define POS_H

struct pos_t {
	int index;
	int line;
	int column;
};

typedef struct pos_t pos_t;

#define POS_INIT ((struct pos_t) {.index=0, .line=1, .column=1})

#endif // POS_H

