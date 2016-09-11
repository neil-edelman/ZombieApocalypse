struct Text;

struct Text *Text();
void Text_(struct Text **textptr);
char *TextGetLine(struct Text *t);
int TextStartsWith(const struct Text *t, const char *with);
void TextToken(struct Text *t, char tok);
char *TextNextToken(struct Text *t);
char *TextEndOfLine(const struct Text *t);
