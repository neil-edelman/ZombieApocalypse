#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* strncpy strdup */
#include "Text.h"

/* constants */
static const size_t granularity = 128;

struct Text {
	FILE   *fp;
	char   *fn;
	char   *line;
	size_t linesize;
	char   tok;
	char   *sep;
};

/* public */

/** constructor */
struct Text *Text(const char *fn) {
	struct Text *text;

	if(!fn) return 0;
	if(!(text = malloc(sizeof(struct Text)))) {
		perror("Text constructor");
		Text_(&text);
		return 0;
	}
	text->fp = 0;
	text->fn = 0;
	text->line = 0;
	text->linesize = 0;
	text->tok = ' ';
	text->sep = 0;
	/* buffer */
	if(!(text->fn = strdup(fn)) ||
	   !(text->line = malloc(granularity))) {
		perror("Text buffer");
		Text_(&text);
		return 0;
	}
	text->line[0] = '\0';
	text->linesize = granularity;
	/* open the file */
	if(!(text->fp = fopen(fn, "r"))) {
		perror(fn);
		Text_(&text);
		return 0;
	}
	fprintf(stderr, "Text: opened %s, #%p.\n", fn, (void *)text);

	return text;
}

/** destructor */
void Text_(struct Text **textptr) {
	struct Text *text;

	if(!textptr || !(text = *textptr)) return;
	if(text->fp) {
		fclose(text->fp);
		fprintf(stderr, "~Text: closed %s, #%p.\n", text->fn, (void *)text);
	}
	if(text->line) free(text->line);
	if(text->fn)   free(text->fn);
	free(text);
	*textptr = text = 0;
}

/** next line */
char *TextGetLine(struct Text *t) {
	int readSome = 0;
	int sublen, txtlen = 0;
	char *sub;

	if(!t) return 0;
	for(sub = t->line; ; ) {
		/* read the line */
		if(!fgets(sub, t->linesize - txtlen, t->fp)) {
			if(!feof(t->fp)) perror(t->fn);
			return readSome ? t->line : 0;
		}
		readSome = -1;
		/* what the fuck happened? */
		if(!(sublen = strlen(sub))) break;
		txtlen += sublen;
		if(sub[sublen-1] == '\n' || sub[sublen-1] == '\r') {
			/* it read all the way to \n; delete it */
			while(sublen && (sub[sublen-1] == '\n' || sub[sublen-1] == '\r')) sublen--;
			sub[sublen] = '\0';
			break;
		} else if(feof(t->fp)) {
			/* technically, it's not a text file w/o the trailing \n,
			 but let's not be pedantic */
			break;
		} else {
			char *newLine;
			/* we need a bigger buffer */
			if(!(newLine = realloc((void *)t->line, t->linesize + granularity))) {
				perror("Text buffer realloc");
				return 0;
			}
			t->line = newLine;
			t->linesize += granularity;
			sub = t->line + txtlen;
		}
	}
	/* reset */
	t->sep = t->line;
	return t->line;
}

/** starts with? */
int TextStartsWith(const struct Text *t, const char *with) {
	int len;
	if(!t) return 0;
	if(!with) return -1;
	len = strlen(with);
	return strncmp(t->line, with, len) ? 0 : -1;
}

/** set the token for spliting */
void TextToken(struct Text *t, char tok) {
	if(!t) return;
	if(!tok) t->tok = ' ';
	else     t->tok = tok;
}

/** split */
char *TextNextToken(struct Text *t) {
	char delim[2];
	if(!t || !t->sep || t->sep[0] == '\0') return 0;
	delim[0] = t->tok;
	delim[1] = '\0';
	/* strpbrk(); */
	return strsep(&t->sep, delim);
}

char *TextEndOfLine(const struct Text *t) {
	if(!t) return 0;
	return t->sep;
}
